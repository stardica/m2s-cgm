/*
 * switch.h
 *
 *  Created on: Feb 9, 2015
 *      Author: stardica
 */

#ifndef SWITCH_H_
#define SWITCH_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <lib/util/list.h>

#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>

#include <cgm/tasking.h>

enum port_name
{
	north_queue,
	east_queue,
	south_queue,
	west_queue,
	forward_queue,
	back_queue
};

//ring bus

/*int *ring_adj_mat = {

	L2_0	L2_1	L2_2	L2_3	L2_4	R0	R1	R2	R3	R4	L3_0	L3_1	L3_2	L3_3	SA
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //L2_0
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //L2_1
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //L2_2
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //L2_3
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //L2_4 (GPU)
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //R0
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //R1
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //R2
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //R3
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //R4
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //L3_0
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //L3_1
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //L3_2
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //L3_3
	{0, 	0, 		0, 		0, 		0, 		0, 	0, 	0, 	0, 	0, 	0,		0,		0,		0,		0}, //SA
};*/


struct switch_t{

	//parts
	//cache queues
	int port_num;
	int *cost_mat;
	int *route_map;

	//for switches with 4 ports
	struct list_t *north_queue;
	struct list_t *east_queue;
	struct list_t *south_queue;
	struct list_t *west_queue;

	//for switches with 6 ports
	struct list_t *forward_queue;
	struct list_t *back_queue;

};

extern struct switch_t *switches;
extern eventcount volatile *switches_ec;
extern task *switches_tasks;
extern int switch_pid;

//function prototypes
void switch_init(void);
void switch_create(void);
void switch_create_tasks(void);
void switch_ctrl(void);

void get_path(void);


#endif /* SWITCH_H_ */
