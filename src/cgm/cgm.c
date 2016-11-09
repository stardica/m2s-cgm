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

#include <arch/x86/emu/context.h>


#include <cgm/cgm.h>
#include <cgm/configure.h>
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

int Histograms = 0;

char *cgm_debug_output_path = "";
char *cgm_stats_output_path = "";
char *cgm_stats_file_name = "";

long long resume_cycle = 0;

//globals for tasking
eventcount volatile *sim_start;
eventcount volatile *sim_finish;
eventcount volatile *watchdog;

int SINGLE_CORE = 0;
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

enum time_type_t current_time_type = cpu_time;

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
	//int j = 0;
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

	//get the benchmark name
	bname = basename(argv[i++]);
	sprintf(buff + strlen(buff), "%s", bname);

	cgm_stat->benchmark_name = strdup(buff);

	memset(buff, '\0', 400);

	//now get the benchamrk's args
	while(i < argc)
		sprintf(buff + strlen(buff), "%s ", argv[i++]);

	cgm_stat->args = strdup(buff);

	cgm_stats_alloc(cgm_stat);
	cgm_stats_alloc(cgm_startup_stats);
	cgm_stats_alloc(cgm_parallel_stats);
	cgm_stats_alloc(cgm_wrapup_stats);

	//assign a type to our containers
	cgm_stat->stats_type = systemStats;
	cgm_stat->execution_success = false;
	cgm_startup_stats->stats_type = startupSection;
	cgm_parallel_stats->stats_type = parallelSection;
	cgm_wrapup_stats->stats_type = wrapupSection;


	return;
}

void cgm_stats_alloc(struct cgm_stats_t *cgm_stat_container){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	//int gpu_group_cache_num = (num_cus/4);

	/*configure data structures that are arrays*/

	cgm_stat_container->state = not_consolidated;

	cgm_stat_container->core_idle_time = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->core_total_busy = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_drain_time  = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->core_total_stalls = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->core_lsq_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_lsq_stall_load = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_lsq_stall_store = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_iq_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_syscall_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_rename_stalls = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->core_num_syscalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_syscall_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_rob_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_rob_stall_load = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_rob_stall_store = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_stall_syscall = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_rob_stall_other = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_first_fetch_cycle = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_fetch_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_last_commit_cycle = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_issued_memory_insts = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_commited_memory_insts = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_bytes_rx = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->core_bytes_tx = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->cu_total_busy = (long long *)calloc(num_cus, sizeof(long long));
	cgm_stat_container->cu_total_stalls = (long long *)calloc(num_cus, sizeof(long long));
	cgm_stat_container->cu_total_mapped = (long long *)calloc(num_cus, sizeof(long long));
	cgm_stat_container->cu_total_unmapped = (long long *)calloc(num_cus, sizeof(long long));

	cgm_stat_container->l1_i_Occupancy = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_CacheUtilization = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalAdvances = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalAcesses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_WbMerges = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_MergeRetries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_EvictInv = (long long *)calloc(num_cores, sizeof(long long));
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
	cgm_stat_container->l1_i_UpgradeMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_retries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_CoalescePut = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_CoalesceGet = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_mshr_entries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_Stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalWriteBackRecieved = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalWriteBackSent = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalWriteBackDropped = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalDowngrades = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalGetxFwdInvals = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalUpgradeAcks = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalUpgradeInvals = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_i_TotalWriteBlocks = (long long *)calloc(num_cores, sizeof(long long));



	cgm_stat_container->l1_d_Occupancy = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_CacheUtilization = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalAdvances = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalAcesses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_WbMerges = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_MergeRetries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_EvictInv = (long long *)calloc(num_cores, sizeof(long long));
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
	cgm_stat_container->l1_d_UpgradeMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_retries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_CoalescePut = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_CoalesceGet = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_mshr_entries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_Stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalWriteBackRecieved = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalWriteBackSent = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalWriteBackDropped = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalDowngrades = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalGetxFwdInvals = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalUpgradeAcks = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalUpgradeInvals = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_TotalWriteBlocks = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->l2_Occupancy = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_CacheUtilization = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalAdvances = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalAcesses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_WbMerges = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_MergeRetries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_EvictInv = (long long *)calloc(num_cores, sizeof(long long));
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
	cgm_stat_container->l2_UpgradeMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_retries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_CoalescePut = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_CoalesceGet = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_mshr_entries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_Stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalWriteBackRecieved = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalWriteBackSent = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalWriteBackDropped = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalDowngrades = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalGetxFwdInvals = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalUpgradeAcks = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalUpgradeInvals = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_TotalWriteBlocks = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_LoadMisses = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->l2_gets_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_get_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_getx_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_write_back_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_write_block_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_downgrade_ack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_get_nack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_getx_nack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_get_fwd_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_downgrade_nack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_getx_fwd_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_getx_fwd_inval_ack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_getx_fwd_nack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_upgrade_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_upgrade_ack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_upgrade_nack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_upgrade_putx_n_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_upgrade_inval_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_flush_block_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_flush_block_ack_ = (long long *)calloc(num_cores, sizeof(long long));


	cgm_stat_container->l3_Occupancy = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_CacheUtilization = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalAdvances = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalAcesses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_WbMerges = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_MergeRetries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_EvictInv = (long long *)calloc(num_cores, sizeof(long long));
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
	cgm_stat_container->l3_UpgradeMisses = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_retries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_CoalescePut = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_CoalesceGet = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_mshr_entries = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_Stalls = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalWriteBackRecieved = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalWriteBackSent = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalSharingWriteBackSent = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalWriteBackDropped = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalDowngrades = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalGetxFwdInvals = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalUpgradeAcks = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalUpgradeInvals = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_TotalWriteBlocks = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->l3_gets_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_get_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_getx_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_write_back_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_flush_block_ack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_write_block_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_downgrade_ack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_downgrade_nack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_getx_fwd_ack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_getx_fwd_nack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_getx_fwd_upgrade_nack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_get_fwd_upgrade_nack_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_upgrade_ = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_upgrade_ack_ = (long long *)calloc(num_cores, sizeof(long long));


	cgm_stat_container->switch_occupance = (long long *)calloc(num_cores + 1, sizeof(long long));

	cgm_stat_container->switch_total_links = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_max_links = (int *)calloc(num_cores + 1, sizeof(int));
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


	//IO Controllers
	cgm_stat_container->l1_i_down_io_occupance = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l1_d_down_io_occupance = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_up_io_occupance = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l2_down_io_occupance = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_up_io_occupance = (long long *)calloc(num_cores, sizeof(long long));
	cgm_stat_container->l3_down_io_occupance = (long long *)calloc(num_cores, sizeof(long long));

	cgm_stat_container->switch_north_io_occupance = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_east_io_occupance = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_south_io_occupance = (long long *)calloc(num_cores + 1, sizeof(long long));
	cgm_stat_container->switch_west_io_occupance = (long long *)calloc(num_cores + 1, sizeof(long long));

	return;
}

