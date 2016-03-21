/*
 * queue.c
 *
 *  Created on: Nov 23, 2014
 *      Author: stardica
 */




#include <arch/common/arch.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <stddef.h>

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

enum stats_dump_config_t stats_dump_config;

struct cgm_stats_t *cgm_stat;
struct cgm_stats_t *cgm_startup_stats;
struct cgm_stats_t *cgm_parallel_stats;
struct cgm_stats_t *cgm_wrapup_stats;
struct cpu_gpu_stats_t *cpu_gpu_stats;

//macros required for my sanity
#define JOINLL(stat) cgm_startup_stats->stat + cgm_parallel_stats->stat + cgm_wrapup_stats->stat
#define JOINMAX(stat) MAX(cgm_wrapup_stats->stat, MAX(cgm_startup_stats->stat, cgm_parallel_stats->stat));
#define JOINMIN(stat) MIN(cgm_wrapup_stats->stat, MIN(cgm_startup_stats->stat, cgm_parallel_stats->stat));
#define JOINAVE(stat) (cgm_startup_stats->stat + cgm_parallel_stats->stat + cgm_wrapup_stats->stat)/3

//special case sigh...
#define JOINFIRSTFETCH(core, stat)	(core == 0) ? cgm_startup_stats->stat : cgm_parallel_stats->stat

void init_cgm_stats(int argc, char **argv){

	char time_buff[250];
	memset(time_buff, '\0', 250);
	char buff[400];
	memset(buff, '\0', 400);
	int i = 1;
	int j = 0;
	//int k = 0;
	char *bname;

	/* Obtain current time. */
	time_t current_time;
    struct tm *time_info;// = time(NULL);
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_buff, sizeof(time_buff), "%m_%d_%Y_%H_%M_%S", time_info);

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

	cgm_stat->execution_success = false;
	cgm_stat->benchmark_name = strdup(buff);

	cgm_stats_alloc(cgm_stat);
	cgm_stats_alloc(cgm_startup_stats);
	cgm_stats_alloc(cgm_parallel_stats);
	cgm_stats_alloc(cgm_wrapup_stats);

	//assign a type to our containers
	cgm_stat->stats_type = systemStats;
	cgm_startup_stats->stats_type = startupSection;
	cgm_parallel_stats->stats_type = parallelSection;
	cgm_wrapup_stats->stats_type = wrapupSection;


	return;
}

void cgm_stats_alloc(struct cgm_stats_t *cgm_stat_container){

	int num_cores = x86_cpu_num_cores;
	//int num_cus = si_gpu_num_compute_units;
	//int gpu_group_cache_num = (num_cus/4);

	/*configure data structures that are arrays*/

	cgm_stat_container->state = not_consolidated;

	cgm_stat_container->core_num_syscalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_syscall_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_rob_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_rob_stall_load = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_rob_stall_store = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_rob_stall_other = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_first_fetch_cycle = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_fetch_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_last_commit_cycle = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_issued_memory_insts = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_commited_memory_insts = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->l1_i_TotalThreadLoops = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalAcesses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalHits = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalReads = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalWrites = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalGets = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalGet = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalGetx = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalUpgrades = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalReadMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalWriteMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalWriteBacks = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_invalid_hits = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_assoc_conflict = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_upgrade_misses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_retries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_coalesces = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_mshr_entries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_stalls = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->l1_d_TotalThreadLoops = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalAcesses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalHits = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalReads = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalWrites = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalGets = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalGet = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalGetx = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalUpgrades = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalReadMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalWriteMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalWriteBacks = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_invalid_hits = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_assoc_conflict = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_upgrade_misses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_retries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_coalesces = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_mshr_entries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_stalls = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->l2_TotalThreadLoops = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalAcesses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalHits = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalReads = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalWrites = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalGets = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalGet = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalGetx = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalUpgrades = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalReadMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalWriteMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalWriteBacks = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_invalid_hits = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_assoc_conflict = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_upgrade_misses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_retries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_coalesces = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_mshr_entries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_stalls = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->l3_TotalThreadLoops = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalAcesses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalHits = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalReads = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalWrites = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalGets = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalGet = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalGetx = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalUpgrades = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalReadMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalWriteMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalWriteBacks = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_invalid_hits = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_assoc_conflict = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_upgrade_misses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_retries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_coalesces = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_mshr_entries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_stalls = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->switch_total_links = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_max_links = (int *)calloc(num_cores + 1, sizeof(int));
	cgm_stat_container->switch_total_wakes = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_north_io_transfers = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_north_io_transfer_cycles = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_north_io_bytes_transfered = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_east_io_transfers = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_east_io_transfer_cycles = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_east_io_bytes_transfered = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_south_io_transfers = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_south_io_transfer_cycles = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_south_io_bytes_transfered = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_west_io_transfers = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_west_io_transfer_cycles = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_west_io_bytes_transfered = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_north_txqueue_max_depth = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_north_txqueue_ave_depth = (double *)calloc(num_cores + 1, sizeof(double));
	cgm_stat_container->switch_east_txqueue_max_depth = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_east_txqueue_ave_depth = (double *)calloc(num_cores + 1, sizeof(double));
	cgm_stat_container->switch_south_txqueue_max_depth = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_south_txqueue_ave_depth = (double *)calloc(num_cores + 1, sizeof(double));
	cgm_stat_container->switch_west_txqueue_max_depth = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_west_txqueue_ave_depth = (double *)calloc(num_cores + 1, sizeof(double));
	cgm_stat_container->switch_north_tx_inserts = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_east_tx_inserts = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_south_tx_inserts = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_west_tx_inserts = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_north_rxqueue_max_depth = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_north_rxqueue_ave_depth = (double *)calloc(num_cores + 1, sizeof(double));
	cgm_stat_container->switch_east_rxqueue_max_depth = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_east_rxqueue_ave_depth = (double *)calloc(num_cores + 1, sizeof(double));
	cgm_stat_container->switch_south_rxqueue_max_depth = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_south_rxqueue_ave_depth = (double *)calloc(num_cores + 1, sizeof(double));
	cgm_stat_container->switch_west_rxqueue_max_depth = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_west_rxqueue_ave_depth = (double *)calloc(num_cores + 1, sizeof(double));
	cgm_stat_container->switch_north_rx_inserts = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_east_rx_inserts = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_south_rx_inserts = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_west_rx_inserts = (long long *)calloc(num_cores + 1, sizeof(long long));

	return;
}

void init_cpu_gpu_stats(void){

	int num_cores = x86_cpu_num_cores;

	/*configure data structures*/
	cpu_gpu_stats->core_num_syscalls = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_syscall_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_rob_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_rob_stall_load = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_rob_stall_store = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_rob_stall_other = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_first_fetch_cycle = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_fetch_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_last_commit_cycle = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_issued_memory_insts = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_commited_memory_insts = (long long *)calloc(num_cores, sizeof(long long));

	return;
}



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

void cgm_stat_finish_create(int argc, char **argv){

	init_cgm_stats(argc, argv);
	init_cpu_gpu_stats();

	return;
}

