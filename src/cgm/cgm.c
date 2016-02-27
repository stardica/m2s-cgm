/*
 * queue.c
 *
 *  Created on: Nov 23, 2014
 *      Author: stardica
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libgen.h>

#include <cgm/cgm.h>
/*#include <cgm/dram.h>*/



long long access_id = 0;
//long long lspq_access_id = 0;
struct list_t *cgm_access_record;
char *cgm_config_file_name_and_path = "";

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
char *cgm_stats_file_name = "";

//globals for tasking
eventcount volatile *sim_start;
eventcount volatile *sim_finish;
eventcount volatile *watchdog;

int mem_system_off = 0;
int watch_dog = 0;
int run_watch_dog = 0;
int wd_current_set = 0;
int wd_current_tag = 0;

long long last_issued_lsq_access_id = 0;
unsigned int last_issued_lsq_access_blk = 0x0;
long long last_committed_lsq_access_id = 0;
unsigned int last_committed_lsq_access_blk = 0x0;

long long last_issued_fetch_access_id = 0;
unsigned int last_issued_fetch_access_blk = 0x0;
long long last_committed_fetch_access_id = 0;
unsigned int last_committed_fetch_access_blk = 0x0;

struct cgm_stats_t *cgm_stat;

void m2scgm_init(void){

	/* Initial information*/
	printf("---Welcome to M2S-CGM---\n");
	printf("---A x86 Based CPU-GPU Heterogeneous Computing Simulator---\n");
	printf("\n");
	printf("---Simulator Init---\n");

	return;
}

void cgm_check_config_files(char **argv){

	if(!strcmp(x86_config_file_name, ""))
	{
		fatal("cgm_init(): x86 config file not specified\n");
	}
	else if(!strcmp(si_gpu_config_file_name, ""))
	{
		fatal("cgm_init(): si config file not specified\n");
	}
	else if(!strcmp(cgm_config_file_name_and_path, ""))
	{
		fatal("cgm_init(): mem config file not specified\n");
	}

	return;
}

void cgm_init(int argc, char **argv){

	char time_buff[250];
	memset(time_buff, '\0', 250);
	char buff[400];
	memset(buff, '\0', 400);
	int i = 1;
	char *bname;

	cgm_stat = (void *) calloc(1, sizeof(struct cgm_stats_t));

	/* Obtain current time. */
	time_t current_time;
    struct tm *time_info;// = time(NULL);
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_buff, sizeof(time_buff), "%m%d%Y%H%M%S", time_info);

    cgm_stat->date_time_file = strdup(time_buff);

    memset(time_buff, '\0', 250);
    strftime(time_buff, sizeof(time_buff), "%a %b %Y %H:%M:%S", time_info);

    cgm_stat->date_time_pretty = strdup(time_buff);
	cgm_stat->start_wall_time = get_wall_time();

	//get the benchmark name and args
	bname = basename(argv[i++]);
	sprintf(buff + strlen(buff), "%s ", bname);

	while(i < argc)
		sprintf(buff + strlen(buff), "%s ", argv[i++]);

	cgm_stat->benchmark_name = strdup(buff);

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
	dramsim_init();

	return;
}

