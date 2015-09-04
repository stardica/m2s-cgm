/*
 * sys-agent.c
 *
 *  Created on: Dec 1, 2014
 *      Author: stardica
 */


#include <cgm/sys-agent.h>

/*
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
*/


struct system_agent_t *system_agent;

int system_agent_pid = 0;
int system_agent_io_up_pid = 0;
int system_agent_io_down_pid = 0;

eventcount volatile *system_agent_ec;
task *system_agent_task;

eventcount volatile *system_agent_io_down_ec;
task *system_agent_io_down_task;

eventcount volatile *system_agent_io_up_ec;
task *system_agent_io_up_task;


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

	//CTRL
	system_agent_ec = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_ec");
	system_agent_ec = new_eventcount(strdup(buff));

	system_agent_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_ctrl");
	system_agent_task = create_task(sys_agent_ctrl, DEFAULT_STACK_SIZE, strdup(buff));


	//IO
	system_agent_io_up_ec = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_io_up");
	system_agent_io_up_ec = new_eventcount(strdup(buff));

	system_agent_io_down_ec = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_io_down");
	system_agent_io_down_ec = new_eventcount(strdup(buff));

	system_agent_io_up_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_io_down_task");
	system_agent_io_up_task = create_task(sys_agent_ctrl_io_up, DEFAULT_STACK_SIZE, strdup(buff));

	system_agent_io_down_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_io_down_task");
	system_agent_io_down_task = create_task(sys_agent_ctrl_io_down, DEFAULT_STACK_SIZE, strdup(buff));

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

	/*printf("routing\n");*/

	if(access_type == cgm_access_mc_get)
	{
		/*while(!memctrl_can_access())
		{
			//stalling
			printf("SA stalling down\n");
			P_PAUSE(1);
		}*/

		P_PAUSE(system_agent->latency);

		list_remove(system_agent->last_queue, message_packet);
		//list_enqueue(mem_ctrl->Rx_queue_top, message_packet);
		list_enqueue(system_agent->Tx_queue_bottom, message_packet);
		advance(system_agent_io_down_ec);

		CGM_DEBUG(sysagent_debug_file,"%s access_id %llu cycle %llu as %s sent to mem ctrl\n",
				system_agent->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type));

	}
	else if(access_type == cgm_access_mc_put)
	{

		//set the dest and sources
		//message_packet->access_type = cgm_access_put;
		message_packet->dest_id = message_packet->src_id;
		message_packet->dest_name = message_packet->src_name;
		message_packet->src_name = system_agent->name;
		message_packet->src_id = str_map_string(&node_strn_map, system_agent->name);

		/*while(!switch_can_access(system_agent->switch_queue))
		{
			printf("SA stalling up\n");
			P_PAUSE(1);
		}*/

		P_PAUSE(system_agent->latency);

		//success
		list_remove(system_agent->last_queue, message_packet);
		//list_enqueue(system_agent->switch_queue, message_packet);
		list_enqueue(system_agent->Tx_queue_top, message_packet);

		//future_advance(&switches_ec[system_agent->switch_id], WIRE_DELAY(switches[system_agent->switch_id].wire_latency));
		//advance(&switches_ec[system_agent->switch_id]);
		advance(system_agent_io_up_ec);

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


void sys_agent_ctrl_io_up(void){

	int my_pid = system_agent_io_up_pid;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(system_agent_io_up_ec, step);
		step++;

		message_packet = list_dequeue(system_agent->Tx_queue_top);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
		transfer_time = (message_packet->size/system_agent->up_bus_width);

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		P_PAUSE(transfer_time);

		list_enqueue(system_agent->switch_queue, message_packet);
		advance(&switches_ec[system_agent->switch_id]);
	}
	return;
}

void sys_agent_ctrl_io_down(void){

	int my_pid = system_agent_io_down_pid;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(system_agent_io_down_ec, step);
		step++;

		message_packet = list_dequeue(system_agent->Tx_queue_bottom);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
		transfer_time = (message_packet->size/system_agent->down_bus_width);

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		P_PAUSE(transfer_time);

		/*while(transfer_time > 0)
		{
			P_PAUSE(1);
			transfer_time--;
			//printf("Access_is %llu cycle %llu transfer %d\n", access_id, P_TIME, transfer_time);
		}*/

		list_enqueue(mem_ctrl->Rx_queue_top, message_packet);
		advance(mem_ctrl_ec);
	}
	return;
}


void sys_agent_ctrl(void){

	int my_pid = system_agent_pid++;
	struct cgm_packet_t *message_packet;
	long long step = 1;
	//int i = 0;

	/*long long access_id = 0;*/
	/*enum cgm_access_kind_t access_type;*/
	/*unsigned int addr;*/

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(system_agent_ec, step);

		if(!memctrl_can_access() || !switch_can_access(system_agent->switch_queue))
		{
			//printf("SA stalling down\n");
			P_PAUSE(1);
		}
		else
		{
			step++;

			//if we are here there should be a message in the queue
			message_packet = sysagent_get_message();
			assert(message_packet);

			//access_type = message_packet->access_type;
			/*access_id = message_packet->access_id;*/
			/*addr = message_packet->address;*/

			CGM_DEBUG(sysagent_debug_file,"%s access_id %llu cycle %llu src %s dest %s\n",
					system_agent->name, message_packet->access_id, P_TIME, message_packet->src_name, message_packet->dest_name);


			//star todo this is where we will receive our other directory coherence messages
			//for now lets just patch it up.
			system_agent_route(message_packet);
		}
	}

	fatal("sys_agent_ctrl task is broken\n");
	return;
}
