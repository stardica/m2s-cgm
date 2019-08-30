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
#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <lib/util/string.h>
#include <arch/x86/timing/bpred.h>
#include <arch/x86/timing/core.h>
#include <arch/x86/timing/fetch-queue.h>
#include <arch/x86/timing/inst-queue.h>
#include <arch/x86/timing/load-store-queue.h>
#include <arch/x86/timing/reg-file.h>
#include <arch/x86/timing/thread.h>
#include <arch/x86/timing/trace-cache.h>
#include <arch/x86/timing/uop-queue.h>


/*
 * Class 'X86Thread'
 */

CLASS_IMPLEMENTATION(X86Thread);

void X86ThreadCreate(X86Thread *self, X86Core *core)
{
	/* Initialize */
	self->core = core;
	self->cpu = core->cpu;

	/* Structures */
	X86ThreadInitUopQueue(self);
	X86ThreadInitLSQ(self);
	X86ThreadInitIQ(self);
	X86ThreadInitRegFile(self);
	X86ThreadInitFetchQueue(self);
	X86ThreadInitBranchPred(self);
	X86ThreadInitTraceCache(self);
}


void X86ThreadDestroy(X86Thread *self)
{
	/* Structures */
	X86ThreadFreeUopQueue(self);
	X86ThreadFreeLSQ(self);
	X86ThreadFreeIQ(self);
	X86ThreadFreeRegFile(self);
	X86ThreadFreeFetchQueue(self);
	X86ThreadFreeBranchPred(self);
	X86ThreadFreeTraceCache(self);

	/* Finalize */
	self->name = str_free(self->name);
}


void X86ThreadSetName(X86Thread *self, char *name)
{
	self->name = str_set(self->name, name);
}


int X86ThreadIsPipelineEmpty(X86Thread *self)
{
	return !self->rob_count && !self->fetch_queue->count && !self->uop_queue->count;
}


void thread_dump_fetch_queue(X86Thread *self){

	int i = 0;
	struct x86_uop_t *uop = NULL;

	LIST_FOR_EACH(self->fetch_queue, i)
	{
		//get pointer to access in queue and check it's status.
		uop = list_get(self->fetch_queue, i);
		if(uop)
		{
			printf("\t Core id %d fetch slot %d rob_uop id %llu op_code %d\n",
				self->core->id, i, uop->id, uop->uinst->opcode);
		}
	}

	return;
}


void thread_dump_uop_queue(X86Thread *self){

	int i = 0;
	struct x86_uop_t *uop = NULL;

	LIST_FOR_EACH(self->uop_queue, i)
	{
		//get pointer to access in queue and check it's status.
		uop = list_get(self->uop_queue, i);
		if(uop)
		{
			printf("\t Core id %d uop queue slot %d rob_uop id %llu syscall_ready %d op_code %d\n",
				self->core->id, i, uop->id, uop->syscall_ready, uop->uinst->opcode);
		}
	}

	return;
}

/*void dump_uinst_queue(struct list_t *x86_uinst_list){

	int i = 0;
	struct x86_uinst_t *uinst = NULL;

	LIST_FOR_EACH(x86_uinst_list, i)
	{
		//get pointer to access in queue and check it's status.
		uinst = list_get(x86_uinst_list, i);
		if(uinst)
		{
			printf("\t slot %d rob_uop id %llu op_code %d\n",
				i, uinst->id, uinst->opcode);
		}
	}

	return;
}*/


void thread_dump_inst_queue(X86Thread *self){

	int i = 0;
	struct x86_uop_t *uop = NULL;

	LINKED_LIST_FOR_EACH(self->iq)
	{
		//get pointer to access in queue and check it's status.
		uop = linked_list_get(self->iq);
		if(uop)
		{
			printf("\t Core id %d inst slot %d rob_uop id %llu op_code %d\n",
				self->core->id, i, uop->id, uop->uinst->opcode);
		}
	}

	return;
}

void thread_dump_load_queue(X86Thread *self){

	int i = 0;
	struct x86_uop_t *uop = NULL;

	LINKED_LIST_FOR_EACH(self->lq)
	{
		//get pointer to access in queue and check it's status.
		uop = linked_list_get(self->lq);
		if(uop)
		{
			printf("\t Core id %d load slot %d rob_uop id %llu op_code %d\n",
				self->core->id, i, uop->id, uop->uinst->opcode);
		}
	}

	return;
}

void thread_dump_store_queue(X86Thread *self){

	int i = 0;
	struct x86_uop_t *uop = NULL;

	LINKED_LIST_FOR_EACH(self->sq)
	{
		//get pointer to access in queue and check it's status.
		uop = linked_list_get(self->sq);
		if(uop)
		{
			printf("\t Core id %d store slot %d rob_uop id %llu op_code %d\n",
				self->core->id, i, uop->id, uop->uinst->opcode);
		}
	}

	return;
}

void thread_dump_pred_queue(X86Thread *self){

	int i = 0;
	struct x86_uop_t *uop = NULL;

	LINKED_LIST_FOR_EACH(self->preq)
	{
		//get pointer to access in queue and check it's status.
		uop = linked_list_get(self->preq);
		if(uop)
		{
			printf("\t Core id %d preq slot %d rob_uop id %llu op_code %d\n",
				self->core->id, i, uop->id, uop->uinst->opcode);
		}
	}

	return;
}
