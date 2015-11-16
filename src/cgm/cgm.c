/*
 * queue.c
 *
 *  Created on: Nov 23, 2014
 *      Author: stardica
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cgm/cgm.h>




long long access_id = 0;
//long long lspq_access_id = 0;
struct list_t *cgm_access_record;
char *cgm_config_file_name_and_path;

//files for debugging and stats;
FILE *CPU_cache_debug_file;
int CPU_cache_debug = 0;

FILE *GPU_cache_debug_file;
int GPU_cache_debug = 0;

FILE *switch_debug_file;
int switch_debug = 0;

FILE *hub_iommu_debug_file;
int hub_iommu_debug = 0;

FILE *sysagent_debug_file;
int sysagent_debug = 0;

FILE *memctrl_debug_file;
int memctrl_debug = 0;

FILE *protocol_debug_file;
int protocol_debug = 0;

FILE *mshr_debug_file;
int mshr_debug;

FILE *ort_debug_file;
int ort_debug = 0;

FILE *cgm_stats_file;
int cgm_stats = 0;

FILE *mem_trace_file;
int mem_trace = 0;

FILE *load_store_log_file;
int load_store_debug = 0;

char *cgm_debug_output_path = "";
char *cgm_stats_output_path = "";

//globals for tasking
eventcount volatile *sim_start;
eventcount volatile *sim_finish;
eventcount volatile *watchdog;

int protection_faults = 0;
int fetches = 0;
int loads = 0;
int stores = 0;
int mem_system_off = 0;
int watch_dog = 0;

//stats
long long cpu_rob_stalls = 0;
long long cpu_fetch_stalls = 0;
long long cpu_load_store_stalls = 0;

void m2scgm_init(void){

	/* Initial information*/
	printf("---Welcome to M2S-CGM---\n");
	printf("---A x86 Based CPU-GPU Heterogeneous Computing Simulator---\n");
	printf("\n");
	printf("---Simulator Init---\n");

	return;
}

void cgm_init(char **argv){

	//bring the benchmark name
	benchmark_name = strdup(argv[1]);

	//set up internal structures
	cgm_access_record = list_create();

	//set up the threads
	cgm_create_tasks();

	//init interrupt support
	interrupt_init();

	//init memory system structures
	cache_init();
	switch_init();
	hub_iommu_init();
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

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "watchdog");
	watchdog = new_eventcount(strdup(buff));

	//tasks
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "cpu_gpu_run");
	create_task(cpu_gpu_run, DEFAULT_STACK_SIZE, strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "cgm_start");
	create_task(cgm_mem_run, DEFAULT_STACK_SIZE, strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "watchdog");
	create_task(cgm_watchdog, DEFAULT_STACK_SIZE, strdup(buff));

	//create the task for future advance.
	//this is specific to future_advance()
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "Wakeupcall");
	wakeup_task = create_task(wakeupcall, DEFAULT_STACK_SIZE, strdup(buff));
	initialize_wakeupcall(wakeup_task);

	return;
}

