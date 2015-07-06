/*
 * mem-ctrl.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */


#include <cgm/mem-ctrl.h>


/*
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
*/

//#include <DRAMSim/DRAMSim.h>
//#include <dramsim/DRAMSim.h>


//structure declarations
struct mem_ctrl_t *mem_ctrl;
eventcount volatile *mem_ctrl_ec;
eventcount volatile *mem_ctrl_io_ec;
task *mem_ctrl_task;
task *mem_ctrl_io_task;
int mem_ctrl_pid = 0;
int mem_ctrl_io_pid = 0;

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
	snprintf(buff, 100, "mem_ctrl_ec");
	mem_ctrl_ec = new_eventcount(strdup(buff));

	mem_ctrl_io_ec = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl_io_ec");
	mem_ctrl_io_ec = new_eventcount(strdup(buff));

	mem_ctrl_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl_task");
	mem_ctrl_task = create_task(memctrl_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	mem_ctrl_io_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl_io");
	mem_ctrl_io_task = create_task(memctrl_ctrl_io, DEFAULT_STACK_SIZE, strdup(buff));

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

void memctrl_ctrl_io(void){

	int my_pid = mem_ctrl_io_pid;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(mem_ctrl_io_ec, step);
		step++;

		message_packet = list_dequeue(mem_ctrl->Tx_queue);
		assert(message_packet);

		access_id = message_packet->access_id;
		transfer_time = (message_packet->size/mem_ctrl->bus_width);

		P_PAUSE(transfer_time);

		/*while(transfer_time > 0)
		{
			P_PAUSE(1);
			transfer_time--;
			//printf("Access_is %llu cycle %llu transfer %d\n", access_id, P_TIME, transfer_time);
		}*/

		list_enqueue(mem_ctrl->system_agent_queue, message_packet);
		advance(system_agent_ec);
	}
	return;
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

		if(!sys_agent_can_access_bottom())
		{
			printf("MC stalling up\n");
			P_PAUSE(1);
		}
		else
		{
			step++;

			//star todo connect up DRAMsim here.
			message_packet = list_dequeue(mem_ctrl->Rx_queue_top);
			assert(message_packet);

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;
			addr = message_packet->address;

			CGM_DEBUG(memctrl_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u\n",
					mem_ctrl->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr);

			P_PAUSE(mem_ctrl->DRAM_latency);

			/*message_packet->access_type = cgm_access_puts;
			list_enqueue(mem_ctrl->system_agent_queue, message_packet);
			advance(system_agent_ec);*/

			message_packet->access_type = cgm_access_puts;
			message_packet->size = l3_caches[0].block_size;
			list_enqueue(mem_ctrl->Tx_queue, message_packet);
			advance(mem_ctrl_io_ec);
		}
	}
	return;
}
