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
/*#include <lib/util/debug.h>*/


#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>


#include <cgm/cgm.h>
#include <cgm/cgm-struct.h>
#include <cgm/tasking.h>

/*#include <cgm/hub-iommu.h>*/
/*#include <cgm/cache.h>*/
/*#include <cgm/sys-agent.h>*/


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

	/*--------------*/

	l2_cache_4,
	switch_4,
	l3_cache_4,

	l2_cache_5,
	switch_5,
	l3_cache_5,

	l2_cache_6,
	switch_6,
	l3_cache_6,

	l2_cache_7,
	switch_7,
	l3_cache_7,

	hub_iommu_8,
	switch_8,
	sys_agent_8,

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

	l1_i_cache_4,
	l1_d_cache_4,

	l1_i_cache_5,
	l1_d_cache_5,

	l1_i_cache_6,
	l1_d_cache_6,

	l1_i_cache_7,
	l1_d_cache_7,

	l1_number

};

enum cpu_l2_map{
		l2_caches_0,
		l2_caches_1,
		l2_caches_2,
		l2_caches_3,
		l2_caches_4,
		l2_caches_5,
		l2_caches_6,
		l2_caches_7,

		gpu_l2_caches_0_c,
		gpu_l2_caches_1_c,
		gpu_l2_caches_2_c,
		gpu_l2_caches_3_c,
		gpu_l2_caches_4_c,
		gpu_l2_caches_5_c,
		gpu_l2_caches_6_c,
		gpu_l2_caches_7_c,
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

enum switch_crossbar_lane_map{
	crossbar_request,
	crossbar_reply,
	crossbar_coherenece,
	crossbar_num_lanes
};

enum switch_io_lane_map{
	io_request,
	io_reply,
	io_coherenece,
	io_num_lanes
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

extern int switch_north_io_pid;
extern int switch_east_io_pid;
extern int switch_south_io_pid;
extern int switch_west_io_pid;

int switch_io_delay_factor;

/*extern eventcount volatile *switches_io_ec;
extern task *switches_io_tasks;
extern int switch_io_pid;*/

//function prototypes
void switch_init(void);
void switch_create(void);
void switch_create_tasks(void);
void switch_ctrl(void);
void switch_north_io_ctrl(void);
void switch_east_io_ctrl(void);
void switch_south_io_ctrl(void);
void switch_west_io_ctrl(void);

float switch_get_distance(int dest_node, int src_node);
int switch_can_access(struct list_t *queue);
struct crossbar_t *switch_crossbar_create(void);
void switch_crossbar_clear_state(struct switch_t *switches);
void switch_crossbar_link(struct switch_t *switches);
enum port_name switch_get_route(struct switch_t *switches, struct cgm_packet_t *message_packet);
void switch_set_link(struct switch_t *switches, enum port_name tx_queue);
enum port_name get_next_queue_rb(enum port_name queue);
struct cgm_packet_t *get_from_queue(struct switch_t *switches);
struct list_t *switch_get_in_queue(struct switch_t *switches, enum port_name queue);
void remove_from_queue(struct switch_t *switches, struct cgm_packet_t *message_packet);

void switch_check_access_type(struct cgm_packet_t *message_packet);

void switch_dump_stats(struct cgm_stats_t *cgm_stat_container);
void switch_reset_stats(void);
void switch_store_stats(struct cgm_stats_t *cgm_stat_container);

void switch_dump_queue(struct list_t *queue);


//void route_create(void);
//void get_path(void);

#endif /* SWITCH_H_ */
