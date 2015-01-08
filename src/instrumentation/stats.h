/*
 * stats.h
 *
 *  Created on: Oct 16, 2014
 *      Author: stardica
 */

#include <pthread.h>
#include <arch/x86/timing/uop.h>
#include <arch/x86/timing/load-store-queue.h>
#include <lib/util/list.h>
#include <lib/util/list.h>
#include <mem-system/memory.h>
#include <mem-system/mod-stack.h>
#include <mem-system/nmoesi-protocol.h>


#ifndef STATS_H_
#define STATS_H_

#define MAXUOPID 10
pthread_mutex_t instrumentation_mutex;

//fetch
extern volatile int fetch_queue_max_occupancy;
extern volatile long long Fetched_Insts;
extern volatile long long Fetched_Mem_Insts;
extern volatile long long Fetched_Loads;
extern volatile long long Fetched_Stores;
extern volatile long long Fetched_Pref;
extern volatile long long Interger_Inst;
extern volatile long long Logic_Inst;
extern volatile long long Floating_Point_Inst;
extern volatile long long Control_Inst;
extern volatile long long Conditional_Inst;
extern volatile long long Unconditional_Inst;
extern volatile long long XMM_Inst;

//Issue
extern volatile int load_queue_max_occupancy;
extern volatile int store_queue_max_occupancy;
extern volatile long long Issued_Insts;
extern volatile long long Issued_Mem_Insts;
extern volatile long long Issued_Loads;
extern volatile long long Issued_Stores;
extern volatile long long Issued_Pref;

//Commit
extern volatile int Rob_Queue_max_Occupancy;
extern volatile long long Rob_Insts_Queued;
extern volatile long long Commit_Insts;
extern volatile long long Commit_Mem_Insts;
extern volatile long long Commit_Loads;
extern volatile long long Commit_Stores;
extern volatile long long Committed_Pref;

//Memory
extern volatile long long number_of_load_events;
extern volatile long long number_of_store_events;
extern volatile long long number_of_noncached_store_events;

//global
extern volatile long long Current_Cycle;
extern struct list_t *Uop_List;

extern struct Uop_Status_t{

	long long UopID;
	int ctx_pid;
	int type;
	int Fetched;
	int Issued;
	int Committed;
};


//init
void instrumentation_init(void);
void instrumentation_done(void);
struct Uop_Status_t *Uop_status_init(void);

//stop the sim after configuration
//void stop(void);



//Pipeline
void FectchStats(struct x86_uop_t *uop, int ctx_pid);
void FetchQueueOccupancy(int occ);
void IssuedLSQStats(struct x86_uop_t *uop);
void LoadQueueOccupancy(int occ);
void StoreQueueOccupancy(int occ);
void CommitStats(struct x86_uop_t *uop, int ctx_pid);
void RobQueueOccupancy(int occ);

//Memory System
void PrintMem(struct mem_t *mem);
void MemEvent(int event, struct mod_stack_t *stack);
void PrintMessage(int code);
void PrintModNetList(struct list_t *mem_mod_list, struct list_t *net_list);


//dump stats
void PrintStats(void);
void PrintUopList(void);
void PrintCycle(void);


#endif /* STATS_H_ */
