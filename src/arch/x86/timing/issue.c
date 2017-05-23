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

#include <m2s.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <mem-image/mmu.h>

#include <cgm/cgm.h>
/*#include <mem-system/module.h>*/

//#include <instrumentation/stats.h>
#include <arch/x86/timing/core.h>
#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/event-queue.h>
#include <arch/x86/timing/fu.h>
#include <arch/x86/timing/inst-queue.h>
#include <arch/x86/timing/issue.h>
#include <arch/x86/timing/load-store-queue.h>
#include <arch/x86/timing/reg-file.h>
#include <arch/x86/timing/thread.h>
#include <arch/x86/timing/trace-cache.h>


/*
 * Class 'X86Thread'
 */


int bar_issue = 0;

static int X86ThreadIssueSQ(X86Thread *self, int quantum)
{
	X86Cpu *cpu = self->cpu;
	X86Core *core = self->core;

	struct x86_uop_t *store;
	struct linked_list_t *sq = self->sq;
#if CGM
#else
	struct mod_client_info_t *client_info;
#endif

	/* Process SQ */
	linked_list_head(sq);
	while (!linked_list_is_end(sq) && quantum)
	{
		/* Get store */
		store = linked_list_get(sq);

		assert(store->uinst->opcode == x86_uinst_store || store->uinst->opcode == x86_uinst_store_ex
				||store->uinst->opcode == x86_uinst_cpu_flush || store->uinst->opcode == x86_uinst_cpu_fence);

		/*if(store->uinst->opcode == x86_uinst_cpu_fence)
			fatal("issue fence\n");*/

		//star changed this...
		//Only ready stores issue
		if (store->uinst->opcode == x86_uinst_store || store->uinst->opcode == x86_uinst_store_ex)
		{

			/*if(store->id == 1880)
			{
				printf("issued store break %llu cycle %llu\n", store->id, P_TIME);

				getchar();
			}*/

			assert(store->in_rob);

			if (!X86ThreadIsUopReady(self, store))
			{


				linked_list_next(sq);
				continue;
			}
		}

		assert(store->phy_address_ready == 0);

		if(!mmu_data_translate(self, store))
		{
			//translate the address
			//warning("looping id %llu cycle %llu\n", load->id, P_TIME);
			linked_list_next(sq);
			continue;
		}



		/*old M2S code...
		//Only committed stores issue
		if (store->in_rob && store->uinst->opcode != x86_uinst_cpu_fence)
		{
			assert(store->uinst->opcode != x86_uinst_cpu_fence);
			break;
		}*/


		/* Check that memory system entry is ready */
#if CGM
		if (!cgm_can_issue_access(self, store->phy_addr))
			break;

#else
		if (!mod_can_access(self->data_mod, store->phy_addr))
			break;
#endif

		/* Remove store from store queue */
		X86ThreadRemoveFromSQ(self);

#if CGM
		//star todo ?
		/* create and fill the mod_client_info_t object */
		//client_info = mod_client_info_create(self->mem_ctrl_ptr);
		//client_info->prefetcher_eip = store->eip;

		//set access type
		if(store->uinst->opcode == x86_uinst_store || store->uinst->opcode == x86_uinst_store_ex)
		{
			//store should only issue when its ready and should be in the ROB still.

			/*if(store->id == 1880)
			{
				printf("issued store %llu cycle %llu\n", store->id, P_TIME);

				getchar();
			}*/

			store->ready = 1;
			assert(store->in_rob);

			cgm_issue_lspq_access(self, cgm_access_store, store->id, store->phy_addr, core->event_queue, store);
		}
		else if (store->uinst->opcode == x86_uinst_cpu_flush)
		{
			//should already be ready from dispatch
			assert(store->ready == 1);

			cgm_issue_lspq_access(self, cgm_access_cpu_flush, store->id, store->phy_addr, core->event_queue, store);
		}
		else if (store->uinst->opcode == x86_uinst_gpu_flush)
		{

			fatal("gpu flush in sq\n");

			//should already be ready from dispatch
			assert(store->ready == 1);

			cgm_issue_lspq_access(self, cgm_access_gpu_flush, store->id, store->phy_addr, core->event_queue, store);
		}
		else if (store->uinst->opcode == x86_uinst_cpu_fence)
		{
			//fence should stall the core until all prior flushes complete....
			store->ready = 1;
			assert(store->in_rob);

			cgm_issue_lspq_access(self, cgm_access_cpu_fence, store->id, store->phy_addr, core->event_queue, store);
		}
		else
		{
			fatal("X86ThreadIssueSQ(): invalid access type\n");
		}

#else
		/* create and fill the mod_client_info_t object */
		client_info = mod_client_info_create(self->data_mod);
		client_info->prefetcher_eip = store->eip;

		/* Issue store */
		mod_access(self->data_mod, mod_access_store, store->phy_addr, NULL, core->event_queue, store, client_info);
#endif

		/* The cache system will place the store at the head of the
		 * event queue when it is ready. For now, mark "in_event_queue" to
		 * prevent the uop from being freed. */
		store->in_event_queue = 1;
		store->issued = 1;
		store->issue_when = asTiming(cpu)->cycle;
	
		/* Statistics */
		core->num_issued_uinst_array[store->uinst->opcode]++;
		core->lsq_reads++;
		core->reg_file_int_reads += store->ph_int_idep_count;
		core->reg_file_fp_reads += store->ph_fp_idep_count;
		self->num_issued_uinst_array[store->uinst->opcode]++;
		self->lsq_reads++;
		self->reg_file_int_reads += store->ph_int_idep_count;
		self->reg_file_fp_reads += store->ph_fp_idep_count;
		cpu->num_issued_uinst_array[store->uinst->opcode]++;
		if (store->trace_cache)
			self->trace_cache->num_issued_uinst++;

		/* One more instruction, update quantum. */
		quantum--;
		
		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(&mmu[self->core->id], store->phy_addr, mmu_access_write);
	}
	return quantum;
}


