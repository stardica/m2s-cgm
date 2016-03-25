/*
 * mem-ctrl.h
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */
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

struct mem_ctrl_t{

	//Physical Characteristics
	char *name;
	int wire_latency;
	int DRAM_latency;
	int latency;

	unsigned int block_size;
	unsigned int block_mask;

	//pointer to memory image.
	struct mem_t *mem;

	struct list_t *Rx_queue_top;
	struct list_t *Tx_queue_top;

	struct list_t *pending_accesses;

	//ptr to system agent Rx queue
	struct list_t *system_agent_queue;

	//bus
	int bus_width;

	/*stats*/
	long long busy_cycles;
	long long num_reads;
	long long num_writes;
	double ave_dram_read_lat;
	double ave_dram_write_lat;
	double ave_dram_total_lat;
	long long read_min;
	long long read_max;
	long long write_min;
	long long write_max;
	long long dram_max_queue_depth;
	double dram_ave_queue_depth;
	long long dram_busy_cycles;
	long long rx_max;
	long long tx_max;
	long long bytes_read;
	long long bytes_wrote;
	long long io_busy_cycles;

};

//events
extern eventcount volatile *mem_ctrl_ec;
extern eventcount volatile *mem_ctrl_io_ec;
extern task *mem_ctrl_task;
extern task *mem_ctrl_io_task;
extern int mem_ctrl_pid;
extern int mem_ctrl_io_pid;

extern struct mem_ctrl_t *mem_ctrl;

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
