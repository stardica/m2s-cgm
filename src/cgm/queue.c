/*
 * queue.c
 *
 *  Created on: Nov 23, 2014
 *      Author: stardica
 */


#include <stdio.h>
#include <cgm/queue.h>
#include <cgm/cgm.h>
#include <cgm/tasking.h>
#include <cgm/packet.h>


int queue_size = 8;

//core 1
//L1 Instruction cache
struct list_t *q_l1i_0_CoreRequest; //into cache
struct list_t *q_l1i_0_L1iReply; //into CPU

//L1 Data cache
struct list_t *q_l1d_0_CoreRequest;
struct list_t *q_l1d_0_L1dReply;

//L2 cache
struct list_t *q_l2_0_L1iRequest;
struct list_t *q_l2_0_L2L1iReply;
struct list_t *q_l2_0_L1dRequest;
struct list_t *q_l2_0_L2L1dReply;

//L3 cache
struct list_t *q_l3_0_L2Request;
struct list_t *q_l3_0_L3Reply;


//struct queue_data_packet_t *queue_data_packet;

eventcount *queue_has_data;

void queue_init(void){

	//create memory system queue structures.

	//core 1
	//L1 inst cache
	q_l1i_0_CoreRequest = list_create();
	q_l1i_0_CoreRequest->name = "q_l1i_0_CoreRequest";
	q_l1i_0_L1iReply = list_create();
	q_l1i_0_L1iReply->name = "q_l1i_0_L1iReply";

	//L1 data cache
	q_l1d_0_CoreRequest = list_create();
	q_l1d_0_CoreRequest->name = "q_l1d_0_CoreRequest";
	q_l1d_0_L1dReply = list_create();
	q_l1d_0_L1dReply->name = "q_l1d_0_L1dReply";

	//L2 cache
	q_l2_0_L1iRequest = list_create();
	q_l2_0_L1iRequest->name = "q_l2_0_L1iRequest";
	q_l2_0_L2L1iReply = list_create();
	q_l2_0_L2L1iReply->name = "q_l2_0_L2L1iReply";
	q_l2_0_L1dRequest = list_create();
	q_l2_0_L1dRequest->name = "q_l2_0_L1dRequest";
	q_l2_0_L2L1dReply = list_create();
	q_l2_0_L2L1dReply->name = "q_l2_0_L1dRequest";

	//L3 cache
	q_l3_0_L2Request = list_create();
	q_l3_0_L2Request->name = "q_l3_0_L2Request";
	q_l3_0_L3Reply = list_create();
	q_l3_0_L3Reply->name = "q_l3_0_L3Reply";


	return;
}

/*void queue_ctrl(void){

	long long i = 1;

	while(1)
	{

		await(start, i);
		printf("in queue_ctrl\n");
		advance(stop);

		i++;
	}

	return;
}*/

struct queue_data_packet_t * data_packet_create(void){

	struct queue_data_packet_t * new_data_packet;

	new_data_packet = (void *) malloc(sizeof(struct cgm_packet_t));

	return new_data_packet;
}

//star todo check for max size in each list.
int queue_insert(struct list_t *queue, struct queue_data_packet_t * data_packet){

	//check if queue is full
	int i = 0;
	i = list_count(queue);
	if(i > queue_size)
	{

		printf("Max queue size reached in %s\n", queue->name);
		return 1;
	}


	list_enqueue(queue, data_packet);

	//debug
	//printf("Queue %s is size %d\n", queue->name, list_count(queue));

	return 0;
}

int queue_remove(struct list_t *queue, struct queue_data_packet_t * data_packet){


	return 0;
}

void queue_free(void){

	//todo free all of our queues.

}
