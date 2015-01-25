/*
 * mem-ctrl.h
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */
#ifndef MEMCTRL_H_
#define MEMCTRL_H_

#include <lib/util/list.h>
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

	//pointers to queues
	struct list_t *fetch_request_queue;
	struct list_t *issue_request_queue;

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
int memctrl_can_access(struct mem_ctrl_t *ctrl, unsigned int addr);

void memctrl_ctrl_request(void);
void memctrl_ctrl_reply(void);
void memctrl_ctrl_service(void);


#endif /* MEMCTRL_H_ */