void cgm_consolidate_stats(void){

	/*take all the various stat sections and build a single stat container for the entire run*/

	int num_cores = x86_cpu_num_cores;
	/*int num_cus = si_gpu_num_compute_units;*/
	//int gpu_group_cache_num = (num_cus/4);
	int i = 0;
	//int max_links = 0;

	/*set the state of the stats container*/
	cgm_stat->state = consolidated;

	cgm_stat->start_startup_section_cycle = cgm_startup_stats->start_startup_section_cycle;
	cgm_stat->end_startup_section_cycle = cgm_startup_stats->end_startup_section_cycle;
	cgm_stat->total_startup_section_cycles = cgm_startup_stats->total_startup_section_cycles;
	cgm_stat->start_parallel_section_cycle = cgm_parallel_stats->start_parallel_section_cycle;
	cgm_stat->end_parallel_section_cycle = cgm_parallel_stats->end_parallel_section_cycle;
	cgm_stat->total_parallel_section_cycles = cgm_parallel_stats->total_parallel_section_cycles;
	cgm_stat->start_wrapup_section_cycle = cgm_wrapup_stats->start_wrapup_section_cycle;
	cgm_stat->end_wrapup_section_cycle = cgm_wrapup_stats->end_wrapup_section_cycle;
	cgm_stat->total_wrapup_section_cycles = cgm_wrapup_stats->total_wrapup_section_cycles;


	//store cgm_stat
	for(i = 0; i < num_cores; i++)
	{
		cgm_stat->core_num_syscalls[i] = JOINLL(core_num_syscalls[i]);
		cgm_stat->core_syscall_stalls[i] = JOINLL(core_syscall_stalls[i]);
		cgm_stat->core_rob_stalls[i] = JOINLL(core_rob_stalls[i]);
		cgm_stat->core_rob_stall_load[i] = JOINLL(core_rob_stall_load[i]);
		cgm_stat->core_rob_stall_store[i] = JOINLL(core_rob_stall_store[i]);
		cgm_stat->core_rob_stall_other[i] = JOINLL(core_rob_stall_other[i]);
		cgm_stat->core_first_fetch_cycle[i] = JOINFIRSTFETCH(i, core_first_fetch_cycle[i]); //special case, but luckily the first fetch is in either the startup or parallel sections.
		cgm_stat->core_fetch_stalls[i] = JOINLL(core_fetch_stalls[i]);
		cgm_stat->core_issued_memory_insts[i] = JOINLL(core_issued_memory_insts[i]);
		cgm_stat->core_commited_memory_insts[i] = JOINLL(core_commited_memory_insts[i]);
		cgm_stat->core_last_commit_cycle[i] = JOINMAX(core_last_commit_cycle[i]); // ok to take the max as it is the Last (greatest cycle).
	}

	//memory system at large
	//cgm_stat->first_mem_access_lat = mem_system_stats->first_mem_access_lat;

	for(i = 0; i < HISTSIZE; i++)
	{
		cgm_stat->fetch_lat_hist[i] = JOINLL(fetch_lat_hist[i]);
		cgm_stat->load_lat_hist[i] = JOINLL(load_lat_hist[i]);
		cgm_stat->store_lat_hist[i] = JOINLL(store_lat_hist[i]);
	}

	cgm_stat->cpu_total_fetches = JOINLL(cpu_total_fetches);
	cgm_stat->fetch_l1_hits = JOINLL(fetch_l1_hits);
	cgm_stat->fetch_l2_hits = JOINLL(fetch_l2_hits);
	cgm_stat->fetch_l3_hits = JOINLL(fetch_l3_hits);
	cgm_stat->fetch_memory = JOINLL(fetch_memory);
	cgm_stat->cpu_total_loads = JOINLL(cpu_total_loads);
	cgm_stat->load_l1_hits = JOINLL(load_l1_hits);
	cgm_stat->load_l2_hits = JOINLL(load_l2_hits);
	cgm_stat->load_l3_hits = JOINLL(load_l3_hits);
	cgm_stat->load_memory = JOINLL(load_memory);
	cgm_stat->load_get_fwd = JOINLL(load_get_fwd);
	cgm_stat->cpu_total_stores = JOINLL(cpu_total_stores);
	cgm_stat->store_l1_hits = JOINLL(store_l1_hits);
	cgm_stat->store_l2_hits = JOINLL(store_l2_hits);
	cgm_stat->store_l3_hits = JOINLL(store_l3_hits);
	cgm_stat->store_memory = JOINLL(store_memory);
	cgm_stat->store_getx_fwd = JOINLL(store_getx_fwd);
	cgm_stat->store_upgrade = JOINLL(store_upgrade);

	//caches
	for(i = 0; i < num_cores; i++)
	{
		cgm_stat->l1_i_TotalThreadLoops[i] = JOINLL(l1_i_TotalThreadLoops[i]);
		cgm_stat->l1_i_TotalAcesses[i] = JOINLL(l1_i_TotalAcesses[i]);
		cgm_stat->l1_i_TotalMisses[i] = JOINLL(l1_i_TotalMisses[i]);
		cgm_stat->l1_i_TotalHits[i] = JOINLL(l1_i_TotalHits[i]);
		cgm_stat->l1_i_TotalReads[i] = JOINLL(l1_i_TotalReads[i]);
		cgm_stat->l1_i_TotalWrites[i] = JOINLL(l1_i_TotalWrites[i]);
		cgm_stat->l1_i_TotalGets[i] = JOINLL(l1_i_TotalGets[i]);
		cgm_stat->l1_i_TotalGet[i] = JOINLL(l1_i_TotalGet[i]);
		cgm_stat->l1_i_TotalGetx[i] = JOINLL(l1_i_TotalGetx[i]);
		cgm_stat->l1_i_TotalUpgrades[i] = JOINLL(l1_i_TotalUpgrades[i]);
		cgm_stat->l1_i_TotalReadMisses[i] = JOINLL(l1_i_TotalReadMisses[i]);
		cgm_stat->l1_i_TotalWriteMisses[i] = JOINLL(l1_i_TotalWriteMisses[i]);
		cgm_stat->l1_i_TotalWriteBacks[i] = JOINLL(l1_i_TotalWriteBacks[i]);
		cgm_stat->l1_i_invalid_hits[i] = JOINLL(l1_i_invalid_hits[i]);
		cgm_stat->l1_i_assoc_conflict[i] = JOINLL(l1_i_assoc_conflict[i]);
		cgm_stat->l1_i_upgrade_misses[i] = JOINLL(l1_i_upgrade_misses[i]);
		cgm_stat->l1_i_retries[i] = JOINLL(l1_i_retries[i]);
		cgm_stat->l1_i_coalesces[i] = JOINLL(l1_i_coalesces[i]);
		cgm_stat->l1_i_mshr_entries[i] = JOINLL(l1_i_mshr_entries[i]);
		cgm_stat->l1_i_stalls[i] = JOINLL(l1_i_stalls[i]);

		cgm_stat->l1_d_TotalThreadLoops[i] = JOINLL(l1_d_TotalThreadLoops[i]);
		cgm_stat->l1_d_TotalAcesses[i] = JOINLL(l1_d_TotalAcesses[i]);
		cgm_stat->l1_d_TotalMisses[i] = JOINLL(l1_d_TotalMisses[i]);
		cgm_stat->l1_d_TotalHits[i] = JOINLL(l1_d_TotalHits[i]);
		cgm_stat->l1_d_TotalReads[i] = JOINLL(l1_d_TotalReads[i]);
		cgm_stat->l1_d_TotalWrites[i] = JOINLL(l1_d_TotalWrites[i]);
		cgm_stat->l1_d_TotalGets[i] = JOINLL(l1_d_TotalGets[i]);
		cgm_stat->l1_d_TotalGet[i] = JOINLL(l1_d_TotalGet[i]);
		cgm_stat->l1_d_TotalGetx[i] = JOINLL(l1_d_TotalGetx[i]);
		cgm_stat->l1_d_TotalUpgrades[i] = JOINLL(l1_d_TotalUpgrades[i]);
		cgm_stat->l1_d_TotalReadMisses[i] = JOINLL(l1_d_TotalReadMisses[i]);
		cgm_stat->l1_d_TotalWriteMisses[i] = JOINLL(l1_d_TotalWriteMisses[i]);
		cgm_stat->l1_d_TotalWriteBacks[i] = JOINLL(l1_d_TotalWriteBacks[i]);
		cgm_stat->l1_d_invalid_hits[i] = JOINLL(l1_d_invalid_hits[i]);
		cgm_stat->l1_d_assoc_conflict[i] = JOINLL(l1_d_assoc_conflict[i]);
		cgm_stat->l1_d_upgrade_misses[i] = JOINLL(l1_d_upgrade_misses[i]);
		cgm_stat->l1_d_retries[i] = JOINLL(l1_d_retries[i]);
		cgm_stat->l1_d_coalesces[i] = JOINLL(l1_d_coalesces[i]);
		cgm_stat->l1_d_mshr_entries[i] = JOINLL(l1_d_mshr_entries[i]);
		cgm_stat->l1_d_stalls[i] = JOINLL(l1_d_stalls[i]);

		cgm_stat->l2_TotalThreadLoops[i] = JOINLL(l2_TotalThreadLoops[i]);
		cgm_stat->l2_TotalAcesses[i] = JOINLL(l2_TotalAcesses[i]);
		cgm_stat->l2_TotalMisses[i] = JOINLL(l2_TotalMisses[i]);
		cgm_stat->l2_TotalHits[i] = JOINLL(l2_TotalHits[i]);
		cgm_stat->l2_TotalReads[i] = JOINLL(l2_TotalReads[i]);
		cgm_stat->l2_TotalWrites[i] = JOINLL(l2_TotalWrites[i]);
		cgm_stat->l2_TotalGets[i] = JOINLL(l2_TotalGets[i]);
		cgm_stat->l2_TotalGet[i] = JOINLL(l2_TotalGet[i]);
		cgm_stat->l2_TotalGetx[i] = JOINLL(l2_TotalGetx[i]);
		cgm_stat->l2_TotalUpgrades[i] = JOINLL(l2_TotalUpgrades[i]);
		cgm_stat->l2_TotalReadMisses[i] = JOINLL(l2_TotalReadMisses[i]);
		cgm_stat->l2_TotalWriteMisses[i] = JOINLL(l2_TotalWriteMisses[i]);
		cgm_stat->l2_TotalWriteBacks[i] = JOINLL(l2_TotalWriteBacks[i]);
		cgm_stat->l2_invalid_hits[i] = JOINLL(l2_invalid_hits[i]);
		cgm_stat->l2_assoc_conflict[i] = JOINLL(l2_assoc_conflict[i]);
		cgm_stat->l2_upgrade_misses[i] = JOINLL(l2_upgrade_misses[i]);
		cgm_stat->l2_retries[i] = JOINLL(l2_retries[i]);
		cgm_stat->l2_coalesces[i] = JOINLL(l2_coalesces[i]);
		cgm_stat->l2_mshr_entries[i] = JOINLL(l2_mshr_entries[i]);
		cgm_stat->l2_stalls[i] = JOINLL(l2_stalls[i]);

		cgm_stat->l3_TotalThreadLoops[i] = JOINLL(l3_TotalThreadLoops[i]);
		cgm_stat->l3_TotalAcesses[i] = JOINLL(l3_TotalAcesses[i]);
		cgm_stat->l3_TotalMisses[i] = JOINLL(l3_TotalMisses[i]);
		cgm_stat->l3_TotalHits[i] = JOINLL(l3_TotalHits[i]);
		cgm_stat->l3_TotalReads[i] = JOINLL(l3_TotalReads[i]);
		cgm_stat->l3_TotalWrites[i] = JOINLL(l3_TotalWrites[i]);
		cgm_stat->l3_TotalGets[i] = JOINLL(l3_TotalGets[i]);
		cgm_stat->l3_TotalGet[i] = JOINLL(l3_TotalGet[i]);
		cgm_stat->l3_TotalGetx[i] = JOINLL(l3_TotalGetx[i]);
		cgm_stat->l3_TotalUpgrades[i] = JOINLL(l3_TotalUpgrades[i]);
		cgm_stat->l3_TotalReadMisses[i] = JOINLL(l3_TotalReadMisses[i]);
		cgm_stat->l3_TotalWriteMisses[i] = JOINLL(l3_TotalWriteMisses[i]);
		cgm_stat->l3_TotalWriteBacks[i] = JOINLL(l3_TotalWriteBacks[i]);
		cgm_stat->l3_invalid_hits[i] = JOINLL(l3_invalid_hits[i]);
		cgm_stat->l3_assoc_conflict[i] = JOINLL(l3_assoc_conflict[i]);
		cgm_stat->l3_upgrade_misses[i] = JOINLL(l3_upgrade_misses[i]);
		cgm_stat->l3_retries[i] = JOINLL(l3_retries[i]);
		cgm_stat->l3_coalesces[i] = JOINLL(l3_coalesces[i]);
		cgm_stat->l3_mshr_entries[i] = JOINLL(l3_mshr_entries[i]);
		cgm_stat->l3_stalls[i] = JOINLL(l3_stalls[i]);
	}

	//switch stats
	for(i = 0; i < (num_cores + 1); i++)
	{
		cgm_stat->switch_total_links[i] = JOINLL(switch_total_links[i]);
		cgm_stat->switch_max_links[i] = JOINMAX(switch_max_links[i]);
		cgm_stat->switch_total_wakes[i] = JOINLL(switch_total_wakes[i]);
		cgm_stat->switch_north_io_transfers[i] = JOINLL(switch_north_io_transfers[i]);
		cgm_stat->switch_north_io_transfer_cycles[i] = JOINLL(switch_north_io_transfer_cycles[i]);
		cgm_stat->switch_north_io_bytes_transfered[i] = JOINLL(switch_north_io_bytes_transfered[i]);
		cgm_stat->switch_east_io_transfers[i] = JOINLL(switch_east_io_transfers[i]);
		cgm_stat->switch_east_io_transfer_cycles[i] = JOINLL(switch_east_io_transfer_cycles[i]);
		cgm_stat->switch_east_io_bytes_transfered[i] = JOINLL(switch_east_io_bytes_transfered[i]);
		cgm_stat->switch_south_io_transfers[i] = JOINLL(switch_south_io_transfers[i]);
		cgm_stat->switch_south_io_transfer_cycles[i] = JOINLL(switch_south_io_transfer_cycles[i]);
		cgm_stat->switch_south_io_bytes_transfered[i] = JOINLL(switch_south_io_bytes_transfered[i]);
		cgm_stat->switch_west_io_transfers[i] = JOINLL(switch_west_io_transfers[i]);
		cgm_stat->switch_west_io_transfer_cycles[i] = JOINLL(switch_west_io_transfer_cycles[i]);
		cgm_stat->switch_west_io_bytes_transfered[i] = JOINLL(switch_west_io_bytes_transfered[i]);
		cgm_stat->switch_north_txqueue_max_depth[i] = JOINMAX(switch_north_txqueue_max_depth[i]);

		cgm_stat->switch_north_txqueue_ave_depth[i] = JOINAVE(switch_north_txqueue_ave_depth[i]);
		cgm_stat->switch_east_txqueue_max_depth[i] = JOINMAX(switch_east_txqueue_max_depth[i]);
		cgm_stat->switch_east_txqueue_ave_depth[i] = JOINAVE(switch_east_txqueue_ave_depth[i]);
		cgm_stat->switch_south_txqueue_max_depth[i] = JOINMAX(switch_south_txqueue_max_depth[i]);
		cgm_stat->switch_south_txqueue_ave_depth[i] = JOINAVE(switch_south_txqueue_ave_depth[i]);
		cgm_stat->switch_west_txqueue_max_depth[i] = JOINMAX(switch_west_txqueue_max_depth[i]);
		cgm_stat->switch_west_txqueue_ave_depth[i] = JOINAVE(switch_west_txqueue_ave_depth[i]);

		cgm_stat->switch_north_tx_inserts[i] = JOINLL(switch_north_tx_inserts[i]);
		cgm_stat->switch_east_tx_inserts[i] = JOINLL(switch_east_tx_inserts[i]);
		cgm_stat->switch_south_tx_inserts[i] = JOINLL(switch_south_tx_inserts[i]);
		cgm_stat->switch_west_tx_inserts[i] = JOINLL(switch_west_tx_inserts[i]);

		cgm_stat->switch_north_rxqueue_max_depth[i] = JOINMAX(switch_north_rxqueue_max_depth[i]);
		cgm_stat->switch_north_rxqueue_ave_depth[i] = JOINAVE(switch_north_rxqueue_ave_depth[i]);
		cgm_stat->switch_east_rxqueue_max_depth[i] = JOINMAX(switch_east_rxqueue_max_depth[i]);
		cgm_stat->switch_east_rxqueue_ave_depth[i] = JOINAVE(switch_east_rxqueue_ave_depth[i]);

		cgm_stat->switch_south_rxqueue_max_depth[i] = JOINMAX(switch_south_rxqueue_max_depth[i]);
		cgm_stat->switch_south_rxqueue_ave_depth[i] = JOINAVE(switch_south_rxqueue_ave_depth[i]);
		cgm_stat->switch_west_rxqueue_max_depth[i] = JOINMAX(switch_west_rxqueue_max_depth[i]);
		cgm_stat->switch_west_rxqueue_ave_depth[i] = JOINAVE(switch_west_rxqueue_ave_depth[i]);

		cgm_stat->switch_north_rx_inserts[i] = JOINLL(switch_north_rx_inserts[i]);
		cgm_stat->switch_east_rx_inserts[i] = JOINLL(switch_east_rx_inserts[i]);
		cgm_stat->switch_south_rx_inserts[i] = JOINLL(switch_south_rx_inserts[i]);
		cgm_stat->switch_west_rx_inserts[i] = JOINLL(switch_west_rx_inserts[i]);
	}

	//system agent
	cgm_stat->system_agent_busy_cycles = JOINLL(system_agent_busy_cycles);
	cgm_stat->system_agent_north_io_busy_cycles = JOINLL(system_agent_north_io_busy_cycles);
	cgm_stat->system_agent_south_io_busy_cycles = JOINLL(system_agent_south_io_busy_cycles);
	cgm_stat->system_agent_mc_loads = JOINLL(system_agent_mc_loads);
	cgm_stat->system_agent_mc_stores = JOINLL(system_agent_mc_stores);
	cgm_stat->system_agent_mc_returns = JOINLL(system_agent_mc_returns);
	cgm_stat->system_agent_max_north_rxqueue_depth = JOINMAX(system_agent_max_north_rxqueue_depth);
	cgm_stat->system_agent_ave_north_rxqueue_depth = JOINAVE(system_agent_ave_north_rxqueue_depth);
	cgm_stat->system_agent_max_south_rxqueue_depth = JOINMAX(system_agent_max_south_rxqueue_depth);
	cgm_stat->system_agent_ave_south_rxqueue_depth = JOINAVE(system_agent_ave_south_rxqueue_depth);
	cgm_stat->system_agent_max_north_txqueue_depth = JOINMAX(system_agent_max_north_txqueue_depth);
	cgm_stat->system_agent_ave_north_txqueue_depth = JOINAVE(system_agent_ave_north_txqueue_depth);
	cgm_stat->system_agent_max_south_txqueue_depth = JOINMAX(system_agent_max_south_txqueue_depth);
	cgm_stat->system_agent_ave_south_txqueue_depth = JOINAVE(system_agent_ave_south_txqueue_depth);
	/*cgm_stat->system_agent_north_gets = system_agent->north_gets;
	cgm_stat->system_agent_south_gets = system_agent->south_gets;
	cgm_stat->system_agent_north_puts = system_agent->north_puts;
	cgm_stat->system_agent_south_puts = system_agent->south_puts;*/

	//Memory controller and DRAMSim
	cgm_stat->mem_ctrl_busy_cycles = JOINLL(mem_ctrl_busy_cycles);
	cgm_stat->mem_ctrl_num_reads = JOINLL(mem_ctrl_num_reads);
	cgm_stat->mem_ctrl_num_writes = JOINLL(mem_ctrl_num_writes);
	cgm_stat->mem_ctrl_ave_dram_read_lat = JOINAVE(mem_ctrl_ave_dram_read_lat);
	cgm_stat->mem_ctrl_ave_dram_write_lat = JOINAVE(mem_ctrl_ave_dram_write_lat);
	cgm_stat->mem_ctrl_ave_dram_total_lat = JOINAVE(mem_ctrl_ave_dram_total_lat);
	cgm_stat->mem_ctrl_read_min = JOINMIN(mem_ctrl_read_min);
	cgm_stat->mem_ctrl_read_max = JOINMAX(mem_ctrl_read_max);
	cgm_stat->mem_ctrl_write_min = JOINMIN(mem_ctrl_write_min);
	cgm_stat->mem_ctrl_write_max = JOINMAX(mem_ctrl_write_max);
	cgm_stat->mem_ctrl_dram_max_queue_depth = JOINMAX(mem_ctrl_dram_max_queue_depth);
	cgm_stat->mem_ctrl_dram_ave_queue_depth = JOINAVE(mem_ctrl_dram_ave_queue_depth);
	cgm_stat->mem_ctrl_dram_busy_cycles = JOINLL(mem_ctrl_dram_busy_cycles);
	cgm_stat->mem_ctrl_rx_max = JOINMAX(mem_ctrl_rx_max);
	cgm_stat->mem_ctrl_tx_max = JOINMAX(mem_ctrl_tx_max);
	cgm_stat->mem_ctrl_bytes_read = JOINLL(mem_ctrl_bytes_read);
	cgm_stat->mem_ctrl_bytes_wrote = JOINLL(mem_ctrl_bytes_wrote);
	cgm_stat->mem_ctrl_io_busy_cycles = JOINLL(mem_ctrl_io_busy_cycles);

	return;
}


