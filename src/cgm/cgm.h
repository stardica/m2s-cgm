/*
 * cgm-mem.h
 *
 *  Created on: Nov 24, 2014
 *      Author: stardica
 */

#ifndef CGM_H_
#define CGM_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <m2s.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>

#include <mem-image/memory.h>

#include <cgm/cache.h>
#include <cgm/directory.h>
#include <cgm/cgm-struct.h>
#include <cgm/sys-agent.h>
#include <cgm/ini-parse.h>
#include <cgm/tasking.h>
#include <cgm/interrupt.h>
#include <cgm/switch.h>
#include <cgm/dram.h>

#include <arch/x86/timing/core.h>
#include <arch/x86/timing/thread.h>

#include <arch/x86/timing/cpu.h>
#include <arch/si/timing/gpu.h>
#include <arch/si/timing/vector-mem-unit.h>
#include <arch/si/timing/scalar-unit.h>
#include <arch/si/timing/lds-unit.h>
#include <arch/si/timing/compute-unit.h>



#define P_TIME (etime.count >> 1)
#define P_PAUSE(p_delay)	epause((p_delay)<<1)
#define SYSTEM_LATENCY_FACTOR 4
#define SYSTEM_PAUSE(p_delay) P_PAUSE(p_delay * SYSTEM_LATENCY_FACTOR)

#define AWAIT_P_PHI0 if (etime.count & 0x1) epause(1)
#define AWAIT_P_PHI1 if (!(etime.count & 0x1)) epause(1)
#define PRINT(message, ...)	printf(message, __VA_ARGS__); fflush(stdout)
#define WATCHBLOCK (unsigned int) 0x00090080
#define WATCHLINE 0
//Level 0 = no blk trace, 1 = l1-L2, 2 = L2-L3, 3 all
#define LEVEL 3
#define DUMP 0
#define CPUTICK 1

#define SKIP 1000000

//config file
extern char *cgm_config_file_name_and_path;

//debugging
extern FILE *CPU_cache_debug_file;
extern int CPU_cache_debug;

extern FILE *GPU_cache_debug_file;
extern int GPU_cache_debug;

extern FILE *switch_debug_file;
extern int switch_debug;

extern FILE *hub_iommu_debug_file;
extern int hub_iommu_debug;

extern FILE *sysagent_debug_file;
extern int sysagent_debug;

extern FILE *memctrl_debug_file;
extern int memctrl_debug;

extern FILE *protocol_debug_file;
extern int protocol_debug;

extern FILE *mshr_debug_file;
extern int mshr_debug;

extern FILE *ort_debug_file;
extern int ort_debug;

extern FILE *cgm_stats_file;
extern int cgm_stats;

extern FILE *mem_trace_file;
extern int mem_trace;

extern FILE *load_store_log_file;
extern int load_store_debug;

extern char *cgm_debug_output_path;
extern char *cgm_stats_output_path;
extern char *cgm_stats_file_name;

//memctrl_debug || sysagent_debug switch_debug )

