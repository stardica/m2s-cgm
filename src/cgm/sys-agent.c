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
#include <lib/util/string.h>

#include <cgm/cgm.h>
#include <cgm/sys-agent.h>
#include <cgm/mem-ctrl.h>
#include <cgm/tasking.h>
#include <cgm/cache.h>
#include <cgm/packet.h>
#include <cgm/switch.h>
#include <cgm/protocol.h>


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


int sys_agent_can_access_top(void){

	//check if in queue is full
	if(QueueSize <= list_count(system_agent->Rx_queue_top))
	{
		return 0;
	}

	//we can access the system agent
	return 1;
}

int sys_agent_can_access_bottom(void){

	//check if in queue is full
	if(QueueSize <= list_count(system_agent->Rx_queue_bottom))
	{
		return 0;
	}

	//we can access the system agent
	return 1;
}


struct cgm_packet_t *sysagent_get_message(void){

	//star this is round robin
	struct cgm_packet_t *new_message;

	//star todo to give priority stay on a particular queue as long as it is not empty;

	new_message = list_get(system_agent->next_queue, 0);

	//keep pointer to last queue
	system_agent->last_queue = system_agent->next_queue;

	//rotate the queues
	if(system_agent->next_queue == system_agent->Rx_queue_top)
	{
		system_agent->next_queue = system_agent->Rx_queue_bottom;
	}
	else if(system_agent->next_queue == system_agent->Rx_queue_bottom)
	{
		system_agent->next_queue = system_agent->Rx_queue_top;
	}
	else
	{
		fatal("get_message() pointers arn't working");
	}

		//if we didn't get a message try again (now that the queues are rotated)
	if(new_message == NULL)
	{

		new_message = list_get(system_agent->next_queue, 0);

		//keep pointer to last queue
		system_agent->last_queue = system_agent->next_queue;
		//rotate the queues.
			if(system_agent->next_queue == system_agent->Rx_queue_top)
			{
				system_agent->next_queue = system_agent->Rx_queue_bottom;
			}
			else if(system_agent->next_queue == system_agent->Rx_queue_bottom)
			{
				system_agent->next_queue = system_agent->Rx_queue_top;
			}
			else
			{
				fatal("get_message() pointers arn't working");
			}
	}

	//shouldn't be exiting without a message
	assert(new_message != NULL);
	return new_message;
}

void system_agent_route(struct cgm_packet_t *message_packet){

	enum cgm_access_kind_t access_type;
	unsigned int addr = 0;
	long long access_id = 0;

	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;


	CGM_DEBUG(sysagent_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u\n",
		system_agent->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr);

	if(access_type == cgm_access_gets)
	{

		while(!memctrl_can_access())
		{
			P_PAUSE(1);
		}

		list_remove(system_agent->last_queue, message_packet);
		list_enqueue(mem_ctrl->Rx_queue_top, message_packet);
		future_advance(mem_ctrl_ec, WIRE_DELAY(mem_ctrl->wire_latency));

		CGM_DEBUG(sysagent_debug_file,"%s access_id %llu cycle %llu as %s sent to mem ctrl\n",
				system_agent->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type));

	}
	else if(access_type == cgm_access_puts)
	{

		//set the dest and sources
		message_packet->access_type = cgm_access_puts;
		message_packet->dest_id = message_packet->src_id;
		message_packet->dest_name = message_packet->src_name;
		message_packet->src_name = system_agent->name;
		message_packet->src_id = str_map_string(&node_strn_map, system_agent->name);

		while(!switch_can_access(system_agent->switch_queue))
		{
			P_PAUSE(1);
		}

		//success
		list_remove(system_agent->last_queue, message_packet);
		list_enqueue(system_agent->switch_queue, message_packet);

		future_advance(&switches_ec[system_agent->switch_id], WIRE_DELAY(switches[system_agent->switch_id].wire_latency));

		CGM_DEBUG(sysagent_debug_file,"%s access_id %llu cycle %llu as %s reply from mem ctrl\n",
				system_agent->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type));

	}

	return;
}

void sys_dir_lookup(struct cgm_packet_t *message_packet){


	return;
}

void sys_mem_access(struct cgm_packet_t *message_packet){


	return;
}


void sys_agent_ctrl(void){

	int my_pid = system_agent_pid++;
	struct cgm_packet_t *message_packet;
	long long step = 1;
	//int i = 0;

	long long access_id = 0;
	enum cgm_access_kind_t access_type;
	//unsigned int addr;

	set_id((unsigned int)my_pid);


	while(1)
	{
		await(system_agent_ec, step);
		step++;

		//if we are here there should be a message in the queue
		message_packet = sysagent_get_message();
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		//addr = message_packet->address;

		//star todo this is where we will receive our other directory coherence messages
		//for no lets just patch it up.
		if(access_type == cgm_access_gets || access_type == cgm_access_puts)
		{
			system_agent_route(message_packet);
		}
		else
		{
			fatal("sys_agent_ctrl(): access_id %llu bad access type %s at cycle %llu\n",
					access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
		}
	}

	fatal("sys_agent_ctrl task is broken\n");
	return;
}
