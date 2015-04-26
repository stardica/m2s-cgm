/*
 * cgm-mem.h
 *
 *  Created on: Nov 24, 2014
 *      Author: stardica
 */

#ifndef CGM_H_
#define CGM_H_

#include <stdio.h>

#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/debug.h>

#include <arch/x86/timing/core.h>
#include <arch/x86/timing/thread.h>
#include <arch/x86/timing/uop.h>
#include <arch/si/timing/vector-mem-unit.h>
#include <arch/si/timing/scalar-unit.h>
#include <arch/si/timing/lds-unit.h>

#include <cgm/tasking.h>
#include <cgm/cache.h>
#include <cgm/protocol.h>

#define P_TIME (etime.count >> 1)
#define P_PAUSE(p_delay)	epause((p_delay)<<1)
#define AWAIT_P_PHI0 if (etime.count & 0x1) epause(1)
#define AWAIT_P_PHI1 if (!(etime.count & 0x1)) epause(1)

#define CLOSE (fclose(cgm_debug))

//config file
extern char *cgm_config_file_name_and_path;

//debugging
extern FILE *cache_debug_file;
extern int cache_debug;

extern FILE *switch_debug_file;
extern int switch_debug;

extern FILE *sysagent_debug_file;
extern int sysagent_debug;

extern FILE *memctrl_debug_file;
extern int memctrl_debug;

extern FILE *protocol_debug_file;
extern int protocol_debug;

extern FILE *mshr_debug_file;
extern int mshr_debug;

extern FILE *cgm_stats_file;
extern int cgm_stats;

extern char *cgm_debug_output_path;
extern char *cgm_stats_output_path;

//memctrl_debug || sysagent_debug switch_debug )

//debugging macros
//pass string and put into the correct file.
#define CGM_DEBUG(file, ... ) 	if(cache_debug == 1){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid cache_debug file specified");}}\
								else if (switch_debug == 1){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid switch_debug file specified");}}\
								else if (sysagent_debug == 1){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid sysagent_debug file specified");}}\
								else if (memctrl_debug == 1){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid memctrl_debug file specified");}}\
								else if (protocol_debug == 1){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid protocol_debug file specified");}}\
								else if (mshr_debug == 1){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_DEBUG(): invalid mshr_debug file specified");}}

#define CGM_STATS(file, ... ) 	if(cgm_stats == 1){if(fprintf(file, __VA_ARGS__) < 0){fatal("CGM_STATS(): invalid file specified");}}

#define CLOSE_FILES	if(cache_debug == 1){CGM_DEBUG(cache_debug_file,"simulation end cycle %llu\n", P_TIME); fclose (cache_debug_file);}\
					if(switch_debug == 1){CGM_DEBUG(switch_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (switch_debug_file);}\
					if(sysagent_debug == 1){CGM_DEBUG(sysagent_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (sysagent_debug_file);}\
					if(memctrl_debug == 1){CGM_DEBUG(memctrl_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (memctrl_debug_file);}\
					if(protocol_debug == 1){CGM_DEBUG(protocol_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (protocol_debug_file);}\
					if(mshr_debug == 1){CGM_DEBUG(mshr_debug_file,"simulation end cycle %llu\n", P_TIME);fclose (mshr_debug_file);}\
					if(cgm_stats == 1){fclose (cgm_stats_file);}

#define STOP 	CLOSE_FILES; getchar()

//global access ids
extern long long fetch_access_id;
extern long long lspq_access_id;

//global access record
extern struct list_t *cgm_access_record;


extern eventcount volatile *sim_start;
extern eventcount volatile *sim_finish;



//function prototypes
void cgm_init(void);
void cgm_configure(void);
void cgm_create_tasks(void);
void cgm_mem_run(void);
void cpu_gpu_run(void);
void cgm_dump_summary(void);

//caches
int cgm_can_issue_access(X86Thread *self, unsigned int addr);
int cgm_can_fetch_access(X86Thread *self, unsigned int addr);
int cgm_in_flight_access(long long id);
long long cgm_fetch_access(X86Thread *self, unsigned int addr);
void cgm_issue_lspq_access(X86Thread *self, enum cgm_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item);
void cgm_vector_access(struct si_vector_mem_unit_t *vector_mem, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
void cgm_scalar_access(struct si_scalar_unit_t *scalar_unit, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
void cgm_lds_access(struct si_lds_t *lds, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);

//interrupts
void cgm_interrupt(X86Thread *self, struct x86_uop_t *uop);

int remove_from_global(long long id);

#endif /* CGM_H_ */
