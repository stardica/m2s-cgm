/*
 * switch.h
 *
 *  Created on: Feb 9, 2015
 *      Author: stardica
 */

#ifndef SWITCH_H_
#define SWITCH_H_


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <assert.h>


#include <lib/util/list.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>

#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>

#include <cgm/cgm.h>
#include <cgm/tasking.h>

#include <cgm/hub-iommu.h>
#include <cgm/packet.h>
#include <cgm/cache.h>
#include <cgm/sys-agent.h>


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
	hub_iommu_4,
	switch_4,
	sys_agent_4,
	node_number
};

enum l1_map{

	l1_i_cache_0 = 0,
	l1_d_cache_0,
	l1_i_cache_1,
	l1_d_cache_1,
	l1_i_cache_2,
	l1_d_cache_2,
	l1_i_cache_3,
	l1_d_cache_3,
	l1_number

};

enum cpu_l2_map{
		l2_caches_0,
		l2_caches_1,
		l2_caches_2,
		l2_caches_3,
		l2_number
};

enum gpu_l1_map{

	gpu_s_cache_0 = 0,
	gpu_v_cache_0,
	gpu_s_cache_1,
	gpu_v_cache_1,
	gpu_s_cache_2,
	gpu_v_cache_2,
	gpu_s_cache_3,
	gpu_v_cache_3,
	gpu_s_cache_4,
	gpu_v_cache_4,
	gpu_s_cache_5,
	gpu_v_cache_5,
	gpu_s_cache_6,
	gpu_v_cache_6,
	gpu_s_cache_7,
	gpu_v_cache_7,
	gpu_s_cache_8,
	gpu_v_cache_8,
	gpu_s_cache_9,
	gpu_v_cache_9,
	gpu_s_cache_10,
	gpu_v_cache_10,
	gpu_s_cache_11,
	gpu_v_cache_11,
	gpu_s_cache_12,
	gpu_v_cache_12,
	gpu_s_cache_13,
	gpu_v_cache_13,
	gpu_s_cache_14,
	gpu_v_cache_14,
	gpu_s_cache_15,
	gpu_v_cache_15,
	gpu_s_cache_16,
	gpu_v_cache_16,
	gpu_s_cache_17,
	gpu_v_cache_17,
	gpu_s_cache_18,
	gpu_v_cache_18,
	gpu_s_cache_19,
	gpu_v_cache_19,
	gpu_s_cache_20,
	gpu_v_cache_20,
	gpu_s_cache_21,
	gpu_v_cache_21,
	gpu_s_cache_22,
	gpu_v_cache_22,
	gpu_s_cache_23,
	gpu_v_cache_23,
	gpu_s_cache_24,
	gpu_v_cache_24,
	gpu_s_cache_25,
	gpu_v_cache_25,
	gpu_s_cache_26,
	gpu_v_cache_26,
	gpu_s_cache_27,
	gpu_v_cache_27,
	gpu_s_cache_28,
	gpu_v_cache_28,
	gpu_s_cache_29,
	gpu_v_cache_29,
	gpu_s_cache_30,
	gpu_v_cache_30,
	gpu_s_cache_31,
	gpu_v_cache_31,
	gpu_l1_number
};

enum gpu_l2_map{
	gpu_l2_caches_0,
	gpu_l2_caches_1,
	gpu_l2_caches_2,
	gpu_l2_caches_3,
	gpu_l2_caches_4,
	gpu_l2_caches_5,
	gpu_l2_caches_6,
	gpu_l2_caches_7,
	gpu_l2_number
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
	float switch_median_node;
	int port_num;
	int latency;

	/*int num_routes;
	struct route_t *my_routes;*/

	enum port_name queue;
	enum arbitrate arb_style;
	unsigned int wire_latency;

	//for switches with 4 ports
	struct list_t *north_queue;
	//struct list_t *north_queue_lane1;
	//struct list_t *north_queue_lane2;
	struct list_t *east_queue;
	//struct list_t *east_queue_lane1;
	//struct list_t *east_queue_lane2;
	struct list_t *south_queue;
	//struct list_t *south_queue_lane1;
	//struct list_t *south_queue_lane2;
	struct list_t *west_queue;
	//struct list_t *west_queue_lane1;
	//struct list_t *west_queue_lane2;

	//for switches with 6 ports
	//struct list_t *forward_queue_lane1;
	//struct list_t *forward_queue_lane2;
	//struct list_t *back_queue_lane1;
	//struct list_t *back_queue_lane2;

	//pointers to neighbors
	//for ring busses you just need an east/west queue ptr
	//struct list_t *next_north;
	struct list_t *next_east;
	//struct list_t *next_south;
	struct list_t *next_west;

	struct list_t *current_queue;

	//struct list_t *next_forward;
	//struct list_t *next_back;

	int next_east_id;
	int next_west_id;

};

extern struct str_map_t node_strn_map;
extern struct str_map_t l1_strn_map;
extern struct str_map_t l2_strn_map;
extern struct str_map_t gpu_l1_strn_map;
extern struct str_map_t gpu_l2_strn_map;


extern struct switch_t *switches;
extern eventcount volatile *switches_ec;
extern task *switches_tasks;
extern int switch_pid;

//function prototypes
void switch_init(void);
void switch_create(void);
void switch_create_tasks(void);
void switch_ctrl(void);

float switch_get_distance(int dest_node, int src_node);
int switch_can_access(struct list_t *queue);
enum port_name get_next_queue_rb(enum port_name queue);
struct cgm_packet_t *get_from_queue(struct switch_t *switches);
void remove_from_queue(struct switch_t *switches, struct cgm_packet_t *message_packet);

//void route_create(void);
//void get_path(void);

#endif /* SWITCH_H_ */
