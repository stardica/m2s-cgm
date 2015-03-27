/*
 * stats.c
 *
 *  Created on: Nov 9, 2014
 *      Author: stardica
 */

#include <stdio.h>
#include <stdlib.h>
#include <instrumentation/stats.h>

#include <arch/x86/timing/rob.h>
#include <arch/x86/timing/fetch-queue.h>
#include <arch/x86/timing/load-store-queue.h>
#include <arch/x86/timing/uop.h>
#include <arch/x86/timing/cpu.h>

#include <arch/x86/emu/context.h>

#include <lib/util/list.h>

#include <mem-image/memory.h>

#include <cgm/cache.h>
#include <cgm/tasking.h>




//star >> instrumentation variables

//Fetch
volatile int fetch_queue_max_occupancy = 0;
volatile long long Fetched_Insts = 0;
volatile long long Fetched_Mem_Insts = 0;
volatile long long Fetched_Loads = 0;
volatile long long Fetched_Stores = 0;
volatile long long Fetched_Pref = 0;
volatile long long Interger_Inst = 0;
volatile long long Logic_Inst = 0;
volatile long long Floating_Point_Inst = 0;
volatile long long Control_Inst = 0;
volatile long long Conditional_Inst = 0;
volatile long long Unconditional_Inst = 0;
volatile long long XMM_Inst = 0;

//Issue
volatile int load_queue_max_occupancy = 0;
volatile int store_queue_max_occupancy = 0;
volatile long long Issued_Insts = 0;
volatile long long Issued_Mem_Insts = 0;
volatile long long Issued_Loads = 0;
volatile long long Issued_Stores = 0;
volatile long long Issued_Pref = 0;

//Commit
volatile int Rob_Queue_max_Occupancy = 0;
volatile long long Rob_Mem_Insts_Queued = 0;
volatile long long Committed_Insts = 0;
volatile long long Committed_Mem_Insts = 0;
volatile long long Committed_Loads = 0;
volatile long long Committed_Stores = 0;
volatile long long Committed_Pref = 0;

//Memory
volatile long long number_of_load_events = 0;
volatile long long number_of_store_events = 0;
volatile long long number_of_noncached_store_events = 0;

//Global
volatile unsigned long Current_Cycle = 0;
struct list_t *Uop_List;
struct Uop_Status_t *Uop_Status;

extern volatile long long l1_i_cache_0_hit = 0;
extern volatile long long l1_i_cache_0_miss = 0;


void instrumentation_init(void){

	Uop_List = list_create();

	/*debug
	printf("Uop list created with size %d\n", list_count(Uop_List));
	fflush(stdout);
	getchar();*/

	return;

}

struct Uop_Status_t *Uop_status_init(void){

	struct Uop_Status_t *Uop_Status;

	Uop_Status = (struct Uop_Status_t *) malloc(sizeof(struct Uop_Status_t));
	Uop_Status->Committed = 0;
	Uop_Status->Fetched = 0;
	Uop_Status->Issued = 0;
	Uop_Status->UopID = 0;
	Uop_Status->ctx_pid = 0;
	Uop_Status->type = 0;

	return Uop_Status;

}

/*void stop(void){

	fflush(stdout);
	getchar();
	return;
}*/


/* Context */
void PrintContext(X86Context *self){

	printf("---Context created PID %d with address space ID %d---\n", self->pid, self->address_space_index);
	fflush(stdout);
	//getchar();

	return;
}

void instrumentation_done(void){

	list_free(Uop_List);

	return;
}