void cgm_watchdog(void){

	long long t_1 = 1;
	int i = 0;
	int j = 0;
	int k = 0;
	int num_cores = x86_cpu_num_cores;
	int num_sets = 0;
	int assoc = 0;

	int block_state = 0;

	int c0_tag;
	int c1_tag;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;

	int cache_block_hit;
	int cache_block_state;

	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//probe the address for set, tag, and offset.
	//cgm_cache_probe_address(&l1_d_caches[0], WATCHBLOCK, set_ptr, tag_ptr, offset_ptr);

	int size;

	while(1)
	{
		await(watchdog, t_1);
		t_1++;

		if(P_TIME >= 1454200 && P_TIME <= 1500000)
		{
			printf("%s ort %d cycle %llu\n", gpu_s_caches[0].name, list_count(gpu_s_caches[0].ort_list), P_TIME);
			printf("%s ort %d cycle %llu\n", gpu_s_caches[1].name, list_count(gpu_s_caches[1].ort_list), P_TIME);
			printf("%s ort %d cycle %llu\n", gpu_s_caches[2].name, list_count(gpu_s_caches[2].ort_list), P_TIME);
			printf("%s ort %d cycle %llu\n", gpu_s_caches[3].name, list_count(gpu_s_caches[3].ort_list), P_TIME);
		}

		/*if(list_count(l1_d_caches[0].Rx_queue_top) > QueueSize)
		{
			printf("%s Rx_top %d cycle %llu\n", l1_d_caches[0].name, list_count(l1_d_caches[0].Rx_queue_top), P_TIME);
			getchar();
		}*/

/*
		if(P_TIME > 41970)
		{
			printf("WD: %s Rx_top %d Rx_bottom %d ORT %d cycle %llu\n",
					l1_d_caches[0].name, list_count(l1_d_caches[0].Rx_queue_top), list_count(l1_d_caches[0].Rx_queue_bottom), cache_get_ORT_size(&(l1_d_caches[0])), P_TIME);
			printf("WD: %s Rx_top %d Rx_bottom %d ORT %d cycle %llu\n",
					l2_caches[0].name, list_count(l2_caches[0].Rx_queue_top), list_count(l2_caches[0].Rx_queue_bottom), cache_get_ORT_size(&(l2_caches[0])), P_TIME);
			printf("WD: %s Rx_top %d Rx_bottom %d ORT %d cycle %llu\n",
					l3_caches[0].name, list_count(l3_caches[0].Rx_queue_top), list_count(l3_caches[0].Rx_queue_bottom), cache_get_ORT_size(&(l3_caches[0])), P_TIME);

			printf("WD: %s tx_n %d tx_e %d tx_s %d tx_w %d cycle %llu\n",
					switches[0].name, list_count(switches[0].Tx_north_queue), list_count(list_count(switches[0].), cache_get_ORT_size(&(l3_caches[0])), P_TIME);

		}*/

		/*if(P_TIME > 53000)
		{
			STOP;
		}*/



		/*for(i = 0; i < num_cores; i++)
		{
			if(list_count(l1_d_caches[i].write_back_buffer) > QueueSize)
			{
				size = list_count(l1_d_caches[i].write_back_buffer);
				printf("WD: %s WB queue size %d cycle %llu\n", l1_d_caches[i].name, size, P_TIME);
				//getchar();
			}
		}

		for(i = 0; i < num_cores; i++)
		{
			if(list_count(l2_caches[i].write_back_buffer) > QueueSize)
			{
				size = list_count(l2_caches[i].write_back_buffer);
				printf("WD: %s WB queue size %d cycle %llu\n", l2_caches[i].name, size, P_TIME);
				//getchar();
			}
		}

		for(i = 0; i < num_cores; i++)
		{
			if(list_count(l3_caches[i].write_back_buffer) > QueueSize)
			{
				size = list_count(l3_caches[i].write_back_buffer);
				printf("WD: %s WB queue size %d cycle %llu\n", l3_caches[i].name, size, P_TIME);
				//getchar();
			}
		}*/


	}
	return;
}

void cgm_dump_stats(void){

	int num_cores = x86_cpu_num_cores;
	int num_threads = x86_cpu_num_threads;
	int num_cus = si_gpu_num_compute_units;



	CGM_STATS(cgm_stats_file, "[General]\n");
	CGM_STATS(cgm_stats_file, "Benchmark = %s\n", benchmark_name);
	CGM_STATS(cgm_stats_file, "\n");
	CGM_STATS(cgm_stats_file, "[CPU]\n");
	CGM_STATS(cgm_stats_file, "NumCores = %d\n", num_cores);
	CGM_STATS(cgm_stats_file, "ThreadsPerCore = %d\n", num_threads);
	CGM_STATS(cgm_stats_file, "TotalCycles = %lld\n", P_TIME);
	CGM_STATS(cgm_stats_file, "ROBStalls = %llu\n", cpu_rob_stalls);
	CGM_STATS(cgm_stats_file, "FetchStalls = %llu\n", cpu_fetch_stalls);
	CGM_STATS(cgm_stats_file, "LoadStoreStalls = %llu\n", cpu_load_store_stalls);
	CGM_STATS(cgm_stats_file, "\n");
	CGM_STATS(cgm_stats_file, "[GPU]\n");
	CGM_STATS(cgm_stats_file, "NumComputeUnits = %d\n", num_cus);
	/*CGM_STATS(cgm_stats_file, "ROBStalls = %llu\n",;*/
	CGM_STATS(cgm_stats_file, "\n");

	return;
}


