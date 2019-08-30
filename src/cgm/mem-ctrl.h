
#ifndef MEMCTRL_H_
#define MEMCTRL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/list.h>

#include <cgm/cgm.h>
#include <cgm/sys-agent.h>
#include <cgm/tasking.h>
#include <cgm/dram.h>

#define DRAM_DELAY(DRAM_latency) (etime.count + ((long long) DRAM_latency * 2))

//events
extern eventcount volatile *mem_ctrl_ec;
extern eventcount volatile *mem_ctrl_io_ec;
extern task *mem_ctrl_task;
extern task *mem_ctrl_io_task;
extern int mem_ctrl_pid;
extern int mem_ctrl_io_pid;

extern struct mem_ctrl_t *mem_ctrl;

extern long long flushes_rx;

//function prototypes
void memctrl_init(void);
void memctrl_create(void);
void memctrl_create_tasks(void);

void memctrl_ctrl(void);
void memctrl_ctrl_io(void);
int memctrl_can_access(void);

void memctrl_dump_stats(struct cgm_stats_t *cgm_stat_container);
void memctrl_reset_stats(void);
void memctrl_store_stats(struct cgm_stats_t *cgm_stat_container);

#endif /* MEMCTRL_H_ */
