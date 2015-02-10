/*
 * mem-ctrl.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */

#include <math.h>
#include <stdlib.h>
#include <cgm/mem-ctrl.h>
#include <cgm/queue.h>
#include <cgm/tasking.h>
#include <cgm/packet.h>

#include <lib/util/linked-list.h>


//structure declarations
struct mem_ctrl_t *mem_ctrl;


//events
eventcount *mem_ctrl_has_request;
eventcount *mem_ctrl_has_reply;
eventcount *mem_ctrl_serviced;


//star todo (1) Add memory controller logic.
//			(2) Add a mutex for master access record queue.
//			(3)

void memctrl_init(void){

	//star currently only one memory controller.
	mem_ctrl = memctrl_create();

	memctrl_queues_init();
	memctrl_tasking_init();

	return;
}

struct mem_ctrl_t  *memctrl_create(void){

	struct mem_ctrl_t *new_mem_ctrl = (void *) malloc(sizeof(struct mem_ctrl_t));

	return new_mem_ctrl;
}

void memctrl_queues_init(void){

	//star todo create list with size? or just check the size when insterting into list?
	mem_ctrl->fetch_request_queue = list_create();
	mem_ctrl->issue_request_queue = list_create();
	mem_ctrl->scalar_request_queue = list_create();
	mem_ctrl->vector_request_queue = list_create();
	mem_ctrl->memctrl_accesses = list_create();

	mem_ctrl->fetch_request_queue->name = "mem_ctrl.Fetch.Request";
	mem_ctrl->issue_request_queue->name = "mem_ctrl.Issue.Request";
	mem_ctrl->scalar_request_queue->name = "mem_ctrl.Scalar.Request";
	mem_ctrl->vector_request_queue->name = "mem_ctrl.Vector.Request";
	mem_ctrl->memctrl_accesses->name = "mem_ctrl.Accesses";

	return;

}

void memctrl_tasking_init(void){

	char *taskname = NULL;
	char *eventname = NULL;

	//create task
	taskname = "mem-ctrl.Request";
	create_task(memctrl_ctrl_request, DEFAULT_STACK_SIZE, taskname);
	taskname = "mem-ctrl.Reply";
	create_task(memctrl_ctrl_reply, DEFAULT_STACK_SIZE, taskname);
	taskname = "mem-ctrl.Action";
	create_task(memctrl_ctrl_service, DEFAULT_STACK_SIZE, taskname);

	//create event counts
	eventname = "mem_ctrl_has_request";
	mem_ctrl_has_request = new_eventcount(eventname);
	eventname = "mem_ctrl_has_reply";
	mem_ctrl_has_reply = new_eventcount(eventname);
	eventname = "mem_ctrl_do_action";
	mem_ctrl_serviced = new_eventcount(eventname);

	return;
}


int memctrl_can_fetch_access(struct mem_ctrl_t *ctrl, unsigned int addr){

	//unsigned int phy_address = addr;
	struct mem_ctrl_t *mem_ctrl_ptr = ctrl;

	//check if request queue is full
	if(mem_ctrl_ptr->queue_size <= list_count(mem_ctrl_ptr->fetch_request_queue))
	{
		return 0;
	}

	// mem_ctrl queue is accessible.
	return 1;
}

int memctrl_can_issue_access(struct mem_ctrl_t *ctrl, unsigned int addr){

	//unsigned int phy_address = addr;
	struct mem_ctrl_t *mem_ctrl_ptr = ctrl;

	//check if request queue is full
	if(mem_ctrl_ptr->queue_size <= list_count(mem_ctrl_ptr->issue_request_queue))
	{
		return 0;
	}

	// mem_ctrl queue is accessible.
	return 1;
}

