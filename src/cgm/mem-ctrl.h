/*
 * mem-ctrl.h
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */
#ifndef MEMCTRL_H_
#define MEMCTRL_H_

#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <cgm/tasking.h>

//Behavioral switches
#define ARBITRATE 1

extern struct mem_ctrl_t{

	//Physical Characteristics
	char *name;
	int block_size;
	int log_block_size;
	int latency;
	int dir_latency;
	int ports;
	int mshr_size;
	int queue_size;
	long long access_id;

	//pointers to CPU entry queues
	struct list_t *fetch_request_queue;
	struct list_t *issue_request_queue;

	//pointer to GPU entry queues
	struct list_t *scalar_request_queue;
	struct list_t *vector_request_queue;

	//access record
	struct list_t *memctrl_accesses;

};

enum mem_ctrl_access_kind_t
{
	mem_ctrl_access_invalid = 0,
	mem_ctrl_access_load,
	mem_ctrl_access_store,
	mem_ctrl_access_nc_store,
	mem_ctrl_access_prefetch
};

//global structures
extern struct mem_ctrl_t *mem_ctrl;

//events
extern eventcount *mem_ctrl_has_request;
extern eventcount *mem_ctrl_has_reply;
extern eventcount *mem_ctrl_serviced;


//function prototypes
void memctrl_init(void);
struct mem_ctrl_t *memctrl_create(void);
void memctrl_queues_init(void);
void memctrl_tasking_init(void);
int memctrl_can_issue_access(struct mem_ctrl_t *ctrl, unsigned int addr);
int memctrl_can_fetch_access(struct mem_ctrl_t *ctrl, unsigned int addr);
int memctrl_in_flight_access(struct mem_ctrl_t *ctrl, long long id);
long long memctrl_fetch_access(struct list_t *request_queue, enum mem_ctrl_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item);
void memctrl_issue_lspq_access(struct list_t *request_queue, enum mem_ctrl_access_kind_t access_kind, unsigned int addr, struct linked_list_t *event_queue, void *event_queue_item);
void memctrl_scalar_access(struct list_t *request_queue, enum mem_ctrl_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
void memctrl_vector_access(struct list_t *request_queue, enum mem_ctrl_access_kind_t access_kind, unsigned int addr, int *witness_ptr);
void memctrl_lds_access(struct list_t *request_queue, enum mem_ctrl_access_kind_t access_kind, unsigned int addr, int *witness_ptr);

void memctrl_ctrl_request(void);
void memctrl_ctrl_reply(void);
void memctrl_ctrl_service(void);


#endif /* MEMCTRL_H_ */
