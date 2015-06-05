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
#include <cgm/packet.h>
#include <cgm/cache.h>


//#include <DRAMSim/DRAMSim.h>
//#include <dramsim/DRAMSim.h>


/*
#include <lib/util/list.h>

#include <cgm/tasking.h>
*/

#define DRAM_DELAY(DRAM_latency) (etime.count + ((long long) DRAM_latency * 2))


struct mem_ctrl_t{

	//Physical Characteristics
	char *name;
	int wire_latency;
	int DRAM_latency;
	int latency;

	struct list_t *Rx_queue_top;

	//ptr to system agent queue
	struct list_t *system_agent_queue;

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
int memctrl_can_access(void);


#endif /* MEMCTRL_H_ */
