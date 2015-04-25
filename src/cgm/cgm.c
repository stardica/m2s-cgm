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

#include <arch/x86/timing/core.h>
#include <arch/x86/timing/thread.h>
#include <arch/x86/timing/uop.h>
#include <arch/si/timing/gpu.h>
#include <arch/si/timing/vector-mem-unit.h>
#include <arch/si/timing/scalar-unit.h>
#include <arch/si/timing/lds-unit.h>
#include <arch/si/timing/compute-unit.h>

#include <cgm/cgm.h>
#include <cgm/cache.h>
#include <cgm/directory.h>
#include <cgm/mem-ctrl.h>
#include <cgm/configure.h>
#include <cgm/sys-agent.h>
#include <cgm/ini-parse.h>
#include <cgm/tasking.h>
#include <cgm/interrupt.h>
#include <cgm/packet.h>
#include <cgm/switch.h>
#include <cgm/protocol.h>


long long access_id = 0;
//long long lspq_access_id = 0;
struct list_t *cgm_access_record;
char *cgm_config_file_name_and_path;

//file for debugging and stats


FILE *cache_debug_file;
int cache_debug = 0;

FILE *switch_debug_file;
int switch_debug = 0;

FILE *sysagent_debug_file;
int sysagent_debug = 0;

FILE *memctrl_debug_file;
int memctrl_debug = 0;

FILE *protocol_debug_file;
int protocol_debug = 0;

FILE *cgm_stats_file;
int cgm_stats = 0;

char *cgm_debug_output_path = "";
char *cgm_stats_output_path = "";

//globals for tasking
eventcount volatile *sim_start;
eventcount volatile *sim_finish;


void cgm_init(void){

	cgm_access_record = list_create();
	cgm_create_tasks();

	//init interrupt support
	interrupt_init();

	//init memory system structures
	cache_init();
	switch_init();
	directory_init();
	sys_agent_init();
	memctrl_init();

	return;
}

void cgm_configure(void){

	int error = 0;

	//star todo add error checking.
	error = cgm_mem_configure();
	if (error) {fatal("cgm_mem_configure() failed\n");}

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
	sim_start = new_eventcount(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "sim_finish");
	sim_finish = new_eventcount(strdup(buff));


	//tasks
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "cpu_gpu_run");
	create_task(cpu_gpu_run, DEFAULT_STACK_SIZE, strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "cgm_start");
	create_task(cgm_start, DEFAULT_STACK_SIZE, strdup(buff));

	//create the task for future advance.
	//this is specific to future_advance()
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "Wakeupcall");
	wakeup_task = create_task(wakeupcall, DEFAULT_STACK_SIZE, strdup(buff));
	initialize_wakeupcall(wakeup_task);

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

		future_advance(sim_finish, (etime.count + 2));
		//advance(sim_finish);

	}

	return;
}

void cgm_interrupt(X86Thread *self, struct x86_uop_t *uop){

	//star todo
	//create the memory system accesses
	//check if we can access both i and d caches.
	//fetch and data caches need to be accessed.

	X86Core *core = self->core;
	//struct x86_uop_t *interrupt_uop;
	int id = core->id;
	int num_cores = x86_cpu_num_cores;

	struct interrupt_t *isr = interrupt_service_routine_create();

	isr->uop = uop;
	isr->core_id = id;
	isr->thread = self;

	//put the uop on the interrupt list
	list_enqueue(interrupt_list, isr);

	//set the flag for the right core
	interrupt_cores[id]++;
	assert(id < num_cores);

	//advance the ISR
	advance(interrupt);

	return;
}


int cgm_can_fetch_access(X86Thread *self, unsigned int addr){


	X86Thread *thread;
	thread = self;

	//check if mshr queue is full

	int mshr_size = thread->i_cache_ptr[thread->core->id].mshr_size;
	int i = 0;
	int j = 0;

	for(i = 0; i < mshr_size; i++)
	{
		if(thread->i_cache_ptr[thread->core->id].mshrs[i].num_entries > 0)
		{
			j ++;
		}
	}

	//printf("j = %d\n", j);
	assert(j <= mshr_size);

	//mshr is full stall
	if(j > mshr_size)
	{
		return 0;
	}

	//check if request queue is full
	if(QueueSize <= list_count(thread->i_cache_ptr[thread->core->id].Rx_queue_top))
	{
		return 0;
	}

	//i_cache is accessible.
	return 1;
}

