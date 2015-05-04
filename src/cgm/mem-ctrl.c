/*
 * mem-ctrl.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/list.h>

#include <cgm/cgm.h>
#include <cgm/mem-ctrl.h>
#include <cgm/sys-agent.h>
#include <cgm/tasking.h>
#include <cgm/packet.h>
#include <cgm/cache.h>
//#include <dramsim/DRAMSim.h>


//structure declarations
struct mem_ctrl_t *mem_ctrl;
eventcount volatile *mem_ctrl_ec;
task *mem_ctrl_task;
int mem_ctrl_pid = 0;


void memctrl_init(void){

	memctrl_create();
	memctrl_create_tasks();

	return;
}

void memctrl_create(void){

	//one mem ctrl per CPU
	mem_ctrl = (void *) malloc(sizeof(struct mem_ctrl_t));

	return;
}

void memctrl_create_tasks(void){

	char buff[100];

	mem_ctrl_ec = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl");
	mem_ctrl_ec = new_eventcount(strdup(buff));


	mem_ctrl_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl");
	mem_ctrl_task = create_task(memctrl_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	return;
}

int memctrl_can_access(void){

	//check if in queue is full
	if(QueueSize <= list_count(mem_ctrl->Rx_queue_top))
	{
		return 0;
	}

	//we can access the system agent
	return 1;
}

//do some work.
void memctrl_ctrl(void){

	int my_pid = mem_ctrl_pid;
	struct cgm_packet_t *message_packet;
	long long step = 1;

	long long access_id = 0;
	enum cgm_access_kind_t access_type;
	unsigned int addr;

	set_id((unsigned int)my_pid);

	while(1)
	{


		//printf("mem_ctrl\n");
		await(mem_ctrl_ec, step);
		step++;

		//star todo connect up DRAMsim here.
		message_packet = list_dequeue(mem_ctrl->Rx_queue_top);
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		addr = message_packet->address;

		CGM_DEBUG(memctrl_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u\n",
		mem_ctrl->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr);

		P_PAUSE(8);

		while(!sys_agent_can_access_bottom())
		{
			P_PAUSE(1);
		}

		message_packet->access_type = cgm_access_puts;

		list_enqueue(mem_ctrl->system_agent_queue, message_packet);
		future_advance(system_agent_ec, DRAM_DELAY(mem_ctrl->DRAM_latency));

	}

	return;
}
