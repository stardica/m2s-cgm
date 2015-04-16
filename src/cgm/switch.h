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


enum node_map{

	l2_cache_0 = 0,
	l2_cache_1,
	l2_cache_2,
	l2_cache_3,
	l2_cache_4,
	switch_0,
	switch_1,
	switch_2,
	switch_3,
	switch_4,
	l3_cache_0,
	l3_cache_1,
	l3_cache_2,
	l3_cache_3,
	sys_agent,
	node_number

};

enum port_name
{
	north_queue = 0,
	east_queue,
	south_queue,
	west_queue,
	forward_queue,
	back_queue
};

struct route_t{

	int dest;
	int *routes;

};

/*int ring_adj_mat[][] = {

	//L2_0	L2_1	L2_2	L2_3	L2_4	R0	R1	R2	R3	R4	L3_0	L3_1	L3_2	L3_3	SA
	{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //L2_0
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
	char *name;
	int port_num;

	int num_routes;
	struct route_t *my_routes;

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
void route_create(void);
void switch_create_tasks(void);
void switch_ctrl(void);

void get_path(void);


#endif /* SWITCH_H_ */