void init_cpu_gpu_stats(void){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int i = 0;

	/*configure data structures*/

	cpu_gpu_stats->core_idle_time = (long long *)calloc(num_cores, sizeof(long long));

	cpu_gpu_stats->core_first_fetch_cycle = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_fetch_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_last_commit_cycle = (long long *)calloc(num_cores, sizeof(long long));

	cpu_gpu_stats->core_total_busy = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_drain_time = (long long *)calloc(num_cores, sizeof(long long));

	cpu_gpu_stats->core_total_stalls = (long long *)calloc(num_cores, sizeof(long long));

	cpu_gpu_stats->core_rob_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_rob_stall_load = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_rob_stall_store = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_rob_stall_other = (long long *)calloc(num_cores, sizeof(long long));


	cpu_gpu_stats->core_num_syscalls = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_num_fences = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_stall_syscall = (long long *)calloc(num_cores, sizeof(long long));

	cpu_gpu_stats->core_lsq_stalls = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_lsq_stall_load = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_lsq_stall_store = (long long *)calloc(num_cores, sizeof(long long));

	cpu_gpu_stats->core_iq_stalls = (long long *)calloc(num_cores, sizeof(long long));

	cpu_gpu_stats->core_rename_stalls = (long long *)calloc(num_cores, sizeof(long long));

	cpu_gpu_stats->core_issued_memory_insts = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_commited_memory_insts = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_bytes_rx = (long long *)calloc(num_cores, sizeof(long long));
	cpu_gpu_stats->core_bytes_tx = (long long *)calloc(num_cores, sizeof(long long));

	/*GPU*/
	cpu_gpu_stats->gpu_total_cycles = 0;
	cpu_gpu_stats->cu_total_busy = (long long *)calloc(num_cus, sizeof(long long));
	cpu_gpu_stats->cu_total_stalls = (long long *)calloc(num_cus, sizeof(long long));
	cpu_gpu_stats->cu_total_mapped = (long long *)calloc(num_cus, sizeof(long long));
	cpu_gpu_stats->cu_total_unmapped = (long long *)calloc(num_cus, sizeof(long long));


	cpu_gpu_stats->bandwidth = (void *)calloc(num_cores, sizeof(struct list_t *));
	for(i = 0; i < num_cores; i ++)
		cpu_gpu_stats->bandwidth[i] = list_create();

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
		cgm_stat->core_bytes_rx[i] = JOINLL(core_bytes_rx[i]);
		cgm_stat->core_bytes_tx[i] = JOINLL(core_bytes_tx[i]);
	}

	//memory system at large
	//cgm_stat->first_mem_access_lat = mem_system_stats->first_mem_access_lat;

	for(i = 0; i < HISTSIZE; i++)
	{
		cgm_stat->fetch_lat_hist[i] = JOINLL(fetch_lat_hist[i]);
		cgm_stat->load_lat_hist[i] = JOINLL(load_lat_hist[i]);
		cgm_stat->store_lat_hist[i] = JOINLL(store_lat_hist[i]);
	}

	cgm_stat->cpu_total_fetch_requests = JOINLL(cpu_total_fetch_requests);
	cgm_stat->cpu_total_fetch_replys = JOINLL(cpu_total_fetch_replys);
	cgm_stat->fetch_l1_hits = JOINLL(fetch_l1_hits);
	cgm_stat->fetch_l2_hits = JOINLL(fetch_l2_hits);
	cgm_stat->l2_total_fetch_requests = JOINLL(l2_total_fetch_requests);
	cgm_stat->fetch_l3_hits = JOINLL(fetch_l3_hits);
	cgm_stat->l3_total_fetch_requests = JOINLL(l3_total_fetch_requests);
	cgm_stat->fetch_memory = JOINLL(fetch_memory);

	cgm_stat->cpu_total_load_requests = JOINLL(cpu_total_load_requests);
	cgm_stat->cpu_total_load_replys = JOINLL(cpu_total_load_replys);
	cgm_stat->load_l1_hits = JOINLL(load_l1_hits);
	cgm_stat->l2_total_load_requests = JOINLL(l2_total_load_requests);
	cgm_stat->load_l2_hits = JOINLL(load_l2_hits);
	cgm_stat->l3_total_load_requests = JOINLL(l3_total_load_requests);
	cgm_stat->load_l3_hits = JOINLL(load_l3_hits);
	cgm_stat->load_memory = JOINLL(load_memory);
	cgm_stat->load_get_fwd = JOINLL(load_get_fwd);
	cgm_stat->l2_load_nack = JOINLL(l2_load_nack);
	cgm_stat->l3_load_nack = JOINLL(l3_load_nack);

	cgm_stat->cpu_total_store_requests = JOINLL(cpu_total_store_requests);
	cgm_stat->cpu_total_store_replys = JOINLL(cpu_total_store_replys);
	cgm_stat->store_l1_hits = JOINLL(store_l1_hits);
	cgm_stat->l2_total_store_requests = JOINLL(l2_total_store_requests);
	cgm_stat->store_l2_hits = JOINLL(store_l2_hits);
	cgm_stat->l3_total_store_requests = JOINLL(l3_total_store_requests);
	cgm_stat->store_l3_hits = JOINLL(store_l3_hits);
	cgm_stat->store_memory = JOINLL(store_memory);
	cgm_stat->store_getx_fwd = JOINLL(store_getx_fwd);
	cgm_stat->store_upgrade = JOINLL(store_upgrade);
	cgm_stat->l2_store_nack = JOINLL(l2_store_nack);
	cgm_stat->l3_store_nack = JOINLL(l3_store_nack);

	//caches
	for(i = 0; i < num_cores; i++)
	{
		cgm_stat->l1_i_Occupancy[i] = JOINLL(l1_i_Occupancy[i]);
		cgm_stat->l1_i_CoalescePut[i] = JOINLL(l1_i_CoalescePut[i]);
		cgm_stat->l1_i_CoalesceGet[i] = JOINLL(l1_i_CoalesceGet[i]);
		cgm_stat->l1_i_TotalHits[i] = JOINLL(l1_i_TotalHits[i]);
		cgm_stat->l1_i_TotalMisses[i] = JOINLL(l1_i_TotalMisses[i]);
		cgm_stat->l1_i_WbMerges[i] = JOINLL(l1_i_WbMerges[i]);
		cgm_stat->l1_i_MergeRetries[i] = JOINLL(l1_i_MergeRetries[i]);
		cgm_stat->l1_i_EvictInv[i] = JOINLL(l1_i_EvictInv[i]);
		cgm_stat->l1_i_TotalWriteBackRecieved[i] = JOINLL(l1_i_TotalWriteBackRecieved[i]);
		cgm_stat->l1_i_TotalWriteBackSent[i] = JOINLL(l1_i_TotalWriteBackSent[i]);
		cgm_stat->l1_i_TotalWriteBackDropped[i] = JOINLL(l1_i_TotalWriteBackDropped[i]);

		cgm_stat->l1_i_TotalDowngrades[i] = JOINLL(l1_i_TotalDowngrades[i]);
		cgm_stat->l1_i_TotalGetxFwdInvals[i] = JOINLL(l1_i_TotalGetxFwdInvals[i]);
		cgm_stat->l1_i_TotalUpgradeAcks[i] = JOINLL(l1_i_TotalUpgradeAcks[i]);
		cgm_stat->l1_i_TotalUpgradeInvals[i] = JOINLL(l1_i_TotalUpgradeInvals[i]);
		cgm_stat->l1_i_TotalWriteBlocks[i] = JOINLL(l1_i_TotalWriteBlocks[i]);

		cgm_stat->l1_i_TotalAdvances[i] = JOINLL(l1_i_TotalAdvances[i]);
		cgm_stat->l1_i_TotalAcesses[i] = JOINLL(l1_i_TotalAcesses[i]);
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
		cgm_stat->l1_i_UpgradeMisses[i] = JOINLL(l1_i_UpgradeMisses[i]);
		cgm_stat->l1_i_retries[i] = JOINLL(l1_i_retries[i]);
		cgm_stat->l1_i_mshr_entries[i] = JOINLL(l1_i_mshr_entries[i]);
		cgm_stat->l1_i_Stalls[i] = JOINLL(l1_i_Stalls[i]);

		cgm_stat->l1_d_Occupancy[i] = JOINLL(l1_d_Occupancy[i]);
		cgm_stat->l1_d_TotalAdvances[i] = JOINLL(l1_d_TotalAdvances[i]);
		cgm_stat->l1_d_TotalAcesses[i] = JOINLL(l1_d_TotalAcesses[i]);
		cgm_stat->l1_d_WbMerges[i] = JOINLL(l1_d_WbMerges[i]);
		cgm_stat->l1_d_MergeRetries[i] = JOINLL(l1_d_MergeRetries[i]);
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
		cgm_stat->l1_d_UpgradeMisses[i] = JOINLL(l1_d_UpgradeMisses[i]);
		cgm_stat->l1_d_retries[i] = JOINLL(l1_d_retries[i]);
		cgm_stat->l1_d_CoalescePut[i] = JOINLL(l1_d_CoalescePut[i]);
		cgm_stat->l1_d_CoalesceGet[i] = JOINLL(l1_d_CoalesceGet[i]);
		cgm_stat->l1_d_mshr_entries[i] = JOINLL(l1_d_mshr_entries[i]);
		cgm_stat->l1_d_Stalls[i] = JOINLL(l1_d_Stalls[i]);
		cgm_stat->l1_d_EvictInv[i] = JOINLL(l1_d_EvictInv[i]);
		cgm_stat->l1_d_TotalWriteBackRecieved[i] = JOINLL(l1_d_TotalWriteBackRecieved[i]);
		cgm_stat->l1_d_TotalWriteBackSent[i] = JOINLL(l1_d_TotalWriteBackSent[i]);
		cgm_stat->l1_d_TotalWriteBackDropped[i] = JOINLL(l1_d_TotalWriteBackDropped[i]);

		cgm_stat->l1_d_TotalDowngrades[i] = JOINLL(l1_d_TotalDowngrades[i]);
		cgm_stat->l1_d_TotalGetxFwdInvals[i] = JOINLL(l1_d_TotalGetxFwdInvals[i]);
		cgm_stat->l1_d_TotalUpgradeAcks[i] = JOINLL(l1_d_TotalUpgradeAcks[i]);
		cgm_stat->l1_d_TotalUpgradeInvals[i] = JOINLL(l1_d_TotalUpgradeInvals[i]);
		cgm_stat->l1_d_TotalWriteBlocks[i] = JOINLL(l1_d_TotalWriteBlocks[i]);

		cgm_stat->l2_Occupancy[i] = JOINLL(l2_Occupancy[i]);
		cgm_stat->l2_TotalAdvances[i] = JOINLL(l2_TotalAdvances[i]);
		cgm_stat->l2_TotalAcesses[i] = JOINLL(l2_TotalAcesses[i]);
		cgm_stat->l2_WbMerges[i] = JOINLL(l2_WbMerges[i]);
		cgm_stat->l2_MergeRetries[i] = JOINLL(l2_MergeRetries[i]);
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
		cgm_stat->l2_UpgradeMisses[i] = JOINLL(l2_UpgradeMisses[i]);
		cgm_stat->l2_TotalUpgradeAcks[i] = JOINLL(l2_TotalUpgradeAcks[i]);
		cgm_stat->l2_retries[i] = JOINLL(l2_retries[i]);
		cgm_stat->l2_CoalescePut[i] = JOINLL(l2_CoalescePut[i]);
		cgm_stat->l2_CoalesceGet[i] = JOINLL(l2_CoalesceGet[i]);
		cgm_stat->l2_mshr_entries[i] = JOINLL(l2_mshr_entries[i]);
		cgm_stat->l2_Stalls[i] = JOINLL(l2_Stalls[i]);
		cgm_stat->l2_EvictInv[i] = JOINLL(l2_EvictInv[i]);
		cgm_stat->l2_TotalWriteBackRecieved[i] = JOINLL(l2_TotalWriteBackRecieved[i]);
		cgm_stat->l2_TotalWriteBackSent[i] = JOINLL(l2_TotalWriteBackSent[i]);
		cgm_stat->l2_TotalWriteBackDropped[i] = JOINLL(l2_TotalWriteBackDropped[i]);
		cgm_stat->l2_TotalWriteBlocks[i] = JOINLL(l2_TotalWriteBlocks[i]);
		cgm_stat->l2_LoadMisses[i] = JOINLL(l2_LoadMisses[i]);
		cgm_stat->l2_TotalUpgradeInvals[i] = JOINLL(l2_TotalUpgradeInvals[i]);

		cgm_stat->l2_gets_[i] = JOINLL(l2_gets_[i]);
		cgm_stat->l2_get_[i] = JOINLL(l2_get_[i]);
		cgm_stat->l2_getx_[i] = JOINLL(l2_getx_[i]);
		cgm_stat->l2_write_back_[i] = JOINLL(l2_write_back_[i]);
		cgm_stat->l2_write_block_[i] = JOINLL(l2_write_block_[i]);
		cgm_stat->l2_downgrade_ack_[i] = JOINLL(l2_downgrade_ack_[i]);
		cgm_stat->l2_get_nack_[i] = JOINLL(l2_get_nack_[i]);
		cgm_stat->l2_getx_nack_[i] = JOINLL(l2_getx_nack_[i]);
		cgm_stat->l2_get_fwd_[i] = JOINLL(l2_get_fwd_[i]);
		cgm_stat->l2_downgrade_nack_[i] = JOINLL(l2_downgrade_nack_[i]);
		cgm_stat->l2_getx_fwd_[i] = JOINLL(l2_getx_fwd_[i]);
		cgm_stat->l2_getx_fwd_inval_ack_[i] = JOINLL(l2_getx_fwd_inval_ack_[i]);
		cgm_stat->l2_getx_fwd_nack_[i] = JOINLL(l2_getx_fwd_nack_[i]);
		cgm_stat->l2_upgrade_[i] = JOINLL(l2_upgrade_[i]);
		cgm_stat->l2_upgrade_ack_[i] = JOINLL(l2_upgrade_ack_[i]);
		cgm_stat->l2_upgrade_nack_[i] = JOINLL(l2_upgrade_nack_[i]);
		cgm_stat->l2_upgrade_putx_n_[i] = JOINLL(l2_upgrade_putx_n_[i]);
		cgm_stat->l2_upgrade_inval_[i] = JOINLL(l2_upgrade_inval_[i]);
		cgm_stat->l2_flush_block_[i] = JOINLL(l2_flush_block_[i]);
		cgm_stat->l2_flush_block_ack_[i] = JOINLL(l2_flush_block_ack_[i]);

		cgm_stat->l3_Occupancy[i] = JOINLL(l3_Occupancy[i]);
		cgm_stat->l3_TotalAdvances[i] = JOINLL(l3_TotalAdvances[i]);
		cgm_stat->l3_TotalAcesses[i] = JOINLL(l3_TotalAcesses[i]);
		cgm_stat->l3_WbMerges[i] = JOINLL(l3_WbMerges[i]);
		cgm_stat->l3_MergeRetries[i] = JOINLL(l3_MergeRetries[i]);
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
		cgm_stat->l3_UpgradeMisses[i] = JOINLL(l3_UpgradeMisses[i]);
		cgm_stat->l3_retries[i] = JOINLL(l3_retries[i]);
		cgm_stat->l3_CoalescePut[i] = JOINLL(l3_CoalescePut[i]);
		cgm_stat->l3_CoalesceGet[i] = JOINLL(l3_CoalesceGet[i]);
		cgm_stat->l3_mshr_entries[i] = JOINLL(l3_mshr_entries[i]);
		cgm_stat->l3_Stalls[i] = JOINLL(l3_Stalls[i]);
		cgm_stat->l3_EvictInv[i] = JOINLL(l3_EvictInv[i]);
		cgm_stat->l3_TotalWriteBackRecieved[i] = JOINLL(l3_TotalWriteBackRecieved[i]);
		cgm_stat->l3_TotalWriteBackSent[i] = JOINLL(l3_TotalWriteBackSent[i]);
		cgm_stat->l3_TotalSharingWriteBackSent[i] = JOINLL(l3_TotalSharingWriteBackSent[i]);
		cgm_stat->l3_TotalWriteBackDropped[i] = JOINLL(l3_TotalWriteBackDropped[i]);
		cgm_stat->l3_TotalWriteBlocks[i] = JOINLL(l3_TotalWriteBlocks[i]);
		cgm_stat->l3_TotalUpgradeInvals[i] = JOINLL(l3_TotalUpgradeInvals[i]);


		cgm_stat->l3_gets_[i] = JOINLL(l3_gets_[i]);
		cgm_stat->l3_get_[i] = JOINLL(l3_get_[i]);
		cgm_stat->l3_getx_[i] = JOINLL(l3_getx_[i]);
		cgm_stat->l3_write_back_[i] = JOINLL(l3_write_back_[i]);
		cgm_stat->l3_flush_block_ack_[i] = JOINLL(l3_flush_block_ack_[i]);
		cgm_stat->l3_write_block_[i] = JOINLL(l3_write_block_[i]);
		cgm_stat->l3_downgrade_ack_[i] = JOINLL(l3_downgrade_ack_[i]);
		cgm_stat->l3_downgrade_nack_[i] = JOINLL(l3_downgrade_nack_[i]);
		cgm_stat->l3_getx_fwd_ack_[i] = JOINLL(l3_getx_fwd_ack_[i]);
		cgm_stat->l3_getx_fwd_nack_[i] = JOINLL(l3_getx_fwd_nack_[i]);
		cgm_stat->l3_getx_fwd_upgrade_nack_[i] = JOINLL(l3_getx_fwd_upgrade_nack_[i]);
		cgm_stat->l3_get_fwd_upgrade_nack_[i] = JOINLL(l3_get_fwd_upgrade_nack_[i]);
		cgm_stat->l3_upgrade_[i] = JOINLL(l3_upgrade_[i]);
		cgm_stat->l3_upgrade_ack_[i] = JOINLL(l3_upgrade_ack_[i]);

	}

	//switch stats
	for(i = 0; i < (num_cores + 1); i++)
	{
		cgm_stat->switch_total_links[i] = JOINLL(switch_total_links[i]);
		cgm_stat->switch_max_links[i] = JOINMAX(switch_max_links[i]);
		cgm_stat->switch_occupance[i] = JOINLL(switch_occupance[i]);
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

	/*set the state of the stats container*/
	cgm_stat_container->state = consolidated;

	cpu_gpu_store_stats(cgm_stat_container);
	mem_system_store_stats(cgm_stat_container);
	cache_store_stats(cgm_stat_container);
	switch_store_stats(cgm_stat_container);
	sys_agent_store_stats(cgm_stat_container);
	memctrl_store_stats(cgm_stat_container);

	return;
}

