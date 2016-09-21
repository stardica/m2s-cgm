/*
 * sys-agent.h
 *
 *  Created on: Dec 1, 2014
 *      Author: stardica
 */


#ifndef SYSAGENT_H_
#define SYSAGENT_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/list.h>
#include <lib/util/string.h>

#include <cgm/cgm.h>
#include <cgm/mem-ctrl.h>
#include <cgm/tasking.h>
/*#include <cgm/cache.h>*/
#include <cgm/switch.h>
#include <cgm/cgm-struct.h>
/*#include <cgm/protocol.h>*/


enum cgm_io_kind_t {
	cgm_io_memctrl = 0,
	cgm_io_net,
	cgm_io_dma,
	cgm_io_pcie,
	cgm_io_retry
};

extern struct system_agent_t *system_agent;

extern eventcount volatile *system_agent_ec;
extern task *system_agent_task;

extern eventcount volatile *system_agent_io_down_ec;
extern task *system_agent_io_down_task;

extern eventcount volatile *system_agent_io_up_ec;
extern task *system_agent_io_up_task;

extern int system_agent_pid;
extern int system_agent_io_down_pid;
extern int system_agent_io_up_pid;

//function prototypes
void sys_agent_init(void);
void sys_agent_create(void);
void sys_agent_create_tasks(void);

struct cgm_packet_t *sysagent_get_message(void);
void system_agent_route(struct cgm_packet_t *message_packet);

int sys_agent_can_access_top(void);
int sys_agent_can_access_bottom(void);

void sys_agent_ctrl(void);
void sys_agent_ctrl_io_up(void);
void sys_agent_ctrl_io_down(void);


void sys_agent_dump_stats(struct cgm_stats_t *cgm_stat_container);
void sys_agent_reset_stats(void);
void sys_agent_store_stats(struct cgm_stats_t *cgm_stat_container);

#endif /* SYSAGENT_H_ */
