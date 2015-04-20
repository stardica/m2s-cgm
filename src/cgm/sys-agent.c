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
	int i = 0;

	long long access_id = 0;
	enum cgm_access_kind_t access_type;
	unsigned int addr;

	set_id((unsigned int)my_pid);



	while(1)
	{

		printf("sys agent waiting\n");

		await(system_agent_ec, step);
		step++;

		printf("made it here 1\n");

		//if we are here there should be a message in the queue
		message_packet = list_get(system_agent->Rx_queue_top, 0);
		assert(message_packet);

		printf("made it here 2\n");

		access_id = message_packet->access_id;
		addr = message_packet->address;
		access_type = message_packet->access_type;

		printf("made it here 3\n");

		if (access_id == 1)
		{
			printf("system_agent\n");
			printf("access id %llu\n", access_id);
			printf("access type %d\n", access_type);
			getchar();
		}



		//system agent passes the message to the correct IO system.
		//for now we just go from SA to memctrl

		//check the mapping of the memory address..

		//star todo fix this. This isn't done right.




		//for now pretend we are both the sys agent and the memctrl
		if(access_type == cgm_access_gets)
		{

			if (access_id == 1)
			{
				printf("entered at %llu\n", (etime.count/2));
			}


			//charge the memctrl delay
			P_PAUSE(250);

			if (access_id == 1)
			{
				printf("after paused %llu\n", (etime.count/2));
				getchar();
			}

			//send back to L3 cache over switching network
			//add source and dest


			message_packet->access_type = cgm_access_puts;
			message_packet->dest_id = message_packet->source_id;
			message_packet->dest_name = message_packet->src_name;

			message_packet->src_name = system_agent->name;
			message_packet->source_id = str_map_string(&node_strn_map, system_agent->name);


			if (access_id == 1)
			{
				printf("Building message packet switch %d\n", my_pid);
				printf("Source %s id %d\n", message_packet->src_name, message_packet->source_id);
				printf("Dest %s id %d\n", message_packet->dest_name, message_packet->dest_id);
				getchar();
			}


			//get the local switch number



			/*while(!switch_can_access(switches[].south_queue))
			{
				P_PAUSE(1);
			}

			printf("made it here 4\n");

			//success
			list_remove(system_agent->Rx_queue_top, message_packet);
			list_enqueue(switches[].south_queue, message_packet);

			future_advance(&switches_ec[], (etime.count + switches[].wire_latency));*/
			//done
		}
	}

	return;

}
