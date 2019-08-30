
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

enum node_map_p34{

	l2_cache_0_p34 = 0,
	switch_0_p34,
	l3_cache_0_p34,

	l2_cache_1_p34,
	switch_1_p34,
	l3_cache_1_p34,

	l2_cache_2_p34,
	switch_2_p34,
	l3_cache_2_p34,

	l2_cache_3_p34,
	switch_3_p34,
	l3_cache_3_p34,

	/*--------------*/

	l2_cache_4_p34,
	switch_4_p34,
	l3_cache_4_p34,

	l2_cache_5_p34,
	switch_5_p34,
	l3_cache_5_p34,

	l2_cache_6_p34,
	switch_6_p34,
	l3_cache_6_p34,

	l2_cache_7_p34,
	switch_7_p34,
	l3_cache_7_p34,

	/*--------------*/

	l2_cache_8_p34,
	switch_8_p34,
	l3_cache_8_p34,

	l2_cache_9_p34,
	switch_9_p34,
	l3_cache_9_p34,

	l2_cache_10_p34,
	switch_10_p34,
	l3_cache_10_p34,

	l2_cache_11_p34,
	switch_11_p34,
	l3_cache_11_p34,

	/*--------------*/

	l2_cache_12_p34,
	switch_12_p34,
	l3_cache_12_p34,

	l2_cache_13_p34,
	switch_13_p34,
	l3_cache_13_p34,

	l2_cache_14_p34,
	switch_14_p34,
	l3_cache_14_p34,

	l2_cache_15_p34,
	switch_15_p34,
	l3_cache_15_p34,

	/*--------------*/

	l2_cache_16_p34,
	switch_16_p34,
	l3_cache_16_p34,

	l2_cache_17_p34,
	switch_17_p34,
	l3_cache_17_p34,

	l2_cache_18_p34,
	switch_18_p34,
	l3_cache_18_p34,

	l2_cache_19_p34,
	switch_19_p34,
	l3_cache_19_p34,

	/*--------------*/

	l2_cache_20_p34,
	switch_20_p34,
	l3_cache_20_p34,

	l2_cache_21_p34,
	switch_21_p34,
	l3_cache_21_p34,

	l2_cache_22_p34,
	switch_22_p34,
	l3_cache_22_p34,

	l2_cache_23_p34,
	switch_23_p34,
	l3_cache_23_p34,

	/*--------------*/

	l2_cache_24_p34,
	switch_24_p34,
	l3_cache_24_p34,

	l2_cache_25_p34,
	switch_25_p34,
	l3_cache_25_p34,

	l2_cache_26_p34,
	switch_26_p34,
	l3_cache_26_p34,

	l2_cache_27_p34,
	switch_27_p34,
	l3_cache_27_p34,

	/*--------------*/

	l2_cache_28_p34,
	switch_28_p34,
	l3_cache_28_p34,

	l2_cache_29_p34,
	switch_29_p34,
	l3_cache_29_p34,

	l2_cache_30_p34,
	switch_30_p34,
	l3_cache_30_p34,

	l2_cache_31_p34,
	switch_31_p34,
	l3_cache_31_p34,

	l2_cache_32_p34,
	switch_32_p34,
	l3_cache_32_p34,

	l2_cache_33_p34,
	switch_33_p34,
	l3_cache_33_p34,

	/*--------------*/

	hub_iommu_34_p34,
	switch_34_p34,
	sys_agent_34_p34,

	node_number_p34
};

enum node_map_p32{

	l2_cache_0_p32 = 0,
	switch_0_p32,
	l3_cache_0_p32,

	l2_cache_1_p32,
	switch_1_p32,
	l3_cache_1_p32,

	l2_cache_2_p32,
	switch_2_p32,
	l3_cache_2_p32,

	l2_cache_3_p32,
	switch_3_p32,
	l3_cache_3_p32,

	/*--------------*/

	l2_cache_4_p32,
	switch_4_p32,
	l3_cache_4_p32,

	l2_cache_5_p32,
	switch_5_p32,
	l3_cache_5_p32,

	l2_cache_6_p32,
	switch_6_p32,
	l3_cache_6_p32,

	l2_cache_7_p32,
	switch_7_p32,
	l3_cache_7_p32,

	/*--------------*/

	l2_cache_8_p32,
	switch_8_p32,
	l3_cache_8_p32,

	l2_cache_9_p32,
	switch_9_p32,
	l3_cache_9_p32,

	l2_cache_10_p32,
	switch_10_p32,
	l3_cache_10_p32,

	l2_cache_11_p32,
	switch_11_p32,
	l3_cache_11_p32,

	/*--------------*/

	l2_cache_12_p32,
	switch_12_p32,
	l3_cache_12_p32,

