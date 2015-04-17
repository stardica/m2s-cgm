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
#include <cgm/packet.h>


//star todo add some sophistication with the scheduler and routing algorithms.

enum node_map{

	l2_cache_0 = 0,
	switch_0,
	l3_cache_0,
	l2_cache_1,
	switch_1,
	l3_cache_1,
	l2_cache_2,
	switch_2,
	l3_cache_2,
	l2_cache_3,
	switch_3,
	l3_cache_3,
	l2_cache_4,
	switch_4,
	sys_agent,
	node_number

};

enum port_name
{
	north_queue = 0,
	east_queue,
	south_queue,
	west_queue,
	port_num

};

enum arbitrate{

	round_robin = 0,
	prioity

};

struct switch_t{

	//parts
	//cache queues
	char *name;
	int switch_node_number;
	float switch_median_node_num;
	int port_num;

	int num_routes;
	struct route_t *my_routes;

	enum port_name queue;
	enum arbitrate arb_style;
	unsigned int wire_latency;

	//for switches with 4 ports
	struct list_t *north_queue;
	struct list_t *east_queue;
	struct list_t *south_queue;
	struct list_t *west_queue;

	//for switches with 6 ports
	struct list_t *forward_queue;
	struct list_t *back_queue;

};

extern struct str_map_t node_strn_map;


extern struct switch_t *switches;
extern eventcount volatile *switches_ec;
extern task *switches_tasks;
extern int switch_pid;

//function prototypes
void switch_init(void);
void switch_create(void);
//void route_create(void);
void switch_create_tasks(void);
void switch_ctrl(void);

enum port_name get_next_queue_rb(enum port_name queue);
struct cgm_packet_t *get_from_queue(struct switch_t *switches);
void remove_from_queue(struct switch_t *switches, struct cgm_packet_t *message_packet);

void get_path(void);


#endif /* SWITCH_H_ */
