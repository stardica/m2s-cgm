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


#include <arch/x86/emu/context.h>
#include <lib/esim/trace.h>
#include <lib/util/list.h>
#include <arch/x86/timing/core.h>
#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/dispatch.h>
#include <arch/x86/timing/inst-queue.h>
#include <arch/x86/timing/load-store-queue.h>
#include <arch/x86/timing/reg-file.h>
#include <arch/x86/timing/rob.h>
#include <arch/x86/timing/thread.h>
#include <arch/x86/timing/trace-cache.h>

#include <cgm/cgm.h>

int is_head_syscall(X86Core *self){

	struct x86_uop_t *rob_uop;
	int i = 0;

	LIST_FOR_EACH(self->rob, i)
	{
		//get pointer to access in queue and check it's status.
		rob_uop = list_get(self->rob, i);
		if(rob_uop)
		{
			if(rob_uop->thread->rob_head == i && rob_uop->uinst->opcode == x86_uinst_syscall)
				return 1;
		}
	}

	return 0;
}


/* Return the reason why a thread cannot be dispatched. If it can,
 * return x86_dispatch_stall_used. */
static enum x86_dispatch_stall_t X86ThreadCanDispatch(X86Thread *self)
{
	X86Core *core = self->core;
	struct list_t *uopq = self->uop_queue;
	struct x86_uop_t *uop;

	/*is a syscall at the head of the rob?*/
	/* If iq/lq/sq/rob full, done */
	if (is_head_syscall(core))
	{
		return x86_dispatch_stall_syscall;
	}

	/* Uop queue empty. */
	uop = list_get(uopq, 0);
	if (!uop)
	{
		return !self->ctx || !X86ContextGetState(self->ctx, X86ContextRunning) ? x86_dispatch_stall_ctx : x86_dispatch_stall_uop_queue;
	}

	/* If iq/lq/sq/rob full, done */
	if (!X86CoreCanEnqueueInROB(core, uop))
	{
		return x86_dispatch_stall_rob;
	}

	if (!(uop->flags & X86_UINST_MEM) && !X86ThreadCanInsertInIQ(self, uop))
	{
		return x86_dispatch_stall_iq;
	}

	if ((uop->flags & X86_UINST_MEM) && !X86ThreadCanInsertInLSQ(self, uop))
	{
		return x86_dispatch_stall_lsq;
	}

	if (!X86ThreadCanRenameUop(self, uop))
	{
		return x86_dispatch_stall_rename;
	}
	
	return x86_dispatch_stall_used;
}

long long last_id = 0;
long long total_syscalls = 1;
long long idle_stall = 0;
int test = 0;

long long total_dispatches = 0;
long long total_syscalls_fence = 0;