void cgm_store_stats(struct cgm_stats_t *cgm_stat_container){

	int num_cores = x86_cpu_num_cores;
	/*int num_cus = si_gpu_num_compute_units;*/
	//int gpu_group_cache_num = (num_cus/4);
	int i = 0;

	/*set the state of the stats container*/
	cgm_stat_container->state = consolidated;

	//store cgm_stat_container
	for(i = 0; i < num_cores; i++)
	{
		cgm_stat_container->core_num_syscalls[i] = cpu_gpu_stats->core_num_syscalls[i];
		cgm_stat_container->core_syscall_stalls[i] = cpu_gpu_stats->core_syscall_stalls[i];
		cgm_stat_container->core_rob_stalls[i] = cpu_gpu_stats->core_rob_stalls[i];
		cgm_stat_container->core_rob_stall_load[i] = cpu_gpu_stats->core_rob_stall_load[i];
		cgm_stat_container->core_rob_stall_store[i] = cpu_gpu_stats->core_rob_stall_store[i];
		cgm_stat_container->core_rob_stall_other[i] = cpu_gpu_stats->core_rob_stall_other[i];
		cgm_stat_container->core_first_fetch_cycle[i] = cpu_gpu_stats->core_first_fetch_cycle[i];
		cgm_stat_container->core_fetch_stalls[i] = cpu_gpu_stats->core_fetch_stalls[i];
		cgm_stat_container->core_last_commit_cycle[i] = cpu_gpu_stats->core_last_commit_cycle[i];
		cgm_stat_container->core_issued_memory_insts[i] = cpu_gpu_stats->core_issued_memory_insts[i];
		cgm_stat_container->core_commited_memory_insts[i] = cpu_gpu_stats->core_commited_memory_insts[i];
	}

	//memory system at large
	cgm_stat_container->first_mem_access_lat = mem_system_stats->first_mem_access_lat;

	for(i = 0; i < HISTSIZE; i++)
	{
		cgm_stat_container->fetch_lat_hist[i] = mem_system_stats->fetch_lat_hist[i];
		cgm_stat_container->load_lat_hist[i] = mem_system_stats->load_lat_hist[i];
		cgm_stat_container->store_lat_hist[i] = mem_system_stats->store_lat_hist[i];
	}

	cgm_stat_container->cpu_total_fetches = mem_system_stats->cpu_total_fetches;
	cgm_stat_container->fetch_l1_hits = mem_system_stats->fetch_l1_hits;
	cgm_stat_container->fetch_l2_hits = mem_system_stats->fetch_l2_hits;
	cgm_stat_container->fetch_l3_hits = mem_system_stats->fetch_l3_hits;
	cgm_stat_container->fetch_memory = mem_system_stats->fetch_memory;
	cgm_stat_container->cpu_total_loads = mem_system_stats->cpu_total_loads;
	cgm_stat_container->load_l1_hits = mem_system_stats->load_l1_hits;
	cgm_stat_container->load_l2_hits = mem_system_stats->load_l2_hits;
	cgm_stat_container->load_l3_hits = mem_system_stats->load_l3_hits;
	cgm_stat_container->load_memory = mem_system_stats->load_memory;
	cgm_stat_container->load_get_fwd = mem_system_stats->load_get_fwd;
	cgm_stat_container->cpu_total_stores = mem_system_stats->cpu_total_stores;
	cgm_stat_container->store_l1_hits = mem_system_stats->store_l1_hits;
	cgm_stat_container->store_l2_hits = mem_system_stats->store_l2_hits;
	cgm_stat_container->store_l3_hits = mem_system_stats->store_l3_hits;
	cgm_stat_container->store_memory = mem_system_stats->store_memory;
	cgm_stat_container->store_getx_fwd = mem_system_stats->store_getx_fwd;
	cgm_stat_container->store_upgrade = mem_system_stats->store_upgrade;

	//caches
	for(i = 0; i < num_cores; i++)
	{
		cgm_stat_container->l1_i_TotalThreadLoops[i] = l1_i_caches[i].TotalThreadLoops;
		cgm_stat_container->l1_i_TotalAcesses[i] = l1_i_caches[i].TotalAcesses;
		cgm_stat_container->l1_i_TotalMisses[i] = l1_i_caches[i].TotalMisses;
		cgm_stat_container->l1_i_TotalHits[i] = l1_i_caches[i].TotalHits;
		cgm_stat_container->l1_i_TotalReads[i] = l1_i_caches[i].TotalReads;
		cgm_stat_container->l1_i_TotalWrites[i] = l1_i_caches[i].TotalWrites;
		cgm_stat_container->l1_i_TotalGets[i] = l1_i_caches[i].TotalGets;
		cgm_stat_container->l1_i_TotalGet[i] = l1_i_caches[i].TotalGet;
		cgm_stat_container->l1_i_TotalGetx[i] = l1_i_caches[i].TotalGetx;
		cgm_stat_container->l1_i_TotalUpgrades[i] = l1_i_caches[i].TotalUpgrades;
		cgm_stat_container->l1_i_TotalReadMisses[i] = l1_i_caches[i].TotalReadMisses;
		cgm_stat_container->l1_i_TotalWriteMisses[i] = l1_i_caches[i].TotalWriteMisses;
		cgm_stat_container->l1_i_TotalWriteBacks[i] = l1_i_caches[i].TotalWriteBacks;
		cgm_stat_container->l1_i_invalid_hits[i] = l1_i_caches[i].invalid_hits;
		cgm_stat_container->l1_i_assoc_conflict[i] = l1_i_caches[i].assoc_conflict;
		cgm_stat_container->l1_i_upgrade_misses[i] = l1_i_caches[i].upgrade_misses;
		cgm_stat_container->l1_i_retries[i] = l1_i_caches[i].retries;
		cgm_stat_container->l1_i_coalesces[i] = l1_i_caches[i].coalesces;
		cgm_stat_container->l1_i_mshr_entries[i] = l1_i_caches[i].mshr_entries;
		cgm_stat_container->l1_i_stalls[i] = l1_i_caches[i].stalls;

		cgm_stat_container->l1_d_TotalThreadLoops[i] = l1_d_caches[i].TotalThreadLoops;
		cgm_stat_container->l1_d_TotalAcesses[i] = l1_d_caches[i].TotalAcesses;
		cgm_stat_container->l1_d_TotalMisses[i] = l1_d_caches[i].TotalMisses;
		cgm_stat_container->l1_d_TotalHits[i] = l1_d_caches[i].TotalHits;
		cgm_stat_container->l1_d_TotalReads[i] = l1_d_caches[i].TotalReads;
		cgm_stat_container->l1_d_TotalWrites[i] = l1_d_caches[i].TotalWrites;
		cgm_stat_container->l1_d_TotalGets[i] = l1_d_caches[i].TotalGets;
		cgm_stat_container->l1_d_TotalGet[i] = l1_d_caches[i].TotalGet;
		cgm_stat_container->l1_d_TotalGetx[i] = l1_d_caches[i].TotalGetx;
		cgm_stat_container->l1_d_TotalUpgrades[i] = l1_d_caches[i].TotalUpgrades;
		cgm_stat_container->l1_d_TotalReadMisses[i] = l1_d_caches[i].TotalReadMisses;
		cgm_stat_container->l1_d_TotalWriteMisses[i] = l1_d_caches[i].TotalWriteMisses;
		cgm_stat_container->l1_d_TotalWriteBacks[i] = l1_d_caches[i].TotalWriteBacks;
		cgm_stat_container->l1_d_invalid_hits[i] = l1_d_caches[i].invalid_hits;
		cgm_stat_container->l1_d_assoc_conflict[i] = l1_d_caches[i].assoc_conflict;
		cgm_stat_container->l1_d_upgrade_misses[i] = l1_d_caches[i].upgrade_misses;
		cgm_stat_container->l1_d_retries[i] = l1_d_caches[i].retries;
		cgm_stat_container->l1_d_coalesces[i] = l1_d_caches[i].coalesces;
		cgm_stat_container->l1_d_mshr_entries[i] = l1_d_caches[i].mshr_entries;
		cgm_stat_container->l1_d_stalls[i] = l1_d_caches[i].stalls;

		cgm_stat_container->l2_TotalThreadLoops[i] = l2_caches[i].TotalThreadLoops;
		cgm_stat_container->l2_TotalAcesses[i] = l2_caches[i].TotalAcesses;
		cgm_stat_container->l2_TotalMisses[i] = l2_caches[i].TotalMisses;
		cgm_stat_container->l2_TotalHits[i] = l2_caches[i].TotalHits;
		cgm_stat_container->l2_TotalReads[i] = l2_caches[i].TotalReads;
		cgm_stat_container->l2_TotalWrites[i] = l2_caches[i].TotalWrites;
		cgm_stat_container->l2_TotalGets[i] = l2_caches[i].TotalGets;
		cgm_stat_container->l2_TotalGet[i] = l2_caches[i].TotalGet;
		cgm_stat_container->l2_TotalGetx[i] = l2_caches[i].TotalGetx;
		cgm_stat_container->l2_TotalUpgrades[i] = l2_caches[i].TotalUpgrades;
		cgm_stat_container->l2_TotalReadMisses[i] = l2_caches[i].TotalReadMisses;
		cgm_stat_container->l2_TotalWriteMisses[i] = l2_caches[i].TotalWriteMisses;
		cgm_stat_container->l2_TotalWriteBacks[i] = l2_caches[i].TotalWriteBacks;
		cgm_stat_container->l2_invalid_hits[i] = l2_caches[i].invalid_hits;
		cgm_stat_container->l2_assoc_conflict[i] = l2_caches[i].assoc_conflict;
		cgm_stat_container->l2_upgrade_misses[i] = l2_caches[i].upgrade_misses;
		cgm_stat_container->l2_retries[i] = l2_caches[i].retries;
		cgm_stat_container->l2_coalesces[i] = l2_caches[i].coalesces;
		cgm_stat_container->l2_mshr_entries[i] = l2_caches[i].mshr_entries;
		cgm_stat_container->l2_stalls[i] = l2_caches[i].stalls;

		cgm_stat_container->l3_TotalThreadLoops[i] = l3_caches[i].TotalThreadLoops;
		cgm_stat_container->l3_TotalAcesses[i] = l3_caches[i].TotalAcesses;
		cgm_stat_container->l3_TotalMisses[i] = l3_caches[i].TotalMisses;
		cgm_stat_container->l3_TotalHits[i] = l3_caches[i].TotalHits;
		cgm_stat_container->l3_TotalReads[i] = l3_caches[i].TotalReads;
		cgm_stat_container->l3_TotalWrites[i] = l3_caches[i].TotalWrites;
		cgm_stat_container->l3_TotalGets[i] = l3_caches[i].TotalGets;
		cgm_stat_container->l3_TotalGet[i] = l3_caches[i].TotalGet;
		cgm_stat_container->l3_TotalGetx[i] = l3_caches[i].TotalGetx;
		cgm_stat_container->l3_TotalUpgrades[i] = l3_caches[i].TotalUpgrades;
		cgm_stat_container->l3_TotalReadMisses[i] = l3_caches[i].TotalReadMisses;
		cgm_stat_container->l3_TotalWriteMisses[i] = l3_caches[i].TotalWriteMisses;
		cgm_stat_container->l3_TotalWriteBacks[i] = l3_caches[i].TotalWriteBacks;
		cgm_stat_container->l3_invalid_hits[i] = l3_caches[i].invalid_hits;
		cgm_stat_container->l3_assoc_conflict[i] = l3_caches[i].assoc_conflict;
		cgm_stat_container->l3_upgrade_misses[i] = l3_caches[i].upgrade_misses;
		cgm_stat_container->l3_retries[i] = l3_caches[i].retries;
		cgm_stat_container->l3_coalesces[i] = l3_caches[i].coalesces;
		cgm_stat_container->l3_mshr_entries[i] = l3_caches[i].mshr_entries;
		cgm_stat_container->l3_stalls[i] = l3_caches[i].stalls;
	}

	//switch stats
	for(i = 0; i < (num_cores + 1); i++)
	{
		cgm_stat_container->switch_total_links[i] = switches[i].switch_total_links;
		cgm_stat_container->switch_max_links[i] = switches[i].switch_max_links;
		cgm_stat_container->switch_total_wakes[i] = switches[i].switch_total_wakes;
		cgm_stat_container->switch_north_io_transfers[i] = switches[i].switch_north_io_transfers;
		cgm_stat_container->switch_north_io_transfer_cycles[i] = switches[i].switch_north_io_transfer_cycles;
		cgm_stat_container->switch_north_io_bytes_transfered[i] = switches[i].switch_north_io_bytes_transfered;
		cgm_stat_container->switch_east_io_transfers[i] = switches[i].switch_east_io_transfers;
		cgm_stat_container->switch_east_io_transfer_cycles[i] = switches[i].switch_east_io_transfer_cycles;
		cgm_stat_container->switch_east_io_bytes_transfered[i] = switches[i].switch_east_io_bytes_transfered;
		cgm_stat_container->switch_south_io_transfers[i] = switches[i].switch_south_io_transfers;
		cgm_stat_container->switch_south_io_transfer_cycles[i] = switches[i].switch_south_io_transfer_cycles;
		cgm_stat_container->switch_south_io_bytes_transfered[i] = switches[i].switch_south_io_bytes_transfered;
		cgm_stat_container->switch_west_io_transfers[i] = switches[i].switch_west_io_transfers;
		cgm_stat_container->switch_west_io_transfer_cycles[i] = switches[i].switch_west_io_transfer_cycles;
		cgm_stat_container->switch_west_io_bytes_transfered[i] = switches[i].switch_west_io_bytes_transfered;
		cgm_stat_container->switch_north_txqueue_max_depth[i] = switches[i].north_txqueue_max_depth;
		cgm_stat_container->switch_north_txqueue_ave_depth[i] = switches[i].north_txqueue_ave_depth;
		cgm_stat_container->switch_east_txqueue_max_depth[i] = switches[i].east_txqueue_max_depth;
		cgm_stat_container->switch_east_txqueue_ave_depth[i] = switches[i].east_txqueue_ave_depth;
		cgm_stat_container->switch_south_txqueue_max_depth[i] = switches[i].south_txqueue_max_depth;
		cgm_stat_container->switch_south_txqueue_ave_depth[i] = switches[i].south_txqueue_ave_depth;
		cgm_stat_container->switch_west_txqueue_max_depth[i] = switches[i].west_txqueue_max_depth;
		cgm_stat_container->switch_west_txqueue_ave_depth[i] = switches[i].west_txqueue_ave_depth;

		cgm_stat_container->switch_north_tx_inserts[i] = switches[i].north_tx_inserts;
		cgm_stat_container->switch_east_tx_inserts[i] = switches[i].east_tx_inserts;
		cgm_stat_container->switch_south_tx_inserts[i] = switches[i].south_tx_inserts;
		cgm_stat_container->switch_west_tx_inserts[i] = switches[i].west_tx_inserts;

		cgm_stat_container->switch_north_rxqueue_max_depth[i] = switches[i].north_rxqueue_max_depth;
		cgm_stat_container->switch_north_rxqueue_ave_depth[i] = switches[i].north_rxqueue_ave_depth;
		cgm_stat_container->switch_east_rxqueue_max_depth[i] = switches[i].east_rxqueue_max_depth;
		cgm_stat_container->switch_east_rxqueue_ave_depth[i] = switches[i].east_rxqueue_ave_depth;

		cgm_stat_container->switch_south_rxqueue_max_depth[i] = switches[i].south_rxqueue_max_depth;
		cgm_stat_container->switch_south_rxqueue_ave_depth[i] = switches[i].south_rxqueue_ave_depth;
		cgm_stat_container->switch_west_rxqueue_max_depth[i] = switches[i].west_rxqueue_max_depth;
		cgm_stat_container->switch_west_rxqueue_ave_depth[i] = switches[i].west_rxqueue_ave_depth;

		cgm_stat_container->switch_north_rx_inserts[i] = switches[i].north_rx_inserts;
		cgm_stat_container->switch_east_rx_inserts[i] = switches[i].east_rx_inserts;
		cgm_stat_container->switch_south_rx_inserts[i] = switches[i].south_rx_inserts;
		cgm_stat_container->switch_west_rx_inserts[i] = switches[i].west_rx_inserts;
	}

	//system agent
	cgm_stat_container->system_agent_busy_cycles = system_agent->busy_cycles;
	cgm_stat_container->system_agent_north_io_busy_cycles = system_agent->north_io_busy_cycles;
	cgm_stat_container->system_agent_south_io_busy_cycles = system_agent->south_io_busy_cycles;
	cgm_stat_container->system_agent_mc_loads = system_agent->mc_loads;
	cgm_stat_container->system_agent_mc_stores = system_agent->mc_stores;
	cgm_stat_container->system_agent_mc_returns = system_agent->mc_returns;
	cgm_stat_container->system_agent_max_north_rxqueue_depth = system_agent->max_north_rxqueue_depth;
	cgm_stat_container->system_agent_ave_north_rxqueue_depth = system_agent->ave_north_rxqueue_depth;
	cgm_stat_container->system_agent_max_south_rxqueue_depth = system_agent->max_south_rxqueue_depth;
	cgm_stat_container->system_agent_ave_south_rxqueue_depth = system_agent->ave_south_rxqueue_depth;
	cgm_stat_container->system_agent_max_north_txqueue_depth = system_agent->max_north_txqueue_depth;
	cgm_stat_container->system_agent_ave_north_txqueue_depth = system_agent->ave_north_txqueue_depth;
	cgm_stat_container->system_agent_max_south_txqueue_depth = system_agent->max_south_txqueue_depth;
	cgm_stat_container->system_agent_ave_south_txqueue_depth = system_agent->ave_south_txqueue_depth;
	/*cgm_stat_container->system_agent_north_gets = system_agent->north_gets;
	cgm_stat_container->system_agent_south_gets = system_agent->south_gets;
	cgm_stat_container->system_agent_north_puts = system_agent->north_puts;
	cgm_stat_container->system_agent_south_puts = system_agent->south_puts;*/

	//Memory controller and DRAMSim
	cgm_stat_container->mem_ctrl_busy_cycles = mem_ctrl->busy_cycles;
	cgm_stat_container->mem_ctrl_num_reads = mem_ctrl->num_reads;
	cgm_stat_container->mem_ctrl_num_writes = mem_ctrl->num_writes;
	cgm_stat_container->mem_ctrl_ave_dram_read_lat = mem_ctrl->ave_dram_read_lat;
	cgm_stat_container->mem_ctrl_ave_dram_write_lat = mem_ctrl->ave_dram_write_lat;
	cgm_stat_container->mem_ctrl_ave_dram_total_lat = mem_ctrl->ave_dram_total_lat;
	cgm_stat_container->mem_ctrl_read_min = mem_ctrl->read_min;
	cgm_stat_container->mem_ctrl_read_max = mem_ctrl->read_max;
	cgm_stat_container->mem_ctrl_write_min = mem_ctrl->write_min;
	cgm_stat_container->mem_ctrl_write_max = mem_ctrl->write_max;
	cgm_stat_container->mem_ctrl_dram_max_queue_depth = mem_ctrl->dram_max_queue_depth;
	cgm_stat_container->mem_ctrl_dram_ave_queue_depth = mem_ctrl->dram_ave_queue_depth;
	cgm_stat_container->mem_ctrl_dram_busy_cycles = mem_ctrl->dram_busy_cycles;
	cgm_stat_container->mem_ctrl_rx_max = mem_ctrl->rx_max;
	cgm_stat_container->mem_ctrl_tx_max = mem_ctrl->tx_max;
	cgm_stat_container->mem_ctrl_bytes_read = mem_ctrl->bytes_read;
	cgm_stat_container->mem_ctrl_bytes_wrote = mem_ctrl->bytes_wrote;
	cgm_stat_container->mem_ctrl_io_busy_cycles = mem_ctrl->io_busy_cycles;

	return;
}