void FectchStats(struct x86_uop_t *uop, int ctx_pid){


	if(uop->flags & X86_UINST_INT)
	{
		Fetched_Insts++;
		Interger_Inst++;
	}
	else if(uop->flags & X86_UINST_LOGIC)
	{
		Fetched_Insts++;
		Logic_Inst++;
	}
	else if(uop->flags & X86_UINST_FP)
	{
		Fetched_Insts++;
		Floating_Point_Inst++;
	}
	else if(uop->flags & X86_UINST_MEM)
	{

		//Wrap this up in a function.
		Uop_Status = Uop_status_init();

		Uop_Status->UopID = uop->id;
		Uop_Status->Fetched = 1;
		Uop_Status->ctx_pid = ctx_pid;

		Fetched_Insts++;
		Fetched_Mem_Insts++;
		if(uop->uinst->opcode == 52)
		{

			Uop_Status->type = uop->uinst->opcode;
			Fetched_Loads++;

			//debug
			//printf("Fetched a load instruction with ID %lld\n",uop->id);
		}
		if(uop->uinst->opcode == 53)
		{

			Uop_Status->type = uop->uinst->opcode;
			Fetched_Stores++;

			//debug
			//printf("Fetched a store instruction with ID %lld\n",uop->id);
		}

		if(uop->uinst->opcode == 54)
		{

			Uop_Status->type = uop->uinst->opcode;
			Fetched_Pref++;

			//debug
			//printf("Fetched a store instruction with ID %lld\n",uop->id);
		}

		list_add(Uop_List, Uop_Status);

		/*//debug
		printf("mem inst fetch and added to list current size is %d\n", list_count(Uop_List));
		fflush(stdout);
		getchar();*/

	}
	else if(uop->flags & X86_UINST_CTRL)
	{
		Fetched_Insts++;
		Control_Inst++;
	}
	else if(uop->flags & X86_UINST_COND)
	{
		Fetched_Insts++;
		Conditional_Inst++;
	}
	else if(uop->flags & X86_UINST_UNCOND)
	{
		Fetched_Insts++;
		Unconditional_Inst++;
	}
	else if(uop->flags & X86_UINST_XMM)
	{
		Fetched_Insts++;
		XMM_Inst++;
	}


	return;
}


void FetchQueueOccupancy(int occ){


	//note only works for 1 core 1 thread.
	if (fetch_queue_max_occupancy < occ)
	{
		fetch_queue_max_occupancy = occ;
	}

	if (occ == 0)
	{

		printf("Fetch queue empty in cycle %lld.\n", Current_Cycle);

	}

	/*if (occ > x86_fetch_queue_size){

		printf("Fetch queue exceeded max size in cycle %lld.\n", Current_Cycle);

	}*/

}


void IssuedLSQStats(struct x86_uop_t *uop){

	int i = 0;
	struct Uop_Status_t *Uop_In_List;


	//for (i = 0; i <= list_count(Uop_List); i++)
	for (i = 0; i <= MAXUOPID; i++)
	{

		Uop_In_List = list_get(Uop_List, i);


		if((Uop_In_List != NULL) && (uop->id == Uop_In_List->UopID))
		{


			Uop_In_List->Issued = 1;
			Issued_Mem_Insts++;

			//printf("fetched uop comitted\n");
			//fflush(stdout);

			if(Uop_In_List->type == 52)
			{
				Issued_Loads++;

				//debug
				//printf("Fetched a load instruction with ID %lld\n",uop->id);

			}

			if(Uop_In_List->type == 53)
			{

				Issued_Stores++;
				//debug
				//printf("Fetched a store instruction with ID %lld\n",uop->id);
			}

			if(Uop_In_List->type == 54)
			{

				Issued_Pref++;

				//debug
				//printf("Fetched a store instruction with ID %lld\n",uop->id);
			}


			//debug
			//printf("mem inst fetch and added to list current size is %d\n", list_count(Uop_List));
			//fflush(stdout);
			//getchar();

		}


	}

	Issued_Insts++;

	return;
}

void LoadQueueOccupancy(int occ){

	//note only works for 1 core 1 thread.
	if (load_queue_max_occupancy < occ)
	{
		load_queue_max_occupancy = occ;
	}

	if (occ == 0)
	{

		printf("Load queue empty in cycle %lld.\n", Current_Cycle);
		volatile int fetch_queue_max_occupancy = 0;
		volatile long long Fetched_Insts = 0;

	}
	else if (occ > x86_lsq_size)
	{

		printf("Load queue exceeded max size in cycle %lld.\n", Current_Cycle);

	}

}


void StoreQueueOccupancy(int occ){

	//note only works for 1 core 1 thread.
	if (store_queue_max_occupancy < occ)
	{
		store_queue_max_occupancy = occ;
	}

	if (occ == 0)
	{

		printf("Store queue empty in cycle %lld.\n", Current_Cycle);

	}
	else if (occ > x86_lsq_size)
	{

		printf("Store queue exceeded max size in cycle %lld.\n", Current_Cycle);

	}


}