	l2_cache_13_p32,
	switch_13_p32,
	l3_cache_13_p32,

	l2_cache_14_p32,
	switch_14_p32,
	l3_cache_14_p32,

	l2_cache_15_p32,
	switch_15_p32,
	l3_cache_15_p32,

	/*--------------*/

	l2_cache_16_p32,
	switch_16_p32,
	l3_cache_16_p32,

	l2_cache_17_p32,
	switch_17_p32,
	l3_cache_17_p32,

	l2_cache_18_p32,
	switch_18_p32,
	l3_cache_18_p32,

	l2_cache_19_p32,
	switch_19_p32,
	l3_cache_19_p32,

	/*--------------*/

	l2_cache_20_p32,
	switch_20_p32,
	l3_cache_20_p32,

	l2_cache_21_p32,
	switch_21_p32,
	l3_cache_21_p32,

	l2_cache_22_p32,
	switch_22_p32,
	l3_cache_22_p32,

	l2_cache_23_p32,
	switch_23_p32,
	l3_cache_23_p32,

	/*--------------*/

	l2_cache_24_p32,
	switch_24_p32,
	l3_cache_24_p32,

	l2_cache_25_p32,
	switch_25_p32,
	l3_cache_25_p32,

	l2_cache_26_p32,
	switch_26_p32,
	l3_cache_26_p32,

	l2_cache_27_p32,
	switch_27_p32,
	l3_cache_27_p32,

	/*--------------*/

	l2_cache_28_p32,
	switch_28_p32,
	l3_cache_28_p32,

	l2_cache_29_p32,
	switch_29_p32,
	l3_cache_29_p32,

	l2_cache_30_p32,
	switch_30_p32,
	l3_cache_30_p32,

	l2_cache_31_p32,
	switch_31_p32,
	l3_cache_31_p32,

	/*--------------*/

	hub_iommu_32_p32,
	switch_32_p32,
	sys_agent_32_p32,

	node_number_p32
};


enum node_map_p22{

	l2_cache_0_p22 = 0,
	switch_0_p22,
	l3_cache_0_p22,

	l2_cache_1_p22,
	switch_1_p22,
	l3_cache_1_p22,

	l2_cache_2_p22,
	switch_2_p22,
	l3_cache_2_p22,

	l2_cache_3_p22,
	switch_3_p22,
	l3_cache_3_p22,

	/*--------------*/

	l2_cache_4_p22,
	switch_4_p22,
	l3_cache_4_p22,

	l2_cache_5_p22,
	switch_5_p22,
	l3_cache_5_p22,

	l2_cache_6_p22,
	switch_6_p22,
	l3_cache_6_p22,

	l2_cache_7_p22,
	switch_7_p22,
	l3_cache_7_p22,

	/*--------------*/

	l2_cache_8_p22,
	switch_8_p22,
	l3_cache_8_p22,

	l2_cache_9_p22,
	switch_9_p22,
	l3_cache_9_p22,

	l2_cache_10_p22,
	switch_10_p22,
	l3_cache_10_p22,

	l2_cache_11_p22,
	switch_11_p22,
	l3_cache_11_p22,

	/*--------------*/

	l2_cache_12_p22,
	switch_12_p22,
	l3_cache_12_p22,

	l2_cache_13_p22,
	switch_13_p22,
	l3_cache_13_p22,

	l2_cache_14_p22,
	switch_14_p22,
	l3_cache_14_p22,

	l2_cache_15_p22,
	switch_15_p22,
	l3_cache_15_p22,

	/*--------------*/

	l2_cache_16_p22,
	switch_16_p22,
	l3_cache_16_p22,

	l2_cache_17_p22,
	switch_17_p22,
	l3_cache_17_p22,

	l2_cache_18_p22,
	switch_18_p22,
	l3_cache_18_p22,

	l2_cache_19_p22,
	switch_19_p22,
	l3_cache_19_p22,

	/*--------------*/

	l2_cache_20_p22,
	switch_20_p22,
	l3_cache_20_p22,

	l2_cache_21_p22,
	switch_21_p22,
	l3_cache_21_p22,

	/*--------------*/

	hub_iommu_22_p22,
	switch_22_p22,
	sys_agent_22_p22,

	node_number_p22
};

enum node_map_p18{

	l2_cache_0_p18 = 0,
	switch_0_p18,
	l3_cache_0_p18,

	l2_cache_1_p18,
	switch_1_p18,
	l3_cache_1_p18,

	l2_cache_2_p18,
	switch_2_p18,
	l3_cache_2_p18,

	l2_cache_3_p18,
	switch_3_p18,
	l3_cache_3_p18,

	/*--------------*/

	l2_cache_4_p18,
	switch_4_p18,
	l3_cache_4_p18,

	l2_cache_5_p18,
	switch_5_p18,
	l3_cache_5_p18,

