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


/* Return the reason why a thread cannot be dispatched. If it can,
 * return x86_dispatch_stall_used. */
static enum x86_dispatch_stall_t X86ThreadCanDispatch(X86Thread *self)
{
	X86Core *core = self->core;
	struct list_t *uopq = self->uop_queue;
	struct x86_uop_t *uop;

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


static int X86ThreadDispatch(X86Thread *self, int quantum)
{
	X86Core *core = self->core;
	X86Cpu *cpu = self->cpu;

	struct x86_uop_t *uop;
	struct x86_uop_t *rob_uop = NULL;
	enum x86_dispatch_stall_t stall;

	while (quantum)
	{
		/* Check if we can decode */
		stall = X86ThreadCanDispatch(self);

		if (stall != x86_dispatch_stall_used)
		{
			//star added this taking some stats here
			if(stall == x86_dispatch_stall_ctx || stall == x86_dispatch_stall_uop_queue)
			{
				//no uop
				/*if(self->rob_count == 64)
				{
					rob_uop = list_get(self->core->rob, self->rob_head);

					printf("core %d ROB dispatch full reason %d rob head %d id %llu cycle %llu\n", core->id, stall, rob_uop->uinst->opcode, rob_uop->id, P_TIME);
				}*/

				if(self->rob_count == 0 && stall == x86_dispatch_stall_uop_queue)
					cpu_gpu_stats->core_fetch_stalls[core->id]++;
			}
			else if(stall == x86_dispatch_stall_rob)
			{
				//printf("stall on rob\n");
				/*ROB is full collect stats for each of the different cores*/
				assert(self->rob_count == 64);
				rob_uop = list_get(self->core->rob, self->rob_head);

				if(rob_uop->uinst->opcode == x86_uinst_load)
				{
					cpu_gpu_stats->core_rob_stall_load[core->id]++;
					cpu_gpu_stats->core_rob_stalls[core->id]++;
				}
				else if(rob_uop->uinst->opcode == x86_uinst_store)
				{
					cpu_gpu_stats->core_rob_stall_store[core->id]++;
					cpu_gpu_stats->core_rob_stalls[core->id]++;
				}
				else if (rob_uop->uinst->opcode == x86_uinst_syscall)
				{
					//printf("syscallstalls\n");
					cpu_gpu_stats->core_rob_stall_syscall[core->id]++;
				}
				else
				{
					//cpu_gpu_stats->core_rob_stall_other[core->id]++;
					//cpu_gpu_stats->core_rob_stalls[core->id]++;
				}
			}
			else if(stall == x86_dispatch_stall_lsq)
			{
				assert(self->rob_count < 64);
				//printf("stall on lsq\n");
				//mem_system_stats->cpu_ls_stalls++;
			}
			else if (stall == x86_dispatch_stall_iq)
			{
				assert(self->rob_count < 64);
			}
			else if (stall == x86_dispatch_stall_rename)
			{
				assert(self->rob_count < 64);
				/*printf("stall on rename\n");
				getchar();*/
			}
			else
			{
				fatal("X86ThreadDispatch(): invalid stall type\n");
			}

			core->dispatch_stall[stall] += quantum;
			break;
		}

	
		/* Get entry from uop queue */
		uop = list_remove_at(self->uop_queue, 0);
		assert(x86_uop_exists(uop));
		uop->in_uop_queue = 0;
		
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
