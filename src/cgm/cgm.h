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

extern long long fetch_access_id;
extern long long lspq_access_id;
extern char *cgm_config_file_name_and_path;
extern struct list_t *cgm_access_record;

extern eventcount volatile *sim_start;
extern eventcount volatile *sim_finish;

/*extern FILE *fetch_trace;
extern FILE *issue_trace;*/

//function prototypes
void cgm_init(void);
void cgm_configure(void);

void cgm_create_tasks(void);
void cgm_start(void);
void cpu_gpu_run(void);
void cgm_dump_summary(void);


int cgm_can_issue_access(X86Thread *self, unsigned int addr);
int cgm_can_fetch_access(X86Thread *self, unsigned int addr);
int cgm_in_flight_access(long long id);
long long cgm_fetch_access(X86Thread *self, unsigned int addr);
void cgm_issue_lspq_access(X86Thread *self, enum cgm_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item);

//void cgm_scalar_access(struct list_t *s_cache_ptr, request_queue, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
//void cgm_vector_access(struct list_t *v_cache_ptr, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
//void cgm_lds_access(struct list_t *request_queue, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
int remove_from_global(long long id);

#endif /* CGM_H_ */