	l2_cache_6_p18,
	switch_6_p18,
	l3_cache_6_p18,

	l2_cache_7_p18,
	switch_7_p18,
	l3_cache_7_p18,

	/*--------------*/

	l2_cache_8_p18,
	switch_8_p18,
	l3_cache_8_p18,

	l2_cache_9_p18,
	switch_9_p18,
	l3_cache_9_p18,

	l2_cache_10_p18,
	switch_10_p18,
	l3_cache_10_p18,

	l2_cache_11_p18,
	switch_11_p18,
	l3_cache_11_p18,

	/*--------------*/

	l2_cache_12_p18,
	switch_12_p18,
	l3_cache_12_p18,

	l2_cache_13_p18,
	switch_13_p18,
	l3_cache_13_p18,

	l2_cache_14_p18,
	switch_14_p18,
	l3_cache_14_p18,

	l2_cache_15_p18,
	switch_15_p18,
	l3_cache_15_p18,

	l2_cache_16_p18,
	switch_16_p18,
	l3_cache_16_p18,

	l2_cache_17_p18,
	switch_17_p18,
	l3_cache_17_p18,

	hub_iommu_18_p18,
	switch_18_p18,
	sys_agent_18_p18,

	node_number_p18
};

enum node_map_p16{

	l2_cache_0_p16 = 0,
	switch_0_p16,
	l3_cache_0_p16,

	l2_cache_1_p16,
	switch_1_p16,
	l3_cache_1_p16,

	l2_cache_2_p16,
	switch_2_p16,
	l3_cache_2_p16,

	l2_cache_3_p16,
	switch_3_p16,
	l3_cache_3_p16,

	/*--------------*/

	l2_cache_4_p16,
	switch_4_p16,
	l3_cache_4_p16,

	l2_cache_5_p16,
	switch_5_p16,
	l3_cache_5_p16,

	l2_cache_6_p16,
	switch_6_p16,
	l3_cache_6_p16,

	l2_cache_7_p16,
	switch_7_p16,
	l3_cache_7_p16,

	/*--------------*/

	l2_cache_8_p16,
	switch_8_p16,
	l3_cache_8_p16,

	l2_cache_9_p16,
	switch_9_p16,
	l3_cache_9_p16,

	l2_cache_10_p16,
	switch_10_p16,
	l3_cache_10_p16,

	l2_cache_11_p16,
	switch_11_p16,
	l3_cache_11_p16,

	/*--------------*/

	l2_cache_12_p16,
	switch_12_p16,
	l3_cache_12_p16,

	l2_cache_13_p16,
	switch_13_p16,
	l3_cache_13_p16,

	l2_cache_14_p16,
	switch_14_p16,
	l3_cache_14_p16,

	l2_cache_15_p16,
	switch_15_p16,
	l3_cache_15_p16,

	hub_iommu_16_p16,
	switch_16_p16,
	sys_agent_16_p16,

	node_number_p16
};


enum node_map_p8{

	l2_cache_0_p8 = 0,
	switch_0_p8,
	l3_cache_0_p8,

	l2_cache_1_p8,
	switch_1_p8,
	l3_cache_1_p8,

	l2_cache_2_p8,
	switch_2_p8,
	l3_cache_2_p8,

	l2_cache_3_p8,
	switch_3_p8,
	l3_cache_3_p8,

	/*--------------*/

	l2_cache_4_p8,
	switch_4_p8,
	l3_cache_4_p8,

	l2_cache_5_p8,
	switch_5_p8,
	l3_cache_5_p8,

	l2_cache_6_p8,
	switch_6_p8,
	l3_cache_6_p8,

	l2_cache_7_p8,
	switch_7_p8,
	l3_cache_7_p8,

	hub_iommu_8_p8,
	switch_8_p8,
	sys_agent_8_p8,

	node_number_p8
};

enum node_map_p4{

	l2_cache_0_p4 = 0,
	switch_0_p4,
	l3_cache_0_p4,

	l2_cache_1_p4,
	switch_1_p4,
	l3_cache_1_p4,

	l2_cache_2_p4,
	switch_2_p4,
	l3_cache_2_p4,

	l2_cache_3_p4,
	switch_3_p4,
	l3_cache_3_p4,

	hub_iommu_4_p4,
	switch_4_p4,
	sys_agent_4_p4,

	node_number_p4
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

	/*----------*/

	l1_i_cache_8,
	l1_d_cache_8,

	l1_i_cache_9,
	l1_d_cache_9,

	l1_i_cache_10,
	l1_d_cache_10,

	l1_i_cache_11,
	l1_d_cache_11,

	l1_i_cache_12,
	l1_d_cache_12,

	l1_i_cache_13,
	l1_d_cache_13,

