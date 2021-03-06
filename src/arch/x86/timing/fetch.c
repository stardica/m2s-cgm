/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <m2s.h>
#include <arch/x86/emu/context.h>
#include <arch/x86/emu/regs.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include <mem-image/mmu.h>

/*#include <mem-system/module.h>*/

//#include <instrumentation/stats.h>

#include <arch/x86/timing/bpred.h>
#include <arch/x86/timing/core.h>
#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/event-queue.h>
#include <arch/x86/timing/fetch.h>
#include <arch/x86/timing/fetch-queue.h>
#include <arch/x86/timing/thread.h>
#include <arch/x86/timing/trace-cache.h>
#include <arch/x86/timing/uop.h>

#include <cgm/cgm.h>
#include <cgm/interrupt.h>

/*
 * Class 'X86Thread'
 */

static int X86ThreadCanFetch(X86Thread *self){

	//printf("X86ThreadCanFetch()\n");
	X86Cpu *cpu = self->cpu;
	X86Context *ctx = self->ctx;


	unsigned int phy_addr;
	unsigned int block;


	/* Context must be running */
	if (!ctx || !X86ContextGetState(ctx, X86ContextRunning))
		return 0;
	
	/* Fetch stalled or context evict signal activated */
	if (self->fetch_stall_until >= asTiming(cpu)->cycle || ctx->evict_signal)
		return 0;
	
	/* Fetch queue must have not exceeded the limit of stored bytes
	 * to be able to store new micro-instructions. */
	if (self->fetchq_occ >= x86_fetch_queue_size)
		return 0;
	
	//if(cpu_gpu_stats->core_pipe_drain[self->core->id] == 1)
	//	fatal("time to drain\n");


	/*star added this; fetch must stall when the system is processing a system call
	so that it will not process a 2nd syscall while it is still working on the current syscall*/
	if (cpu_gpu_stats->core_num_fences[self->core->id] > 0)
	{
		return 0;
	}

	/* If the next fetch address belongs to a new block, cache system
	 * must be accessible to read it. */

//star looks like this is working ok.
#if CGM

	//int i = (*self->i_cache_ptr + self->id_in_core)->assoc;
	block = self->fetch_neip & ~(self->i_cache_ptr[self->core->id].block_size - 1);
	/*if (MSG == 1)
	{
		printf("blocksize %d\n",self->i_cache_ptr[self->core->id].block_size);
		printf("~(self->mem_ctrl_ptr->block_size - 1) 0x%08x\n", ~(self->i_cache_ptr[self->core->id].block_size - 1));
		printf("self->fetch_block 0x%08x\n", self->fetch_block);
		printf("self->fetch_neip 0x%08x\n", self->fetch_neip);
		printf("block 0x%08x\n", block);
		fflush(stdout);
		//getchar();
	}*/

#else
	block = self->fetch_neip & ~(self->inst_mod->block_size - 1);
	/*if (MSG == 2)
	{
		printf("blocksize %d\n",self->inst_mod->block_size);
		printf("~(self->inst_mod->block_size - 1) 0x%08x\n", ~(self->inst_mod->block_size - 1));
		printf("self->fetch_block 0x%08x\n", self->fetch_block);
		printf("self->fetch_neip 0x%08x\n", self->fetch_neip);
		printf("block 0x%08x\n", block);
		fflush(stdout);
		//getchar();
	}*/
#endif

	if (block != self->fetch_block)
	{
		/*printf("entered access");
		fflush(stdout);
		getchar();*/



		phy_addr = 0xFFFFFFFF; //mmu_translate(self->ctx->address_space_index, self->fetch_neip, mmu_access_fetch);
#if CGM
		//if (!cgm_can_fetch_access(self->i_cache_ptr, phy_addr))
		if (!cgm_can_fetch_access(self, phy_addr))
		{
#else
		if (!mod_can_access(self->inst_mod, phy_addr))
		{
#endif
			return 0;
		}
	}

	/* We can fetch */
	return 1;
}


int print_warning = 0;

void mem_access_addr_translate(X86Thread *self, struct x86_uop_t *uop){


	/*x86_uinst_load,		//52
	x86_uinst_store,		//53
	x86_uinst_load_ex,		//54
	x86_uinst_store_ex,		//55
	x86_uinst_prefetch,		//56
	x86_uinst_cpu_flush,	//57
	x86_uinst_gpu_flush,	//58
	x86_uinst_cpu_fence,	//59
	x86_uinst_cpu_load_fence,	//60*/

	/*if(uop->uinst->opcode >= 61)
	{

		core_dump_rob(x86_cpu->cores[0]);

		struct x86_uop_t *uop = NULL;

		uop = list_get(x86_cpu->cores[0]->rob, x86_cpu->cores[0]->threads[0]->rob_tail);

		printf("\n");

		thread_dump_fetch_queue(x86_cpu->cores[0]->threads[0]);

		printf("\n");

		thread_dump_uop_queue(x86_cpu->cores[0]->threads[0]);

		dump_unist_queue(x86_uinst_list);

		printf("\nrob tail id %llu\n", uop->id);

		fatal("caught the drain\n");
	}*/

	//if(uop->id == 521)
		//fatal("fetch here\n");

	int pf = 0;
	int *pfptr = &pf;

	if(uop->uinst->opcode >= 52 && uop->uinst->opcode <= 60)
	{
		if(uop->uinst->opcode == x86_uinst_load_ex || uop->uinst->opcode == x86_uinst_store_ex)
		{

			if(print_warning < 1){warning("fix me in mmu load/store ex\n"); print_warning++;}
			uop->phy_addr = mmu_translate(1, uop->uinst->address, mmu_access_load_store, pfptr);
		}
		else if(uop->uinst->opcode == x86_uinst_cpu_flush || uop->uinst->opcode == x86_uinst_gpu_flush
				|| uop->uinst->opcode == x86_uinst_cpu_fence || uop->uinst->opcode == x86_uinst_cpu_load_fence)
		{

			if(print_warning < 1){warning("fix me mmu need to take care of this flushes\n");print_warning++;}

			if(cgm_gpu_cache_protocol == cgm_protocol_mesi)
			{
				uop->phy_addr = mmu_translate(0, uop->uinst->address, mmu_access_load_store, pfptr);
			}
			else if(cgm_gpu_cache_protocol == cgm_protocol_non_coherent)
			{
				uop->phy_addr = mmu_translate(1, uop->uinst->address, mmu_access_load_store, pfptr);
			}
			else
			{
				fatal("X86ThreadFetchInst(): bad cache protocol\n");
			}
		}
		else
		{
			uop->phy_addr = mmu_translate(self->ctx->address_space_index, uop->uinst->address, mmu_access_load_store, pfptr);

			/*if(uop->uinst->address >= 0x8122eb8 && uop->uinst->address <= 0x8132eb8)
				printf("accessing the array address 0x%08x\n", uop->uinst->address);*/
		}

	}
	else
	{
		fatal("X86ThreadFetchInst(): Invalid memory uop\n");
	}

	if(uop->uinst->opcode == 56)
	{
		fatal("caught a prefetch in fetch\n");
	}

	return;
}

void dump_unist_queue(struct list_t *queue){

	int i = 0;
	struct x86_uinst_t *uinst = NULL;

	printf("Unist queue size %d\n", list_count(queue));

	LIST_FOR_EACH(queue, i)
	{
		//get pointer to access in queue and check it's status.
		uinst = list_get(queue, i);
		printf("\t uinst_id %llu opcode %u\n", uinst->id, uinst->opcode);

		/*if(uinst->opcode == x86_uinst_cpu_fence)
			getchar();*/
	}

	return;
}

/* Run the emulation of one x86 macro-instruction and create its uops.
 * If any of the uops is a control uop, this uop will be the return value of
 * the function. Otherwise, the first decoded uop is returned. */
static struct x86_uop_t *X86ThreadFetchInst(X86Thread *self, int fetch_trace_cache)
{

	X86Cpu *cpu = self->cpu;
	X86Core *core = self->core;
	X86Context *ctx = self->ctx;

	struct x86_uop_t *uop;
	struct x86_uop_t *ret_uop;

	struct x86_uinst_t *uinst;
	int uinst_count;
	int uinst_index;

	/* Functional simulation */
	self->fetch_eip = self->fetch_neip;

	X86ContextSetEip(ctx, self->fetch_eip);
	X86ContextExecute(ctx);

	//star >> increment the instruction pointer.
	self->fetch_neip = self->fetch_eip + ctx->inst.size;

	/*if(self->fetch_eip < min)
	{
		min = self->fetch_eip;
		printf("I$ min = 0x%08x\n", min);
	}
	if(self->fetch_eip > max)
	{
		max = self->fetch_eip;
		printf("I$ max = 0x%08x\n", max);
	}*/


	/* If no micro-instruction was generated by this instruction, create a
	 * 'nop' micro-instruction. This makes sure that there is always a micro-
	 * instruction representing the regular control flow of macro-instructions
	 * of the program. It is important for the traces stored in the trace
	 * cache. */
	if (!x86_uinst_list->count)
	{
		x86_uinst_new(ctx, x86_uinst_nop, 0, 0, 0, 0, 0, 0, 0);
	}

	/* Micro-instructions created by the x86 instructions can be found now
	 * in 'x86_uinst_list'. */
	uinst_count = list_count(x86_uinst_list);


	uinst_index = 0;
	ret_uop = NULL;
	while (list_count(x86_uinst_list))
	{
		/* Get uinst from head of list */
		uinst = list_remove_at(x86_uinst_list, 0);

		/* Create uop */
		uop = x86_uop_create();
		uop->uinst = uinst;
		assert(uinst->opcode >= 0 && uinst->opcode < x86_uinst_opcode_count);
		uop->flags = x86_uinst_info[uinst->opcode].flags;
		uop->id = cpu->uop_id_counter++;
		uop->id_in_core = core->uop_id_counter++;

		uop->ctx = ctx;
		uop->thread = self;

		uop->mop_count = uinst_count;
		uop->mop_size = ctx->inst.size;
		uop->mop_id = uop->id - uinst_index;
		uop->mop_index = uinst_index;
		uop->protection_fault = 0;

		uop->eip = self->fetch_eip;
		uop->in_fetch_queue = 1;
		uop->trace_cache = fetch_trace_cache;
		uop->specmode = X86ContextGetState(ctx, X86ContextSpecMode);
		uop->fetch_address = self->fetch_address;
		uop->fetch_access = self->fetch_access;
		uop->neip = ctx->regs->eip;
		uop->pred_neip = self->fetch_neip;
		uop->target_neip = ctx->target_eip;


		//star added this to catch interrupts at issue.
		//all of the interrupt related data is known by now.
		//this is kind of Frankensteiny, but it works
		if(uop->uinst->opcode == x86_uinst_syscall)
		{
			//if both flags are set its an OpenCL syscall
			if(opencl_syscall_flag)
			{

				/*if(opencl_syscall_flag == 4) //memcpy
				{
					uop->interrupt = opencl_syscall_flag;
					uop->interrupt_type = opencl_interrupt;
					uop->int_src_ptr = int_src_ptr;
					uop->int_dest_ptr = int_dest_ptr;
					uop->int_size = int_size;
				}
				else //all others
				{*/
					uop->interrupt = opencl_syscall_flag;
					uop->interrupt_type = opencl_interrupt;
				/*}*/
			}
			else
			{
				uop->interrupt = 1;
				uop->interrupt_type = system_interrupt;
			}

			//reset all of the flags for the next round
			syscall_flag = 0;
			opencl_syscall_flag = 0;
			/*int_src_ptr = 0;
			int_dest_ptr=0;
			int_size=0;*/
		}
		else
		{
			uop->interrupt = 0;
			uop->interrupt_type = non_interrupt;
		}

		/* Process uop dependences and classify them in integer, floating-point,
		 * flags, etc. */
		x86_uop_count_deps(uop);

		/* Calculate physical address of a memory access */
		if (uop->flags & X86_UINST_MEM)
		{
			if(tlb_simple)
			{
				mem_access_addr_translate(self, uop);
			}
		}



		/* Trace */
		if (x86_tracing())
		{
			char str[MAX_STRING_SIZE];
			char inst_name[MAX_STRING_SIZE];
			char uinst_name[MAX_STRING_SIZE];

			char *str_ptr;

			int str_size;

			str_ptr = str;
			str_size = sizeof str;

			 //Command
			str_printf(&str_ptr, &str_size, "x86.new_inst id=%lld core=%d",
				uop->id_in_core, core->id);

			// Speculative mode
			if (uop->specmode)
				str_printf(&str_ptr, &str_size, " spec=\"t\"");

			// Macro-instruction name
			if (!uinst_index)
			{
				x86_inst_dump_buf(&ctx->inst, inst_name, sizeof inst_name);
				str_printf(&str_ptr, &str_size, " asm=\"%s\"", inst_name);
			}

			// Rest
			x86_uinst_dump_buf(uinst, uinst_name, sizeof uinst_name);
			str_printf(&str_ptr, &str_size, " uasm=\"%s\" stg=\"fe\"", uinst_name);

			// Dump
			x86_trace("%s\n", str);
		}

		/* Select as returned uop */
		if (!ret_uop || (uop->flags & X86_UINST_CTRL))
			ret_uop = uop;

		/* Insert into fetch queue */
		list_add(self->fetch_queue, uop);
		if (fetch_trace_cache)
			self->trace_cache_queue_occ++;


		/* Statistics */
		//star >> added for instrumentation
		//pthread_mutex_lock(&instrumentation_mutex);
		//FetchQueueOccupancy(list_count(self->fetch_queue));
		//FectchStats(uop, ctx->pid);
		//pthread_mutex_unlock(&instrumentation_mutex);


		cpu->num_fetched_uinst++;
		self->num_fetched_uinst++;
		if (fetch_trace_cache)
			self->trace_cache->num_fetched_uinst++;

		/* Next uinst */
		uinst_index++;
	}

	/* Increase fetch queue occupancy if instruction does not come from
	 * trace cache, and return. */
	if (ret_uop && !fetch_trace_cache)
		self->fetchq_occ += ret_uop->mop_size;
	return ret_uop;
}


/* Try to fetch instruction from trace cache.
 * Return true if there was a hit and fetching succeeded. */
static int X86ThreadFetchTraceCache(X86Thread *self)
{

	fatal("X86ThreadFetchTraceCache()\n");
	struct x86_uop_t *uop;

	int mpred;
	int hit;
	int mop_count;
	int i;

	unsigned int eip_branch;  /* next branch address */
	unsigned int *mop_array;
	unsigned int neip;

	/* No room in trace cache queue */
	assert(x86_trace_cache_present);
	if (self->trace_cache_queue_occ >= x86_trace_cache_queue_size)
		return 0;
	
	/* Access BTB, branch predictor, and trace cache */
#if CGM
	//star todo We only need to fill this in if we are going to add in the trace cache stuff.
	fatal("somehow we got into the trace cache stuff in fetch.c");
	eip_branch = X86ThreadGetNextBranch(self, self->fetch_neip, self->i_cache_ptr->block_size);
#else
	eip_branch = X86ThreadGetNextBranch(self, self->fetch_neip, self->inst_mod->block_size);
#endif


	mpred = eip_branch ? X86ThreadLookupBranchPredMultiple(self, eip_branch, x86_trace_cache_branch_max) : 0;
	hit = X86ThreadLookupTraceCache(self, self->fetch_neip, mpred, &mop_count, &mop_array, &neip);
	if (!hit)
		return 0;
	
	/* Fetch instruction in trace cache line. */
	for (i = 0; i < mop_count; i++)
	{
		/* If instruction caused context to suspend or finish */
		if (!X86ContextGetState(self->ctx, X86ContextRunning))
			break;
		
		/* Insert decoded uops into the trace cache queue. In the simulation,
		 * the uop is inserted into the fetch queue, but its occupancy is not
		 * increased. */
		self->fetch_neip = mop_array[i];
		uop = X86ThreadFetchInst(self, 1);
		if (!uop)  /* no uop was produced by this macroinst */
			continue;

		/* If instruction is a branch, access branch predictor just in order
		 * to have the necessary information to update it at commit. */
		if (uop->flags & X86_UINST_CTRL)
		{
			X86ThreadLookupBranchPred(self, uop);
			uop->pred_neip = i == mop_count - 1 ? neip :
				mop_array[i + 1];
		}
	}

	/* Set next fetch address as returned by the trace cache, and exit. */
	self->fetch_neip = neip;

	return 1;
}

static void X86ThreadFetch(X86Thread *self)
{
	X86Context *ctx = self->ctx;
	struct x86_uop_t *uop;

	unsigned int phy_addr;
	unsigned int block;
	unsigned int target;

	int taken;


	/*if(self->ctx->pid == 101 || self->ctx->pid == 102 || self->ctx->pid == 103)//self->ctx->pid == 100 ||
		printf("Thread %d running on core %d\n", self->ctx->pid, self->core->id);*/


	/* Try to fetch from trace cache first */
	//star no trace cache this is ignored.
	if (x86_trace_cache_present && X86ThreadFetchTraceCache(self))
	{
		fatal("X86ThreadFetch(): \"if (x86_trace_cache_present && X86ThreadFetchTraceCache(self))\" check this\n");
		//star >> never enters here
		//printf("Pulled from trace cache\n");
		//fflush(stdout);
		//getchar();
		return;
	}
	
	/* If new block to fetch is not the same as the previously fetched (and stored)
	 * block, access the instruction cache. */
	//virtual addresses.

#if CGM
	//block = self->fetch_neip & ~(self->inst_mod->block_size - 1);
	block = self->fetch_neip & ~(self->i_cache_ptr[self->core->id].block_size - 1);

	if (block != self->fetch_block)
	{
		/*fetches++;*/
		//star this prints out the current fetch address
		/*printf("fetch neip vtrl_addr 0x%08x, phy_addr 0x%08x page_id %d\n",
				self->fetch_neip, mmu_get_phyaddr(0, self->fetch_neip), mmu_get_page_id(0, self->fetch_neip, mmu_access_fetch));*/

		if(self->ctx->address_space_index != 0)
			fatal("X86ThreadFetch(): bad address space index as %d\n", self->ctx->address_space_index);

		assert(self->ctx->address_space_index == 0);

		//changes here
		//translate the address, wait for MMU to finish translation, will get run though TLB and PTW, fi faulted make no progress
		//in the real world we would be trapping to the OS on a fault.

		//warning("fetch vtl 0x%08x phy 0x%08x cycle %llu\n", self->fetch_neip, self->fetch_address, P_TIME);
		//getchar();

		if(!tlb_simple)
		{
			if(!mmu_fetch_translate(self, block))
			{
				return;
			}

			//access the cache
			//returns access ID
			self->fetch_access = cgm_fetch_access(self, self->fetch_address);
			self->btb_reads++;
		}
		else
		{
			//this runs the address translations with the original M2S code.
			int page_fault = 0;
			int *page_fault_ptr = &page_fault;

			phy_addr = mmu_translate(self->ctx->address_space_index, self->fetch_neip, mmu_access_fetch, page_fault_ptr);
			self->fetch_block = block;
			self->fetch_address = phy_addr;

			self->fetch_access = cgm_fetch_access(self, phy_addr);
			self->btb_reads++;

			/*MMU statistics*/
			if (*mmu_report_file_name)
				mmu_access_page(&mmu[self->core->id], phy_addr, mmu_access_execute);
		}

	}

	/* Fetch all instructions within the block up to the first predict-taken branch. */
	while ((self->fetch_neip & ~(self->i_cache_ptr[self->core->id].block_size - 1)) == block)
	{


#else
	block = self->fetch_neip & ~(self->inst_mod->block_size - 1);

	if (block != self->fetch_block)
	{
		phy_addr = mmu_translate(self->ctx->address_space_index, self->fetch_neip);
		self->fetch_block = block;
		self->fetch_address = phy_addr;
		self->fetch_access = mod_access(self->inst_mod, mod_access_load, phy_addr, NULL, NULL, NULL, NULL);
		self->btb_reads++;

		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(phy_addr, mmu_access_execute);
	}

	/* Fetch all instructions within the block up to the first predict-taken branch. */
	while ((self->fetch_neip & ~(self->inst_mod->block_size - 1)) == block)
	{
#endif

		/* If instruction caused context to suspend or finish */
		if (!X86ContextGetState(ctx, X86ContextRunning))
			break;

		/* If fetch queue full, stop fetching */
		if (self->fetchq_occ >= x86_fetch_queue_size)
			break;

		/* Insert macro-instruction into the fetch queue. Since the macro-instruction
		 * information is only available at this point, we use it to decode
		 * instruction now and insert uops into the fetch queue. However, the
		 * fetch queue occupancy is increased with the macro-instruction size. */

		uop = X86ThreadFetchInst(self, 0);

		//star if this flag is high the emulator processed an OpenCL syscall
		//add some data to the uop so we can catch it in the issue stage
		//and process the latencies for the interrupt service request.

		if (!ctx->inst.size)  /* x86_isa_inst invalid - no forward progress in loop */
			break;
		if (!uop)  /* no uop was produced by this macro-instruction */
			continue;

		/* Instruction detected as branches by the BTB are checked for branch
		 * direction in the branch predictor. If they are predicted taken,
		 * stop fetching from this block and set new fetch address. */
		if (uop->flags & X86_UINST_CTRL)
		{
			target = X86ThreadLookupBTB(self, uop);

			/*printf("target 0x%08x\n", target);*/

			taken = target && X86ThreadLookupBranchPred(self, uop);
			if (taken)
			{
				self->fetch_neip = target;
				uop->pred_neip = target;

				break;
			}
		}
	}

	return;
}


/*
 * Class 'X86Core'
 */

static void X86CoreFetch(X86Core *self)
{
	X86Thread *thread;

	int i;

	switch (x86_cpu_fetch_kind)
	{
		//star pulled out some of these options for clarity.
		case x86_cpu_fetch_kind_shared:
		{
			//Fetch from all threads
			/*for (i = 0; i < x86_cpu_num_threads; i++)
				if (X86ThreadCanFetch(self->threads[i]))
					X86ThreadFetch(self->threads[i]);*/

			fatal("X86CoreFetch(): \"x86_cpu_fetch_kind_shared\" add this back in");
			break;
		}

		case x86_cpu_fetch_kind_timeslice:
		{

			//star testing entry
			//printf("entered time slice\n");
			//fflush(stdout);

			/* Round-robin fetch */

			for (i = 0; i < x86_cpu_num_threads; i++)
			{
				self->fetch_current = (self->fetch_current + 1) % x86_cpu_num_threads;
				thread = self->threads[self->fetch_current];

				if (X86ThreadCanFetch(thread))
				{
					X86ThreadFetch(thread);
					break;
				}
			}
			break;
		}
	
		//star pulled out some of these options for clarity.
		case x86_cpu_fetch_kind_switchonevent:
		{
			/*X86Cpu *cpu = self->cpu;
			int must_switch;
			int new_index;

			X86Thread *new_thread;

			// If current thread is stalled, it means that we just switched to it.
			// * No fetching and no switching either.
			thread = self->threads[self->fetch_current];
			if (thread->fetch_stall_until >= asTiming(cpu)->cycle)
				break;

			 //Switch thread if:
			// - Quantum expired for current thread.
			// - Long latency instruction is in progress.
			must_switch = !X86ThreadCanFetch(thread);
			must_switch = must_switch || asTiming(cpu)->cycle - self->fetch_switch_when > x86_cpu_thread_quantum + x86_cpu_thread_switch_penalty;
			must_switch = must_switch || X86ThreadLongLatencyInEventQueue(thread);

			 //Switch thread
			if (must_switch)
			{
				 //Find a new thread to switch to
				for (new_index = (thread->id_in_core + 1) % x86_cpu_num_threads;
					new_index != thread->id_in_core;
					new_index = (new_index + 1) % x86_cpu_num_threads)
			{
				// Do not choose it if it is not eligible for fetching
				new_thread = self->threads[new_index];
				if (!X86ThreadCanFetch(new_thread))
					continue;
					
				// Choose it if we need to switch
				if (must_switch)
					break;

				// Do not choose it if it is unfair
				if (new_thread->num_committed_uinst_array >
						thread->num_committed_uinst_array + 100000)
					continue;

				// Choose it if it is not stalled
				if (!X86ThreadLongLatencyInEventQueue(new_thread))
					break;
			}
				
			 //Thread switch successful?
			if (new_index != thread->id_in_core)
			{
				self->fetch_current = new_index;
				self->fetch_switch_when = asTiming(cpu)->cycle;
				new_thread->fetch_stall_until = asTiming(cpu)->cycle +
						x86_cpu_thread_switch_penalty - 1;
			}
		}

		//Fetch
		thread = self->threads[self->fetch_current];
		if (X86ThreadCanFetch(thread))
			X86ThreadFetch(thread);*/

		fatal("X86CoreFetch(): \"x86_cpu_fetch_kind_switchonevent\" add this back in");
		break;
	}

	default:
		panic("%s: wrong fetch policy", __FUNCTION__);
	}

	return;
}




/*
 * Class 'X86Cpu'
 */

void X86CpuFetch(X86Cpu *self)
{
	int i;

	self->stage = "fetch";
	for (i = 0; i < x86_cpu_num_cores; i++)
		X86CoreFetch(self->cores[i]);
}
