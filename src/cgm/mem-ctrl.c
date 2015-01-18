/*
 * mem-ctrl.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */

#include <math.h>
#include <cgm/mem-ctrl.h>
#include <cgm/queue.h>
#include <cgm/tasking.h>

//Arbitration
//1 = round robin
#define ARBITRATE 1

struct mem_ctrl_t *mem_ctrl;

//queues
struct list_t *FetchRequest;
struct list_t *FetchReply;
struct list_t *IssueRequest;
struct list_t *IssueReply;

//events
eventcount *mem_ctrl_has_request;
eventcount *mem_ctrl_has_reply;
eventcount *mem_ctrl_serviced;

//star todo add memory controller logic.

void memctrl_init(void){

	mem_ctrl = memctrl_create();

	memctrl_queues_init();
	memctrl_tasking_init();

	return;
}

struct mem_ctrl_t  *memctrl_create(void){

	struct mem_ctrl_t *new_mem_ctrl = (void *) malloc(sizeof(struct mem_ctrl_t));

	new_mem_ctrl->name = "mem_ctrl";
	new_mem_ctrl->block_size = 256;
	new_mem_ctrl->log_block_size = log(new_mem_ctrl->block_size)/log(2);
	new_mem_ctrl->latency = 300;
	new_mem_ctrl->dir_latency = 12;

	/*printf("%d\n", new_mem_ctrl->block_size);
	printf("%d\n", new_mem_ctrl->log_block_size);
	fflush(stdout);
	getchar();*/

	return new_mem_ctrl;
}

void memctrl_queues_init(void){

	//create memory controller queues
	/*FetchRequest = list_create();
	FetchRequest->name = "Fetch.Request";
	FetchReply = list_create();
	FetchReply->name = "Fetch.Reply";

	IssueRequest = list_create();
	IssueRequest->name = "Issue.Request";
	IssueReply = list_create();
	IssueReply->name = "Issue.Reply";*/

	//assign pointer to queueus;
	//star todo create list with size.
	mem_ctrl->fetch_request_queue = list_create();
	mem_ctrl->fetch_reply_queue = list_create();
	mem_ctrl->issue_request_queue = list_create();
	mem_ctrl->issue_reply_queue = list_create();


	mem_ctrl->fetch_request_queue->name = "Fetch.Request";
	mem_ctrl->fetch_reply_queue->name = "Fetch.Reply";
	mem_ctrl->issue_request_queue->name = "Issue.Request";
	mem_ctrl->issue_reply_queue->name = "Issue.Reply";

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

