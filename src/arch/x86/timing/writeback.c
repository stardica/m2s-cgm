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


#include <lib/esim/trace.h>
#include <lib/util/linked-list.h>
#include <arch/x86/timing/core.h>
#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/recover.h>
#include <arch/x86/timing/reg-file.h>
#include <arch/x86/timing/thread.h>
#include <arch/x86/timing/writeback.h>

#include <cgm/cgm.h>



/*
 * Class 'X86Core'
 */


int is_rob_head(X86Core *self, int uop_id){

	struct x86_uop_t *rob_uop;
	int i = 0;

	LIST_FOR_EACH(self->rob, i)
	{
		//get pointer to access in queue and check it's status.
		rob_uop = list_get(self->rob, i);
		if(rob_uop)
		{
			if(rob_uop->id == uop_id && rob_uop->thread->rob_head == i)
				return 1;
		}
	}

	return 0;
}


long long last_sys_call_id = 0;

void X86CoreWriteback(X86Core *self)
{
	X86Cpu *cpu = self->cpu;
	X86Thread *thread;

	//int i = 0;
	//struct x86_uop_t *uop_test;

	struct x86_uop_t *uop;
	struct x86_uop_t *uop_next;


	//struct x86_uop_t *rob_uop;

	int recover = 0;

	for (;;)
	{

		/*Old m2s code commented out, we need to pick the oldest element from the event queue
		which maybe behind an element at the top of the event queue.*/

		/*Pick element from the head of the event queue*/
		linked_list_head(self->event_queue);
		uop = linked_list_get(self->event_queue);
		if (!uop)
			break;

		LINKED_LIST_FOR_EACH(self->event_queue)
		{
			uop_next = linked_list_get(self->event_queue);
			assert(uop_next);

			if(uop_next)
			{
				if(uop->id > uop_next->id)
					uop = uop_next;
			}
		}


		//bring syscalls to the head of the event queue and rob. then assess a latency...
		if(uop->uinst->opcode == x86_uinst_syscall)
		{
			assert(uop->interrupt > 0);

			if(is_rob_head(self, uop->id) && uop->syscall_ready == 0)
			{
				assert(uop->syscall_ready == 0);
				uop->when = (P_TIME + 6000);
				uop->syscall_ready = 1;

				/*stats*/
				cpu_gpu_stats->core_num_syscalls[self->id]++;
				warning("wb syscall id %llu writeback syscall %llu cycle %llu\n", uop->id, cpu_gpu_stats->core_num_syscalls[self->id], P_TIME);

				if(uop->id == 50649573)
				{
					printf("event queue size %d\n", self->event_queue->count);
					core_dump_event_queue(self);

					printf("rob size %d\n", uop->thread->rob_count);
					core_dump_rob(self);

					getchar();
				}

			}
			else if(uop->syscall_ready == 0) //fails to catch uops that are already at the head of the rob.
			{
				assert(uop->syscall_ready == 0);
				uop->when = (P_TIME + 1);
			}
		}

		/*printf("rob head %d\n", is_rob_head(self, uop->id));

		printf("event queue size %d\n", self->event_queue->count);
		core_dump_event_queue(self);

		printf("rob size %d\n", uop->thread->rob_count);
		core_dump_rob(self);

		getchar();*/


		/* A memory uop placed in the event queue is always complete.
		 * Other uops are complete when uop->when is equal to current cycle. */
		if (uop->flags & X86_UINST_MEM)
		{
			uop->when = asTiming(cpu)->cycle;
		}

		if (uop->when > asTiming(cpu)->cycle)
		{
			break;
		}

		/*if(uop->uinst->opcode == x86_uinst_syscall)
		{
			assert(uop->syscall_ready == 1);

			printf("rob head %d cycle %llu\n", is_rob_head(self, uop->id), P_TIME);

		printf("event queue size %d\n", self->event_queue->count);
		core_dump_event_queue(self);

		printf("rob size %d\n", uop->thread->rob_count);
		core_dump_rob(self);

		getchar();
		}*/


		/*if(uop->uinst->opcode == x86_uinst_syscall)
		{
			assert(uop->interrupt > 0);

			if(uop->id > last_sys_call_id)
			{
				uop->when = (P_TIME + 6000);
				last_sys_call_id = uop->id;
			}

			printf("rob head %d\n", is_rob_head(self, uop->id));

			printf("event queue size %d\n", self->event_queue->count);
			core_dump_event_queue(self);

			printf("rob size %d\n", uop->thread->rob_count);
			core_dump_rob(self);

			getchar();

		}*/

		/* Check element integrity */
		assert(x86_uop_exists(uop));
		//assert(uop->when == asTiming(cpu)->cycle);
		assert(asTiming(cpu)->cycle = P_TIME);
		assert(uop->thread->core == self);
		assert(uop->ready);
		assert(!uop->completed);
		
		/* Extract element from event queue. */
		linked_list_find(self->event_queue, uop);
		linked_list_remove(self->event_queue);
		uop->in_event_queue = 0;
		thread = uop->thread;
		
		/* If a mispredicted branch is solved and recovery is configured to be
		 * performed at writeback, schedule it for the end of the iteration. */
		if (x86_cpu_recover_kind == x86_cpu_recover_kind_writeback && (uop->flags & X86_UINST_CTRL) && !uop->specmode && uop->neip != uop->pred_neip)
		{
			warning("recover...\n");
			getchar();

			recover = 1;

		}

		/* Trace. Prevent instructions that are not in the ROB from tracing.
		 * These can be either loads that were squashed, or stored that
		 * committed before issuing. */
		if (uop->in_rob)
			x86_trace("x86.inst id=%lld core=%d stg=\"wb\"\n", uop->id_in_core, self->id);

		/* Writeback */
		uop->completed = 1;
		X86ThreadWriteUop(thread, uop);

		//star >> statistics
		self->reg_file_int_writes += uop->ph_int_odep_count;
		self->reg_file_fp_writes += uop->ph_fp_odep_count;
		self->iq_wakeup_accesses++;
		thread->reg_file_int_writes += uop->ph_int_odep_count;
		thread->reg_file_fp_writes += uop->ph_fp_odep_count;
		thread->iq_wakeup_accesses++;
		x86_uop_free_if_not_queued(uop);

		/* Recovery. This must be performed at last, because lots of uops might be
		 * freed, which interferes with the temporary extraction from the event_queue. */
		if (recover)
			X86ThreadRecover(thread);
	}
}


/*
 * Class 'X86Cpu'
 */

void X86CpuWriteback(X86Cpu *self)
{
	int i;

	self->stage = "writeback";
	for (i = 0; i < x86_cpu_num_cores; i++)
	{
		X86CoreWriteback(self->cores[i]);
	}
}