static int X86ThreadDispatch(X86Thread *self, int quantum)
{
	X86Core *core = self->core;
	X86Cpu *cpu = self->cpu;

	struct x86_uop_t *uop;
	struct x86_uop_t *stall_uop = NULL;
	enum x86_dispatch_stall_t stall;


	/*if(self->core->id == 0)
		total_dispatches++;*/

	/*if(cpu_gpu_stats->core_num_fences[self->core->id] > 0)
	{
		assert(self->core->id == 0);

		total_syscalls_fence++;

		//cpu_gpu_stats->core_total_stalls[core->id]++;
		//cpu_gpu_stats->core_stall_syscall[core->id]++;
	}*/


	while (quantum)
	{

		/* Check if we can decode */
		stall = X86ThreadCanDispatch(self);

		/*if(self->core->id == 0 && test != 1 && cpu_gpu_stats->core_num_fences[self->core->id] > 0)
		{
			warning("waiting on fences %llu cycle %llu\n", cpu_gpu_stats->core_num_fences[self->core->id], P_TIME);
			test = 1;
		}*/

		if (stall != x86_dispatch_stall_used)
		{
			/*star added this taking some stats here.
			If the quantum is less than 4 then some progress was made
			and a stall here does not count as a core stall*/

			if(stall == x86_dispatch_stall_syscall && cpu_gpu_stats->core_num_fences[self->core->id] == 0)
			{
				//this if for busy time...
				stall_uop = list_get(self->core->rob, self->rob_head);
				assert(stall_uop->uinst->opcode == x86_uinst_syscall);
				//assert(quantum == x86_cpu_dispatch_width);

				cpu_gpu_stats->core_total_stalls[core->id]++;
				cpu_gpu_stats->core_stall_syscall[core->id]++;

				/*if(last_id < stall_uop->id)
				{
					warning("Dispatch: syscall id %llu type %u total %llu stall when %llu now %llu delta %llu cycle %llu\n",
							stall_uop->id, stall_uop->interrupt_type, total_syscalls, stall_uop->when, P_TIME, (stall_uop->when - P_TIME), P_TIME);
					last_id = stall_uop->id;
					total_syscalls++;

					//makes sure trap time gets charged correctly.
					//assert((stall_uop->when - P_TIME) >= 5000);
				}*/

				/*if (self->core->id == 0 && test == 1 && cpu_gpu_stats->core_num_fences[self->core->id] == 0)
				{
					warning("fence complete... cycle %llu\n", P_TIME);
					//getchar();
					test = 0;
				}*/
			}
			//stalls only count if dispatch makes no progress in a given cycle
			else if(quantum == x86_cpu_dispatch_width && cpu_gpu_stats->core_num_fences[self->core->id] == 0)
			{
				if(stall == x86_dispatch_stall_ctx || stall == x86_dispatch_stall_uop_queue)
				{

					//cpu idle time...
					if(stall == x86_dispatch_stall_ctx)
					{
						cpu_gpu_stats->core_idle_time[core->id]++;
					}
					//core fetch stall, uop queue is empty and ROB is empty...
					if(self->rob_count == 0 && stall == x86_dispatch_stall_uop_queue)
					{
						cpu_gpu_stats->core_total_stalls[core->id]++;
						cpu_gpu_stats->core_fetch_stalls[core->id]++;
					}
					/*CPU drain time. i.e. something is ending (ctx, processes, etc), so no new instructions are coming to dispatch,
						but the ROB still has some uops init. This coutns as CPU busy time.*/
					if(self->rob_count >= 1 && stall == x86_dispatch_stall_uop_queue)
					{

						cpu_gpu_stats->core_total_busy[core->id]++;
						cpu_gpu_stats->core_drain_time[core->id]++;
					}

				}
				else if(stall == x86_dispatch_stall_rob)
				{
					/*ROB is full (CPU is now stalled) collect stats for each of the different cores*/
					assert(self->rob_count == x86_rob_size); //64
					stall_uop = list_get(self->core->rob, self->rob_head);

					cpu_gpu_stats->core_total_stalls[core->id]++;
					cpu_gpu_stats->core_rob_stalls[core->id]++;

					if(stall_uop->uinst->opcode == x86_uinst_load)
					{
						cpu_gpu_stats->core_rob_stall_load[core->id]++;
					}
					else if(stall_uop->uinst->opcode == x86_uinst_store)
					{
						cpu_gpu_stats->core_rob_stall_store[core->id]++;
					}
					else if (stall_uop->uinst->opcode == x86_uinst_syscall)
					{
						fatal("dispatch stall on syscall\n");
						//cpu_gpu_stats->core_rob_stall_syscall[core->id]++;
					}
					else
					{
						cpu_gpu_stats->core_rob_stall_other[core->id]++;
					}
				}
				else if(stall == x86_dispatch_stall_lsq)
				{
					assert(self->rob_count < x86_rob_size);
					assert(self->lsq_count == x86_lsq_size);

					//stall_uop
					uop = list_get(self->uop_queue, 0);

					assert((uop->flags & X86_UINST_MEM) && (uop->uinst->opcode >= x86_uinst_load || uop->uinst->opcode <= x86_uinst_cpu_load_fence));

					cpu_gpu_stats->core_total_stalls[core->id]++;
					cpu_gpu_stats->core_lsq_stalls[core->id]++;

					if(self->sq->count == x86_lsq_size)
					{
						cpu_gpu_stats->core_lsq_stall_store[core->id]++;
					}
					else if(self->lq->count == x86_lsq_size)
					{
						cpu_gpu_stats->core_lsq_stall_load[core->id]++;
					}
					else
					{
						if(uop->uinst->opcode == x86_uinst_load || uop->uinst->opcode == x86_uinst_load_ex
								|| uop->uinst->opcode == x86_uinst_cpu_load_fence)
						{
							cpu_gpu_stats->core_lsq_stall_load[core->id]++;
						}
						else
						{
							assert(uop->uinst->opcode == x86_uinst_store || uop->uinst->opcode == x86_uinst_store_ex
									|| uop->uinst->opcode == x86_uinst_gpu_flush || uop->uinst->opcode == x86_uinst_cpu_fence
									|| uop->uinst->opcode == x86_uinst_cpu_flush);
							cpu_gpu_stats->core_lsq_stall_store[core->id]++;
						}
						//fatal("X86ThreadDispatch(): check this, shouldn't be here lq %d sq %d??\n", self->lq->count, self->sq->count);
					}
				}
				else if (stall == x86_dispatch_stall_iq)
				{
					assert(self->rob_count < x86_rob_size);
					assert(self->iq_count == x86_iq_size);

					fatal("stalled on iq, shouldn't happen with large iq size...\n");

					cpu_gpu_stats->core_total_stalls[core->id]++;
					cpu_gpu_stats->core_iq_stalls[core->id]++;
				}
				else if (stall == x86_dispatch_stall_rename)
				{
					assert(self->rob_count < x86_rob_size);

					cpu_gpu_stats->core_total_stalls[core->id]++;
					cpu_gpu_stats->core_rename_stalls[core->id]++;
				}
				else
				{
					fatal("X86ThreadDispatch(): invalid stall type as %d\n", stall);
				}
			}
			else
			{
				//some progress should have been made...
				if(cpu_gpu_stats->core_num_fences[self->core->id] == 0)
					assert(quantum < x86_cpu_dispatch_width);

				//if(cpu_gpu_stats->core_num_fences[self->core->id] > 0)
				//	assert(quantum < x86_cpu_dispatch_width);

				//catch syscall time...
				if(quantum == x86_cpu_dispatch_width && cpu_gpu_stats->core_num_fences[self->core->id] > 0)
				{
					assert(self->core->id == 0);

					cpu_gpu_stats->core_total_stalls[core->id]++;
					cpu_gpu_stats->core_stall_syscall[core->id]++;
				}

			}


			core->dispatch_stall[stall] += quantum;

			break;
		}

		assert(quantum >= 1 && quantum <= x86_cpu_dispatch_width);

		/* Get entry from uop queue */
		uop = list_remove_at(self->uop_queue, 0);
		assert(x86_uop_exists(uop));
		uop->in_uop_queue = 0;
		

		assert(core->id == 0);

		/*if(uop->id == 57923139)
		{
			warning("dispatched syscall id %llu cycle %llu\n", uop->id, P_TIME);

			getchar();
		}*/

		/*if(uop->uinst->opcode == x86_uinst_cpu_fence)
		{
			warning("Dispatch: core %d id %llu FENCE cycle %llu\n", core->id, uop->id, P_TIME);

			printf("event queue size %d\n", core->event_queue->count);
			core_dump_event_queue(core);

			printf("rob size %d\n", self->rob_count);
			core_dump_rob(core);

			getchar();
		}*/


		/*set up some parameters for the pipeline*/
		if(uop->uinst->opcode == x86_uinst_cpu_flush || uop->uinst->opcode == x86_uinst_gpu_flush)
			uop->ready = 1;


		/* Rename */
		X86ThreadRenameUop(self, uop);
		
		/* Insert in ROB */
		X86CoreEnqueueInROB(core, uop);
		core->rob_writes++;
		self->rob_writes++;
		
		/* Non memory instruction into IQ */
		if (!(uop->flags & X86_UINST_MEM))
		{
			X86ThreadInsertInIQ(self, uop);
			core->iq_writes++;
			self->iq_writes++;
		}
		
		/* Memory instructions into the LSQ */
		if (uop->flags & X86_UINST_MEM)
		{
			X86ThreadInsertInLSQ(self, uop);
			core->lsq_writes++;
			self->lsq_writes++;
			//LSQWrites++;
		}
		
		/* Statistics */
		core->dispatch_stall[uop->specmode ? x86_dispatch_stall_spec : x86_dispatch_stall_used]++;
		self->num_dispatched_uinst_array[uop->uinst->opcode]++;
		core->num_dispatched_uinst_array[uop->uinst->opcode]++;
		cpu->num_dispatched_uinst_array[uop->uinst->opcode]++;
		if (uop->trace_cache)
			self->trace_cache->num_dispatched_uinst++;
		
		/* Another instruction dispatched, update quantum. */
		quantum--;

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"di\"\n", uop->id_in_core, core->id);
	}

	//for work performed during normal cpu time stats busy time...
	if(quantum < x86_cpu_dispatch_width && cpu_gpu_stats->core_num_fences[self->core->id] == 0)
	{
		cpu_gpu_stats->core_total_busy[core->id]++;
	}

	//for work performed during a syscall
	if(quantum < x86_cpu_dispatch_width && cpu_gpu_stats->core_num_fences[self->core->id] > 0)
	{
		cpu_gpu_stats->core_total_stalls[core->id]++;
		cpu_gpu_stats->core_stall_syscall[core->id]++;
	}


	return quantum;
}