void cpu_gpu_store_stats(struct cgm_stats_t *cgm_stat_container){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	//int gpu_group_cache_num = (num_cus/4);
	int i = 0;


	//store cgm_stat_container
	for(i = 0; i < num_cores; i++)
	{
		cgm_stat_container->core_idle_time[i] = cpu_gpu_stats->core_idle_time[i];

		cgm_stat_container->core_total_busy[i] = cpu_gpu_stats->core_total_busy[i];
		cgm_stat_container->core_drain_time[i] = cpu_gpu_stats->core_drain_time[i];

		cgm_stat_container->core_num_syscalls[i] = cpu_gpu_stats->core_num_syscalls[i];
		//cgm_stat_container->core_syscall_stalls[i] = cpu_gpu_stats->core_syscall_stalls[i];
		cgm_stat_container->core_stall_syscall[i] = cpu_gpu_stats->core_stall_syscall[i];
		cgm_stat_container->core_rob_stalls[i] = cpu_gpu_stats->core_rob_stalls[i];
		cgm_stat_container->core_rob_stall_load[i] = cpu_gpu_stats->core_rob_stall_load[i];
		cgm_stat_container->core_rob_stall_store[i] = cpu_gpu_stats->core_rob_stall_store[i];
		cgm_stat_container->core_rob_stall_other[i] = cpu_gpu_stats->core_rob_stall_other[i];
		cgm_stat_container->core_first_fetch_cycle[i] = cpu_gpu_stats->core_first_fetch_cycle[i];
		cgm_stat_container->core_fetch_stalls[i] = cpu_gpu_stats->core_fetch_stalls[i];
		cgm_stat_container->core_last_commit_cycle[i] = cpu_gpu_stats->core_last_commit_cycle[i];
		cgm_stat_container->core_issued_memory_insts[i] = cpu_gpu_stats->core_issued_memory_insts[i];
		cgm_stat_container->core_commited_memory_insts[i] = cpu_gpu_stats->core_commited_memory_insts[i];
		cgm_stat_container->core_bytes_rx[i] = cpu_gpu_stats->core_bytes_rx[i];
		cgm_stat_container->core_bytes_tx[i] = cpu_gpu_stats->core_bytes_tx[i];

		//new stats
		cgm_stat_container->core_total_stalls[i] = cpu_gpu_stats->core_total_stalls[i];
		cgm_stat_container->core_lsq_stalls[i] = cpu_gpu_stats->core_lsq_stalls[i];

		cgm_stat_container->core_lsq_stall_load[i] = cpu_gpu_stats->core_lsq_stall_load[i];
		cgm_stat_container->core_lsq_stall_store[i] = cpu_gpu_stats->core_lsq_stall_store[i];

		cgm_stat_container->core_iq_stalls[i] = cpu_gpu_stats->core_iq_stalls[i];

		cgm_stat_container->core_rename_stalls[i] = cpu_gpu_stats->core_rename_stalls[i];
	}

	cgm_stat_container->systemcall_total_cycles = cpu_gpu_stats->systemcall_total_cycles;
	cgm_stat_container->systemcall_total_rob_stalls = cpu_gpu_stats->systemcall_total_rob_stalls;
	cgm_stat_container->gpu_total_cycles = cpu_gpu_stats->gpu_total_cycles;
	cgm_stat_container->gpu_idle_cycles = cpu_gpu_stats->gpu_idle_cycles;

	for(i = 0; i < num_cus; i++)
	{
		cgm_stat_container->cu_total_busy[i] = cpu_gpu_stats->cu_total_busy[i];
		cgm_stat_container->cu_total_stalls[i] = cpu_gpu_stats->cu_total_stalls[i];
		cgm_stat_container->cu_total_mapped[i] = cpu_gpu_stats->cu_total_mapped[i];
		cgm_stat_container->cu_total_unmapped[i] = cpu_gpu_stats->cu_total_unmapped[i];
	}

	return;
}

void cgm_reset_stats(void){

	cpu_gpu_reset_stats();
	mem_system_reset_stats();
	cache_reset_stats();
	switch_reset_stats();
	sys_agent_reset_stats();
	memctrl_reset_stats();

	return;
}

