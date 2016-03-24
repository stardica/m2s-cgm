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
		hub_iommu_0,
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
	invalid_queue = 0,
	north_queue,
	east_queue,
	south_queue,
	west_queue,
	port_num

};

enum arbitrate{

	round_robin = 0,
	prioity

};

//star todo this is currently only programmed for a 4 port switch
struct crossbar_t{

	int num_ports;
	int num_pairs;

	//in queues
	enum port_name north_in_out_linked_queue;
	enum port_name east_in_out_linked_queue;
	enum port_name south_in_out_linked_queue;
	enum port_name west_in_out_linked_queue;
};

struct switch_t{

	char *name;
	int switch_id;
	int switch_node_number;
	float switch_median_node;
	int port_num;
	int latency;
	int bus_width;

	/*int num_routes;
	struct route_t *my_routes;*/

	enum port_name queue;
	enum arbitrate arb_style;
	unsigned int wire_latency;

	//crossbar
	struct crossbar_t *crossbar;


	//for switches with 4 ports
	struct list_t *north_queue;
	struct list_t *Tx_north_queue;
	//struct list_t *north_queue_lane1;
	//struct list_t *north_queue_lane2;
	struct list_t *east_queue;
	struct list_t *Tx_east_queue;
	//struct list_t *east_queue_lane1;
	//struct list_t *east_queue_lane2;
	struct list_t *south_queue;
	struct list_t *Tx_south_queue;
	//struct list_t *south_queue_lane1;
	//struct list_t *south_queue_lane2;
	struct list_t *west_queue;
	struct list_t *Tx_west_queue;
	//struct list_t *west_queue_lane1;
	//struct list_t *west_queue_lane2;

	//io ctrl
	eventcount volatile *switches_north_io_ec;
	task *switches_north_io_tasks;

	eventcount volatile *switches_east_io_ec;
	task *switches_east_io_tasks;

	eventcount volatile *switches_south_io_ec;
	task *switches_south_io_tasks;

	eventcount volatile *switches_west_io_ec;
	task *switches_west_io_tasks;

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

	/*switch stats*/
	long long switch_total_links;
	int switch_max_links;
	long long switch_total_wakes;
	long long switch_north_io_transfers;
	long long switch_north_io_transfer_cycles;
	long long switch_north_io_bytes_transfered;
	long long switch_east_io_transfers;
	long long switch_east_io_transfer_cycles;
	long long switch_east_io_bytes_transfered;
	long long switch_south_io_transfers;
	long long switch_south_io_transfer_cycles;
	long long switch_south_io_bytes_transfered;
	long long switch_west_io_transfers;
	long long switch_west_io_transfer_cycles;
	long long switch_west_io_bytes_transfered;

	long long north_txqueue_max_depth;
	double north_txqueue_ave_depth;
	long long east_txqueue_max_depth;
	double east_txqueue_ave_depth;
	long long south_txqueue_max_depth;
	double south_txqueue_ave_depth;
	long long west_txqueue_max_depth;
	double west_txqueue_ave_depth;

	long long north_tx_inserts;
	long long east_tx_inserts;
	long long south_tx_inserts;
	long long west_tx_inserts;

	long long north_rxqueue_max_depth;
	double north_rxqueue_ave_depth;
	long long east_rxqueue_max_depth;
	double east_rxqueue_ave_depth;
	long long south_rxqueue_max_depth;
	double south_rxqueue_ave_depth;
	long long west_rxqueue_max_depth;
	double west_rxqueue_ave_depth;

	long long north_rx_inserts;
	long long east_rx_inserts;
	long long south_rx_inserts;
	long long west_rx_inserts;

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
void switch_west_io_ctrl(void);
void switch_south_io_ctrl(void);

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

void switch_dump_stats(struct cgm_stats_t *cgm_stat_container);
void switch_reset_stats(void);

//void route_create(void);
//void get_path(void);

#endif /* SWITCH_H_ */
