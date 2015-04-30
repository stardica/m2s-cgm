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

enum cgm_io_kind_t {
	cgm_io_memctrl = 0,
	cgm_io_net,
	cgm_io_dma,
	cgm_io_pcie,
	cgm_io_retry
};

struct system_agent_t{

	char *name;
	unsigned int wire_latency;
	unsigned int num_ports;

	//queues
	struct list_t *Rx_queue_top;
	struct list_t *Rx_queue_bottom;

	struct list_t *next_queue;
	struct list_t *last_queue;

	//ptr to switch
	struct list_t *switch_queue;

	int switch_id;
};

extern struct system_agent_t *system_agent;

extern eventcount volatile *system_agent_ec;
extern task *system_agent_task;
extern int system_agent_pid;

//function prototypes
void sys_agent_init(void);
void sys_agent_create(void);
void sys_agent_create_tasks(void);

struct cgm_packet_t *sysagent_get_message(void);

int sys_agent_can_access(void);
int sys_agent_can_access_bottom(void);

void sys_agent_ctrl(void);

#endif /* SYSAGENT_H_ */
