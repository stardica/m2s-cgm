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

#include <lib/mhandle/mhandle.h>
#include <lib/util/string.h>
#include <lib/util/linked-list.h>

#include <arch/x86/timing/core.h>
#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/event-queue.h>
#include <arch/x86/timing/fu.h>
#include <arch/x86/timing/rob.h>
#include <arch/x86/timing/thread.h>

#include <cgm/cgm.h>

/*
 * Class 'X86Core'
 */

CLASS_IMPLEMENTATION(X86Core);

void X86CoreCreate(X86Core *self, X86Cpu *cpu)
{
	int i;

	/* Initialize */
	self->cpu = cpu;

	/* Create threads */
	self->threads = xcalloc(x86_cpu_num_threads, sizeof(X86Thread *));
	for (i = 0; i < x86_cpu_num_threads; i++)
		self->threads[i] = new(X86Thread, self);

	/* Prefetcher */
	//self->prefetch_history = prefetch_history_create();

	/* Structures */
	X86CoreInitROB(self);
	X86CoreInitEventQueue(self);
	X86CoreInitFunctionalUnits(self);
}


void X86CoreDestroy(X86Core *self)
{
	int i;

	/* Name */
	self->name = str_free(self->name);

	/* Free threads */
	for (i = 0; i < x86_cpu_num_threads; i++)
		delete(self->threads[i]);
	free(self->threads);

	/* Prefetcher */
	//prefetch_history_free(self->prefetch_history);

	/* Structures */
	X86CoreFreeROB(self);
	X86CoreFreeEventQueue(self);
	X86CoreFreeFunctionalUnits(self);
}


void X86CoreSetName(X86Core *self, char *name)
{
	self->name = str_set(self->name, name);
}

void core_dump_event_queue(X86Core *core){

	struct x86_uop_t *event_queue_uop;
	//struct linked_list_iter_t *iter = linked_list_iter_create(core->event_queue);

	//printf("event queue size %d\n", list_count(core->event_queue->count));

	LINKED_LIST_FOR_EACH(core->event_queue)
	{
		//get pointer to access in queue and check it's status.
		event_queue_uop = linked_list_get(core->event_queue);
		if(event_queue_uop)
		{
			printf("\t Core id %d event_queue_uop id %llu syscall_ready %d op_code %d\n",
				core->id, event_queue_uop->id, event_queue_uop->syscall_ready, event_queue_uop->uinst->opcode);
		}
	}
	return;
}



void core_dump_rob(X86Core *core){

	int i = 0;
	struct x86_uop_t *rob_uop = NULL;

	LIST_FOR_EACH(core->rob, i)
	{
		//get pointer to access in queue and check it's status.
		rob_uop = list_get(core->rob, i);
		if(rob_uop)
		{
			printf("\t Core id %d ROB slot %d rob_uop id %llu syscall_ready %d op_code %d start_cycle %llu %s\n",
				core->id, i, rob_uop->id, rob_uop->syscall_ready, rob_uop->uinst->opcode, rob_uop->uinst->start_cycle, (i == rob_uop->thread->rob_head) ? "<-- ROB head" : "");
		}
	}

	return;
}
