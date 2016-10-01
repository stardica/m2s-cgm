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

void X86CoreWriteback(X86Core *self)
{
	X86Cpu *cpu = self->cpu;
	X86Thread *thread;

	int i = 0;
	struct x86_uop_t *uop_test;

	struct x86_uop_t *uop;
	//struct x86_uop_t *rob_uop;

	int recover = 0;

	for (;;)
	{
		/* Pick element from the head of the event queue */
		linked_list_head(self->event_queue);
		uop = linked_list_get(self->event_queue);
		if (!uop)
			break;

		i++;
		/* A memory uop placed in the event queue is always complete.
		 * Other uops are complete when uop->when is equal to current cycle. */
		if (uop->flags & X86_UINST_MEM)
		{
			uop->when = asTiming(cpu)->cycle;
		}

		if (uop->when > asTiming(cpu)->cycle)
		{

			if(P_TIME > 7058 && P_TIME < 907058)
			{
				int i = 0;
				LIST_FOR_EACH(self->rob, i)
				{
					//get pointer to access in queue and check it's status.
					uop_test = list_get(self->rob, i);

					if(uop_test)
					{
						printf("\tslot %d packet id %llu\n", i, uop_test->id);
					}
				}

				printf("breaking alot! uop id %llu cycle %llu\n", uop->id, P_TIME);
				getchar();
			}

			break;
		}

		assert(asTiming(cpu)->cycle = P_TIME);


		if (uop->id == 788)
		{
			warning("id 788 when %llu\n", uop->when);
			getchar();
		}



		if (uop->uinst->opcode == x86_uinst_syscall)
		{
			assert(uop->interrupt == 1);

			warning("uop id %llu write back done cycle %llu curr cycle %llu\n", uop->id, uop->when, P_TIME);

			printf("syscall stalls.. %llu\n", cpu_gpu_stats->core_rob_stall_syscall[0]);

			getchar();

			cpu_gpu_stats->core_syscall_stalls[self->id]+= uop->interrupt_lat;
		}

		//core_dump_event_queue(self);
		//getchar();

		/*printf("core_id %d found syscall %llu at head of writeback ROB size is %d iq size %d lsq size %d uop size %d\n",
			self->id, uop->id, uop->thread->rob_count, uop->thread->iq_count, uop->thread->lsq_count, list_count(uop->thread->uop_queue));

		printf(" iq queue size %d\n", linked_list_count(uop->thread->iq));

		if(uop->id > 1986)
			getchar();*/
		
		/*if(uop->uinst->opcode == x86_uinst_load)*/
		//printf("finishing id %llu op %d when %llu cycle %llu cpu cycle %llu\n", uop->id, uop->uinst->opcode, uop->when, P_TIME, asTiming(cpu)->cycle);

		/* Check element integrity */
		assert(x86_uop_exists(uop));
		//assert(uop->when == asTiming(cpu)->cycle);
		assert(uop->thread->core == self);
		assert(uop->ready);
		assert(!uop->completed);
		
		/* Extract element from event queue. */
		linked_list_remove(self->event_queue);
		uop->in_event_queue = 0;
		thread = uop->thread;
		
		/* If a mispredicted branch is solved and recovery is configured to be
		 * performed at writeback, schedule it for the end of the iteration. */
		if (x86_cpu_recover_kind == x86_cpu_recover_kind_writeback && (uop->flags & X86_UINST_CTRL) && !uop->specmode && uop->neip != uop->pred_neip)
			recover = 1;

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

	//getchar();

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