static int X86ThreadIssueLQ(X86Thread *self, int quant)
{
	X86Core *core = self->core;
	X86Cpu *cpu = self->cpu;

	struct linked_list_t *lq = self->lq;
	struct x86_uop_t *load;
#if CGM
#else
	struct mod_client_info_t *client_info;
#endif

	/* Process lq */
	linked_list_head(lq);
	while (!linked_list_is_end(lq) && quant)
	{
		/*Get element from load queue. If it is not ready, go to the next one */
		//star >> makes uOp wait until operands are ready.
		load = linked_list_get(lq);

		//printf("load id %llu cycle %llu\n", load->id, P_TIME);

		assert(load->phy_address_ready == 0);

		if (!load->ready && !X86ThreadIsUopReady(self, load))
		{
			linked_list_next(lq);
			continue;
		}

		if(!mmu_data_translate(self, load))
		{
			//translate the address
			//warning("looping id %llu cycle %llu\n", load->id, P_TIME);
			linked_list_next(lq);
			continue;
		}

		//if(load->id == 2)
		//	fatal("in issue\n");

		//Dependencies satisfied and address translation complete.
		//uop is ready to go to cache.
		load->ready = 1;

#if CGM
		//star todo add the memory access check here.
		if (!cgm_can_issue_access(self, load->phy_addr))
		{
#else
		/* Check that memory system is accessible */
		if (!mod_can_access(self->data_mod, load->phy_addr))
		{

#endif
			linked_list_next(lq);
			continue;
		}


		X86ThreadRemoveFromLQ(self);

			/*//star >> added instrumentation here.
		pthread_mutex_lock(&instrumentation_mutex);
		IssuedLSQStats(load);
		pthread_mutex_unlock(&instrumentation_mutex);*/

		/* Remove from load queue */
		assert(load->uinst->opcode == x86_uinst_load || load->uinst->opcode == x86_uinst_load_ex
				|| load->uinst->opcode == x86_uinst_cpu_load_fence || load->uinst->opcode == x86_uinst_gpu_flush);

#if CGM

		/* Access memory system */
		//loads++;
		/*if(load->phy_addr == 0)
		{
			load->protection_fault = 1;
		}*/

		printf("performing a load cycle %llu\n", P_TIME);


		if(load->uinst->opcode == x86_uinst_cpu_load_fence)
		{
			cgm_issue_lspq_access(self, cgm_access_cpu_load_fence, load->id, load->phy_addr, core->event_queue, load);
		}
		else if (load->uinst->opcode == x86_uinst_gpu_flush)
		{
			//should already be ready from dispatch
			assert(load->ready == 1);
			cgm_issue_lspq_access(self, cgm_access_gpu_flush, load->id, load->phy_addr, core->event_queue, load);
		}
		else
		{
			/*if(load->uinst->opcode == x86_uinst_load_ex)
				printf("loadex id %llu cycle %llu\n", load->uinst->id, P_TIME);*/
			cgm_issue_lspq_access(self, cgm_access_load, load->id, load->phy_addr, core->event_queue, load);
		}



#else
		/* create and fill the mod_client_info_t object */
		//star >> repos is just a repository of objects.
		client_info = mod_client_info_create(self->data_mod);
		client_info->prefetcher_eip = load->eip;

		/* Access memory system */
		//star added test.
		//PrintUOPStatus(load);
		mod_access(self->data_mod, mod_access_load, load->phy_addr, NULL, core->event_queue, load, client_info);
#endif

		/* The cache system will place the load at the head of the
		 * event queue when it is ready. For now, mark "in_event_queue" to
		 * prevent the uop from being freed. */
		load->in_event_queue = 1;
		load->issued = 1;
		load->issue_when = asTiming(cpu)->cycle;
		
		/* Statistics */
		core->num_issued_uinst_array[load->uinst->opcode]++;
		core->lsq_reads++;
		core->reg_file_int_reads += load->ph_int_idep_count;
		core->reg_file_fp_reads += load->ph_fp_idep_count;
		self->num_issued_uinst_array[load->uinst->opcode]++;
		self->lsq_reads++;
		self->reg_file_int_reads += load->ph_int_idep_count;
		self->reg_file_fp_reads += load->ph_fp_idep_count;
		cpu->num_issued_uinst_array[load->uinst->opcode]++;
		if (load->trace_cache)
			self->trace_cache->num_issued_uinst++;

		/* One more instruction issued, update quantum. */
		quant--;
		
		//LOADISSUED = core->num_issued_uinst_array[load->uinst->opcode];

		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(&mmu[self->core->id], load->phy_addr, mmu_access_read);

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"i\"\n", load->id_in_core, core->id);
	}
	
	return quant;
}


