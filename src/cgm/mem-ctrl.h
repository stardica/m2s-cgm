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
	int latency;
	int num_ports;

	struct list_t *Rx_queue_top;

};

//global structures
extern struct mem_ctrl_t *mem_ctrl;

//events
extern eventcount volatile *mem_ctrl_ec;
extern task *mem_ctrl_task;
extern int mem_ctrl_pid;


//function prototypes
void memctrl_init(void);
void memctrl_create(void);
void memctrl_create_tasks(void);
void memctrl_ctrl(void);


#endif /* MEMCTRL_H_ */
