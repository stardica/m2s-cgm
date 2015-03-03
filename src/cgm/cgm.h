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

enum cgm_access_kind_t
{
	cgm_access_invalid = 0,
	cgm_access_load,
	cgm_access_store,
	cgm_access_nc_store,
	cgm_access_nc_load,
	cgm_access_prefetch
};

extern char *cgm_config_file_name_and_path;
extern long long access_id;
extern struct list_t *cgm_access_record;

extern eventcount volatile *sim_start;
extern eventcount volatile *sim_finish;

//function prototypes
void cgm_init(void);
void cgm_configure(void);

void cgm_create_tasks(void);
void cgm_start(void);
void cpu_gpu_run(void);


int cgm_can_issue_access(X86Thread *self, unsigned int addr);
int cgm_can_fetch_access(X86Thread *self, unsigned int addr);
int cgm_in_flight_access(long long id);
long long cgm_fetch_access(X86Thread *self, unsigned int addr);
void cgm_issue_lspq_access(X86Thread *self, enum cgm_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item);
//void cgm_scalar_access(struct list_t *s_cache_ptr, request_queue, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
//void cgm_vector_access(struct list_t *v_cache_ptr, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
//void cgm_lds_access(struct list_t *request_queue, enum cgm_access_kind_t access_kind, unsigned int addr, int *witness_ptr);

#endif /* CGM_H_ */
