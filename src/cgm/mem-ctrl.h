/*
 * mem-ctrl.h
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */


#ifndef MEMCTRL_H_
#define MEMCTRL_H_

#include <lib/util/list.h>
#include <cgm/tasking.h>

extern struct mem_ctrl_t{

	//star todo add pointer to queues.
	char *name;
	int block_size;
	int log_block_size;
	int latency;
	int dir_latency;
	//int mshr_size;

	//pointers to queueus
	struct list_t *fetch_request_queue;
	struct list_t *fetch_reply_queue;
	struct list_t *issue_request_queue;
	struct list_t *issue_reply_queue;

};


//queues
extern struct list_t *FetchRequest;
extern struct list_t *FetchReply;
extern struct list_t *IssueRequest;
extern struct list_t *IssueReply;

//events
extern eventcount *mem_ctrl_has_request;
extern eventcount *mem_ctrl_has_reply;
extern eventcount *mem_ctrl_serviced;


//function prototypes

//Initialization
void memctrl_init(void);
struct mem_ctrl_t *memctrl_create(void);
void memctrl_queues_init(void);
void memctrl_tasking_init(void);

//tasking
void memctrl_ctrl_request(void);
void memctrl_ctrl_reply(void);
void memctrl_ctrl_service(void);


#endif /* MEMCTRL_H_ */