void cgm_reset_stats(void){

	int num_cores = x86_cpu_num_cores;
	/*int num_cus = si_gpu_num_compute_units;*/
	//int gpu_group_cache_num = (num_cus/4);
	int i = 0;

	//reset cgm_stat
	for(i = 0; i < num_cores; i++)
	{
		cpu_gpu_stats->core_num_syscalls[i] = 0;
		cpu_gpu_stats->core_syscall_stalls[i] = 0;
		cpu_gpu_stats->core_rob_stalls[i] = 0;
		cpu_gpu_stats->core_rob_stall_load[i] = 0;
		cpu_gpu_stats->core_rob_stall_store[i] = 0;
		cpu_gpu_stats->core_rob_stall_other[i] = 0;
		cpu_gpu_stats->core_first_fetch_cycle[i] = 0;
		cpu_gpu_stats->core_fetch_stalls[i] = 0;
		cpu_gpu_stats->core_last_commit_cycle[i] = 0;
		cpu_gpu_stats->core_issued_memory_insts[i] = 0;
		cpu_gpu_stats->core_commited_memory_insts[i] = 0;
	}

	//memory system at large
	mem_system_stats->first_mem_access_lat = 0;

	for(i = 0; i < HISTSIZE; i++)
	{
		mem_system_stats->fetch_lat_hist[i] = 0;
		mem_system_stats->load_lat_hist[i] = 0;
		mem_system_stats->store_lat_hist[i] = 0;
	}

	mem_system_stats->cpu_total_fetches = 0;
	mem_system_stats->fetch_l1_hits = 0;
	mem_system_stats->fetch_l2_hits = 0;
	mem_system_stats->fetch_l3_hits = 0;
	mem_system_stats->fetch_memory = 0;
	mem_system_stats->cpu_total_loads = 0;
	mem_system_stats->load_l1_hits = 0;
	mem_system_stats->load_l2_hits = 0;
	mem_system_stats->load_l3_hits = 0;
	mem_system_stats->load_memory = 0;
	mem_system_stats->load_get_fwd = 0;
	mem_system_stats->cpu_total_stores = 0;
	mem_system_stats->store_l1_hits = 0;
	mem_system_stats->store_l2_hits = 0;
	mem_system_stats->store_l3_hits = 0;
	mem_system_stats->store_memory = 0;
	mem_system_stats->store_getx_fwd = 0;
	mem_system_stats->store_upgrade = 0;

	//caches
	for(i = 0; i < num_cores; i++)
	{
		l1_i_caches[i].TotalThreadLoops = 0;
		l1_i_caches[i].TotalAcesses = 0;
		l1_i_caches[i].TotalMisses = 0;
		l1_i_caches[i].TotalHits = 0;
		l1_i_caches[i].TotalReads = 0;
		l1_i_caches[i].TotalWrites = 0;
		l1_i_caches[i].TotalGets = 0;
		l1_i_caches[i].TotalGet = 0;
		l1_i_caches[i].TotalGetx = 0;
		l1_i_caches[i].TotalUpgrades = 0;
		l1_i_caches[i].TotalReadMisses = 0;
		l1_i_caches[i].TotalWriteMisses = 0;
		l1_i_caches[i].TotalWriteBacks = 0;
		l1_i_caches[i].invalid_hits = 0;
		l1_i_caches[i].assoc_conflict = 0;
		l1_i_caches[i].upgrade_misses = 0;
		l1_i_caches[i].retries = 0;
		l1_i_caches[i].coalesces = 0;
		l1_i_caches[i].mshr_entries = 0;
		l1_i_caches[i].stalls = 0;

		l1_d_caches[i].TotalThreadLoops = 0;
		l1_d_caches[i].TotalAcesses = 0;
		l1_d_caches[i].TotalMisses = 0;
		l1_d_caches[i].TotalHits = 0;
		l1_d_caches[i].TotalReads = 0;
		l1_d_caches[i].TotalWrites = 0;
		l1_d_caches[i].TotalGets = 0;
		l1_d_caches[i].TotalGet = 0;
		l1_d_caches[i].TotalGetx = 0;
		l1_d_caches[i].TotalUpgrades = 0;
		l1_d_caches[i].TotalReadMisses = 0;
		l1_d_caches[i].TotalWriteMisses = 0;
		l1_d_caches[i].TotalWriteBacks = 0;
		l1_d_caches[i].invalid_hits = 0;
		l1_d_caches[i].assoc_conflict = 0;
		l1_d_caches[i].upgrade_misses = 0;
		l1_d_caches[i].retries = 0;
		l1_d_caches[i].coalesces = 0;
		l1_d_caches[i].mshr_entries = 0;
		l1_d_caches[i].stalls = 0;

		l2_caches[i].TotalThreadLoops = 0;
		l2_caches[i].TotalAcesses = 0;
		l2_caches[i].TotalMisses = 0;
		l2_caches[i].TotalHits = 0;
		l2_caches[i].TotalReads = 0;
		l2_caches[i].TotalWrites = 0;
		l2_caches[i].TotalGets = 0;
		l2_caches[i].TotalGet = 0;
		l2_caches[i].TotalGetx = 0;
		l2_caches[i].TotalUpgrades = 0;
		l2_caches[i].TotalReadMisses = 0;
		l2_caches[i].TotalWriteMisses = 0;
		l2_caches[i].TotalWriteBacks = 0;
		l2_caches[i].invalid_hits = 0;
		l2_caches[i].assoc_conflict = 0;
		l2_caches[i].upgrade_misses = 0;
		l2_caches[i].retries = 0;
		l2_caches[i].coalesces = 0;
		l2_caches[i].mshr_entries = 0;
		l2_caches[i].stalls = 0;

		l3_caches[i].TotalThreadLoops = 0;
		l3_caches[i].TotalAcesses = 0;
		l3_caches[i].TotalMisses = 0;
		l3_caches[i].TotalHits = 0;
		l3_caches[i].TotalReads = 0;
		l3_caches[i].TotalWrites = 0;
		l3_caches[i].TotalGets = 0;
		l3_caches[i].TotalGet = 0;
		l3_caches[i].TotalGetx = 0;
		l3_caches[i].TotalUpgrades = 0;
		l3_caches[i].TotalReadMisses = 0;
		l3_caches[i].TotalWriteMisses = 0;
		l3_caches[i].TotalWriteBacks = 0;
		l3_caches[i].invalid_hits = 0;
		l3_caches[i].assoc_conflict = 0;
		l3_caches[i].upgrade_misses = 0;
		l3_caches[i].retries = 0;
		l3_caches[i].coalesces = 0;
		l3_caches[i].mshr_entries = 0;
		l3_caches[i].stalls = 0;
	}

	//switch stats
	for(i = 0; i < (num_cores + 1); i++)
	{
		switches[i].switch_total_links = 0;
		switches[i].switch_max_links = 0;
		switches[i].switch_total_wakes = 0;
		switches[i].switch_north_io_transfers = 0;
		switches[i].switch_north_io_transfer_cycles = 0;
		switches[i].switch_north_io_bytes_transfered = 0;
		switches[i].switch_east_io_transfers = 0;
		switches[i].switch_east_io_transfer_cycles = 0;
		switches[i].switch_east_io_bytes_transfered = 0;
		switches[i].switch_south_io_transfers = 0;
		switches[i].switch_south_io_transfer_cycles = 0;
		switches[i].switch_south_io_bytes_transfered = 0;
		switches[i].switch_west_io_transfers = 0;
		switches[i].switch_west_io_transfer_cycles = 0;
		switches[i].switch_west_io_bytes_transfered = 0;
		switches[i].north_txqueue_max_depth = 0;
		switches[i].north_txqueue_ave_depth = 0;
		switches[i].east_txqueue_max_depth = 0;
		switches[i].east_txqueue_ave_depth = 0;
		switches[i].south_txqueue_max_depth = 0;
		switches[i].south_txqueue_ave_depth = 0;
		switches[i].west_txqueue_max_depth = 0;
		switches[i].west_txqueue_ave_depth = 0;

		switches[i].north_tx_inserts = 0;
		switches[i].east_tx_inserts = 0;
		switches[i].south_tx_inserts = 0;
		switches[i].west_tx_inserts = 0;

		switches[i].north_rxqueue_max_depth = 0;
		switches[i].north_rxqueue_ave_depth = 0;
		switches[i].east_rxqueue_max_depth = 0;
		switches[i].east_rxqueue_ave_depth = 0;
		switches[i].south_rxqueue_max_depth = 0;
		switches[i].south_rxqueue_ave_depth = 0;
		switches[i].west_rxqueue_max_depth = 0;
		switches[i].west_rxqueue_ave_depth = 0;

		switches[i].north_rx_inserts = 0;
		switches[i].east_rx_inserts = 0;
		switches[i].south_rx_inserts = 0;
		switches[i].west_rx_inserts = 0;
	}

	//system agent
	system_agent->busy_cycles = 0;
	system_agent->north_io_busy_cycles = 0;
	system_agent->south_io_busy_cycles = 0;
	system_agent->mc_loads = 0;
	system_agent->mc_stores = 0;
	system_agent->mc_returns = 0;
	system_agent->max_north_rxqueue_depth = 0;
	system_agent->ave_north_rxqueue_depth = 0;
	system_agent->max_south_rxqueue_depth = 0;
	system_agent->ave_south_rxqueue_depth = 0;
	system_agent->max_north_txqueue_depth = 0;
	system_agent->ave_north_txqueue_depth = 0;
	system_agent->max_south_txqueue_depth = 0;
	system_agent->ave_south_txqueue_depth = 0;
	system_agent->north_gets = 0;
	system_agent->south_gets = 0;
	system_agent->north_puts = 0;
	system_agent->south_puts = 0;

	//Memory controller and DRAMSim
	mem_ctrl->busy_cycles = 0;
	mem_ctrl->num_reads = 0;
	mem_ctrl->num_writes = 0;
	mem_ctrl->ave_dram_read_lat = 0;
	mem_ctrl->ave_dram_write_lat = 0;
	mem_ctrl->ave_dram_total_lat = 0;
	mem_ctrl->read_min = 0;
	mem_ctrl->read_max = 0;
	mem_ctrl->write_min = 0;
	mem_ctrl->write_max = 0;
	mem_ctrl->dram_max_queue_depth = 0;
	mem_ctrl->dram_ave_queue_depth = 0;
	mem_ctrl->dram_busy_cycles = 0;
	mem_ctrl->rx_max = 0;
	mem_ctrl->tx_max = 0;
	mem_ctrl->bytes_read = 0;
	mem_ctrl->bytes_wrote = 0;
	mem_ctrl->io_busy_cycles = 0;

	return;
}