static int X86ThreadIssuePreQ(X86Thread *self, int quantum)
{
	X86Core *core = self->core;
	X86Cpu *cpu = self->cpu;

	struct linked_list_t *preq = self->preq;
	struct x86_uop_t *prefetch;

	/* Process preq */
	linked_list_head(preq);
	while (!linked_list_is_end(preq) && quantum)
	{

		fatal("Entered X86ThreadIssuePreQ. This is unimplemented in CGM for now.\n");

		/* Get element from prefetch queue. If it is not ready, go to the next one */
		prefetch = linked_list_get(preq);
		if (!prefetch->ready && !X86ThreadIsUopReady(self, prefetch))
		{
			linked_list_next(preq);
			continue;
		}

		//star todo this is broken
#if CGM

		//if (prefetch_history_is_redundant(core->prefetch_history, self->d_cache_ptr, prefetch->phy_addr))
		{
#else
		/* 
		 * Make sure its not been prefetched recently. This is just to avoid unnecessary
		 * memory traffic. Even though the cache will realize a "hit" on redundant
		 * prefetches, its still helpful to avoid going to the memory (cache). 
		 */
		if (prefetch_history_is_redundant(core->prefetch_history, self->data_mod, prefetch->phy_addr))
		{
#endif
			/* remove from queue. do not prefetch. */
			assert(prefetch->uinst->opcode == x86_uinst_prefetch);
			X86ThreadRemovePreQ(self);
			prefetch->completed = 1;
			x86_uop_free_if_not_queued(prefetch);
			continue;
		}

		prefetch->ready = 1;

#if CGM
		//star todo
		if (!cgm_can_issue_access(self, prefetch->phy_addr))
		{
#else
		/* Check that memory system is accessible */
		if (!mod_can_access(self->data_mod, prefetch->phy_addr))
		{
#endif
			linked_list_next(preq);
			continue;
		}

		/* Remove from prefetch queue */
		assert(prefetch->uinst->opcode == x86_uinst_prefetch);
		X86ThreadRemovePreQ(self);

#if CGM
		//star todo
		/* Access memory system */

		cgm_issue_lspq_access(self, cgm_access_prefetch, prefetch->id, prefetch->phy_addr, core->event_queue, prefetch);
#else
		/* Access memory system */
		mod_access(self->data_mod, mod_access_prefetch, prefetch->phy_addr, NULL, core->event_queue, prefetch, NULL);
#endif

		/* Record prefetched address */
		//prefetch_history_record(core->prefetch_history, prefetch->phy_addr);

		/* The cache system will place the prefetch at the head of the
		 * event queue when it is ready. For now, mark "in_event_queue" to
		 * prevent the uop from being freed. */
		prefetch->in_event_queue = 1;
		prefetch->issued = 1;
		prefetch->issue_when = asTiming(cpu)->cycle;
		
		/* Statistics */
		core->num_issued_uinst_array[prefetch->uinst->opcode]++;
		core->lsq_reads++;
		core->reg_file_int_reads += prefetch->ph_int_idep_count;
		core->reg_file_fp_reads += prefetch->ph_fp_idep_count;
		self->num_issued_uinst_array[prefetch->uinst->opcode]++;
		self->lsq_reads++;
		self->reg_file_int_reads += prefetch->ph_int_idep_count;
		self->reg_file_fp_reads += prefetch->ph_fp_idep_count;
		cpu->num_issued_uinst_array[prefetch->uinst->opcode]++;
		if (prefetch->trace_cache)
			self->trace_cache->num_issued_uinst++;

		/* One more instruction issued, update quantum. */
		quantum--;
		
		/* MMU statistics */
		if (*mmu_report_file_name)
			mmu_access_page(&mmu[self->core->id], prefetch->phy_addr, mmu_access_read);

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"i\"\n", prefetch->id_in_core, core->id);
	}
	
	return quantum;
}


static int X86ThreadIssueIQ(X86Thread *self, int quant)
{
	X86Cpu *cpu = self->cpu;
	X86Core *core = self->core;

	struct linked_list_t *iq = self->iq;
	struct x86_uop_t *uop;
	int lat;

	/* Find instruction to issue */
	linked_list_head(iq);
	while (!linked_list_is_end(iq) && quant)
	{
		/* Get element from IQ */
		uop = linked_list_get(iq);

		assert(x86_uop_exists(uop));
		assert(!(uop->flags & X86_UINST_MEM));

		if (!uop->ready && !X86ThreadIsUopReady(self, uop))
		{
			linked_list_next(iq);
			continue;
		}

		uop->ready = 1;  /* avoid next call to 'X86ThreadIsUopReady' */
		
		/* Run the instruction in its corresponding functional unit.
		 * If the instruction does not require a functional unit, 'X86CoreReserveFunctionalUnit'
		 * returns 1 cycle latency. If there is no functional unit available,
		 * 'X86CoreReserveFunctionalUnit' returns 0. */



		lat = X86CoreReserveFunctionalUnit(core, uop);
		if (!lat)
		{
			linked_list_next(iq);
			continue;
		}
		

		/* Instruction was issued to the corresponding fu.
		 * Remove it from IQ */
		X86ThreadRemoveFromIQ(self);
		
		/* Schedule inst in Event Queue */
		assert(!uop->in_event_queue);
		assert(lat > 0);
		uop->issued = 1;
		uop->issue_when = asTiming(cpu)->cycle;
		uop->when = asTiming(cpu)->cycle + lat;

		/*if(uop->interrupt > 0)
		{
			uop->when = asTiming(cpu)->cycle + 10000;
		}*/

		/*if (uop->id == 57923139)
		{
			assert(uop->interrupt == 1);
			warning("syscall %llu issued cycle %llu done %llu\n", uop->id, P_TIME, uop->when);
			getchar();

			int i = 0;
			struct x86_uop_t *uop_test;
			LIST_FOR_EACH(self->core->rob, i)
			{
				//get pointer to access in queue and check it's status.
				uop_test = list_get(self->core->rob, i);

				if(uop_test)
				{
					printf("\tslot %d packet id %llu\n", i, uop_test->id);
				}
			}

		}*/

		/*if(uop->interrupt == 1)
			fatal("dropping uop %llu into rob cycle %llu\n", uop->id, P_TIME);*/


		X86CoreInsertInEventQueue(core, uop);
		
		//star run the interrupt
		/*if(uop->interrupt > 0)
		{
			assert(uop->uinst->opcode == x86_uinst_syscall);
			cpu_gpu_stats->core_num_syscalls[self->core->id]++;
			//cgm_interrupt(self, uop);
		}*/


		/* Statistics */
		core->num_issued_uinst_array[uop->uinst->opcode]++;
		core->iq_reads++;
		core->reg_file_int_reads += uop->ph_int_idep_count;
		core->reg_file_fp_reads += uop->ph_fp_idep_count;
		self->num_issued_uinst_array[uop->uinst->opcode]++;
		self->iq_reads++;
		self->reg_file_int_reads += uop->ph_int_idep_count;
		self->reg_file_fp_reads += uop->ph_fp_idep_count;
		cpu->num_issued_uinst_array[uop->uinst->opcode]++;

		if (uop->trace_cache)
		{
			self->trace_cache->num_issued_uinst++;
		}

		/* One more instruction issued, update quantum. */
		quant--;

		/* Trace */
		x86_trace("x86.inst id=%lld core=%d stg=\"i\"\n", uop->id_in_core, core->id);
	}
	
	return quant;
}


static int X86ThreadIssueLSQ(X86Thread *self, int quantum)
{

	//translate address here?
	quantum = X86ThreadIssueLQ(self, quantum);
	quantum = X86ThreadIssueSQ(self, quantum);
	quantum = X86ThreadIssuePreQ(self, quantum);

	return quantum;
}





/*
 * Class 'X86Core'
 */

static void X86CoreIssue(X86Core *self)
{
	X86Thread *thread;

	int skip;
	int quantum;

	switch (x86_cpu_issue_kind)
	{
	//star >> commented out for clarity
	/*case x86_cpu_issue_kind_shared:
	{
		 Issue LSQs
		quantum = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do
		{
			self->issue_current = (self->issue_current + 1) % x86_cpu_num_threads;
			thread = self->threads[self->issue_current];
			quantum = X86ThreadIssueLSQ(thread, quantum);
			skip--;
		} while (skip && quantum);

		 Issue IQs
		quantum = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do
		{
			self->issue_current = (self->issue_current + 1) % x86_cpu_num_threads;
			thread = self->threads[self->issue_current];
			quantum = X86ThreadIssueIQ(thread, quantum);
			skip--;
		} while (skip && quantum);
		
		break;
	}*/
	
	case x86_cpu_issue_kind_timeslice:
	{
		/* Issue LSQs */
		quantum = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do
		{
			self->issue_current = (self->issue_current + 1) % x86_cpu_num_threads;
			thread = self->threads[self->issue_current];
			quantum = X86ThreadIssueLSQ(thread, quantum);
			skip--;
		} while (skip && quantum == x86_cpu_issue_width);

		/* Issue IQs */
		quantum = x86_cpu_issue_width;
		skip = x86_cpu_num_threads;
		do
		{
			self->issue_current = (self->issue_current + 1) % x86_cpu_num_threads;
			thread = self->threads[self->issue_current];
			quantum = X86ThreadIssueIQ(thread, quantum);
			skip--;
		} while (skip && quantum == x86_cpu_issue_width);

		break;
	}

	default:
		panic("%s: invalid issue kind", __FUNCTION__);
	}
}




/*
 * Class 'X86Cpu'
 */

void X86CpuIssue(X86Cpu *self)
{
	int i;

	self->stage = "issue";
	for (i = 0; i < x86_cpu_num_cores; i++)
		X86CoreIssue(self->cores[i]);
}
