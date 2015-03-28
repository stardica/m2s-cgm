/*
 * cgm-mem.h
 *
 *  Created on: Nov 24, 2014
 *      Author: stardica
 */

#ifndef CGM_H_
#define CGM_H_

#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>

#include <arch/x86/timing/thread.h>
#include <arch/si/timing/vector-mem-unit.h>
#include <arch/si/timing/scalar-unit.h>
#include <arch/si/timing/lds-unit.h>

#include <cgm/tasking.h>
#include <cgm/cache.h>

#define P_TIME (etime.count >> 1)
#define P_PAUSE(p_delay)	epause((p_delay)<<1)
#define AWAIT_P_PHI0 if (etime.count & 0x1) epause(1)
#define AWAIT_P_PHI1 if (!(etime.count & 0x1)) epause(1)

enum cgm_access_kind_t
{
	cgm_access_invalid = 0,
	cgm_access_fetch,
	cgm_access_load,
	cgm_access_store,
	cgm_access_nc_store,
	cgm_access_nc_load,
	cgm_access_prefetch,
	cgm_access_l2_load_reply,
	cgm_access_l2_store_reply
};

//global flags
extern int opencl_syscall_flag;
extern int syscall_flag;



extern long long fetch_access_id;
extern long long lspq_access_id;

extern char *cgm_config_file_name_and_path;

extern struct list_t *cgm_access_record;

extern eventcount volatile *sim_start;
extern eventcount volatile *sim_finish;
extern eventcount volatile *interupt;

//stat files
extern FILE *cgm_stats;

//function prototypes
void cgm_init(void);
void cgm_configure(void);

void cgm_create_tasks(void);
void cgm_start(void);
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

//interupts
void interupt_service_request(void);



int remove_from_global(long long id);

#endif /* CGM_H_ */
