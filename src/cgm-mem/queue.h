/*
 * queue.h
 *
 *  Created on: Nov 23, 2014
 *      Author: stardica
 */


#include <lib/util/list.h>
#include <cgm-mem/tasking.h>


#ifndef QUEUE_H_
#define QUEUE_H_

//star todo function to check maximum queue size.


//queue naming convention q_cache_CoreNumber_In/out
extern int queue_size;

//L1 Instruction cache
extern struct list_t *q_l1i_0_CoreRequest; //into cache
extern struct list_t *q_l1i_0_L1iReply; //into CPU

//L1 Data cache
extern struct list_t *q_l1d_0_CoreRequest;
extern struct list_t *q_l1d_0_L1dReply;

//L2 cache
extern struct list_t *q_l2_0_L1iRequest;
extern struct list_t *q_l2_0_L2L1iReply;
extern struct list_t *q_l2_0_L1dRequest;
extern struct list_t *q_l2_0_L2L1dReply;

//L3 cache
extern struct list_t *q_l3_0_L2Request;
extern struct list_t *q_l3_0_L3Reply;

//Memory controller
extern struct list_t *q_mc_0_L3Request;
extern struct list_t *q_mc_0_L3Reply;

//data
extern struct queue_data_packet_t{

	char * queue_name;
	char * task;
	int address;
	char data;

};

extern eventcount *queue_has_data;


//function prototypes
void queue_init(void);
struct queue_data_packet_t * data_packet_create(void);
int queue_insert(struct list_t *queue, struct queue_data_packet_t * data_packet);
int queue_remove(struct list_t *queue, struct queue_data_packet_t * data_packet);
void queue_ctrl(void);

void queue_free(void);


#endif /* QUEUE_H_ */