	l1_i_cache_14,
	l1_d_cache_14,

	l1_i_cache_15,
	l1_d_cache_15,

	/*----------*/

	l1_i_cache_16,
	l1_d_cache_16,

	l1_i_cache_17,
	l1_d_cache_17,

	l1_i_cache_18,
	l1_d_cache_18,

	l1_i_cache_19,
	l1_d_cache_19,

	l1_i_cache_20,
	l1_d_cache_20,

	l1_i_cache_21,
	l1_d_cache_21,

	l1_i_cache_22,
	l1_d_cache_22,

	l1_i_cache_23,
	l1_d_cache_23,

	/*----------*/

	l1_i_cache_24,
	l1_d_cache_24,

	l1_i_cache_25,
	l1_d_cache_25,

	l1_i_cache_26,
	l1_d_cache_26,

	l1_i_cache_27,
	l1_d_cache_27,

	l1_i_cache_28,
	l1_d_cache_28,

	l1_i_cache_29,
	l1_d_cache_29,

	l1_i_cache_30,
	l1_d_cache_30,

	l1_i_cache_31,
	l1_d_cache_31,

	l1_i_cache_32,
	l1_d_cache_32,

	l1_i_cache_33,
	l1_d_cache_33,

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

		l2_caches_8,
		l2_caches_9,
		l2_caches_10,
		l2_caches_11,
		l2_caches_12,
		l2_caches_13,
		l2_caches_14,
		l2_caches_15,

		l2_caches_16,
		l2_caches_17,
		l2_caches_18,
		l2_caches_19,
		l2_caches_20,
		l2_caches_21,
		l2_caches_22,
		l2_caches_23,

		l2_caches_24,
		l2_caches_25,
		l2_caches_26,
		l2_caches_27,
		l2_caches_28,
		l2_caches_29,
		l2_caches_30,
		l2_caches_31,

		l2_caches_32,
		l2_caches_33,

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


extern struct str_map_t node_strn_map_p34;
extern struct str_map_t node_strn_map_p32;
extern struct str_map_t node_strn_map_p22;
extern struct str_map_t node_strn_map_p18;
extern struct str_map_t node_strn_map_p16;
extern struct str_map_t node_strn_map_p8;
extern struct str_map_t node_strn_map_p4;

extern struct str_map_t *node_strn_map;

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

struct cgm_packet_t *switch_north_io_ctrl_get_packet(struct switch_t *switches, enum switch_io_lane_map current_io_lane);
struct cgm_packet_t *switch_east_io_ctrl_get_packet(struct switch_t *switches, enum switch_io_lane_map current_io_lane);
struct cgm_packet_t *switch_south_io_ctrl_get_packet(struct switch_t *switches, enum switch_io_lane_map current_io_lane);
struct cgm_packet_t *switch_west_io_ctrl_get_packet(struct switch_t *switches, enum switch_io_lane_map current_io_lane);

eventcount volatile *switch_get_io_ec_counter(struct switch_t *switches);
struct list_t *switch_get_rx_queue(struct switch_t *switches, enum port_name queue);
struct list_t *switch_get_tx_queue(struct switch_t *switches, enum port_name queue);
struct cgm_packet_t *switch_get_rx_packet(struct switch_t *switches);
enum port_name crossbar_get_port_link_status(struct switch_t *switches);
struct list_t *crossbar_get_output_queue(struct switch_t *switches);
enum switch_crossbar_lane_map switch_get_next_crossbar_lane(enum switch_crossbar_lane_map current_crossbar_lane);
enum switch_io_lane_map get_next_io_lane_rb(enum switch_io_lane_map current_io_lane);
float switch_get_distance(int dest_node, int src_node);
int switch_can_access(struct list_t *queue);
struct crossbar_t *switch_crossbar_create(void);
void switch_crossbar_clear_state(struct switch_t *switches);
void switch_crossbar_link(struct switch_t *switches);
enum port_name switch_get_route(struct switch_t *switches, struct cgm_packet_t *message_packet);
void switch_set_link(struct switch_t *switches, enum port_name tx_port);
enum port_name get_next_queue_rb(enum port_name queue);
struct cgm_packet_t *get_from_queue(struct switch_t *switches);
//struct list_t *switch_get_in_queue(struct switch_t *switches, enum port_name queue);
void remove_from_queue(struct switch_t *switches, struct cgm_packet_t *message_packet);

void switch_check_access_type(struct cgm_packet_t *message_packet);

void switch_dump_stats(struct cgm_stats_t *cgm_stat_container);
void switch_reset_stats(void);
void switch_store_stats(struct cgm_stats_t *cgm_stat_container);

void switch_dump_queue(struct list_t *queue);


//void route_create(void);
//void get_path(void);

#endif /* SWITCH_H_ */