void cgm_init(int argc, char **argv){

	//Consolidated stat containers
	cgm_stat = (void *) calloc(1, sizeof(struct cgm_stats_t));
	cgm_startup_stats = (void *) calloc(1, sizeof(struct cgm_stats_t));
	cgm_parallel_stats = (void *) calloc(1, sizeof(struct cgm_stats_t));
	cgm_wrapup_stats = (void *) calloc(1, sizeof(struct cgm_stats_t));

	//other containers that don't fit well somewhere else.
	mem_system_stats = (void *) calloc(1, sizeof(struct mem_system_stats_t));
	cpu_gpu_stats = (void *) calloc(1, sizeof(struct cpu_gpu_stats_t));

	cgm_stat_finish_create(argc, argv);

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


		/*if()
		{
			if(l2_caches[1].sets[69].blocks[0].state == cgm_cache_block_invalid)
			{
				//assert(l2_caches[1].sets[69].blocks[0].tag == 57);
				fatal("\tWD: block changed blk_addr 0x%08x\n",
						cgm_cache_build_address(&l2_caches[1], l2_caches[1].sets[69].id, l2_caches[1].sets[69].blocks[0].tag));
			}
		}*/


		/*printf("\tWD: ort_queue_size %d cycle %llu\n", list_count(l1_d_caches[0].ort_list), P_TIME);*/
	}
	return;
}