//need to retire acceses as they finish.
int memctrl_in_flight_access(struct mem_ctrl_t *ctrl, long long id){

	struct mem_ctrl_t *mem_ctrl_ptr = ctrl;
	struct cgm_packet_t *packet;
	int count = 0;
	int index = 0;

	count = list_count(mem_ctrl_ptr->memctrl_accesses);

	/* Look for access */
	for (index = 0; index <= count; index++)
	{
		//take memctrl access out of queue and check it's status.
		packet = list_get(mem_ctrl_ptr->memctrl_accesses, index);

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


long long memctrl_fetch_access(struct list_t *request_queue, enum mem_ctrl_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item){

	struct cgm_packet_t *new_packet = packet_create();

	//star todo take data from fetch
	//set packet id to access id.
	mem_ctrl->access_id++;
	new_packet->access_id = mem_ctrl->access_id;
	new_packet->in_flight = 1;

	//add to master list of accesses.
	//may need a mutex
	list_enqueue(mem_ctrl->memctrl_accesses, new_packet);


	//insert into memctrl request queue
	//list_enqueue(mem_ctrl->fetch_request_queue, new_packet);

	//mem_ctrl do work here
	//remove from queue, process, and put on reply queue.

	//retire access in master list.
	list_dequeue(mem_ctrl->memctrl_accesses);
	free(new_packet);

	return mem_ctrl->access_id;
}

void memctrl_issue_lspq_access(struct list_t *request_queue, enum mem_ctrl_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item){

	struct cgm_packet_t *new_packet = packet_create();

	new_packet->in_flight = 1;
	new_packet->event_queue = event_queue;
	new_packet->data = event_queue_item;

	//put back on the core event queue to end memory system access.
	linked_list_add(new_packet->event_queue, new_packet->data);
	free(new_packet);

	return;
}

void memctrl_scalar_access(struct list_t *request_queue, enum mem_ctrl_access_kind_t access_kind, unsigned int addr, int *witness_ptr){

	struct cgm_packet_t *new_packet = packet_create();

	/*printf("In memctrl witness pointer value %d\n", *witness_ptr);
	getchar();*/


	new_packet->in_flight = 1;
	new_packet->address = addr;
	new_packet->witness_ptr = witness_ptr;
	new_packet->event_queue = NULL;
	new_packet->data = NULL;

	/*printf("In memctrl witness pointer value %d\n", *new_packet->witness_ptr);
	getchar();*/


	(*new_packet->witness_ptr)++;

	/*printf("In memctrl witness pointer value after inc %d\n", *new_packet->witness_ptr);
	printf("In memctrl witness pointer value after inc %d\n", *witness_ptr);
	getchar();*/

	free(new_packet);

	return;
}

void memctrl_vector_access(struct list_t *request_queue, enum mem_ctrl_access_kind_t access_kind, unsigned int addr, int *witness_ptr){


	struct cgm_packet_t *new_packet = packet_create();

		/*printf("In memctrl witness pointer value %d\n", *witness_ptr);
		getchar();*/


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
void memctrl_lds_access(struct list_t *request_queue, enum mem_ctrl_access_kind_t access_kind, unsigned int addr, int *witness_ptr){

	struct cgm_packet_t *new_packet = packet_create();

	/*printf("In memctrl witness pointer value %d\n", *witness_ptr);
	getchar();*/

	new_packet->in_flight = 1;
	new_packet->address = addr;
	new_packet->witness_ptr = witness_ptr;
	new_packet->event_queue = NULL;
	new_packet->data = NULL;


	(*new_packet->witness_ptr)++;

	free(new_packet);

	return;
}

//the CPU advances mem-ctrl with a memory request here
void memctrl_ctrl_request(void){


	return;
}

//mem-ctrl advances the CPU or, probably, will update the CPU and go into a wait state here
void memctrl_ctrl_reply(void){

	return;
}

//do some work.
void memctrl_ctrl_service(void){

	long long i = 1;

		while(1)
		{

			printf("in mem_ctrl\n");
			await(mem_ctrl_has_request, i);
			//take message off of in bound queue, process it, put reply on out bound queue.
			advance(mem_ctrl_has_reply);
			advance(mem_ctrl_serviced);

			i++;
		}

	return;

}