void cgm_configure(struct mem_t *mem){

	int error = 0;

	error = cgm_mem_configure(mem);
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

void tick(void){

	/*advance tasks on each cycle here*/

	if(DRAMSim == 1)
	{
		advance(dramsim);
	}

	if(watch_dog == 1)
	{
		advance(watchdog);
	}

	return;
}

void cgm_watchdog(void){

	long long t_1 = 1;

	while(1)
	{
		await(watchdog, t_1);
		t_1++;

		if(run_watch_dog == 1)
		{
			cache_dump_request_queue(switches[2].south_queue);
		}
		/*printf("\tWD: ort_queue_size %d cycle %llu\n", list_count(l1_d_caches[0].ort_list), P_TIME);*/
	}
	return;
}

void cgm_dump_stats(void){

	int num_cores = x86_cpu_num_cores;
	int num_threads = x86_cpu_num_threads;
	int num_cus = si_gpu_num_compute_units;



	//get the time
	cgm_stat->end_wall_time = get_wall_time();

	//calculate simulation runtime (wall clock)
	cgm_stat->sim_time = (cgm_stat->end_wall_time - cgm_stat->start_wall_time);

	unsigned int cpu_freq_hz = (unsigned int) x86_cpu_frequency * (unsigned int) MHZ;
	double cpu_sim_time = (double) P_TIME / (double) (cpu_freq_hz);

	/* General statistics */
	CGM_STATS(cgm_stats_file, "[General]\n");
	CGM_STATS(cgm_stats_file, "Benchmark = %s\n", cgm_stat->benchmark_name);
	CGM_STATS(cgm_stats_file, "Day&Time = %s\n", cgm_stat->date_time_pretty);
	CGM_STATS(cgm_stats_file, "TotalCycles = %lld\n", P_TIME);
	CGM_STATS(cgm_stats_file, "SimulationRunTimeSeconds(cpu) = %.9f\n", cpu_sim_time);
	CGM_STATS(cgm_stats_file, "SimulationRunTimeSeconds(wall) = %.2f\n", cgm_stat->sim_time);
	CGM_STATS(cgm_stats_file, "SimulatedCyclesPerSec = %.2f\n", (double)P_TIME/cgm_stat->sim_time);
	CGM_STATS(cgm_stats_file, "\n");
	CGM_STATS(cgm_stats_file, "[CPU]\n");
	CGM_STATS(cgm_stats_file, "NumCores = %d\n", num_cores);
	CGM_STATS(cgm_stats_file, "ThreadsPerCore = %d\n", num_threads);
	CGM_STATS(cgm_stats_file, "ROBStalls = %llu\n", cgm_stat->cpu_rob_stalls);
	CGM_STATS(cgm_stats_file, "FetchStalls = %llu\n", cgm_stat->cpu_fetch_stalls);
	CGM_STATS(cgm_stats_file, "LoadStoreStalls = %llu\n", cgm_stat->cpu_ls_stalls);
	CGM_STATS(cgm_stats_file, "\n");
	CGM_STATS(cgm_stats_file, "[GPU]\n");
	CGM_STATS(cgm_stats_file, "NumComputeUnits = %d\n", num_cus);
	CGM_STATS(cgm_stats_file, "\n");
	CGM_STATS(cgm_stats_file, "[MemSystem]\n");
	CGM_STATS(cgm_stats_file, "FirstAccessLat(Fetch) = %d\n", cgm_stat->first_mem_access_lat);
	CGM_STATS(cgm_stats_file, "TotalFetches = %llu\n", cgm_stat->cpu_total_fetches);
	CGM_STATS(cgm_stats_file, "FetchesL1 = %llu\n", cgm_stat->fetch_l1_hits);
	CGM_STATS(cgm_stats_file, "FetchesL2 = %llu\n", cgm_stat->fetch_l2_hits);
	CGM_STATS(cgm_stats_file, "FetchesL3 = %llu\n", cgm_stat->fetch_l3_hits);
	CGM_STATS(cgm_stats_file, "FetchesMemory = %llu\n", cgm_stat->fetch_memory);
	CGM_STATS(cgm_stats_file, "TotalLoads = %llu\n", cgm_stat->cpu_total_loads);
	CGM_STATS(cgm_stats_file, "LoadsL1 = %llu\n", cgm_stat->load_l1_hits);
	CGM_STATS(cgm_stats_file, "LoadsL2 = %llu\n", cgm_stat->load_l2_hits);
	CGM_STATS(cgm_stats_file, "LoadsL3 = %llu\n", cgm_stat->load_l3_hits);
	CGM_STATS(cgm_stats_file, "LoadsMemory = %llu\n", cgm_stat->load_memory);
	CGM_STATS(cgm_stats_file, "LoadsGetFwd = %llu\n", cgm_stat->load_get_fwd);
	CGM_STATS(cgm_stats_file, "TotalStore = %llu\n", cgm_stat->cpu_total_stores);
	CGM_STATS(cgm_stats_file, "StoresL1 = %llu\n", cgm_stat->store_l1_hits);
	CGM_STATS(cgm_stats_file, "StoresL2 = %llu\n", cgm_stat->store_l2_hits);
	CGM_STATS(cgm_stats_file, "StoresL3 = %llu\n", cgm_stat->store_l3_hits);
	CGM_STATS(cgm_stats_file, "StoresMemory = %llu\n", cgm_stat->store_memory);
	CGM_STATS(cgm_stats_file, "StoresGetxFwd = %llu\n", cgm_stat->store_getx_fwd);
	CGM_STATS(cgm_stats_file, "StoresUpgrade = %llu\n", cgm_stat->store_upgrade);
	CGM_STATS(cgm_stats_file, "\n");

	return;
}

void cgm_dump_histograms(void){

	int i = 0;

	FILE *fetch_lat_log_file = fopen ("/home/stardica/Desktop/m2s-cgm/src/scripts/fetch_lat_log_file.out", "w+");
	FILE *load_lat_log_file = fopen ("/home/stardica/Desktop/m2s-cgm/src/scripts/load_lat_log_file.out", "w+");
	FILE *store_lat_log_file = fopen ("/home/stardica/Desktop/m2s-cgm/src/scripts/store_lat_log_file.out", "w+");

	/* Histograms */
	//fprintf(fetch_lat_log_file, "[FetchLatHist]\n");
	for(i = 0; i < HISTSIZE; i++)
	{
		if(cgm_stat->fetch_lat_hist[i] > 0)
			fprintf(fetch_lat_log_file, "%d %llu\n", i, cgm_stat->fetch_lat_hist[i]);
	}

	for(i = 0; i < HISTSIZE; i++)
	{
		if(cgm_stat->load_lat_hist[i] > 0)
			fprintf(load_lat_log_file, "%d %llu\n", i, cgm_stat->load_lat_hist[i]);
	}

	for(i = 0; i < HISTSIZE; i++)
	{
		if(cgm_stat->store_lat_hist[i] > 0)
			fprintf(store_lat_log_file, "%d %llu\n", i, cgm_stat->store_lat_hist[i]);
	}


	/*OLD CODE
	for(i = 0; i < HISTSIZE; i++)
	{
		while(cgm_stat->fetch_lat_hist[i] > 0)
		{
			fprintf(fetch_lat_log_file, "%d\n", i);
			cgm_stat->fetch_lat_hist[i]--;
		}
	}

	for(i = 0; i < HISTSIZE; i++)
	{
		while(cgm_stat->load_lat_hist[i] > 0)
		{
			fprintf(load_lat_log_file, "%d\n", i);
			cgm_stat->load_lat_hist[i]--;
		}
	}

	for(i = 0; i < HISTSIZE; i++)
	{
		while(cgm_stat->store_lat_hist[i] > 0)
		{
			fprintf(store_lat_log_file, "%d\n", i);
			cgm_stat->store_lat_hist[i]--;
		}
	}*/



	fclose (load_lat_log_file);
	fclose (fetch_lat_log_file);
	fclose (store_lat_log_file);

	return;
}


void cgm_dump_summary(void){

	printf("\n---Printing Stats to file %s---\n", cgm_stat->stat_file_name);


	cgm_dump_stats();
	cache_dump_stats();
	switch_dump_stats();
	sys_agent_dump_stats();
	memctrl_dump_stats();
	cgm_dump_histograms();

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

	//this can remove the memory system for testing purposes
	if(mem_system_off == 1)
	{
		list_dequeue(cgm_access_record);
		status_packet_destroy(new_packet_status);
		packet_destroy(new_packet);
		return access_id;
	}

	//get the core ID number should be <= number of cores
	id = thread->core->id;
	assert(id < num_cores);

	/*stats*/
	cgm_stat->cpu_total_fetches++;
	l1_i_caches[id].TotalAcesses++;

	last_issued_fetch_access_id = access_id;
	last_issued_fetch_access_blk = addr & thread->i_cache_ptr[id].block_address_mask;

	//Add (2) to the target L1 I Cache Rx Queue
	if(access_kind == cgm_access_fetch)
	{
		if((((addr & l1_i_caches[0].block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
		{
			if(LEVEL == 1 || LEVEL == 3)
			{
				printf("block 0x%08x %s id %llu type %d start cycle %llu\n",
						(addr & l1_i_caches[0].block_address_mask), thread->i_cache_ptr[id].name, new_packet->access_id, new_packet->cpu_access_type, P_TIME);
			}
		}

		//Drop the packet into the L1 I Cache Rx queue
		list_enqueue(thread->i_cache_ptr[id].Rx_queue_top, new_packet);

		advance(&l1_i_cache[id]);

		l1_i_caches[id].TotalReads++;
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
	if(mem_system_off == 1)
	{
		//put back on the core event queue to end memory system access.
		linked_list_add(event_queue, event_queue_item);
		packet_destroy(new_packet);
		return;
	}

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

	//get the core ID number should be <= number of cores
	id = thread->core->id;
	assert(id < num_cores);

	/*stats*/
	l1_d_caches[id].TotalAcesses++;
	last_issued_lsq_access_id = access_id;
	last_issued_lsq_access_blk = addr & thread->d_cache_ptr[id].block_address_mask;

	/*printf("\t lsq issuing access_id %llu\n", access_id);*/

	if(access_kind == cgm_access_load)
	{
		cgm_stat->cpu_total_loads++;
	}
	else if(access_kind == cgm_access_store)
	{
	 	cgm_stat->cpu_total_stores++;
	}

	//For memory system load store request
	if(access_kind == cgm_access_load || access_kind == cgm_access_store)
	{

		if((((addr & l1_d_caches[0].block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
		{
			if(LEVEL == 1 || LEVEL == 3)
			{
				printf("block 0x%08x %s id %llu type %d start cycle %llu\n",
						(addr & l1_d_caches[0].block_address_mask), thread->d_cache_ptr[id].name, new_packet->access_id, new_packet->cpu_access_type, P_TIME);
			}
		}

		//Drop the packet into the L1 D Cache Rx queue
		list_enqueue(thread->d_cache_ptr[id].Rx_queue_top, new_packet);

		//advance the L1 D Cache Ctrl task
		advance(&l1_d_cache[id]);

		/*stats*/
		if(access_kind == cgm_access_load)
		{
			l1_d_caches[id].TotalReads++;
		}
		else if(access_kind == cgm_access_store)
		{
			l1_d_caches[id].TotalWrites++;
		}
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
	if(mem_system_off == 1)
	{
		(*witness_ptr)++;
		packet_destroy(new_packet);
		return;
	}

	/*printf("access address 0x%08x\n", addr);*/

	/*(*witness_ptr)++;
	packet_destroy(new_packet);
	return;*/

	//Add to the target L1 Cache Rx Queue
	if(access_kind == cgm_access_load_v || access_kind == cgm_access_store_v || access_kind == cgm_access_nc_store)
	{
		//get the core ID number should be <= number of cores
		id = vector_mem_ptr->compute_unit->id;
		assert( id < num_cus);

		unsigned int temp = addr;
		temp = temp & gpu_v_caches[id].block_address_mask;

		//Drop the packet into the GPU LDS unit Rx queue
		list_enqueue(vector_mem_ptr->compute_unit->gpu_v_cache_ptr[id].Rx_queue_top, new_packet);

		//advance the L1 I Cache Ctrl task
		advance(&gpu_v_cache[id]);

		gpu_v_caches[id].TotalAcesses++;
	}
	else
	{
		fatal("cgm_vector_access() unsupported access type\n");
	}

	/*stats*/
	if(access_kind == cgm_access_load_v)
	{
		cgm_stat->gpu_total_loads++;
	}

	if(access_kind == cgm_access_store_v || access_kind == cgm_access_nc_store)
	{
	 	cgm_stat->gpu_total_stores++;
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
	if(mem_system_off == 1)
	{
		(*witness_ptr)++;
		packet_destroy(new_packet);
		return;
	}

	/*(*witness_ptr)++;
	packet_destroy(new_packet);
	return;*/

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

		gpu_s_caches[id].TotalAcesses++;
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

void PrintCycle(void){

	if((P_TIME % SKIP) == 0)
	{
		printf("---Cycles %lluM---\n", (P_TIME)/1000000);
		fflush(stdout);
	}

	return;
}


double get_wall_time(void){
	struct timeval time;
	gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}


double get_cpu_time(void){
	return (double)clock() / CLOCKS_PER_SEC;
}


long long cgm_get_time(void)
{
	struct timeval tv;
	long long value;

	gettimeofday(&tv, NULL);
	value = (long long) tv.tv_sec * 1000000 + tv.tv_usec;
	return value;
}

void cgm_dump_system(void){

	int i = 0;
	int num_cores = x86_cpu_num_cores;

	printf("---Deadlock Detected Dumping System Status---\n"
			"---last real cycle %llu---\n"
			"---Last committed memory lsq access %llu last committed lsq blk address 0x%08x---\n"
			"---Last issued lsq memory access %llu last issued lsq blk address 0x%08x---\n"
			"---Last committed memory fetch access %llu last committed fetch blk address 0x%08x---\n"
			"---Last issued fetch memory access %llu last issued fetch blk address 0x%08x---\n",
			(P_TIME - 1000000),
			last_committed_lsq_access_id, last_committed_lsq_access_blk, last_issued_lsq_access_id, last_issued_lsq_access_blk,
			last_committed_fetch_access_id, last_committed_fetch_access_blk, last_issued_fetch_access_id, last_issued_fetch_access_blk);

	printf("\n---L1_d_caches---\n");
	for(i = 0; i < num_cores; i++)
	{
		printf("---%s Rx top queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].Rx_queue_top));
		cache_dump_request_queue(l1_d_caches[i].Rx_queue_top);
		printf("---%s Rx bottom queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].Rx_queue_bottom));
		printf("---%s Tx bottom queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].Tx_queue_bottom));
		printf("---%s Coherence queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].Coherance_Rx_queue));
		printf("---%s Pending request queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].pending_request_buffer));
		printf("---%s Write back queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].write_back_buffer));
		printf("---%s ORT size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].ort_list));
		ort_dump(&l1_d_caches[i]);
		printf("\n");
	}

	printf("---L2_caches---\n");
	for(i = 0; i < num_cores; i++)
	{
		printf("---%s Rx top queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].Rx_queue_top));
		printf("---%s Rx bottom queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].Rx_queue_bottom));
		printf("---%s Tx top queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].Tx_queue_top));
		printf("---%s Tx bottom queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].Tx_queue_bottom));
		printf("---%s Coherence queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].Coherance_Rx_queue));
		printf("---%s Pending request queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].pending_request_buffer));
		printf("---%s Write back queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].write_back_buffer));
		cache_dump_write_back(&l2_caches[i]);
		printf("---%s ORT size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].ort_list));
		ort_dump(&l2_caches[i]);
		printf("\n");
	}

	printf("---L3_caches---\n");
	for(i = 0; i < num_cores; i++)
	{
		printf("---%s Rx top queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].Rx_queue_top));
		cache_dump_request_queue(l3_caches[i].Rx_queue_top);
		printf("---%s Rx bottom queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].Rx_queue_bottom));
		cache_dump_request_queue(l3_caches[i].Rx_queue_bottom);
		printf("---%s Tx top queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].Tx_queue_top));
		printf("---%s Tx bottom queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].Tx_queue_bottom));
		printf("---%s Coherence queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].Coherance_Rx_queue));
		printf("---%s Pending request queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].pending_request_buffer));
		printf("---%s Write back queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].write_back_buffer));
		cache_dump_write_back(&l3_caches[i]);
		printf("---%s ORT size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].ort_list));
		ort_dump(&l3_caches[i]);
		printf("\n");
	}

	printf("---Switches---\n");
	for(i = 0; i < (num_cores + 1); i++)
	{
		printf("---%s North in queue size %d---\n",
				switches[i].name, list_count(switches[i].north_queue));
		printf("---%s North out queue size %d---\n",
				switches[i].name, list_count(switches[i].Tx_north_queue));
		printf("---%s East in queue size %d---\n",
				switches[i].name, list_count(switches[i].east_queue));
		printf("---%s East out queue size %d---\n",
				switches[i].name, list_count(switches[i].Tx_east_queue));
		printf("---%s South in queue size %d---\n",
				switches[i].name, list_count(switches[i].south_queue));
		printf("---%s South out queue size %d---\n",
				switches[i].name, list_count(switches[i].Tx_south_queue));
		printf("---%s West in queue size %d---\n",
				switches[i].name, list_count(switches[i].west_queue));
		printf("---%s West out queue size %d---\n",
				switches[i].name, list_count(switches[i].Tx_west_queue));
		printf("\n");
	}

	printf("---System Agent---\n");
	printf("---%s Rx top queue size %d---\n",
			system_agent->name, list_count(system_agent->Rx_queue_top));
	printf("---%s Rx bottom queue size %d---\n",
			system_agent->name, list_count(system_agent->Rx_queue_bottom));
	printf("---%s Tx top queue size %d---\n",
			system_agent->name, list_count(system_agent->Tx_queue_top));
	printf("---%s Tx bottom queue size %d---\n",
			system_agent->name, list_count(system_agent->Tx_queue_bottom));
	printf("\n");

	printf("---Memory Controller---\n");
	printf("---%s Rx top queue size %d---\n",
			mem_ctrl->name, list_count(mem_ctrl->Rx_queue_top));
	printf("---%s DRAM access buffer size %d---\n",
			mem_ctrl->name, list_count(mem_ctrl->pending_accesses));
	printf("---%s Tx top queue size %d---\n",
			mem_ctrl->name, list_count(mem_ctrl->Tx_queue_top));
	printf("\n");

	return;
}
