/*
 * queue.c
 *
 *  Created on: Nov 23, 2014
 *      Author: stardica
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <m2s.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>

#include <arch/x86/timing/thread.h>

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
eventcount volatile *sim_start;
eventcount volatile *sim_finish;

void cgm_init(void){

	//star todo add error checking.
	cgm_access_record = list_create();
	cgm_create_tasks();

	//init memory system structures
	cache_init();
	memctrl_init();

	return;
}

void cgm_configure(void){

	//star todo add error checking.
	cgm_mem_configure();
	cgm_cpu_configure();

#if GPU
	cgm_gpu_configure();
#endif

	return;
}

void cgm_create_tasks(void){

	char buff[100];

	//eventcounts
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "sim_start");
	sim_start = new_eventcount(buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "sim_finish");
	sim_finish = new_eventcount(buff);


	//tasks
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "run");
	create_task(cpu_gpu_run, DEFAULT_STACK_SIZE, buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "startup and stop");
	create_task(cgm_start, DEFAULT_STACK_SIZE, buff);

	return;
}


void cgm_start(void){

	if(TSK == 1)
	{
		printf("cgm_start() advance sim_start\n");
	}

	advance(sim_start);

	if(TSK == 1)
	{
		printf("cgm_start() await sim_finish\n");
	}

	await(sim_finish, 1);

	if(TSK == 1)
	{
		printf("cgm_start() sim ending\n");
	}

	return;
}

void cpu_gpu_run(void){

	long long t_1 = 1;

	while(1)
	{

		if(TSK == 1 && t_1 == 1)
		{
			printf("cpu_gpu_run()\n");
		}

		await(sim_start, t_1);
		t_1++;

		m2s_loop();

	}
	return;
}


int cgm_can_fetch_access(X86Thread *self, unsigned int addr){

	//star todo figure out where to put the mshr check.
	//unsigned int phy_address = addr;

	X86Thread *thread;
	thread = self;

	//check if request queue is full
	if(QueueSize <= list_count(thread->d_cache_ptr[thread->core->id].Rx_queue))
	{
		//printf("rx queue is %d of %d\n", list_count(thread->d_cache_ptr[thread->core->id].Rx_queue), QueueSize);
		return 0;
	}

	//printf("rx queue is %d of %d\n", list_count(thread->d_cache_ptr[thread->core->id].Rx_queue), QueueSize);

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


int cgm_in_flight_access(long long id){

	//printf("uop access id is %lld\n", id);

	//star todo need to retire access as they finish.
	struct cgm_packet_t *packet;
	int count = 0;
	int index = 0;

	count = list_count(cgm_access_record);

	/* Look for access */
	for (index = 0; index <= count; index++)
	{
		//get pointer to access in queue and check it's status.
		packet = list_get(cgm_access_record, index);

		//return 0 if list is empty. return 1 if packet is found
		if (!packet)
		{
			return 0;
		}
		else if(packet->access_id == id)
		{
			fatal("packet in global access queue\n");

			return 1;
		}
	}

	/* packets are present but this one wasn't found */
	return 0;

}


long long cgm_fetch_access(X86Thread *self, unsigned int addr){

	X86Thread *thread;
	thread = self;

	char buff[100];

	struct cgm_packet_t *new_packet = packet_create();
	struct cgm_packet_t *new_packet_copy = packet_create();

	//set packet id to access id.
	access_id++;
	new_packet->access_id = access_id;
	new_packet->address = addr;
	new_packet->in_flight = 1;
	new_packet->c_load = 1;
	new_packet->access_type = cgm_access_load;

	memset(buff, '\0', 100);
	snprintf(buff, 100, "fetch_access.%llu", access_id);
	new_packet->name = strdup(buff);



	/*printf("new_packet->address = addr; 0x%08x\n", new_packet->address);
	printf("new_packet->name = %s\n", new_packet->name);
	printf("queue name bubba %s\n", thread->i_cache_ptr[thread->core->id].Rx_queue->name);*/

	//add to master list of accesses and 1st level i_cache
	list_enqueue(cgm_access_record, new_packet);


	if(thread->core->id == 0)
	{
		advance(l1_i_cache_0);
	}
	else if (thread->core->id == 1)
	{
		advance(l1_i_cache_1);
	}
	else if (thread->core->id == 2)
	{
		advance(l1_i_cache_2);
	}
	else if (thread->core->id == 3)
	{
		advance(l1_i_cache_3);
	}
	else
	{
		fatal("cgm_fetch_access() core id has a problem\n");
	}

	//leave this for testing.
	//printf("dequeue\n");
	//list_dequeue(cgm_access_record);

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