void CommitStats(struct x86_uop_t *uop, int ctx_pid){

	int i = 0;
	struct Uop_Status_t *Uop_In_List;


	//for (i = 0; i <= list_count(Uop_List); i++)
	for (i = 0; i <= MAXUOPID; i++)
	{

		Uop_In_List = list_get(Uop_List, i);


		if((Uop_In_List != NULL) && (uop->id == Uop_In_List->UopID))
		{


			Uop_In_List->Committed = 1;
			Committed_Mem_Insts++;

			//printf("fetched uop comitted\n");
			//fflush(stdout);

			if(Uop_In_List->type == 52)
			{
				Committed_Loads++;

				//debug
				//printf("Fetched a load instruction with ID %lld\n",uop->id);

			}
			if(Uop_In_List->type == 53)
			{

				Committed_Stores++;
				//debug
				//printf("Fetched a store instruction with ID %lld\n",uop->id);
			}

			if(Uop_In_List->type == 54)
			{

				Committed_Pref++;

				//debug
				//printf("Fetched a store instruction with ID %lld\n",uop->id);
			}


			//debug
			//printf("mem inst fetch and added to list current size is %d\n", list_count(Uop_List));
			//fflush(stdout);
			//getchar();

		}


	}

	Committed_Insts++;

	return;
}

void RobQueueOccupancy(int occ){

	//note only works for 1 core 1 thread.
	if (Rob_Queue_max_Occupancy < occ)
	{
		Rob_Queue_max_Occupancy = occ;
	}

	if (occ == 0)
	{

		printf("Fetch queue empty in cycle %lld.\n", Current_Cycle);

	}
	else if (occ > x86_rob_size)
	{

		//printf("ROB exceeded max size in cycle %lld.\n", Current_Cycle);

	}

	/*if (occ > 128){

		printf("Fetch queue exceeded max size in cycle %lld.\n", Current_Cycle);

	}*/

}

void MemEvent(int event, struct mod_stack_t *stack){

	//printf("Stack ID %lld\n",stack->id);
	//printf("Stack access kind %d\n",stack->access_kind);
	//fflush(stdout);
	//getchar();

	if(event == EV_MOD_NMOESI_LOAD)
	{

		number_of_load_events++;

	}
	else if(event == EV_MOD_NMOESI_STORE)
	{

		number_of_store_events++;

	}
	else if(event == EV_MOD_NMOESI_NC_STORE)
	{

		number_of_noncached_store_events++;

	}

	return;

}

void PrintMessage(int code){

	if(code == 1)
	{
		printf("Executing esim event at cycle %lld\n", Current_Cycle);
	}

	if(code == 2)
	{



	}

	return;
}


/*void PrintModNetList(struct list_t *mem_mod_list, struct list_t *net_list){

	struct mod_t *mod;
	struct net_t *net;
	struct cache_t *cache;

	int i = list_count(mem_mod_list);
	int j = list_count(net_list);
	int k = 0;


	printf("Memory init and configure stats:\n\n");
	printf("Size of mod list %d\n\n", i);
	for(k = 0 ; k < i ; k++)
	{

		mod = list_get(mem_mod_list, k);
		//printf("Module ID: %s, Range, %d - %d\n", mod->name, mod->range.bounds.low, mod->range.bounds.high);
		printf("Module ID: %s\n", mod->name);

		cache = mod->cache;
		printf("Cache name: %s\n", cache->name);
		printf("Cache assoc: %d\n", cache->assoc);
		printf("Cache num sets: %d\n", cache->num_sets);
		printf("Cache block size: %d\n", cache->block_size);
		printf("Cache policy: %d\n", cache->policy);
		printf("Cache empty check: %d\n", (cache->sets->blocks == NULL ? 1 : 0));
		printf("\n");

	}
	printf("\nSize of net list %d\n\n", j);
	for(k = 0 ; k < j ; k++)
	{

		net = list_get(net_list, k);
		printf("Net ID: %s\n", net->name);

	}

	return;

}*/