void cpu_gpu_reset_stats(void){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	//int gpu_group_cache_num = (num_cus/4);
	int i = 0;

	//reset cgm_stat
	for(i = 0; i < num_cores; i++)
	{

		cpu_gpu_stats->core_idle_time[i] = 0;

		cpu_gpu_stats->core_total_busy[i] = 0;
		cpu_gpu_stats->core_drain_time[i] = 0;

		cpu_gpu_stats->core_num_syscalls[i] = 0;
		cpu_gpu_stats->core_stall_syscall[i] = 0;
		cpu_gpu_stats->core_rob_stalls[i] = 0;
		cpu_gpu_stats->core_rob_stall_load[i] = 0;
		cpu_gpu_stats->core_rob_stall_store[i] = 0;
		cpu_gpu_stats->core_rob_stall_other[i] = 0;
		cpu_gpu_stats->core_first_fetch_cycle[i] = 0;
		cpu_gpu_stats->core_fetch_stalls[i] = 0;
		cpu_gpu_stats->core_last_commit_cycle[i] = 0;
		cpu_gpu_stats->core_issued_memory_insts[i] = 0;
		cpu_gpu_stats->core_commited_memory_insts[i] = 0;
		cpu_gpu_stats->core_bytes_rx[i] = 0;
		cpu_gpu_stats->core_bytes_tx[i] = 0;

		//new stats
		//new stats
		cpu_gpu_stats->core_total_stalls[i] = 0;
		cpu_gpu_stats->core_lsq_stalls[i] = 0;

		cpu_gpu_stats->core_lsq_stall_load[i] = 0;
		cpu_gpu_stats->core_lsq_stall_store[i] = 0;

		cpu_gpu_stats->core_iq_stalls[i] = 0;

		cpu_gpu_stats->core_rename_stalls[i] = 0;

	}

	cpu_gpu_stats->gpu_idle_cycles = 0;
	cpu_gpu_stats->gpu_total_cycles = 0;
	cpu_gpu_stats->systemcall_total_cycles = 0;
	cpu_gpu_stats->systemcall_total_rob_stalls = 0;

	for(i = 0; i < num_cus; i++)
	{
		cpu_gpu_stats->cu_total_busy[i] = 0;
		cpu_gpu_stats->cu_total_stalls[i] = 0;
		cpu_gpu_stats->cu_total_mapped[i] = 0;
		cpu_gpu_stats->cu_total_unmapped[i] = 0;
	}

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

void cgm_configure(void){

	int error = 0;

	error = cgm_mem_configure();
	if (error) {fatal("cgm_mem_configure() failed\n");}

	cgm_cpu_configure();

#if GPU
	cgm_gpu_configure();
#endif


	fflush(stdout);
	fflush(stderr);

	/*print errors or warnings if needed*/
	if(SINGLE_CORE == 1)
	{
		warning("All threads allocated to a single core check SINGLE_CORE\n");
	}

	fflush(stderr);


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


#define CACHEQUEUES(cache, size) 	if(list_count(cache.Rx_queue_top) > size || 		\
									list_count(cache.Rx_queue_bottom) > size || 		\
									list_count(cache.Tx_queue_top) > size || 			\
									list_count(cache.Tx_queue_bottom) > size || 		\
									list_count(cache.Coherance_Tx_queue) > size || 		\
									list_count(cache.Coherance_Rx_queue) > size || 		\
									list_count(cache.retry_queue) > size || 			\
									list_count(cache.write_back_buffer) > size || 		\
									list_count(cache.pending_request_buffer) > size)	\
									{													\
										warning("WD: %s exceeded a queue Rx_T %d Rx_b %d Tx_t %d Tx_b %d C_Tx %d C_Rx %d retry %d write %d pending %d\n",	\
											cache.name, list_count(cache.Rx_queue_top), list_count(cache.Rx_queue_bottom),									\
														list_count(cache.Tx_queue_top),	list_count(cache.Tx_queue_bottom),									\
														list_count(cache.Coherance_Tx_queue), list_count(cache.Coherance_Rx_queue),							\
														list_count(cache.retry_queue), list_count(cache.write_back_buffer),									\
														list_count(cache.pending_request_buffer));															\
									}


#define SWITCHQUEUES(switches, size) 	if(list_count(switches.north_queue) > size || 	\
									list_count(switches.Tx_north_queue) > size || 		\
									list_count(switches.east_queue) > size || 			\
									list_count(switches.Tx_east_queue) > size || 		\
									list_count(switches.south_queue) > size || 			\
									list_count(switches.Tx_south_queue) > size || 		\
									list_count(switches.west_queue) > size || 			\
									list_count(switches.Tx_west_queue) > size) 			\
									{													\
										warning("WD: %s exceeded a queue n %d tx_n %d e %d tx_e %d s %d tx_s %d w %d tx_w %d\n",	\
											switches.name, list_count(switches.north_queue), list_count(switches.Tx_north_queue),	\
													list_count(switches.east_queue), list_count(switches.Tx_east_queue),			\
													list_count(switches.south_queue), list_count(switches.Tx_south_queue),			\
													list_count(switches.west_queue), list_count(switches.Tx_west_queue));			\
									}


#define SYSTEMAGENTQUEUES(systemagent, size) 	if(list_count(systemagent->Rx_queue_top) > size || 	\
									list_count(systemagent->Rx_queue_bottom) > size || 				\
									list_count(systemagent->Tx_queue_top) > size || 				\
									list_count(systemagent->Tx_queue_bottom) > size) 				\
									{																\
										warning("WD: %s exceeded a queue rx_t %d rx_b %d tx_t %d tx_b %d\n",									\
											systemagent->name, list_count(systemagent->Rx_queue_top), list_count(systemagent->Rx_queue_bottom),	\
													list_count(systemagent->Tx_queue_top), list_count(systemagent->Tx_queue_bottom));			\
									}


#define MEMCTRLQUEUES(memctrl, size) 	if(list_count(memctrl->Rx_queue_top) > size || 			\
									list_count(memctrl->Tx_queue_top) > size || 				\
									list_count(memctrl->pending_accesses) > 32) 				\
									{															\
										warning("WD: %s exceeded a queue rx_t %d tx_t %d pending %d\n",																	\
											memctrl->name, list_count(memctrl->Rx_queue_top), list_count(memctrl->Tx_queue_top), list_count(memctrl->pending_accesses));\
									}


void cgm_watchdog(void){

	long long t_1 = 1;

	int num_cores = x86_cpu_num_cores;
	int i = 0;

	while(1)
	{
		await(watchdog, t_1);
		t_1++;

		//watch all queue depths....

		//caches
		for(i=0; i< num_cores; i++)
		{
			CACHEQUEUES(l1_i_caches[i], QueueSize)
			CACHEQUEUES(l1_d_caches[i], QueueSize)
			CACHEQUEUES(l2_caches[i], QueueSize)
			CACHEQUEUES(l3_caches[i], QueueSize)
		}

		//switches
		for(i=0; i<=num_cores; i++)
		{
			SWITCHQUEUES(switches[i], QueueSize);
		}

		SYSTEMAGENTQUEUES(system_agent, QueueSize)

		MEMCTRLQUEUES(mem_ctrl, QueueSize)

	}
	return;
}