void cgm_dump_general_stats(void){

	double cpu_sim_time = 0;
	unsigned int cpu_freq_hz = 0;

	//get the time
	cgm_stat->end_wall_time = get_wall_time();

	//calculate simulation runtime (wall clock)
	cgm_stat->sim_time = (cgm_stat->end_wall_time - cgm_stat->start_wall_time);

	cpu_freq_hz = (unsigned int) x86_cpu_frequency * (unsigned int) MHZ;
	cpu_sim_time = (double) P_TIME / (double) (cpu_freq_hz);

	/* General statistics */
	CGM_STATS(cgm_stats_file, "[General]\n");
	CGM_STATS(cgm_stats_file, "ExecutionSuccessful = %s\n", (cgm_stat->execution_success == true) ? ("Yes") : ("No"));
	CGM_STATS(cgm_stats_file, "Benchmark = %s\n", cgm_stat->benchmark_name);
	CGM_STATS(cgm_stats_file, "Day&Time = %s\n", cgm_stat->date_time_pretty);
	CGM_STATS(cgm_stats_file, "SimulationRunTimeSeconds(cpu) = %.9f\n", cpu_sim_time);
	CGM_STATS(cgm_stats_file, "SimulationRunTimeSeconds(wall) = %.2f\n", cgm_stat->sim_time);
	CGM_STATS(cgm_stats_file, "SimulatedCyclesPerSec = %.2f\n", (double)P_TIME/cgm_stat->sim_time);
	CGM_STATS(cgm_stats_file, "TotalCycles = %lld\n", P_TIME);
	CGM_STATS(cgm_stats_file, "\n");

	return;
}