//debugging macros
//pass string and put into the correct file.
#define CGM_DEBUG(file, ... ) 	if(CPU_cache_debug == 1 && file == CPU_cache_debug_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid cache_debug file specified");}}\
								else if (GPU_cache_debug == 1 && file == GPU_cache_debug_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid switch_debug file specified");}}\
								else if (switch_debug == 1 && file == switch_debug_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid switch_debug file specified");}}\
								else if (hub_iommu_debug == 1 && file == hub_iommu_debug_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid hub_iommu_debug file specified");}}\
								else if (sysagent_debug == 1 && file == sysagent_debug_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid sysagent_debug file specified");}}\
								else if (memctrl_debug == 1 && file == memctrl_debug_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid memctrl_debug file specified");}}\
								else if (protocol_debug == 1 && file == protocol_debug_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid protocol_debug file specified");}}\
								else if (mshr_debug == 1 && file == mshr_debug_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid mshr_debug file specified");}}\
								else if (ort_debug == 1 && file == ort_debug_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid ort_debug file specified");}}\
								else if (load_store_debug == 1 && file == load_store_log_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid mshr_debug file specified");}}

#define CGM_STATS(file, ... ) 	if(cgm_stats == 1 && file == cgm_stats_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_STATS(): invalid stats file specified");}}\
								else if (mem_trace == 1 && file == mem_trace_file){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_STATS(): invalid mem trace file specified");}}

#define CLOSE_FILES	if(CPU_cache_debug == 1){CGM_DEBUG(CPU_cache_debug_file,"simulation end cycle %llu\n", P_TIME); fclose (CPU_cache_debug_file);}\
					if(GPU_cache_debug == 1){CGM_DEBUG(GPU_cache_debug_file,"simulation end cycle %llu\n", P_TIME); fclose (GPU_cache_debug_file);}\
					if(switch_debug == 1){CGM_DEBUG(switch_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (switch_debug_file);}\
					if(hub_iommu_debug == 1){CGM_DEBUG(hub_iommu_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (hub_iommu_debug_file);}\
					if(sysagent_debug == 1){CGM_DEBUG(sysagent_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (sysagent_debug_file);}\
					if(memctrl_debug == 1){CGM_DEBUG(memctrl_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (memctrl_debug_file);}\
					if(protocol_debug == 1){CGM_DEBUG(protocol_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (protocol_debug_file);}\
					if(mshr_debug == 1){CGM_DEBUG(mshr_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (mshr_debug_file);}\
					if(ort_debug == 1){CGM_DEBUG(ort_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (ort_debug_file);}\
					if(load_store_debug == 1){CGM_DEBUG(load_store_log_file,"simulation end cycle %llu\n", P_TIME);fclose (load_store_log_file);}\
					if(cgm_stats == 1){fclose (cgm_stats_file);}\
					if(mem_trace == 1){fclose (mem_trace_file);}

#define STOP 	CLOSE_FILES;\
				exit(0);

extern struct cgm_stats_t *cgm_stat;


//global access record
extern struct list_t *cgm_access_record;

extern eventcount volatile *sim_start;
extern eventcount volatile *sim_finish;
extern eventcount volatile *watchdog;

extern int mem_system_off;
extern int watch_dog;
extern int run_watch_dog;
extern int wd_current_set;
extern int wd_current_tag;

extern long long last_issued_lsq_access_id;
extern unsigned int last_issued_lsq_access_blk;
extern long long last_committed_lsq_access_id;
extern unsigned int last_committed_lsq_access_blk;

extern long long last_issued_fetch_access_id;
extern unsigned int last_issued_fetch_access_blk;
extern long long last_committed_fetch_access_id;
extern unsigned int last_committed_fetch_access_blk;

//set up related
void m2scgm_init(void);
void cgm_init(int argc, char **argv);
void cgm_check_config_files(char **argv);
void cgm_configure(struct mem_t *mem);
void cgm_create_tasks(void);
void cgm_mem_run(void);
void cpu_gpu_run(void);
void cgm_dump_summary(void);
void cgm_dump_stats(void);
void cgm_dump_histograms(void);
void cgm_stat_finish_create(int argc, char **argv);

void tick(void);
void cgm_watchdog(void);
void dram_tick(void);

//cache access related
int cgm_can_fetch_access(X86Thread *self, unsigned int addr);
int cgm_can_issue_access(X86Thread *self, unsigned int addr);
int cgm_in_flight_access(long long id);
long long cgm_fetch_access(X86Thread *self, unsigned int addr);
void cgm_issue_lspq_access(X86Thread *self, enum cgm_access_kind_t access_kind, long long uop_id, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item);
void cgm_scalar_access(struct si_scalar_unit_t *scalar_unit, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
void cgm_vector_access(struct si_vector_mem_unit_t *vector_mem, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
void cgm_lds_access(struct si_lds_t *lds, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
int remove_from_global(long long id);


void cgm_dump_system(void);

//debugging and stats related
void PrintCycle(void);

double get_wall_time(void);
double get_cpu_time(void);

#endif /* CGM_H_ */