void cgm_dump_general_stats(void){

	int num_cores = x86_cpu_num_cores;
	int num_threads = x86_cpu_num_threads;
	int num_cus = si_gpu_num_compute_units;

	double cpu_sim_time = 0;
	unsigned int cpu_freq_hz = 0;
	unsigned int gpu_freq_hz = 0;

	//int i = 0;

	//get the time
	cgm_stat->end_wall_time = get_wall_time();

	//calculate simulation runtime (wall clock)
	cgm_stat->sim_time = (cgm_stat->end_wall_time - cgm_stat->start_wall_time);

	cpu_freq_hz = (unsigned int) x86_cpu_frequency * (unsigned int) MHZ;
	cpu_sim_time = (double) P_TIME / (double) (cpu_freq_hz);

	gpu_freq_hz = (unsigned int) si_gpu_frequency * (unsigned int) MHZ;

	/* General statistics */
	CGM_STATS(cgm_stats_file, "[General]\n");
	CGM_STATS(cgm_stats_file, "ExecutionSuccessful = %s\n", (cgm_stat->execution_success == true) ? ("Yes") : ("No"));
	CGM_STATS(cgm_stats_file, "CheckPoint = %s\n", (x86_load_checkpoint_file_name[0]) ? ("Yes") : ("No"));
	CGM_STATS(cgm_stats_file, "Benchmark = %s\n", cgm_stat->benchmark_name);
	CGM_STATS(cgm_stats_file, "Args = %s\n", cgm_stat->args);
	CGM_STATS(cgm_stats_file, "Day&Time = %s\n", cgm_stat->date_time_pretty);
	CGM_STATS(cgm_stats_file, "SimulationRunTimeSeconds(cpu) = %.9f\n", cpu_sim_time);
	CGM_STATS(cgm_stats_file, "SimulationRunTimeSeconds(wall) = %.2f\n", cgm_stat->sim_time);
	CGM_STATS(cgm_stats_file, "SimulatedCyclesPerSec = %.2f\n", (double)P_TIME/cgm_stat->sim_time);
	CGM_STATS(cgm_stats_file, "TotalCycles = %lld\n", P_TIME);
	CGM_STATS(cgm_stats_file, "ParallelSectionCycles = %lld\n", (cgm_parallel_stats->total_parallel_section_cycles > 0) ? cgm_parallel_stats->total_parallel_section_cycles : (long long) 0);
	CGM_STATS(cgm_stats_file, "CPU_NumCores = %d\n", num_cores);
	CGM_STATS(cgm_stats_file, "CPU_ThreadsPerCore = %d\n", num_threads);
	CGM_STATS(cgm_stats_file, "CPU_FreqGHz = %u\n", (cpu_freq_hz)/(GHZ));
	CGM_STATS(cgm_stats_file, "GPU_NumCUs = %u\n", num_cus);
	CGM_STATS(cgm_stats_file, "GPU_FreqGHz = %u\n", (gpu_freq_hz)/(GHZ));
	CGM_STATS(cgm_stats_file, "GPU_CycleFactor = %u\n", (1 * (x86_cpu_frequency/si_gpu_frequency)));
	CGM_STATS(cgm_stats_file, "Mem_FreqGHz = %u\n", ((cpu_freq_hz)/(SYSTEM_LATENCY_FACTOR))/(GHZ));
	CGM_STATS(cgm_stats_file, "Mem_LatFactor = %u\n", SYSTEM_LATENCY_FACTOR);
	CGM_STATS(cgm_stats_file, "Config_Single_Core = %s\n", (SINGLE_CORE == 0) ? "No" : "Yes");
	CGM_STATS(cgm_stats_file, "Config_CPUProtocoltype = %s\n", (cgm_cache_protocol == cgm_protocol_mesi) ? "MESI" : "NC");
	CGM_STATS(cgm_stats_file, "Config_GPUProtocoltype = %s\n", (cgm_gpu_cache_protocol == cgm_protocol_mesi) ? "MESI" : "NC");
	CGM_STATS(cgm_stats_file, "Config_GPUConnectType = %s\n", (hub_iommu_connection_type == hub_to_mc) ? "MC" : "L3");
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

#include <arch/x86/timing/dispatch.h>

void cgm_dump_cpu_gpu_stats(struct cgm_stats_t *cgm_stat_container){

	int num_cores = x86_cpu_num_cores;
	//int num_threads = x86_cpu_num_threads;
	int num_cus = si_gpu_num_compute_units;
	int i = 0;
	//long long run_time = 0;
	//long long idle_time = 0;
	//long long busy_time = 0;
	//long long stall_time = 0;
	//long long system_time = 0;
	long long exe_time = 0;
	long long gpu_exe_time = 0;

	/*core stats*/
	for(i = 0; i < num_cores; i++)
	{
		/*CGM_STATS(cgm_stats_file, "[Core_%d]\n", i);*/


		//printf("total cpu dispatches %llu\n", total_dispatches);

		exe_time = cgm_stat_container->core_total_busy[i] + cgm_stat_container->core_idle_time[i] \
				+ cgm_stat_container->core_total_stalls[i]; // + cgm_stat_container->gpu_total_cycles;
		CGM_STATS(cgm_stats_file, "core_%d_ExeTime = %llu\n", i, exe_time);

		CGM_STATS(cgm_stats_file, "core_%d_TotalBusy = %llu\n", i, cgm_stat_container->core_total_busy[i]);

		CGM_STATS(cgm_stats_file, "core_%d_DrainTime = %llu\n", i, cgm_stat_container->core_drain_time[i]);

		CGM_STATS(cgm_stats_file, "core_%d_IdleTime = %llu\n", i, cgm_stat_container->core_idle_time[i]);

		CGM_STATS(cgm_stats_file, "core_%d_TotalStalls = %llu\n", i, cgm_stat_container->core_total_stalls[i]);

		CGM_STATS(cgm_stats_file, "core_%d_StallFetch = %llu\n", i, cgm_stat_container->core_fetch_stalls[i]);

		CGM_STATS(cgm_stats_file, "core_%d_ROBStalls = %llu\n", i, cgm_stat_container->core_rob_stalls[i]);
		CGM_STATS(cgm_stats_file, "core_%d_ROBStallLoad = %llu\n", i, cgm_stat_container->core_rob_stall_load[i]);
		CGM_STATS(cgm_stats_file, "core_%d_ROBStallStore = %llu\n", i, cgm_stat_container->core_rob_stall_store[i]);
		CGM_STATS(cgm_stats_file, "core_%d_ROBStallOther = %llu\n", i, cgm_stat_container->core_rob_stall_other[i]);

		CGM_STATS(cgm_stats_file, "core_%d_LSQStalls = %llu\n", i, cgm_stat_container->core_lsq_stalls[i]);
		CGM_STATS(cgm_stats_file, "core_%d_LSQStallLoad = %llu\n", i, cgm_stat_container->core_lsq_stall_load[i]);
		CGM_STATS(cgm_stats_file, "core_%d_LSQStallStore = %llu\n", i, cgm_stat_container->core_lsq_stall_store[i]);

		CGM_STATS(cgm_stats_file, "core_%d_IQStalls = %llu\n", i, cgm_stat_container->core_iq_stalls[i]);

		CGM_STATS(cgm_stats_file, "core_%d_RenameStalls = %llu\n", i, cgm_stat_container->core_rename_stalls[i]);

		CGM_STATS(cgm_stats_file, "core_%d_NumSyscalls = %llu\n", i, cgm_stat_container->core_num_syscalls[i]);
		CGM_STATS(cgm_stats_file, "core_%d_StallSyscall = %llu\n", i, cgm_stat_container->core_stall_syscall[i]);

		CGM_STATS(cgm_stats_file, "core_%d_SystemTimePct = %0.6f\n", i, (double)exe_time/(double)cgm_stat_container->core_stall_syscall[i]);
		CGM_STATS(cgm_stats_file, "core_%d_StallTimePct = %0.6f\n", i, (double)exe_time/(double)cgm_stat_container->core_total_stalls[i]);
		CGM_STATS(cgm_stats_file, "core_%d_BusyTimePct = %0.6f\n", i, (double)exe_time/(double)cgm_stat_container->core_total_busy[i]);


		if(cgm_stat_container->stats_type == systemStats)
		{

			/*CGM_STATS(cgm_stats_file, "core_%d_FirstFetchCycle = %llu\n", i,  cgm_stat_container->core_first_fetch_cycle[i]);
			CGM_STATS(cgm_stats_file, "core_%d_LastCommitCycle = %llu\n", i, cgm_stat_container->core_last_commit_cycle[i]);

			//CGM_STATS(cgm_stats_file, "NumIssuedMemoryInst = %llu\n", cgm_stat_container->core_issued_memory_insts[i]);
			//CGM_STATS(cgm_stats_file, "NumCommitedMemoryInst = %llu\n", cgm_stat_container->core_commited_memory_insts[i]);
			run_time = cgm_stat_container->core_last_commit_cycle[i] - cgm_stat_container->core_first_fetch_cycle[i];

			warning("run_time %llu parallel_time %llu\n", run_time, cgm_stat_container->total_parallel_section_cycles);

			CGM_STATS(cgm_stats_file, "core_%d_RunTime = %llu\n", i, run_time);

			idle_time = P_TIME - run_time;
			CGM_STATS(cgm_stats_file, "core_%d_IdleTime = %llu\n", i, idle_time);

			system_time = cgm_stat_container->core_syscall_stalls[i];
			CGM_STATS(cgm_stats_file, "core_%d_SystemTime = %llu\n", i, system_time);

			stall_time = (cgm_stat_container->core_rob_stalls[i] + cgm_stat_container->core_fetch_stalls[i]);
			CGM_STATS(cgm_stats_file, "core_%d_StallTime = %llu\n", i, stall_time);

			busy_time = (run_time - (stall_time + system_time));
			CGM_STATS(cgm_stats_file, "core_%d_BusyTime = %llu\n", i, busy_time);

			CGM_STATS(cgm_stats_file, "core_%d_IdlePct = %0.2f\n", i, (double)idle_time/(double)P_TIME);
			CGM_STATS(cgm_stats_file, "core_%d_RunPct = %0.2f\n", i, (double)run_time/(double)P_TIME);*/
		}
		else if (cgm_stat_container->stats_type == parallelSection)
		{

			//CGM_STATS(cgm_stats_file, "core_%d_FirstFetchCycle = %llu\n", i, (long long) 0);
			//CGM_STATS(cgm_stats_file, "core_%d_LastCommitCycle = %llu\n", i, (long long) 0);

			//run_time = cgm_stat_container->total_parallel_section_cycles;
			//CGM_STATS(cgm_stats_file, "core_%d_RunTime = %llu\n", i, run_time);

			//idle_time = 0;
			//CGM_STATS(cgm_stats_file, "core_%d_IdleTime = %llu\n", i, idle_time);

			//system_time = cgm_stat_container->core_stall_syscall[i];
			//CGM_STATS(cgm_stats_file, "core_%d_SystemTime = %llu\n", i, system_time);

			//stall_time = cgm_stat_container->core_total_stalls[i] - cgm_stat_container->core_stall_syscall[i];
			//CGM_STATS(cgm_stats_file, "core_%d_StallTime = %llu\n", i, stall_time);

			//busy_time = run_time - cgm_stat_container->core_total_stalls[i];
			//CGM_STATS(cgm_stats_file, "core_%d_BusyTime = %llu\n", i, busy_time);

			//CGM_STATS(cgm_stats_file, "core_%d_IdlePct = %0.6f\n", i, (double)idle_time/(double)run_time);
			//CGM_STATS(cgm_stats_file, "core_%d_RunPct = %0.6f\n", i, (double)run_time/(double)run_time);
		}
		else
		{
			fatal("cgm_dump_cpu_gpu_stats(): not set up for these stats yet\n");
		}

		//CGM_STATS(cgm_stats_file, "core_%d_StallfetchPct = %0.2f\n", i, (double)cgm_stat_container->core_fetch_stalls[i]/(double)stall_time);
		//CGM_STATS(cgm_stats_file, "core_%d_StallLoadPct = %0.2f\n", i, (double)cgm_stat_container->core_rob_stall_load[i]/(double)stall_time);
		//CGM_STATS(cgm_stats_file, "core_%d_StallStorePct = %0.2f\n", i, (double)cgm_stat_container->core_rob_stall_store[i]/(double)stall_time);
		//CGM_STATS(cgm_stats_file, "StallSyscallPct = %0.2f\n", (double)cgm_stat_container->core_rob_stall_syscall[i]/(double)stall_time);
		//CGM_STATS(cgm_stats_file, "core_%d_StallOtherPct = %0.2f\n", i, (double)cgm_stat_container->core_rob_stall_other[i]/(double)stall_time);
		/*CGM_STATS(cgm_stats_file, "\n");*/
	}


	gpu_exe_time = cgm_stat_container->gpu_idle_cycles + cgm_stat_container->gpu_total_cycles;
	CGM_STATS(cgm_stats_file, "GPU_ExeTime = %lld\n", gpu_exe_time);
	CGM_STATS(cgm_stats_file, "GPU_GPURunTime = %lld\n", cgm_stat_container->gpu_total_cycles);
	CGM_STATS(cgm_stats_file, "GPU_GPUIdleTime = %lld\n", cgm_stat_container->gpu_idle_cycles);

	for(i = 0; i < num_cus; i++)
	{
		CGM_STATS(cgm_stats_file, "GPU_%d_TotalBusy = %llu\n", i, cgm_stat_container->cu_total_busy[i]);
		CGM_STATS(cgm_stats_file, "GPU_%d_TotalStalls = %llu\n", i, cgm_stat_container->cu_total_stalls[i]);
		CGM_STATS(cgm_stats_file, "GPU_%d_TotalMapped = %llu\n", i, cgm_stat_container->cu_total_mapped[i]);
		CGM_STATS(cgm_stats_file, "GPU_%d_TotalUnMapped = %llu\n", i, cgm_stat_container->cu_total_unmapped[i]);
	}

	return;
}

void cgm_dump_bandwidth(void){

	int i = 0;
	int j = 0;
	int num_cores = x86_cpu_num_cores;
	int num_active_cores = 0;
	struct mem_system_bandwidth_t * bandwidth = NULL;

	int num_epochs = list_count(cpu_gpu_stats->bandwidth[0]);
	int epoch_size = EPOCH;

	FILE *bandwidth_log_file = fopen ("/home/stardica/Desktop/m2s-cgm/src/scripts/bandwidth_log_file.out", "w+");

	//get number of active cores
	for(i = 0; i < num_cores; i++)
	{
		if(list_count(cpu_gpu_stats->bandwidth[i]) > 0)
		{
			num_active_cores++;
		}
	}

	/*print epochs*/
	fprintf(bandwidth_log_file, "%d\n", num_active_cores);
	fprintf(bandwidth_log_file, "%llu\n", P_TIME);
	/*max theoretical bandwidth for the epoch
	size of the epoch, adjust for memsystem latency factor times the bus width*/
	fprintf(bandwidth_log_file, "%llu\n", (long long) (EPOCH/SYSTEM_LATENCY_FACTOR)*switches[0].bus_width);
	fprintf(bandwidth_log_file, "%d\n", num_epochs);
	fprintf(bandwidth_log_file, "%d\n", epoch_size);
	fprintf(bandwidth_log_file, "%f\n", (double)P_TIME/(double)epoch_size);

	for(i = (num_epochs - 1); i >= 0; i--)
	{
		//reverse the order of the print out so it runs in chronological order.
		for(j = 0; j < num_active_cores; j++)
		{
			bandwidth = list_get(cpu_gpu_stats->bandwidth[j], i);
			fprintf(bandwidth_log_file, "%llu %llu ", bandwidth->bytes_tx, bandwidth->bytes_rx);
		}

		fprintf(bandwidth_log_file, "\n");
	}


	fclose (bandwidth_log_file);

	return;
}


void cgm_dump_histograms(void){

	int i = 0;

	char buff[400];

	memset(buff, '\0', 400);
	sprintf(buff, "%s%s", cgm_stats_output_path, "fetch_lat_log_file.out");
	FILE *fetch_lat_log_file = fopen (buff, "w+");

	memset(buff, '\0', 400);
	sprintf(buff, "%s%s", cgm_stats_output_path, "load_lat_log_file.out");
	FILE *load_lat_log_file = fopen (buff, "w+");

	memset(buff, '\0', 400);
	sprintf(buff, "%s%s", cgm_stats_output_path, "store_lat_log_file.out");
	FILE *store_lat_log_file = fopen (buff, "w+");

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

#define ADDSTATS(stat) (cgm_parallel_stats->stat[0] + cgm_parallel_stats->stat[1] + cgm_parallel_stats->stat[2] + cgm_parallel_stats->stat[3])

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

	/*star todo add these stats to the regular output.*/
	/*printf("l2_gets_ %llu\n", ADDSTATS(l2_gets_));
	printf("l2_get_ %llu\n", ADDSTATS(l2_get_));
	printf("l2_getx_ %llu\n", ADDSTATS(l2_getx_));
	printf("l2_write_back_ %llu\n", ADDSTATS(l2_write_back_));
	printf("l2_write_block_ %llu\n", ADDSTATS(l2_write_block_));
	printf("l2_downgrade_ack_ %llu\n", ADDSTATS(l2_downgrade_ack_));
	printf("l2_get_nack_ %llu\n", ADDSTATS(l2_get_nack_));
	printf("l2_getx_nack_ %llu\n", ADDSTATS(l2_getx_nack_));
	printf("l2_get_fwd_ %llu\n", ADDSTATS(l2_get_fwd_));
	printf("l2_downgrade_nack_ %llu\n", ADDSTATS(l2_downgrade_nack_));
	printf("l2_getx_fwd_ %llu\n", ADDSTATS(l2_getx_fwd_));
	printf("l2_getx_fwd_inval_ack_ %llu\n", ADDSTATS(l2_getx_fwd_inval_ack_));
	printf("l2_getx_fwd_nack_ %llu\n", ADDSTATS(l2_getx_fwd_nack_));
	printf("l2_upgrade_ %llu\n", ADDSTATS(l2_upgrade_));
	printf("l2_upgrade_ack_ %llu\n", ADDSTATS(l2_upgrade_ack_));
	printf("l2_upgrade_nack_ %llu\n", ADDSTATS(l2_upgrade_nack_));
	printf("l2_upgrade_putx_n_ %llu\n", ADDSTATS(l2_upgrade_putx_n_));
	printf("l2_upgrade_inval_ %llu\n", ADDSTATS(l2_upgrade_inval_));
	printf("l2_flush_block_ %llu\n", ADDSTATS(l2_flush_block_));
	printf("l2_flush_block_ack_ %llu\n", ADDSTATS(l2_flush_block_ack_));

	printf("l3_gets_ %llu\n", ADDSTATS(l3_gets_));
	printf("l3_get_ %llu\n", ADDSTATS(l3_get_));
	printf("l3_getx_ %llu\n", ADDSTATS(l3_getx_));
	printf("l3_write_back_ %llu\n", ADDSTATS(l3_write_back_));
	printf("l3_flush_block_ack_ %llu\n", ADDSTATS(l3_flush_block_ack_));
	printf("l3_write_block_ %llu\n", ADDSTATS(l3_write_block_));
	printf("l3_downgrade_ack_ %llu\n", ADDSTATS(l3_downgrade_ack_));
	printf("l3_downgrade_nack_ %llu\n", ADDSTATS(l3_downgrade_nack_));
	printf("l3_getx_fwd_ack_ %llu\n", ADDSTATS(l3_getx_fwd_ack_));
	printf("l3_getx_fwd_nack_ %llu\n", ADDSTATS(l3_getx_fwd_nack_));
	printf("l3_getx_fwd_upgrade_nack_ %llu\n", ADDSTATS(l3_getx_fwd_upgrade_nack_));
	printf("l3_get_fwd_upgrade_nack_ %llu\n", ADDSTATS(l3_get_fwd_upgrade_nack_));
	printf("l3_upgrade_ %llu\n", ADDSTATS(l3_upgrade_));
	printf("l3_upgrade_ack_ %llu\n", ADDSTATS(l3_upgrade_ack_));*/

	//this sets up the full system dump in cgm_stat
	cgm_consolidate_stats();

	cgm_dump_general_stats();

	//dump_stat_bandwidth();
	//cgm_dump_bandwidth();

	/*star todo, fix this... we consolidate stats then try to print them all out individually...*/

	/*dump the full Run stats*/
	CGM_STATS(cgm_stats_file, ";Don't try to read this, use the python scripts to generate easy to read output.\n");
	CGM_STATS(cgm_stats_file, "[FullRunStats]\n");
	cgm_dump_cpu_gpu_stats(cgm_stat);
	mem_system_dump_stats(cgm_stat);
	cache_dump_stats(cgm_stat);
	switch_dump_stats(cgm_stat);
	sys_agent_dump_stats(cgm_stat);
	memctrl_dump_stats(cgm_stat);
	CGM_STATS(cgm_stats_file, "\n");


	/*parallel section stats*/
	CGM_STATS(cgm_stats_file, "[ParallelStats]\n");
	cgm_dump_parallel_section_stats(cgm_parallel_stats);
	cgm_dump_cpu_gpu_stats(cgm_parallel_stats);
	mem_system_dump_stats(cgm_parallel_stats);
	cache_dump_stats(cgm_parallel_stats);
	switch_dump_stats(cgm_parallel_stats);
	sys_agent_dump_stats(cgm_parallel_stats);
	memctrl_dump_stats(cgm_parallel_stats);
	CGM_STATS(cgm_stats_file, "\n");

	/*dump specific areas of interest*/
	CGM_STATS(cgm_stats_file, "[StartupStats]\n");
	cgm_dump_startup_section_stats(cgm_startup_stats);
	CGM_STATS(cgm_stats_file, "\n");

	//wrapup stats
	CGM_STATS(cgm_stats_file, "[WrapupStats]\n");
	cgm_dump_wrapup_section_stats(cgm_wrapup_stats);
	CGM_STATS(cgm_stats_file, "\n");

	/*star todo dump histograms for the various sections that we are intrested in*/

	/*dump the histograms*/
	if(Histograms == 1)
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

	/*int i = 0;
	for(i = 0; i < num_cores; i++)
	{
		printf("size id %d\n", l1_d_caches[l2_caches[i].id].block_size );
	}

	fatal("here\n");*/

	//get the core ID number should be <= number of cores
	id = thread->core->id;
	assert(id < num_cores);

	/*stats*/
	if(cpu_gpu_stats->core_first_fetch_cycle[thread->core->id] == 0)
		cpu_gpu_stats->core_first_fetch_cycle[thread->core->id] = P_TIME;

	mem_system_stats->cpu_total_fetch_requests++;
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

void uop_factory_c_write(X86Context *ctx, unsigned int host_addr, unsigned int guest_addr, int size){

	int i = 0;
	//int j = 0;
	unsigned int blk_aligned_addr = 0;
	unsigned int blk_mask = 0x3F;
	unsigned int blk = 0x40;

	assert(host_addr == guest_addr);

	//align the address
	blk_aligned_addr = host_addr & ~(blk_mask);

	//j = (size - 1);

	for(i = 0; i < size; i++)
	{
		if(!(i % blk))
		{
			assert(blk_aligned_addr <= (host_addr + size));

			x86_uinst_new_mem(ctx, x86_uinst_cpu_flush, blk_aligned_addr, 0, 0, 0, 0, 0, 0, 0, 0);
			blk_aligned_addr = blk_aligned_addr + blk;
		}
	}

	blk_aligned_addr = host_addr & ~(blk_mask);

	//this is a simulated fence...
	x86_uinst_new_mem(ctx, x86_uinst_cpu_fence, blk_aligned_addr, 0, 0, 0, 0, 0, 0, 0, 0);

	//pause stats while these go by...
	assert(cpu_gpu_stats->core_num_fences[ctx->core_index] == 0); //flag should always before we change it...
	cpu_gpu_stats->core_num_fences[ctx->core_index] = 1;

	return;
}

void uop_factory_nc_write(X86Context *ctx, unsigned int host_addr, unsigned int guest_addr, int size){

	int i = 0;
	unsigned int blk_aligned_addr = 0;
	unsigned int blk_mask = 0x3F;
	unsigned int blk = 0x40;

	warning("upp factory nc write cycle %llu\n", P_TIME);

	unsigned int host_load_addr = host_addr;
	unsigned int guest_store_addr = guest_addr;

	//copy memory from one to the other (load & store)
	for(i = 0; i < size; i+=4)
	{
		x86_uinst_new_mem(ctx, x86_uinst_load, host_load_addr, 1, 0, 0, 0, x86_dep_eax, 0, 0, 0);
		x86_uinst_new_mem(ctx, x86_uinst_store_ex, guest_store_addr, 1, x86_dep_eax, 0, 0, 0, 0, 0, 0);

		host_load_addr+=4;
		guest_store_addr+=4;
	}

	//rewind the quest address
	//guest_addr = guest_addr - size;

	//align the address
	blk_aligned_addr = guest_addr & ~(blk_mask);

	for(i = 0; i < size; i++)
	{
		if(!(i % blk))
		{
			x86_uinst_new_mem(ctx, x86_uinst_cpu_flush, blk_aligned_addr, 0, 0, 0, 0, 0, 0, 0, 0);
			blk_aligned_addr = blk_aligned_addr + blk;
		}
	}

	blk_aligned_addr = guest_addr & ~(blk_mask);

	//this is a simulated fence...
	x86_uinst_new_mem(ctx, x86_uinst_cpu_fence, blk_aligned_addr, 0, 0, 0, 0, 0, 0, 0, 0);

	//pause stats while these go by...
	assert(cpu_gpu_stats->core_num_fences[ctx->core_index] == 0); //flag should always before we change it...
	cpu_gpu_stats->core_num_fences[ctx->core_index] = 1;

	return;
}

void uop_factory_c_read(X86Context *ctx, unsigned int host_addr, unsigned int guest_addr, int size){

	int i = 0;
	unsigned int blk_aligned_addr = 0;
	unsigned int blk_mask = 0x3F;
	unsigned int blk = 0x40;

	//align the address
	blk_aligned_addr = host_addr & ~(blk_mask);

	if(host_addr != guest_addr)
		fatal("uop_factory_c_read(): host and guest addr different host 0x%08x quest 0x%08x\n", host_addr, guest_addr);
	assert(host_addr == guest_addr);

	for(i = 0; i < size; i++)
	{
		if(!(i % blk))
		{
			assert(blk_aligned_addr <= (host_addr + size));

			x86_uinst_new_mem(ctx, x86_uinst_gpu_flush, blk_aligned_addr, 0, 0, 0, 0, 0, 0, 0, 0);
			blk_aligned_addr = blk_aligned_addr + blk;
		}
	}


	/*if(blk_aligned_addr > (host_addr + size))
				fatal("flush exceeded size blk_addr 0x%08x host size 0x%08x\n", blk_aligned_addr, (host_addr + size));*/

			/*assert(blk_aligned_addr < (host_addr + size));*/

	/*if(blk_aligned_addr > (host_addr + size))
				fatal("flush exceeded size blk_addr 0x%08x host size 0x%08x\n", blk_aligned_addr, (host_addr + size));*/



	blk_aligned_addr = host_addr & ~(blk_mask);

	//this is a simulated fence...
	x86_uinst_new_mem(ctx, x86_uinst_cpu_load_fence, blk_aligned_addr, 0, 0, 0, 0, 0, 0, 0, 0);

	//pause stats while these go by...
	assert(cpu_gpu_stats->core_num_fences[ctx->core_index] == 0); //flag should always before we change it...
	cpu_gpu_stats->core_num_fences[ctx->core_index] = 1;

	return;
}

void uop_factory_nc_read(X86Context *ctx, unsigned int host_addr, unsigned int guest_addr, int size){

	int i = 0;
	unsigned int blk_aligned_addr = 0;
	unsigned int blk_mask = 0x3F;
	unsigned int blk = 0x40;

	unsigned int guest_load_addr = host_addr;
	unsigned int host_store_addr = guest_addr;

	//flush the GPU
	warning("upp factory nc read cycle %llu\n", P_TIME);

	//align the address
	blk_aligned_addr = guest_addr & ~(blk_mask);

	for(i = 0; i < size; i++)
	{
		if(!(i % blk))
		{
			x86_uinst_new_mem(ctx, x86_uinst_gpu_flush, blk_aligned_addr, 0, 0, 0, 0, 0, 0, 0, 0);
			blk_aligned_addr = blk_aligned_addr + blk;
		}
	}

	//reset our blk aligned address
	blk_aligned_addr = guest_addr & ~(blk_mask);

	//this is a simulated fence...
	x86_uinst_new_mem(ctx, x86_uinst_cpu_load_fence, blk_aligned_addr, 0, 0, 0, 0, 0, 0, 0, 0);

	//copy the data
	for(i = 0; i < size; i+=4)
	{
		x86_uinst_new_mem(ctx, x86_uinst_load_ex, guest_load_addr, 1, 0, 0, 0, x86_dep_eax, 0, 0, 0);
		x86_uinst_new_mem(ctx, x86_uinst_store, host_store_addr, 1, x86_dep_eax, 0, 0, 0, 0, 0, 0);

		guest_load_addr+=4;
		host_store_addr+=4;
	}

	//rewind the quest address
	//guest_addr = guest_addr - size;

	blk_aligned_addr = guest_addr & ~(blk_mask);

	//this is a simulated fence... (actual address doesn't matter).
	x86_uinst_new_mem(ctx, x86_uinst_cpu_load_fence, blk_aligned_addr, 0, 0, 0, 0, 0, 0, 0, 0);

	//pause stats while these go by...
	assert(cpu_gpu_stats->core_num_fences[ctx->core_index] == 0); //flag should always be zero before we change it...
	cpu_gpu_stats->core_num_fences[ctx->core_index] = 2;

	return;
}


void cgm_issue_lspq_access(X86Thread *self, enum cgm_access_kind_t access_kind, long long uop_id,
		unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item){

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

	/*if(uop_id == 788)
		warning("its load access id %llu start %llu\n", new_packet->access_id, new_packet->start_cycle);*/


	/*if(gpu_running == 1)
		printf("cpu memory req\n");*/


	/*if(access_kind == cgm_access_cpu_load_fence)
	{
		fatal("made it to cgm fencing address... blk 0x%08x regular is 0x%08x regular blk 0x%08x\n",
				new_packet->address, mmu_translate(0, 0x00000000, mmu_access_load_store), (mmu_translate(0, 0x00000000, mmu_access_load_store) & l1_d_caches[0].block_address_mask));
	}*/

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
		mem_system_stats->cpu_total_load_requests++;
	}
	else if(access_kind == cgm_access_store)
	{
		mem_system_stats->cpu_total_store_requests++;
	}
	else if(access_kind == cgm_access_cpu_flush)
	{
		//ignore for now
	}

	//For memory system load store request
	if(access_kind == cgm_access_load || access_kind == cgm_access_store
			|| access_kind == cgm_access_cpu_flush || access_kind == cgm_access_gpu_flush
			|| access_kind == cgm_access_cpu_fence || access_kind == cgm_access_cpu_load_fence)
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
		else if(access_kind == cgm_access_cpu_flush)
		{
			//ignore for now
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

	//struct si_vector_mem_unit_t *vector_mem_ptr = vector_mem;
	struct cgm_packet_t *new_packet = packet_create();
	char buff[100];
	access_id++;
	int num_cus = si_gpu_num_compute_units;
	int id = 0;

	//build one packet to pass through the memory system
	memset(buff, '\0', 100);
	snprintf(buff, 100, "vector_mem_access.%lld", access_id);

	new_packet->access_type = access_kind;
	new_packet->gpu_access_type = access_kind;
	new_packet->cpu_access_type = access_kind;
	new_packet->address = addr;
	new_packet->witness_ptr = witness_ptr;
	//new_packet->in_flight = 1;
	new_packet->access_id = access_id;
	new_packet->name = strdup(buff);
	new_packet->start_cycle = P_TIME;

	//leave for debugging purposes
	if(mem_system_off == 1)
	{
		(*witness_ptr)++;
		packet_destroy(new_packet);
		return;
	}

	//Add to the target L1 Cache Rx Queue
	if(access_kind == cgm_access_load || access_kind == cgm_access_store || access_kind == cgm_access_nc_store)
	{
		//get the core ID number should be <= number of cores
		id = vector_mem->compute_unit->id;
		assert( id < num_cus);

		//warning("CU %d accesses v $\n", id);

		if(access_kind == cgm_access_nc_store)
		{
			new_packet->access_type = cgm_access_store;
			new_packet->gpu_access_type = cgm_access_store;
			new_packet->cpu_access_type = cgm_access_store;
		}

		/*if(list_count(vector_mem->compute_unit->gpu_v_cache_ptr->Rx_queue_top) > 16)
		{
			printf("queue size %d cycle %llu\n", list_count(vector_mem->compute_unit->gpu_v_cache_ptr->Rx_queue_top), P_TIME);
			getchar();
		}*/

		//Drop the packet into the Rx queue
		list_enqueue(vector_mem->compute_unit->gpu_v_cache_ptr[id].Rx_queue_top, new_packet);

		//advance the L1 I Cache Ctrl task
		advance(&gpu_v_cache[id]);

		gpu_v_caches[id].TotalAcesses++;

		/*stats*/
		if(access_kind == cgm_access_load)
		{
			mem_system_stats->gpu_total_loads++;
		}

		if(access_kind == cgm_access_store || access_kind == cgm_access_nc_store)
		{
			mem_system_stats->gpu_total_stores++;
		}

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
	new_packet->gpu_access_type = access_kind;
	new_packet->address = addr;
	new_packet->witness_ptr = witness_ptr;
	//new_packet->in_flight = 1;
	new_packet->access_id = access_id;
	new_packet->name = strdup(buff);
	new_packet->start_cycle = P_TIME;


	//leave for debugging purposes
	if(mem_system_off == 1)
	{
		(*witness_ptr)++;
		packet_destroy(new_packet);
		return;
	}

	//Add to the target cache Rx queue
	if(access_kind == cgm_access_load)
	{
		//get the core ID number should be <= number of cores
		id = scalar_unit_ptr->compute_unit->id;
		assert(id < num_cus);

		/*unsigned int temp = addr;
		temp = temp & gpu_s_caches[id].block_address_mask;
		printf("%s id %llu type %d address 0x%08x blk_addr 0x%08x start cycle %llu\n", gpu_s_caches[id].name, new_packet->access_id, new_packet->access_type, addr, temp, P_TIME);*/

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
	if(mem_system_off == 1)
	{
		(*witness_ptr)++;
		free(new_packet);
		return;
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

//#include <unistd.h>

long long last_cycle = 0;
long long curr_cycle = 0;
double last_time = 0;
double curr_time = 0;

int gpu_running = 0;

void PrintCycle(void){

	if((P_TIME % SKIP) == 0)
	{
		curr_time = get_wall_time();
		curr_cycle = P_TIME;

		printf("---Total Cycles %lluM Simulated Cycles Per Sec %d---\n", P_TIME/SKIP, (int) ((curr_cycle - last_cycle)/(curr_time - last_time)));
		fflush(stdout);

		last_time = curr_time;
		last_cycle = curr_cycle;
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

long long cgm_get_oldest_access(void){

	long long system_start = (P_TIME + 2);
	long long oldest_access = 0;
	int i = 0;

	int num_cores = x86_cpu_num_cores;
	int num_switches = num_cores + 1;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);


	//CPU
	for(i = 0; i < num_cores; i++)
	{
		oldest_access = cache_get_oldest_access(&l1_d_caches[i]);

		if(oldest_access < system_start)
			system_start = oldest_access;

		oldest_access = cache_get_oldest_access(&l2_caches[i]);

		if(oldest_access < system_start)
			system_start = oldest_access;

		oldest_access = cache_get_oldest_access(&l3_caches[i]);

		if(oldest_access < system_start)
			system_start = oldest_access;

	}

	//GPU
	for(i = 0; i < num_cus; i++)
	{
		oldest_access = cache_get_oldest_access(&gpu_v_caches[i]);

		if(oldest_access < system_start)
			system_start = oldest_access;
	}


	for(i = 0; i < gpu_group_cache_num; i++)
	{
		oldest_access = cache_get_oldest_access(&gpu_l2_caches[i]);

		if(oldest_access < system_start)
			system_start = oldest_access;
	}


	//switches
	for(i = 0; i < num_switches; i++)
	{
		oldest_access = switch_get_oldest_access(&switches[i]);

		if(oldest_access < system_start)
			system_start = oldest_access;
	}

	oldest_access = hub_get_oldest_access(hub_iommu);

	if(oldest_access < system_start)
		system_start = oldest_access;

	oldest_access = sa_get_oldest_access(system_agent);

	if(oldest_access < system_start)
		system_start = oldest_access;

	oldest_access = mc_get_oldest_access(mem_ctrl);

	if(oldest_access < system_start)
		system_start = oldest_access;

	return system_start;
}


long long switch_get_oldest_access(struct switch_t *switches){

	long long start_time = (P_TIME + 2);
	struct cgm_packet_t *packet = NULL;
	int i = 0;


	LIST_FOR_EACH(switches->west_queue, i)
	{
		packet = list_get(switches->west_queue, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(switches->Tx_west_queue, i)
	{
		packet = list_get(switches->Tx_west_queue, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(switches->south_queue, i)
	{
		packet = list_get(switches->south_queue, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(switches->Tx_south_queue, i)
	{
		packet = list_get(switches->Tx_south_queue, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}


	LIST_FOR_EACH(switches->east_queue, i)
	{
		packet = list_get(switches->east_queue, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(switches->Tx_east_queue, i)
	{
		packet = list_get(switches->Tx_east_queue, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}


	LIST_FOR_EACH(switches->north_queue, i)
	{
		packet = list_get(switches->north_queue, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(switches->Tx_north_queue, i)
	{
		packet = list_get(switches->Tx_north_queue, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	return start_time;
}


long long mc_get_oldest_access(struct mem_ctrl_t *mem_ctrl){

	long long start_time = (P_TIME + 2);
	struct cgm_packet_t *packet = NULL;
	int i = 0;

	LIST_FOR_EACH(mem_ctrl->Rx_queue_top, i)
	{
		packet = list_get(mem_ctrl->Rx_queue_top, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(mem_ctrl->Tx_queue_top, i)
	{
		packet = list_get(mem_ctrl->Tx_queue_top, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	return start_time;
}



long long sa_get_oldest_access(struct system_agent_t *system_agent){

	long long start_time = (P_TIME + 2);
	struct cgm_packet_t *packet = NULL;
	int i = 0;

	LIST_FOR_EACH(system_agent->Rx_queue_top, i)
	{
		packet = list_get(system_agent->Rx_queue_top, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(system_agent->Tx_queue_top, i)
	{
		packet = list_get(system_agent->Tx_queue_top, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(system_agent->Rx_queue_bottom, i)
	{
		packet = list_get(system_agent->Rx_queue_bottom, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(system_agent->Tx_queue_bottom, i)
	{
		packet = list_get(system_agent->Tx_queue_bottom, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	return start_time;
}


long long hub_get_oldest_access(struct hub_iommu_t *hub_iommu){

	long long start_time = (P_TIME + 2);
	struct cgm_packet_t *packet = NULL;
	int i = 0;
	int j = 0;

	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);


	for(j = 0; j < gpu_group_cache_num; j ++)
	{
		LIST_FOR_EACH(hub_iommu->Rx_queue_top[j], i)
		{
			packet = list_get(hub_iommu->Rx_queue_top[j], i);

			if(packet->start_cycle < start_time)
				start_time = packet->start_cycle;
		}
	}

	for(j = 0; j < gpu_group_cache_num; j ++)
	{
		LIST_FOR_EACH(hub_iommu->Tx_queue_top[j], i)
		{
			packet = list_get(hub_iommu->Tx_queue_top[j], i);

			if(packet->start_cycle < start_time)
				start_time = packet->start_cycle;
		}
	}


	LIST_FOR_EACH(hub_iommu->Rx_queue_bottom, i)
	{
		packet = list_get(hub_iommu->Rx_queue_bottom, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(hub_iommu->Tx_queue_bottom, i)
	{
		packet = list_get(hub_iommu->Tx_queue_bottom, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	return start_time;
}


long long cache_get_oldest_access(struct cache_t *cache){

	long long start_time = (P_TIME + 2);
	struct cgm_packet_t *packet = NULL;
	int i = 0;


	LIST_FOR_EACH(cache->Rx_queue_top, i)
	{
		packet = list_get(cache->Rx_queue_top, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(cache->Rx_queue_bottom, i)
	{
		packet = list_get(cache->Rx_queue_bottom, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	if(cache->cache_type == l2_cache_t || cache->cache_type == l3_cache_t)
	{
		LIST_FOR_EACH(cache->Tx_queue_top, i)
		{
			packet = list_get(cache->Tx_queue_top, i);

			if(packet->start_cycle < start_time)
				start_time = packet->start_cycle;
		}
	}

	LIST_FOR_EACH(cache->Tx_queue_bottom, i)
	{
		packet = list_get(cache->Tx_queue_bottom, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(cache->Coherance_Rx_queue, i)
	{
		packet = list_get(cache->Coherance_Rx_queue, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}


	LIST_FOR_EACH(cache->pending_request_buffer, i)
	{
		packet = list_get(cache->pending_request_buffer, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}


	LIST_FOR_EACH(cache->retry_queue, i)
	{
		packet = list_get(cache->retry_queue, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}

	LIST_FOR_EACH(cache->ort_list, i)
	{
		packet = list_get(cache->ort_list, i);

		if(packet->start_cycle < start_time)
			start_time = packet->start_cycle;
	}


	return start_time;
}


void cgm_dump_system(void){

	int i = 0;
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);

	printf("\n\nOldest start_time in the system is %llu\n", cgm_get_oldest_access());

	printf("\n---L1_v_caches---\n");
	for(i = 0; i < num_cus; i++)
	{
		printf("---%s Rx top queue size %d---\n",
				gpu_v_caches[i].name, list_count(gpu_v_caches[i].Rx_queue_top));
		cache_dump_queue(gpu_v_caches[i].Rx_queue_top);
		printf("---%s Rx bottom queue size %d---\n",
				gpu_v_caches[i].name, list_count(gpu_v_caches[i].Rx_queue_bottom));
		cache_dump_queue(gpu_v_caches[i].Rx_queue_bottom);
		printf("---%s Tx bottom queue size %d---\n",
				gpu_v_caches[i].name, list_count(gpu_v_caches[i].Tx_queue_bottom));
		cache_dump_queue(gpu_v_caches[i].Tx_queue_bottom);
		printf("---%s Coherence queue size %d---\n",
				gpu_v_caches[i].name, list_count(gpu_v_caches[i].Coherance_Rx_queue));
		cache_dump_queue(gpu_v_caches[i].Coherance_Rx_queue);
		printf("---%s Pending request queue size %d---\n",
				gpu_v_caches[i].name, list_count(gpu_v_caches[i].pending_request_buffer));
		cache_dump_queue(gpu_v_caches[i].pending_request_buffer);
		printf("---%s Retry queue size %d---\n",
				gpu_v_caches[i].name, list_count(gpu_v_caches[i].retry_queue));
		cache_dump_queue(gpu_v_caches[i].retry_queue);
		printf("---%s Write back queue size %d---\n",
				gpu_v_caches[i].name, list_count(gpu_v_caches[i].write_back_buffer));
		cache_dump_write_back(&gpu_v_caches[i]);
		printf("---%s ORT size %d---\n",
				gpu_v_caches[i].name, list_count(gpu_v_caches[i].ort_list));
		cache_dump_queue(gpu_v_caches[i].ort_list);
		ort_dump(&gpu_v_caches[i]);
		printf("\n");
	}

	printf("\n---gpu_L2_caches---\n");
	for(i = 0; i < gpu_group_cache_num; i++)
	{
		printf("---%s Rx top queue size %d---\n",
				gpu_l2_caches[i].name, list_count(gpu_l2_caches[i].Rx_queue_top));
		cache_dump_queue(gpu_l2_caches[i].Rx_queue_top);
		printf("---%s Rx bottom queue size %d---\n",
				gpu_l2_caches[i].name, list_count(gpu_l2_caches[i].Rx_queue_bottom));
		cache_dump_queue(gpu_l2_caches[i].Rx_queue_bottom);
		printf("---%s Tx top queue size %d---\n",
				gpu_l2_caches[i].name, list_count(gpu_l2_caches[i].Tx_queue_top));
		cache_dump_queue(gpu_l2_caches[i].Tx_queue_top);
		printf("---%s Tx bottom queue size %d---\n",
				gpu_l2_caches[i].name, list_count(gpu_l2_caches[i].Tx_queue_bottom));
		cache_dump_queue(gpu_l2_caches[i].Tx_queue_bottom);
		printf("---%s Coherence queue size %d---\n",
				gpu_l2_caches[i].name, list_count(gpu_l2_caches[i].Coherance_Rx_queue));
		cache_dump_queue(gpu_l2_caches[i].Coherance_Rx_queue);
		printf("---%s Pending request queue size %d---\n",
				gpu_l2_caches[i].name, list_count(gpu_l2_caches[i].pending_request_buffer));
		cache_dump_queue(gpu_l2_caches[i].pending_request_buffer);
		printf("---%s Retry queue size %d---\n",
				gpu_l2_caches[i].name, list_count(gpu_l2_caches[i].retry_queue));
		cache_dump_queue(gpu_l2_caches[i].retry_queue);
		printf("---%s Write back queue size %d---\n",
				gpu_l2_caches[i].name, list_count(gpu_l2_caches[i].write_back_buffer));
		cache_dump_write_back(&gpu_l2_caches[i]);
		printf("---%s ORT size %d---\n",
				gpu_l2_caches[i].name, list_count(gpu_l2_caches[i].ort_list));
		cache_dump_queue(gpu_l2_caches[i].ort_list);
		ort_dump(&gpu_l2_caches[i]);
		printf("\n");
	}

	printf("\n---hub-iommu---\n");

	for(i = 0; i < gpu_group_cache_num; i++)
	{
		printf("---%s Rx top port %d queue size %d---\n",
				hub_iommu->name, i, list_count(hub_iommu->Rx_queue_top[i]));
		cache_dump_queue(hub_iommu->Rx_queue_top[i]);
	}

	printf("---%s Rx bottom queue size %d---\n",
			hub_iommu->name, list_count(hub_iommu->Rx_queue_bottom));
	cache_dump_queue(hub_iommu->Rx_queue_bottom);

	for(i = 0; i < gpu_group_cache_num; i++)
	{
		printf("---%s Tx top port %d queue size %d---\n",
				hub_iommu->name, i, list_count(hub_iommu->Tx_queue_top[i]));
		cache_dump_queue(hub_iommu->Tx_queue_top[i]);
	}

	printf("---%s Tx bottom queue size %d---\n",
			hub_iommu->name, list_count(hub_iommu->Tx_queue_bottom));
	cache_dump_queue(hub_iommu->Tx_queue_bottom);

	printf("\n");


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
		cache_dump_write_back(&l1_d_caches[i]);
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
		switch_dump_queue(switches[i].north_queue);
		printf("---%s North out queue size %d---\n",
				switches[i].name, list_count(switches[i].Tx_north_queue));
		switch_dump_queue(switches[i].Tx_north_queue);
		printf("---%s East in queue size %d---\n",
				switches[i].name, list_count(switches[i].east_queue));
		switch_dump_queue(switches[i].east_queue);
		printf("---%s East out queue size %d---\n",
				switches[i].name, list_count(switches[i].Tx_east_queue));
		switch_dump_queue(switches[i].Tx_east_queue);
		printf("---%s South in queue size %d---\n",
				switches[i].name, list_count(switches[i].south_queue));
		switch_dump_queue(switches[i].south_queue);
		printf("---%s South out queue size %d---\n",
				switches[i].name, list_count(switches[i].Tx_south_queue));
		switch_dump_queue(switches[i].Tx_south_queue);
		printf("---%s West in queue size %d---\n",
				switches[i].name, list_count(switches[i].west_queue));
		switch_dump_queue(switches[i].west_queue);
		printf("---%s West out queue size %d---\n",
				switches[i].name, list_count(switches[i].Tx_west_queue));
				switch_dump_queue(switches[i].Tx_west_queue);
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
	cache_dump_queue(mem_ctrl->pending_accesses);
	printf("---%s Tx top queue size %d---\n",
			mem_ctrl->name, list_count(mem_ctrl->Tx_queue_top));
	printf("\n");

	return;
}

