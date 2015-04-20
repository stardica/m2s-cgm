/*
 * sys-agent.c
 *
 *  Created on: Dec 1, 2014
 *      Author: stardica
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/list.h>

#include <cgm/sys-agent.h>
#include <cgm/tasking.h>
#include <cgm/cache.h>
#include <cgm/packet.h>
#include <cgm/switch.h>


struct system_agent_t *system_agent;

int system_agent_pid = 0;

eventcount volatile *system_agent_ec;
task *system_agent_task;


void sys_agent_init(void){

	sys_agent_create();
	sys_agent_create_tasks();

	return;
}

void sys_agent_create(void){

	system_agent = (void*) malloc(sizeof(struct system_agent_t));

	return;
}

void sys_agent_create_tasks(void){

	char buff[100];

	system_agent_ec = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent");
	system_agent_ec = new_eventcount(strdup(buff));


	system_agent_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_ctrl");
	system_agent_task = create_task(sys_agent_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	return;
}


int sys_agent_can_access(void){

	//check if in queue is full
	if(QueueSize <= list_count(system_agent->Rx_queue_top))
	{
		return 0;
	}

	//we can access the system agent
	return 1;
}


void sys_agent_ctrl(void){

	int my_pid = system_agent_pid++;
	struct cgm_packet_t *message_packet;
	long long step = 1;

	long long access_id = 0;
	enum cgm_access_kind_t access_type;
	unsigned int addr;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(system_agent_ec, step);
		step++;

		//if we are here there should be a message in the queue
		message_packet = list_get(system_agent->Rx_queue_top, 0);
		assert(message_packet);

		//system agent passes the message to the correct IO system.
		//for now we just go from SA to memctrl

		//check the mapping of the memory address..

		//star todo fix this. This isn't done right.
		addr = message_packet->address;
		access_type = message_packet->access_type;
		access_id = message_packet->access_id;


		//for now pretend we are both the sys agent and the memctrl
		if(access_type == cgm_access_gets)
		{

			/*while(!memctrl_can_access())
			{
				P_PAUSE(1);
			}
			 */

			//charge the memctrl delay
			P_PAUSE(250);

			//send back to L3 cache over switching network
			//add source and dest

			message_packet->access_type = cgm_access_gets;

			message_packet->dest_id = message_packet->source_id;
			message_packet->dest_name = message_packet->src_name;

			message_packet->src_name = system_agent->name;
			message_packet->source_id = str_map_string(&node_strn_map, system_agent->name);

			//message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
			//message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

			while(!switch_can_access(switches[my_pid].south_queue))
			{
				P_PAUSE(1);
			}

			//success
			list_remove(system_agent->Rx_queue_top, message_packet);
			list_enqueue(switches[str_map_string(&node_strn_map, system_agent->name) - 1 ].south_queue, message_packet);

			future_advance(&switches_ec[str_map_string(&node_strn_map, system_agent->name) - 1 ], (etime.count + switches[str_map_string(&node_strn_map, system_agent->name) - 1].wire_latency));
			//done
		}
	}

	return;

}
