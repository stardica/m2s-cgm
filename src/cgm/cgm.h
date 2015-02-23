/*
 * cgm-mem.h
 *
 *  Created on: Nov 24, 2014
 *      Author: stardica
 */

#ifndef CGM_H_
#define CGM_H_

#include <cgm/tasking.h>
#include <cgm/cache.h>
#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>

#include <arch/x86/timing/thread.h>

enum cgm_access_kind_t
{
	cgm_access_invalid = 0,
	cgm_access_load,
	cgm_access_store,
	cgm_access_nc_store,
	cgm_access_nc_load,
	cgm_access_prefetch
};

extern long long access_id;

extern struct list_t *cgm_access_record;


extern int host_sim_cpu_num;
extern int host_sim_cpu_core_num;
extern int host_sim_cpu_thread_num;
extern int host_sim_cpu_fetch_queue_size;
extern int host_sim_cpu_lsq_queue_size;

extern long long int mem_cycle;

extern char *cgm_config_file_name_and_path;


// globals for tasking
//extern eventcount *l1_i_cache_ec;
//extern eventcount *l1_d_cache_ec;
//extern eventcount *l2_cache_ec;



//function prototypes
void cgm_init(void);
void cgm_configure(void);

int cgm_can_issue_access(X86Thread *self, unsigned int addr);
int cgm_can_fetch_access(X86Thread *self, unsigned int addr);
int cgm_in_flight_access(X86Thread *self, long long id);
long long cgm_fetch_access(X86Thread *self, unsigned int addr);
void cgm_issue_lspq_access(X86Thread *self, enum cgm_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item);
//void cgm_scalar_access(struct list_t *s_cache_ptr, request_queue, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
//void cgm_vector_access(struct list_t *v_cache_ptr, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
//void cgm_lds_access(struct list_t *request_queue, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);



//star todo recycle these later
void cgm_mem_task_init(void);
void cgm_mem_threads_init(void);
void cgm_mem_sim_loop(void);
void cgm_done(void);


#endif /* CGM_H_ */