void PrintStats(void){

	//star >> print some stats.


	printf("\n---stats---\n\n");

	int num_cores = x86_cpu_num_cores;
	int i = 0;

	printf("Total Cycles %lld\n\n", P_TIME + 1);

	for(i = 0; i < num_cores; i++)
	{
		printf("li_i_cache_%d\n", i);
		printf("Number of set %d\n", l1_i_caches[i].num_sets);
		printf("Block size = %d\n", l1_i_caches[i].block_size);
		printf("* hits %lld\n", l1_i_caches[i].hits);
		printf("* misses %lld\n", l1_i_caches[i].misses);
		printf("\n");
	}

	for(i = 0; i < num_cores; i++)
	{
		printf("li_2_cache_%d\n", i);
		printf("Number of set %d\n", l2_caches[i].num_sets);
		printf("Block size = %d\n", l2_caches[i].block_size);
		printf("* hits %lld\n", l2_caches[i].hits);
		printf("* misses %lld\n", l2_caches[i].misses);
		printf("\n");
	}


	/*printf("Fetch:\n");
	printf("Fetch queue max occupancy %d\n", fetch_queue_max_occupancy);
	printf("Number of fetched instructions %lld\n", Fetched_Insts);
	printf("Number of fetched memory instructions %lld (list Check %d)\n", Fetched_Mem_Insts, list_count(Uop_List));
	printf("* Number of fetched load instructions %lld\n", Fetched_Loads);
	printf("* Number of fetched store instructions %lld\n", Fetched_Stores);
	printf("* Number of fetched perf instructions %lld\n", Fetched_Pref);
	printf("\nIssue:\n");
	printf("Load queue max occupancy %d\n", load_queue_max_occupancy);
	printf("Store queue max occupancy %d\n", store_queue_max_occupancy);
	printf("Number of issued instructions ...\n");
	printf("Number of issued memory instructions %lld\n", Issued_Mem_Insts);
	printf("* Number of issued load instructions %lld\n", Issued_Loads);
	printf("* Number of issued stores instructions %lld\n", Issued_Stores);
	printf("* Number of issued perf instructions %lld\n", Issued_Pref);
	printf("\nCommit:\n");
	printf("Rob queue max occupancy %d\n", Rob_Queue_max_Occupancy);
	printf("Number of committed instructions %lld\n", Committed_Insts);
	printf("Number of committed memory instructions %lld\n", Committed_Mem_Insts);
	printf("* Number of committed load instructions %lld\n", Committed_Loads);
	printf("* Number of committed stores instructions %lld\n", Committed_Stores);
	printf("* Number of committed perf instructions %lld\n", Committed_Pref);
	printf("\nMemory:\n");
	printf("Number of load events (combined) %lld\n", number_of_load_events);
	printf("Number of store events %lld\n", number_of_store_events);
	printf("Number of noncached store events %lld\n", number_of_noncached_store_events);
	printf("Number of stacks created %lld\n", number_of_load_events + number_of_store_events + number_of_noncached_store_events);*/
	printf("---end stats---\n\n");
	fflush(stdout);

	return;
}

void PrintUopList(void){

	int i = 0;
	struct Uop_Status_t *Uop_In_List;

	printf("Uop trace through CPU pipline. (display is set to %d)\n", MAXUOPID);

	//for (i = 0; i <= list_count(Uop_List); i++)
	for (i = 0; i <= MAXUOPID; i++)
	{

		Uop_In_List = list_get(Uop_List, i);

		if(Uop_In_List)
		{


			printf("Uop_in_List ID  %lld PID %d\n", Uop_In_List->UopID, Uop_In_List->ctx_pid);
			printf("Uop_in_List Op Code %d\n", Uop_In_List->type);
			printf("Uop_in_List status %d %d %d\n\n", Uop_In_List->Fetched, Uop_In_List->Issued, Uop_In_List->Committed);
			//fflush(stdout);
			//getchar();
		}

	}

	return;
}

void PrintCycle(int skip){

	if((Current_Cycle % skip) == 0)
	{
		printf("---Cycles %lld---\n", Current_Cycle);
		fflush(stdout);
	}

	return;

}

void PrintMem(struct mem_t *mem){

	int i = 0;

	if(mem)
	{

		int pages = sizeof(mem->pages)/sizeof(mem->pages[0]);

		printf("---mem created for ctx---\n");
		printf("Heap break for CPU contextsheap brk %d\n", mem->heap_break);
		printf("Last accessed address %d\n", mem->last_address);
		printf("Number of ctx links %d\n", mem->num_links);
		printf("Current number of pages %d\n", pages);
		printf("Safe mode(1 == yes)? %d\n", mem->safe);
		fflush(stdout);
		//getchar();

		/*for (i = 0; i < pages; i++)
		{

			printf("size of mem page %d is %d\n", i, sizeof(mem->pages[i]));
			getchar();


		}*/

	}

	return;

}