void cgm_dump_summary(void){

	printf("\n---Printing Stats---\n");

	cgm_dump_stats();
	cache_dump_stats();

	CLOSE_FILES;

	return;
}

void cgm_mem_run(void){

	advance(sim_start);

	//simulation execution

	await(sim_finish, 1);
	//dump stats on exit.

	return;
}

void cpu_gpu_run(void){

	long long t_1 = 1;

	while(1)
	{

		await(sim_start, t_1);
		t_1++;

		m2s_loop();

		/*star todo there is a bug here
		sim_finsih has to be advanced to 1 + the last cycle
		but you don't know out of all the final threads
		which one will run the longest until its done.
		if you play with the delay number you will eventually find
		the correct delay and the simulation will finish correctly.*/
		future_advance(sim_finish, (etime.count + 2));
		//advance(sim_finish);
	}

	return;
}

/*void cgm_interrupt(X86Thread *self, struct x86_uop_t *uop){

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
}*/


int cgm_can_fetch_access(X86Thread *self, unsigned int addr){

	X86Thread *thread;
	thread = self;

	//check if request queue is full
	if(QueueSize <= list_count(thread->i_cache_ptr[thread->core->id].Rx_queue_top))
	{
		cpu_fetch_stalls++;
		return 0;
	}

	//cache queue is accessible.
	return 1;
}

int cgm_can_issue_access(X86Thread *self, unsigned int addr){

	X86Thread *thread;
	thread = self;

	//check if request queue is full
	if(QueueSize <= list_count(thread->d_cache_ptr[thread->core->id].Rx_queue_top))
	{
		cpu_load_store_stalls++;

		return 0;
	}
	//cache queue is accessible.
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
	new_packet->name = strdup(buff);
	new_packet->cache_block_state = cgm_cache_block_null;

	new_packet->start_cycle = P_TIME;
	new_packet->cpu_access_type = cgm_access_fetch;


/*	#include <mem-image/memory.h>
	printf("%d\n", MEM_PAGE_SHIFT);
	printf("%d\n", MEM_PAGE_MASK);
	STOP;*/

	//this can remove the memory system for testing purposes
	//if(mem_system_off == 1 || mem_system_off == 3)

	//////////////testing
	/*if(new_packet->cpu_access_type == cgm_access_fetch)
	{
		//put back on the core event queue to end memory system access.
		list_dequeue(cgm_access_record);
		status_packet_destroy(new_packet_status);
		packet_destroy(new_packet);
		return access_id;
	}*/
	//////////////testing

	//bad access address kill fetch
	/*if(addr == 0)
	{
		protection_faults++;
		list_dequeue(cgm_access_record);
		status_packet_destroy(new_packet_status);
		packet_destroy(new_packet);
		printf("bad fetch\n");
		return access_id;
	}*/


	//Add (2) to the target L1 I Cache Rx Queue
	if(access_kind == cgm_access_fetch)
	{
		//get the core ID number should be <= number of cores
		id = thread->core->id;
		assert(id < num_cores);

		//Drop the packet into the L1 I Cache Rx queue
		list_enqueue(thread->i_cache_ptr[id].Rx_queue_top, new_packet);

		advance(&l1_i_cache[id]);
	}
	else
	{
		fatal("cgm_fetch_access() unsupported access type\n");
	}

	return access_id;
}