static void X86CoreDispatch(X86Core *self)
{
	X86Thread *thread;

	int skip = x86_cpu_num_threads;
	int quantum = x86_cpu_dispatch_width;


	switch (x86_cpu_dispatch_kind)
	{

	case x86_cpu_dispatch_kind_shared:
		
		/*
		int remain;
		do
		{
			self->dispatch_current = (self->dispatch_current + 1) % x86_cpu_num_threads;
			thread = self->threads[self->dispatch_current];
			remain = X86ThreadDispatch(thread, 1);
			skip = remain ? skip - 1 : x86_cpu_num_threads;
			quantum = remain ? quantum : quantum - 1;
		} while (quantum && skip);*/

		fatal("X86CoreDispatch(): \"x86_cpu_dispatch_kind_shared\" add this back in\n");
		break;
	
	case x86_cpu_dispatch_kind_timeslice:
		do
		{
			self->dispatch_current = (self->dispatch_current + 1) % x86_cpu_num_threads;
			thread = self->threads[self->dispatch_current];
			skip--;
		} while (skip && X86ThreadCanDispatch(thread) != x86_dispatch_stall_used);

		X86ThreadDispatch(thread, quantum);

		break;
	}
}


void X86CpuDispatch(X86Cpu *self)
{
	int i;

	self->stage = "dispatch";
	for (i = 0; i < x86_cpu_num_cores; i++)
		X86CoreDispatch(self->cores[i]);
}
