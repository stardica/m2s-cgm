/*
 * mem-ctrl.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */

#include <cgm/mem-ctrl.h>
#include <cgm/queue.h>
#include <cgm/tasking.h>

//Arbitration
//1 = round robin
#define ARBITRATE 1

//queues
struct list_t *FetchRequest;
struct list_t *FetchReply;
struct list_t *IssueRequest;
struct list_t *IssueReply;

eventcount *mem_ctrl_has_request;
eventcount *mem_ctrl_has_reply;
eventcount *mem_ctrl_serviced;

//star todo add memory controller logic.

void memctrl_init(void){

	memctrl_queues_init();
	memctrl_tasking_init();

	return;
}

void memctrl_queues_init(void){

	//Memory controller
	FetchRequest = list_create();
	FetchRequest->name = "Fetch.Request";
	FetchReply = list_create();
	FetchReply->name = "Fetch.Reply";

	IssueRequest = list_create();
	IssueRequest->name = "Issue.Request";
	IssueReply = list_create();
	IssueReply->name = "Issue.Reply";

	return;

}

void memctrl_tasking_init(void){

	char *taskname = NULL;
	char *eventname = NULL;

	//create task
	taskname = "mem-ctrl.Request";
	create_task(memctrl_ctrl_request, DEFAULT_STACK_SIZE, taskname);
	taskname = "mem-ctrl.Reply";
	create_task(memctrl_ctrl_reply, DEFAULT_STACK_SIZE, taskname);
	taskname = "mem-ctrl.Action";
	create_task(memctrl_ctrl_service, DEFAULT_STACK_SIZE, taskname);

	//create event counts
	eventname = "mem_ctrl_has_request";
	mem_ctrl_has_request = new_eventcount(eventname);
	eventname = "mem_ctrl_has_reply";
	mem_ctrl_has_reply = new_eventcount(eventname);
	eventname = "mem_ctrl_do_action";
	mem_ctrl_serviced = new_eventcount(eventname);

	return;
}


//the CPU advances mem-ctrl with a memory request here
void memctrl_ctrl_request(void){


	return;
}

//mem-ctrl advances the CPU or, probably, will update the CPU and go into a wait state here
void memctrl_ctrl_reply(void){

	return;

}

//do some work.
void memctrl_ctrl_service(void){

	long long i = 1;

		while(1)
		{

			printf("in mem_ctrl\n");
			await(mem_ctrl_has_request, i);
			//take message off of in bound queue, process it, put reply on out bound queue.
			advance(mem_ctrl_has_reply);
			advance(mem_ctrl_serviced);

			i++;
		}

	return;
}