void cgm_dump_startup_section_stats(struct cgm_stats_t *cgm_stat_container){

	/*CGM_STATS(cgm_stats_file, "[StartupSection]\n");*/
	CGM_STATS(cgm_stats_file, "StartupTotalCycles = %lld\n", cgm_stat_container->total_startup_section_cycles);
	CGM_STATS(cgm_stats_file, "StartupStartCycle = %lld\n", cgm_stat_container->start_startup_section_cycle);
	CGM_STATS(cgm_stats_file, "StartupEndCycle = %lld\n", cgm_stat_container->end_startup_section_cycle);
	CGM_STATS(cgm_stats_file, "StartupSectionPct = %0.2f\n", (double)cgm_stat_container->total_startup_section_cycles/(double)P_TIME);
	/*CGM_STATS(cgm_stats_file, "\n");*/

	return;
}

void cgm_dump_wrapup_section_stats(struct cgm_stats_t *cgm_stat_container){

	/*CGM_STATS(cgm_stats_file, "[WrapupSection]\n");*/
	CGM_STATS(cgm_stats_file, "WrapupTotalCycles = %lld\n", cgm_stat_container->total_wrapup_section_cycles);
	CGM_STATS(cgm_stats_file, "WrapupStartCycle = %lld\n", cgm_stat_container->start_wrapup_section_cycle);
	CGM_STATS(cgm_stats_file, "WrapupEndCycle = %lld\n", cgm_stat_container->end_wrapup_section_cycle);
	CGM_STATS(cgm_stats_file, "WrapupSectionPct = %0.2f\n", (double)cgm_stat_container->total_wrapup_section_cycles/(double)P_TIME);
	/*CGM_STATS(cgm_stats_file, "\n");*/

	return;
}


void cgm_dump_parallel_section_stats(struct cgm_stats_t *cgm_stat_container){

	/*CGM_STATS(cgm_stats_file, ";Note for now we are just jamming all of the stats under one header\n");
	CGM_STATS(cgm_stats_file, "[ParallelSection]\n");*/
	CGM_STATS(cgm_stats_file, "ParallelTotalCycles = %lld\n", cgm_stat_container->total_parallel_section_cycles);
	CGM_STATS(cgm_stats_file, "ParallelStartCycle = %lld\n", cgm_stat_container->start_parallel_section_cycle);
	CGM_STATS(cgm_stats_file, "ParallelEndCycle = %lld\n", cgm_stat_container->end_parallel_section_cycle);
	CGM_STATS(cgm_stats_file, "ParallelSectionPct = %0.2f\n", (double)cgm_stat_container->total_parallel_section_cycles/(double)P_TIME);
	/*CGM_STATS(cgm_stats_file, "\n");*/

	return;
}


void cgm_dump_cpu_gpu_stats(struct cgm_stats_t *cgm_stat_container){

	int num_cores = x86_cpu_num_cores;
	int num_threads = x86_cpu_num_threads;
	//int num_cus = si_gpu_num_compute_units;
	int i = 0;
	long long run_time = 0;
	long long idle_time = 0;
	long long busy_time = 0;
	long long stall_time = 0;
	long long system_time = 0;

	unsigned int cpu_freq_hz = 0;

	cpu_freq_hz = (unsigned int) x86_cpu_frequency * (unsigned int) MHZ;

	/*CPU stats*/
	/*CGM_STATS(cgm_stats_file, "[CPU]\n");*/
	CGM_STATS(cgm_stats_file, "CPU_NumCores = %d\n", num_cores);
	CGM_STATS(cgm_stats_file, "CPU_ThreadsPerCore = %d\n", num_threads);
	CGM_STATS(cgm_stats_file, "CPU_FrequencyGHz = %u\n", (cpu_freq_hz)/(GHZ));
	/*CGM_STATS(cgm_stats_file, "\n");*/


	/*core stats*/
	for(i = 0; i < num_cores; i++)
	{
		/*CGM_STATS(cgm_stats_file, "[Core_%d]\n", i);*/
		CGM_STATS(cgm_stats_file, "Core_%d_NumSyscalls = %llu\n", i, cgm_stat_container->core_num_syscalls[i]);
		CGM_STATS(cgm_stats_file, "Core_%d_ROBStalls = %llu\n", i, cgm_stat_container->core_rob_stalls[i]);
		CGM_STATS(cgm_stats_file, "Core_%d_ROBStallLoad = %llu\n", i, cgm_stat_container->core_rob_stall_load[i]);
		CGM_STATS(cgm_stats_file, "Core_%d_ROBStallStore = %llu\n", i, cgm_stat_container->core_rob_stall_store[i]);
		CGM_STATS(cgm_stats_file, "Core_%d_ROBStallOther = %llu\n", i, cgm_stat_container->core_rob_stall_other[i]);
		CGM_STATS(cgm_stats_file, "Core_%d_FetchStall = %llu\n", i, cgm_stat_container->core_fetch_stalls[i]);

		if(cgm_stat_container->stats_type == systemStats)
		{

			CGM_STATS(cgm_stats_file, "Core_%d_FirstFetchCycle = %llu\n", i,  cgm_stat_container->core_first_fetch_cycle[i]);
			CGM_STATS(cgm_stats_file, "Core_%d_LastCommitCycle = %llu\n", i, cgm_stat_container->core_last_commit_cycle[i]);

			//CGM_STATS(cgm_stats_file, "NumIssuedMemoryInst = %llu\n", cgm_stat_container->core_issued_memory_insts[i]);
			//CGM_STATS(cgm_stats_file, "NumCommitedMemoryInst = %llu\n", cgm_stat_container->core_commited_memory_insts[i]);
			run_time = cgm_stat_container->core_last_commit_cycle[i] - cgm_stat_container->core_first_fetch_cycle[i];
			CGM_STATS(cgm_stats_file, "Core_%d_RunTime = %llu\n", i, run_time);

			idle_time = P_TIME - run_time;
			CGM_STATS(cgm_stats_file, "Core_%d_IdleTime = %llu\n", i, idle_time);

			system_time = cgm_stat_container->core_syscall_stalls[i];
			CGM_STATS(cgm_stats_file, "Core_%d_SystemTime = %llu\n", i, system_time);

			stall_time = (cgm_stat_container->core_rob_stalls[i] + cgm_stat_container->core_fetch_stalls[i]);
			CGM_STATS(cgm_stats_file, "Core_%d_StallTime = %llu\n", i, stall_time);

			busy_time = (run_time - (stall_time + system_time));
			CGM_STATS(cgm_stats_file, "Core_%d_BusyTime = %llu\n", i, busy_time);

			CGM_STATS(cgm_stats_file, "Core_%d_IdlePct = %0.2f\n", i, (double)idle_time/(double)P_TIME);
			CGM_STATS(cgm_stats_file, "Core_%d_RunPct = %0.2f\n", i, (double)run_time/(double)P_TIME);
		}
		else if (cgm_stat_container->stats_type == parallelSection)
		{

			run_time = cgm_stat_container->total_parallel_section_cycles;
			CGM_STATS(cgm_stats_file, "Core_%d_RunTime = %llu\n", i, run_time);

			idle_time = 0;
			CGM_STATS(cgm_stats_file, "Core_%d_IdleTime = %llu\n", i, idle_time);

			system_time = cgm_stat_container->core_syscall_stalls[i];
			CGM_STATS(cgm_stats_file, "Core_%d_SystemTime = %llu\n", i, system_time);

			stall_time = (cgm_stat_container->core_rob_stalls[i] + cgm_stat_container->core_fetch_stalls[i]);
			CGM_STATS(cgm_stats_file, "Core_%d_StallTime = %llu\n", i, stall_time);

			busy_time = (run_time - (stall_time + system_time));
			CGM_STATS(cgm_stats_file, "Core_%d_BusyTime = %llu\n", i, busy_time);

			CGM_STATS(cgm_stats_file, "Core_%d_IdlePct = %0.2f\n", i, (double)idle_time/(double)run_time);
			CGM_STATS(cgm_stats_file, "Core_%d_RunPct = %0.2f\n", i, (double)run_time/(double)run_time);
		}
		else
		{
			fatal("cgm_dump_cpu_gpu_stats(): not set up for these stats yet\n");
		}

		CGM_STATS(cgm_stats_file, "Core_%d_SystemPct = %0.2f\n", i, (double)system_time/(double)run_time);
		CGM_STATS(cgm_stats_file, "Core_%d_StallPct = %0.2f\n", i, (double)stall_time/(double)run_time);
		CGM_STATS(cgm_stats_file, "Core_%d_BusyPct = %0.2f\n", i, (double)busy_time/(double)run_time);
		CGM_STATS(cgm_stats_file, "Core_%d_StallfetchPct = %0.2f\n", i, (double)cgm_stat_container->core_fetch_stalls[i]/(double)stall_time);
		CGM_STATS(cgm_stats_file, "Core_%d_StallLoadPct = %0.2f\n", i, (double)cgm_stat_container->core_rob_stall_load[i]/(double)stall_time);
		CGM_STATS(cgm_stats_file, "Core_%d_StallStorePct = %0.2f\n", i, (double)cgm_stat_container->core_rob_stall_store[i]/(double)stall_time);
		//CGM_STATS(cgm_stats_file, "StallSyscallPct = %0.2f\n", (double)cgm_stat_container->core_rob_stall_syscall[i]/(double)stall_time);
		CGM_STATS(cgm_stats_file, "Core_%d_StallOtherPct = %0.2f\n", i, (double)cgm_stat_container->core_rob_stall_other[i]/(double)stall_time);
		/*CGM_STATS(cgm_stats_file, "\n");*/
	}

	//CGM_STATS(cgm_stats_file, "FetchStalls = %llu\n", cgm_stat_container->cpu_fetch_stalls);
	//CGM_STATS(cgm_stats_file, "LoadStoreStalls = %llu\n", cgm_stat_container->cpu_ls_stalls);
	//CGM_STATS(cgm_stats_file, "\n");
	/*CGM_STATS(cgm_stats_file, "[GPU]\n");
	CGM_STATS(cgm_stats_file, "NumComputeUnits = %d\n", num_cus);
	CGM_STATS(cgm_stats_file, "\n");*/

	return;
}


