/*
 * sys-agent.h
 *
 *  Created on: Dec 1, 2014
 *      Author: stardica
 */


#ifndef SYSAGENT_H_
#define SYSAGENT_H_

#include <lib/util/list.h>

#include <cgm/tasking.h>


struct system_agent_t{


	char * name;
	unsigned int wire_latency;
	unsigned int num_ports;

	//queues
	struct list_t *Rx_queue_top;

};

extern struct system_agent_t *system_agent;

extern eventcount volatile *system_agent_ec;
extern task *system_agent_task;
extern int system_agent_pid;

//function prototypes
void sys_agent_init(void);
void sys_agent_create(void);
void sys_agent_create_tasks(void);

int sys_agent_can_access(void);

void sys_agent_ctrl(void);

#endif /* SYSAGENT_H_ */