int cgm_can_issue_access(X86Thread *self, unsigned int addr){


	X86Thread *thread;
	thread = self;

	//check if mshr queue is full

	//star this is the old code.
	/*if(QueueSize <= list_count(thread->d_cache_ptr[thread->core->id].mshr))
	{
		return 0;
	}*/

	int mshr_size = thread->i_cache_ptr[thread->core->id].mshr_size;
	int i = 0;
	int j = 0;

	for(i = 0; i < mshr_size; i++)
	{
		if(thread->i_cache_ptr[thread->core->id].mshrs[i].num_entries > 0)
		{
			j ++;
		}
	}

	assert(j <= mshr_size);

	//mshr is full
	if(j == mshr_size)
	{
		return 0;
	}

	//check if request queue is full
	if(QueueSize <= list_count(thread->d_cache_ptr[thread->core->id].Rx_queue_top))
	{
		return 0;
	}

	// d_cache queue is accessible.
	return 1;
}


int cgm_in_flight_access(long long id){

	struct cgm_packet_status_t *packet;
	int i = 0;

	/* Look for access */
	LIST_FOR_EACH(cgm_access_record, i)
	{
		//get pointer to access in queue and check it's status.
		packet = list_get(cgm_access_record, i);

		//return 0 if list is empty. return 1 if packet is found
		if (!packet)
		{
			return 0;
		}
		else if(packet->access_id == id && packet->in_flight == 1)
		{
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
	access_id++;
	int num_cores = x86_cpu_num_cores;
	enum cgm_access_kind_t access_kind = cgm_access_fetch;
	int id = 0;

	//build two packets (1) to track global accesses and (2) to pass through the memory system
	memset(buff, '\0', 100);
	snprintf(buff, 100, "fetch_access.%llu", access_id);

	//(1)
	struct cgm_packet_status_t *new_packet_status = status_packet_create();
	new_packet_status->access_type = access_kind;
	new_packet_status->access_id = access_id;
	new_packet_status->address = addr;
	new_packet_status->in_flight = 1;

	//enqueue (1) in the global access queue.
	list_insert(cgm_access_record, 0, new_packet_status);

	//(2)
	struct cgm_packet_t *new_packet = packet_create();
	new_packet->access_type = access_kind;
	new_packet->access_id = access_id;
	new_packet->address = addr;
	new_packet->in_flight = 1;
	new_packet->name = strdup(buff);


	//Add (2) to the target L1 I Cache Rx Queue
	if(access_kind == cgm_access_fetch)
	{
		//get the core ID number should be <= number of cores
		id = thread->core->id;
		assert(id < num_cores);

		//Drop the packet into the L1 I Cache Rx queue
		list_enqueue(thread->i_cache_ptr[id].Rx_queue_top, new_packet);

		//advance the L1 I Cache Ctrl task
		//printf("advance(&l1_i_cache[%d])\n", id);
		//getchar();
		advance(&l1_i_cache[id]);
	}
	else
	{
		fatal("cgm_fetch_access() unsupported access type\n");
	}

	//star leave this for testing.
	//printf("dequeue\n");
	//list_dequeue(cgm_access_record);
	//free(new_packet);
	//free(new_packet_status);

	return access_id;
}

void cgm_issue_lspq_access(X86Thread *self, enum cgm_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item){


	X86Thread *thread;
	thread = self;
	char buff[100];
	access_id++;
	int num_cores = x86_cpu_num_cores;
	int id = 0;

	//build one packet to pass through the memory system
	memset(buff, '\0', 100);
	snprintf(buff, 100, "lspq_access.%lld", access_id);

	struct cgm_packet_t *new_packet = packet_create();
	new_packet->access_type = access_kind;
	new_packet->address = addr;
	new_packet->event_queue = event_queue;
	new_packet->data = event_queue_item;
	new_packet->in_flight = 1;
	new_packet->access_id = access_id;
	new_packet->name = strdup(buff);


	//For memory system load store request
	if(access_kind == cgm_access_load || access_kind == cgm_access_store)
	{
		//get the core ID number should be <= number of cores
		id = thread->core->id;
		assert(id < num_cores);

		//Drop the packet into the L1 D Cache Rx queue
		list_enqueue(thread->d_cache_ptr[id].Rx_queue_top, new_packet);

		//advance the L1 D Cache Ctrl task
		advance(&l1_d_cache[id]);

	}
	else if(access_kind == cgm_access_prefetch)
	{
		fatal("cgm_issue_lspq_access() type cgm_access_prefetch currently not implemented\n");
	}
	else
	{
		fatal("cgm_issue_lspq_access() unsupported access type\n");
	}

	//star leave this for testing.
	//put back on the core event queue to end memory system access.
	//linked_list_add(new_packet->event_queue, new_packet->data);
	//free(new_packet);

	return;
}

int remove_from_global(long long id){

	struct cgm_packet_status_t *packet;
	int i = 0;

	LIST_FOR_EACH(cgm_access_record, i)
	{
		//get pointer to access in queue and check it's status.
		packet = list_get(cgm_access_record, i);

		//return 0 if list is empty. return 1 if packet is found
		if (!packet)
		{
			return 0;
		}
		else if(packet->access_id == id)
		{
			list_remove_at(cgm_access_record, i);
			free(packet);

			// this leaves the access in the list, but slows down the simulation a lot.
			//packet->in_flight = 0;
			break;
		}
	}
	return 1;
}

void cgm_vector_access(struct si_vector_mem_unit_t *vector_mem, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr){

	struct si_vector_mem_unit_t *vector_mem_ptr = vector_mem;
	struct cgm_packet_t *new_packet = packet_create();
	char buff[100];
	access_id++;
	int num_cus = si_gpu_num_compute_units;
	int id = 0;

	//build one packet to pass through the memory system
	memset(buff, '\0', 100);
	snprintf(buff, 100, "vector_mem_access.%lld", access_id);

	new_packet->access_type = access_kind;
	new_packet->address = addr;
	new_packet->witness_ptr = witness_ptr;
	new_packet->in_flight = 1;
	new_packet->access_id = access_id;
	new_packet->name = strdup(buff);

	//leave for debugging purposes
	//(*witness_ptr)++;

	//Add to the target L1 Cache Rx Queue
	if(access_kind == cgm_access_load || access_kind == cgm_access_store || access_kind == cgm_access_nc_store)
	{
		//get the core ID number should be <= number of cores
		id = vector_mem_ptr->compute_unit->id;
		assert( id < num_cus);

		//Drop the packet into the GPU LDS unit Rx queue
		list_enqueue(vector_mem_ptr->compute_unit->gpu_v_cache_ptr[id].Rx_queue_top, new_packet);

		//advance the L1 I Cache Ctrl task
		advance(&gpu_v_cache[id]);
	}
	else
	{
		fatal("cgm_vector_access() unsupported access type\n");
	}

	return;
}

void cgm_scalar_access(struct si_scalar_unit_t *scalar_unit, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr){

	struct si_scalar_unit_t *scalar_unit_ptr = scalar_unit;
	struct cgm_packet_t *new_packet = packet_create();
	char buff[100];
	access_id++;
	int num_cus = si_gpu_num_compute_units;
	int id = 0;

	//build one packet to pass through the memory system
	memset(buff, '\0', 100);
	snprintf(buff, 100, "scalar_unit_access.%lld", access_id);

	new_packet->access_type = access_kind;
	new_packet->address = addr;
	new_packet->witness_ptr = witness_ptr;
	new_packet->in_flight = 1;
	new_packet->access_id = access_id;
	new_packet->name = strdup(buff);

	//leave for debugging purposes
	//(*witness_ptr)++;

	//Add to the target L1 Cache Rx Queue
	if(access_kind == cgm_access_load)
	{
		//get the core ID number should be <= number of cores
		id = scalar_unit_ptr->compute_unit->id;
		assert( id < num_cus);

		//set flag on target GPU LDS unit
		//gpu_s_caches_data[id]++;

		//Drop the packet into the GPU LDS unit Rx queue
		list_enqueue(scalar_unit_ptr->compute_unit->gpu_s_cache_ptr[id].Rx_queue_top, new_packet);

		//advance the L1 I Cache Ctrl task
		advance(&gpu_s_cache[id]);
	}
	else
	{
		fatal("cgm_scalar_access() unsupported access type\n");
	}

	return;
}

void cgm_lds_access(struct si_lds_t *lds, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr){


	struct si_lds_t *lds_ptr = lds;
	struct cgm_packet_t *new_packet = packet_create();
	char buff[100];
	access_id++;
	int num_cus = si_gpu_num_compute_units;
	int id = 0;

	//build one packet to pass through the memory system
	memset(buff, '\0', 100);
	snprintf(buff, 100, "lds_access.%lld", access_id);

	new_packet->access_type = access_kind;
	new_packet->address = addr;
	new_packet->witness_ptr = witness_ptr;
	new_packet->in_flight = 1;
	new_packet->access_id = access_id;
	new_packet->name = strdup(buff);


	//Add to the target L1 I Cache Rx Queue
	if(access_kind == cgm_access_load || access_kind == cgm_access_store)
	{
		//get the core ID number should be <= number of cores

		id = lds_ptr->compute_unit->id;
		assert( id < num_cus);

		//set flag on target GPU LDS unit
		//gpu_lds_units_data[id]++;

		//Drop the packet into the GPU LDS unit Rx queue
		list_enqueue(lds_ptr->compute_unit->gpu_lds_unit_ptr[id].Rx_queue_top, new_packet);

		//advance the L1 I Cache Ctrl task
		advance(&gpu_lds_unit[id]);
	}
	else
	{
		fatal("cgm_lds_access() unsupported access type\n");
	}

	return;
}

void cgm_dump_summary(void){

	printf("\n---Printing Stats---\n");

	cache_dump_stats();

	//close files
	CLOSE_FILES;

	return;
}
