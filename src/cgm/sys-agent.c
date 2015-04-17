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


eventcount volatile *system_agent_ec;
struct system_agent_t *system_agent;
task *system_agent_task;
int system_agent_pid = 0;


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

	long long step = 1;

		while(1)
		{
			//printf("in mem_ctrl\n");
			await(system_agent_ec, step);
			step++;
		}

	return;

}