void cgm_dump_mem_system_stats(struct cgm_stats_t *cgm_stat_container){

	/*int num_cores = x86_cpu_num_cores;
	int num_threads = x86_cpu_num_threads;
	int num_cus = si_gpu_num_compute_units;
	int i = 0;
	long long run_time = 0;
	long long idle_time = 0;
	long long busy_time = 0;
	long long stall_time = 0;
	long long system_time = 0;*/

	/*CGM_STATS(cgm_stats_file, "[MemSystem]\n");*/
	CGM_STATS(cgm_stats_file, "MemSystem_FirstAccessLat(Fetch) = %d\n", cgm_stat_container->first_mem_access_lat);
	CGM_STATS(cgm_stats_file, "MemSystem_TotalFetches = %llu\n", cgm_stat_container->cpu_total_fetches);
	CGM_STATS(cgm_stats_file, "MemSystem_FetchesL1 = %llu\n", cgm_stat_container->fetch_l1_hits);
	CGM_STATS(cgm_stats_file, "MemSystem_FetchesL2 = %llu\n", cgm_stat_container->fetch_l2_hits);
	CGM_STATS(cgm_stats_file, "MemSystem_FetchesL3 = %llu\n", cgm_stat_container->fetch_l3_hits);
	CGM_STATS(cgm_stats_file, "MemSystem_FetchesMemory = %llu\n", cgm_stat_container->fetch_memory);
	CGM_STATS(cgm_stats_file, "MemSystem_TotalLoads = %llu\n", cgm_stat_container->cpu_total_loads);
	CGM_STATS(cgm_stats_file, "MemSystem_LoadsL1 = %llu\n", cgm_stat_container->load_l1_hits);
	CGM_STATS(cgm_stats_file, "MemSystem_LoadsL2 = %llu\n", cgm_stat_container->load_l2_hits);
	CGM_STATS(cgm_stats_file, "MemSystem_LoadsL3 = %llu\n", cgm_stat_container->load_l3_hits);
	CGM_STATS(cgm_stats_file, "MemSystem_LoadsMemory = %llu\n", cgm_stat_container->load_memory);
	CGM_STATS(cgm_stats_file, "MemSystem_LoadsGetFwd = %llu\n", cgm_stat_container->load_get_fwd);
	CGM_STATS(cgm_stats_file, "MemSystem_TotalStore = %llu\n", cgm_stat_container->cpu_total_stores);
	CGM_STATS(cgm_stats_file, "MemSystem_StoresL1 = %llu\n", cgm_stat_container->store_l1_hits);
	CGM_STATS(cgm_stats_file, "MemSystem_StoresL2 = %llu\n", cgm_stat_container->store_l2_hits);
	CGM_STATS(cgm_stats_file, "MemSystem_StoresL3 = %llu\n", cgm_stat_container->store_l3_hits);
	CGM_STATS(cgm_stats_file, "MemSystem_StoresMemory = %llu\n", cgm_stat_container->store_memory);
	CGM_STATS(cgm_stats_file, "MemSystem_StoresGetxFwd = %llu\n", cgm_stat_container->store_getx_fwd);
	CGM_STATS(cgm_stats_file, "MemSystem_StoresUpgrade = %llu\n", cgm_stat_container->store_upgrade);
	/*CGM_STATS(cgm_stats_file, "\n");*/

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

	fclose (load_lat_log_file);
	fclose (fetch_lat_log_file);
	fclose (store_lat_log_file);

	return;
}


void cgm_dump_summary(void){

	printf("\n---Printing Stats to file %s---\n", cgm_stat->stat_file_name);

	/*finalize the cgm stats structure i.e. consolidate all stats
	we did this because in simulation you may only want to instrument certain sections of a benchmark
	for example, this gives us the flexibility to pull stats from only the parallel section of a benchmark*/
	cgm_wrapup_stats->end_wrapup_section_cycle = P_TIME;
	cgm_wrapup_stats->total_wrapup_section_cycles = cgm_wrapup_stats->end_wrapup_section_cycle - cgm_wrapup_stats->start_wrapup_section_cycle;
	cgm_store_stats(cgm_wrapup_stats);

	/*There are three system dumps of interest
	(1) full system
	(2) parallel section
	(3) parallel & OCL overhead sections*/

	//this sets up the full system dump in cgm_stat
	cgm_consolidate_stats();

	cgm_dump_general_stats();

	/*dump the full Run stats*/
	CGM_STATS(cgm_stats_file, ";Don't try to read this, use the python scripts to generate easy to read output.\n");
	CGM_STATS(cgm_stats_file, "[FullRunStats]\n");
	cgm_dump_cpu_gpu_stats(cgm_stat);
	cgm_dump_mem_system_stats(cgm_stat);
	cache_dump_stats(cgm_stat);
	switch_dump_stats(cgm_stat);
	sys_agent_dump_stats(cgm_stat);
	memctrl_dump_stats(cgm_stat);
	CGM_STATS(cgm_stats_file, "\n");

	/*dump specific areas of interest*/
	CGM_STATS(cgm_stats_file, "[StartupStats]\n");
	cgm_dump_startup_section_stats(cgm_startup_stats);
	CGM_STATS(cgm_stats_file, "\n");

	//parallel stats

	CGM_STATS(cgm_stats_file, "[ParallelStats]\n");
	cgm_dump_parallel_section_stats(cgm_parallel_stats);

	cgm_dump_cpu_gpu_stats(cgm_parallel_stats);
	cgm_dump_mem_system_stats(cgm_parallel_stats);
	cache_dump_stats(cgm_parallel_stats);
	switch_dump_stats(cgm_parallel_stats);
	sys_agent_dump_stats(cgm_parallel_stats);
	memctrl_dump_stats(cgm_parallel_stats);
	CGM_STATS(cgm_stats_file, "\n");


	//wrapup stats

	CGM_STATS(cgm_stats_file, "[WrapupStats]\n");
	cgm_dump_wrapup_section_stats(cgm_wrapup_stats);
	CGM_STATS(cgm_stats_file, "\n");

	/*star todo dump histograms for the various sections that we are intrested in*/

	/*dump the histograms*/
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
	if(cpu_gpu_stats->core_first_fetch_cycle[thread->core->id] == 0)
		cpu_gpu_stats->core_first_fetch_cycle[thread->core->id] = P_TIME;

	mem_system_stats->cpu_total_fetches++;
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
	cpu_gpu_stats->core_issued_memory_insts[thread->core->id]++;

	/*printf("\t lsq issuing access_id %llu\n", access_id);*/

	if(access_kind == cgm_access_load)
	{
		mem_system_stats->cpu_total_loads++;
	}
	else if(access_kind == cgm_access_store)
	{
		mem_system_stats->cpu_total_stores++;
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
		mem_system_stats->gpu_total_loads++;
	}

	if(access_kind == cgm_access_store_v || access_kind == cgm_access_nc_store)
	{
	 	mem_system_stats->gpu_total_stores++;
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

	printf("\n---L1_d_caches---\n");
	for(i = 0; i < num_cores; i++)
	{
		printf("---%s Rx top queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].Rx_queue_top));
		cache_dump_queue(l1_d_caches[i].Rx_queue_top);
		printf("---%s Rx bottom queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].Rx_queue_bottom));
		cache_dump_queue(l1_d_caches[i].Rx_queue_bottom);
		printf("---%s Tx bottom queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].Tx_queue_bottom));
		cache_dump_queue(l1_d_caches[i].Tx_queue_bottom);
		printf("---%s Coherence queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].Coherance_Rx_queue));
		cache_dump_queue(l1_d_caches[i].Coherance_Rx_queue);
		printf("---%s Pending request queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].pending_request_buffer));
		cache_dump_queue(l1_d_caches[i].pending_request_buffer);
		printf("---%s Retry queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].retry_queue));
		cache_dump_queue(l1_d_caches[i].retry_queue);
		printf("---%s Write back queue size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].write_back_buffer));
		cache_dump_write_back(&l2_caches[i]);
		printf("---%s ORT size %d---\n",
				l1_d_caches[i].name, list_count(l1_d_caches[i].ort_list));
		cache_dump_queue(l1_d_caches[i].ort_list);
		ort_dump(&l1_d_caches[i]);
		printf("\n");
	}

	printf("---L2_caches---\n");
	for(i = 0; i < num_cores; i++)
	{
		printf("---%s Rx top queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].Rx_queue_top));
		cache_dump_queue(l2_caches[i].Rx_queue_top);
		printf("---%s Rx bottom queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].Rx_queue_bottom));
		cache_dump_queue(l2_caches[i].Rx_queue_bottom);
		printf("---%s Tx top queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].Tx_queue_top));
		cache_dump_queue(l2_caches[i].Tx_queue_top);
		printf("---%s Tx bottom queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].Tx_queue_bottom));
		cache_dump_queue(l2_caches[i].Tx_queue_bottom);
		printf("---%s Coherence queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].Coherance_Rx_queue));
		cache_dump_queue(l2_caches[i].Coherance_Rx_queue);
		printf("---%s Pending request queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].pending_request_buffer));
		cache_dump_queue(l2_caches[i].pending_request_buffer);
		printf("---%s Retry queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].retry_queue));
		cache_dump_queue(l2_caches[i].retry_queue);
		printf("---%s Write back queue size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].write_back_buffer));
		cache_dump_write_back(&l2_caches[i]);
		printf("---%s ORT size %d---\n",
				l2_caches[i].name, list_count(l2_caches[i].ort_list));
		cache_dump_queue(l2_caches[i].ort_list);
		ort_dump(&l2_caches[i]);
		printf("\n");
	}

	printf("---L3_caches---\n");
	for(i = 0; i < num_cores; i++)
	{
		printf("---%s Rx top queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].Rx_queue_top));
		cache_dump_queue(l3_caches[i].Rx_queue_top);
		printf("---%s Rx bottom queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].Rx_queue_bottom));
		cache_dump_queue(l3_caches[i].Rx_queue_bottom);
		printf("---%s Tx top queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].Tx_queue_top));
		cache_dump_queue(l3_caches[i].Tx_queue_top);
		printf("---%s Tx bottom queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].Tx_queue_bottom));
		cache_dump_queue(l3_caches[i].Tx_queue_bottom);
		printf("---%s Coherence queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].Coherance_Rx_queue));
		cache_dump_queue(l3_caches[i].Coherance_Rx_queue);
		printf("---%s Pending request queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].pending_request_buffer));
		cache_dump_queue(l3_caches[i].pending_request_buffer);
		printf("---%s Retry queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].retry_queue));
		cache_dump_queue(l3_caches[i].retry_queue);
		printf("---%s Write back queue size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].write_back_buffer));
		cache_dump_write_back(&l3_caches[i]);
		printf("---%s ORT size %d---\n",
				l3_caches[i].name, list_count(l3_caches[i].ort_list));
		cache_dump_queue(l3_caches[i].ort_list);
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
