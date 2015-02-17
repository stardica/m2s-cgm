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



//int host_sim_cpu_fetch_queue_size;
//int host_sim_cpu_lsq_queue_size;

char *cgm_config_file_name_and_path;

//long long int mem_cycle = 0;

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



	//star todo add error checking.
	cgm_mem_configure();
	cgm_cpu_configure();
	cgm_gpu_configure();

	return;
}

int cgm_can_fetch_access(struct cache_t *cache_ptr, unsigned int addr){

	//unsigned int phy_address = addr;
	//struct cache_t *cache_ptr = i_cache_ptr;

	//check if request queue is full
	/*if(QueueSize <= list_count(cache_ptr->Rx_queue))
	{
		return 0;
	}*/

	// mem_ctrl queue is accessible.
	return 1;
}

int cgm_can_issue_access(struct cache_t *d_cache_ptr, unsigned int addr){

	//unsigned int phy_address = addr;
	struct cache_t *cache_ptr = d_cache_ptr;

	//check if request queue is full
	if(QueueSize <= list_count(cache_ptr->Rx_queue))
	{
		return 0;
	}

	// mem_ctrl queue is accessible.
	return 1;
}


int cgm_in_flight_access(struct cache_t *i_cache_ptr, long long id){

	//struct cache_t *cache_ptr = i_cache_ptr;

	//star todo need to retire acceses as they finish.
	struct cgm_packet_t *packet;
	int count = 0;
	int index = 0;

	count = list_count(cgm_access_record);

	/* Look for access */
	for (index = 0; index <= count; index++)
	{
		//take memctrl access out of queue and check it's status.
		packet = list_get(cgm_access_record, index);

		//return 0 if list is empty. return 1 is packet is found
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


long long cgm_fetch_access(struct cache_t *cache_ptr, enum cgm_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item){

	struct cgm_packet_t *new_packet = packet_create();

	//star todo take data from fetch
	//set packet id to access id.
	access_id++;
	new_packet->access_id = access_id;
	new_packet->in_flight = 1;

	//add to master list of accesses.
	//star todo may need a mutex
	list_enqueue(cgm_access_record, new_packet);


	//insert into memctrl request queue
	//list_enqueue(mem_ctrl->fetch_request_queue, new_packet);

	//mem_ctrl do work here
	//remove from queue, process, and put on reply queue.

	//retire access in master list.
	list_dequeue(cgm_access_record);
	free(new_packet);

	return access_id;
}

void cgm_issue_lspq_access(struct cache_t *cache_ptr, enum cgm_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item){

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