void cgm_issue_lspq_access(X86Thread *self, enum cgm_access_kind_t access_kind, long long uop_id, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item){

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
	//new_packet->in_flight = 1;
	new_packet->access_id = access_id;
	new_packet->name = strdup(buff);

	new_packet->start_cycle = P_TIME;
	new_packet->cpu_access_type = access_kind;


	//////////////testing
	/*if(mem_system_off == 2 || mem_system_off == 3)*/
	/*if(new_packet->cpu_access_type == cgm_access_store)
	{
		//put back on the core event queue to end memory system access.
		linked_list_add(event_queue, event_queue_item);
		packet_destroy(new_packet);
		return;
	}
	if(new_packet->cpu_access_type == cgm_access_load)
	{
		//put back on the core event queue to end memory system access.
		linked_list_add(event_queue, event_queue_item);
		packet_destroy(new_packet);
		return;
	}*/
	//////////////testing


	/*if(addr == 0)
	{
		protection_faults++;

		linked_list_add(event_queue, event_queue_item);
		packet_destroy(new_packet);
		//printf("load protection fault access uop_id %llu type %d (2 = load, 3 = store)\n", uop_id, access_kind);
		return;
	}*/

	//For memory system load store request
	if(access_kind == cgm_access_load || access_kind == cgm_access_store)
	{
		//get the core ID number should be <= number of cores
		id = thread->core->id;
		assert(id < num_cores);

		if(((addr & l1_d_caches[0].block_address_mask) == WATCHBLOCK) && WATCHLINE)
		{
			printf("block 0x%08x %s id %llu type %d start cycle %llu\n",
					(addr & l1_d_caches[0].block_address_mask), thread->d_cache_ptr[id].name, new_packet->access_id, new_packet->cpu_access_type, P_TIME);
		}

		//printf("%s id %llu type %d start cycle %llu\n", l1_d_caches[id].name, new_packet->access_id, new_packet->access_type, P_TIME);

		/*if(new_packet->access_id == 106789)
		{

			unsigned int temp = new_packet->address;
			temp = temp & l1_d_caches[id].block_address_mask;

			fatal("address 0x%08x blk_addr 0x%08x\n", new_packet->address, temp);
		}*/

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
			status_packet_destroy(packet);

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
	//new_packet->in_flight = 1;
	new_packet->access_id = access_id;
	new_packet->name = strdup(buff);
	new_packet->gpu_access_type = new_packet->access_type;

	//leave for debugging purposes
	if(mem_system_off == 3)
	{
		(*witness_ptr)++;
		free(new_packet);
		return;
	}

	//Add to the target L1 Cache Rx Queue
	if(access_kind == cgm_access_load_v || access_kind == cgm_access_store_v || access_kind == cgm_access_nc_store)
	{
		//get the core ID number should be <= number of cores
		id = vector_mem_ptr->compute_unit->id;
		assert( id < num_cus);

		unsigned int temp = addr;
		temp = temp & gpu_v_caches[id].block_address_mask;

		//printf("%s id %llu type %d address 0x%08x blk_addr 0x%08x start cycle %llu\n", gpu_v_caches[id].name, new_packet->access_id, new_packet->access_type, addr, temp, P_TIME);

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
	//new_packet->in_flight = 1;
	new_packet->access_id = access_id;
	new_packet->name = strdup(buff);

	new_packet->start_cycle = P_TIME;
	new_packet->gpu_access_type = new_packet->access_type;

	//leave for debugging purposes
	if(mem_system_off == 3)
	{
		(*witness_ptr)++;
		free(new_packet);
		return;
	}

	//Add to the target cache Rx queue
	if(access_kind == cgm_access_load_s)
	{
		//get the core ID number should be <= number of cores
		id = scalar_unit_ptr->compute_unit->id;
		assert(id < num_cus);

		unsigned int temp = addr;
		temp = temp & gpu_s_caches[id].block_address_mask;

		//printf("%s id %llu type %d address 0x%08x blk_addr 0x%08x start cycle %llu\n", gpu_s_caches[id].name, new_packet->access_id, new_packet->access_type, addr, temp, P_TIME);

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

#include <stdio.h>

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
	//new_packet->in_flight = 1;
	new_packet->access_id = access_id;
	new_packet->name = strdup(buff);

	//leave for debugging purposes
	if(mem_system_off == 3)
	{
		(*witness_ptr)++;
		free(new_packet);
	}

	//Add to the LDS queue
	if(access_kind == cgm_access_load || access_kind == cgm_access_store)
	{
		//get the core ID number should be <= number of cores
		id = lds_ptr->compute_unit->id;
		assert( id < num_cus);

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

void PrintCycle(int skip){

	if((P_TIME % skip) == 0)
	{
		printf("---Cycles %llu---\n", P_TIME);
		fflush(stdout);
	}
	return;
}

