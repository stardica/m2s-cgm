/*
 * queue.c
 *
 *  Created on: Nov 23, 2014
 *      Author: stardica
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/list.h>

#include <arch/x86/timing/thread.h>

#include <lib/util/misc.h>

//star todo take any borrowed files and move then to our cgm-mem directory.
#include <cgm/cgm.h>
#include <cgm/queue.h>
#include <cgm/cache.h>
#include <cgm/mem-ctrl.h>
#include <cgm/configure.h>
#include <cgm/sys-agent.h>
#include <cgm/ini-parse.h>
#include <cgm/tasking.h>
#include <cgm/packet.h>


long long access_id = 0;
struct list_t *cgm_access_record;

char *cgm_config_file_name_and_path;


//globals for tasking
eventcount *start;
eventcount *stop;


void cgm_init(void){

	//star todo add error checking.
	cache_init();
	memctrl_init();

	return;
}

void cgm_configure(void){

	cgm_access_record = list_create();

	//star todo add error checking.
	cgm_mem_configure();
	cgm_cpu_configure();

#if GPU
	cgm_gpu_configure();
#endif

	return;
}

int cgm_can_fetch_access(X86Thread *self, unsigned int addr){

	//star todo figure out where to put the mshr check.
	//unsigned int phy_address = addr;

	X86Thread *thread;
	thread = self;

	//check if request queue is full
	if(QueueSize <= list_count(thread->i_cache_ptr[thread->core->id].Rx_queue))
	{
		return 0;
	}

	//printf(" rx queue size %d\n", list_count(self->i_cache_ptr[self->core->id].Rx_queue));

	//i_cache queue is accessible.
	return 1;
}

int cgm_can_issue_access(X86Thread *self, unsigned int addr){

	//unsigned int phy_address = addr;
	X86Thread *thread;
	thread = self;

	//check if request queue is full
	if(QueueSize <= list_count(thread->i_cache_ptr[thread->core->id].Rx_queue))
	{
		return 0;
	}

	// d_cache queue is accessible.
	return 1;
}


int cgm_in_flight_access(X86Thread *self, long long id){


	X86Thread *thread;
	thread = self;

	//star todo need to retire access as they finish.
	struct cgm_packet_t *packet;
	int count = 0;
	int index = 0;

	count = list_count(cgm_access_record);

	/* Look for access */
	for (index = 0; index <= count; index++)
	{
		//take memory access out of queue and check it's status.
		packet = list_get(cgm_access_record, index);

		//return 0 if list is empty. return 1 if packet is found
		if (!packet)
		{
			return 0;
		}
		else if(packet->access_id == id)
		{
			return 1;
		}
	}

	/* Not found */
	return 0;

}


long long cgm_fetch_access(X86Thread *self, unsigned int addr){

	X86Thread *thread;
	thread = self;

	char buff[100];

	struct cgm_packet_t *new_packet = packet_create();

	//struct cgm_packet_t *new_packet;

	//set packet id to access id.
	access_id++;
	new_packet->access_id = access_id;
	new_packet->address = addr;
	new_packet->in_flight = 1;
	new_packet->c_load = 1;
	new_packet->access_type = cgm_access_load;

	memset(buff, '\0', 100);
	snprintf(buff, 100, "fetch_access.%llu", access_id);
	new_packet->name = buff;

	/*printf("new_packet->address = addr; 0x%08x\n", new_packet->address);
	printf("new_packet->name = %s\n", new_packet->name);
	printf("queue name bubba %s\n", thread->i_cache_ptr[thread->core->id].Rx_queue->name);*/

	//add to master list of accesses and 1st level i_cache
	list_enqueue(cgm_access_record, new_packet);

	//add to first level cache Rx queue
	list_enqueue(thread->i_cache_ptr[thread->core->id].Rx_queue, new_packet);

	//access the first level of cache
	//here threads package advance i_cache_ctrl
	l1_i_cache_ctrl(thread->i_cache_ptr[thread->core->id].id);

	return access_id;
}

void cgm_issue_lspq_access(X86Thread *self, enum cgm_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item){

	//printf("cgm_issue_lspq_access()\n");
	X86Thread *thread;
	thread = self;

	struct cgm_packet_t *new_packet = packet_create();

	new_packet->in_flight = 1;
	new_packet->event_queue = event_queue;
	new_packet->data = event_queue_item;

	//put back on the core event queue to end memory system access.
	linked_list_add(new_packet->event_queue, new_packet->data);
	free(new_packet);

	return;
}

/*void cgm_scalar_access(struct list_t *request_queue, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr){

	struct cgm_packet_t *new_packet = packet_create();

	printf("In memctrl witness pointer value %d\n", *witness_ptr);
	getchar();


	new_packet->in_flight = 1;
	new_packet->address = addr;
	new_packet->witness_ptr = witness_ptr;
	new_packet->event_queue = NULL;
	new_packet->data = NULL;

	printf("In memctrl witness pointer value %d\n", *new_packet->witness_ptr);
	getchar();


	(*new_packet->witness_ptr)++;

	printf("In memctrl witness pointer value after inc %d\n", *new_packet->witness_ptr);
	printf("In memctrl witness pointer value after inc %d\n", *witness_ptr);
	getchar();

	free(new_packet);

	return;
}

void cgm_vector_access(struct list_t *request_queue, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr){


	struct cgm_packet_t *new_packet = packet_create();

		printf("In memctrl witness pointer value %d\n", *witness_ptr);
		getchar();


	new_packet->in_flight = 1;
	new_packet->address = addr;
	new_packet->witness_ptr = witness_ptr;
	new_packet->event_queue = NULL;
	new_packet->data = NULL;


	(*new_packet->witness_ptr)++;


	free(new_packet);

	return;
}

//star todo this is wrong, the lds is a local memory module within the GPU.
//implement this as a memory block in the GPU with read write access.
void cgm_lds_access(struct list_t *request_queue, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr){

	struct cgm_packet_t *new_packet = packet_create();

	printf("In memctrl witness pointer value %d\n", *witness_ptr);
	getchar();

	new_packet->in_flight = 1;
	new_packet->address = addr;
	new_packet->witness_ptr = witness_ptr;
	new_packet->event_queue = NULL;
	new_packet->data = NULL;


	(*new_packet->witness_ptr)++;

	free(new_packet);

	return;
}*/




/*void cgm_mem_task_init(void){

	long long i = 1;
	//star the threads simulation.

	advance(start);

	printf("in task init\n");

	await(stop, i);

	return;
}*/



/*void cgm_mem_threads_init(void){

	char *taskname = NULL;
	char *eventname = NULL;

	//create task
	taskname = "cgm_mem_task_init";
	create_task(cgm_mem_task_init, DEFAULT_STACK_SIZE, taskname);
	taskname = "queue_ctrl";
	create_task(queue_ctrl, DEFAULT_STACK_SIZE, taskname);


	//create eventcounts
	eventname = "st1art";
	start = new_eventcount(eventname);
	eventname = "stop";
	stop = new_eventcount(eventname);

	return;
}*/


/*void cgm_mem_sim_loop(void){

	int error = 0;

	error = system_test();
	if(error == 1)
	{
		printf("failed in system_test()\n");
	}


	return;

}*/

/*void cleanup(void){

	star >> todo:
	(1) print stats or put in a file or something for later.
	(2) clean up all global memory objects by running finish functions.
	printf("---CGM-MEM Cleanup()---\n");
	fflush(stdout);
}*/
