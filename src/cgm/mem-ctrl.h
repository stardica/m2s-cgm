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


/*#include <mem-image/memory.h>
#include <cgm/cache.h>
#include <DRAMSim/DRAMSim.h>
#include <dramsim/DRAMSim.h>
#include <lib/util/list.h>
#include <cgm/tasking.h>*/

#define DRAM_DELAY(DRAM_latency) (etime.count + ((long long) DRAM_latency * 2))

#define dram_system_t MultiChannelMemorySystem
extern int DRAMSim;
extern void *DRAM_object_ptr;

struct mem_ctrl_t{

	//Physical Characteristics
	char *name;
	int wire_latency;
	int DRAM_latency;
	int latency;

	//pointer to memory image.
	struct mem_t *mem;

	struct list_t *Rx_queue_top;
	struct list_t *Tx_queue;

	//ptr to system agent Rx queue
	struct list_t *system_agent_queue;

	//bus
	int bus_width;
};

//global structures
extern struct mem_ctrl_t *mem_ctrl;

//needed for DRAMSim functions
typedef int bool;
enum {false, true};

void dramsim_ctrl(void);
void print_dramsim(void);
void dramsim_start(void);
void dramsim_set_cpu_freq(void);
void dramsim_update_cpu_clock(void);

int dramsim_add_transaction(bool read_write, unsigned int addr);

void dramsim_register_call_backs(void);
void dramsim_read_complete(unsigned id, long long address, long long clock_cycle);
void dramsim_write_complete(unsigned id, long long address, long long clock_cycle);
void dramsim_power_callback(double a, double b, double c, double d);

//events
extern eventcount volatile *mem_ctrl_ec;
extern eventcount volatile *mem_ctrl_io_ec;
extern eventcount volatile *dramsim;
extern task *dramsim_cpu_clock;
extern task *mem_ctrl_task;
extern task *mem_ctrl_io_task;
extern int mem_ctrl_pid;
extern int mem_ctrl_io_pid;



//function prototypes
void memctrl_init(void);
void dram_init(void);
void memctrl_create(void);
void memctrl_create_tasks(void);

void memctrl_ctrl(void);
void memctrl_ctrl_io(void);
int memctrl_can_access(void);


#endif /* MEMCTRL_H_ */
