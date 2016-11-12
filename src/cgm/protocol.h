/*
 * protocol.h
 *
 *  Created on: Apr 6, 2015
 *      Author: stardica
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <cgm/switch.h>
#include <cgm/hub-iommu.h>
#include <cgm/sys-agent.h>
#include <cgm/cgm.h>

#include <lib/util/linked-list.h>
#include <cgm/configure.h>

/*((x86_cpu_frequency * MHZ) / 1000) = 1ms*/
/*((x86_cpu_frequency * MHZ) / 500000) = 0.5ms*/
/*((x86_cpu_frequency * MHZ) / 750000) = 0.25ms*/
/*((x86_cpu_frequency * MHZ) / 1000000) = 1us*/
#define EPOCH ((x86_cpu_frequency * MHZ) / 500000)

extern struct str_map_t protocol_kind_strn_map;
extern struct str_map_t cgm_mem_access_strn_map;

extern enum protocol_kind_t cgm_cache_protocol;
extern enum protocol_kind_t cgm_gpu_cache_protocol;

enum bandwidth_type_t{
	bytes_invalid,
	bytes_tx,
	bytes_rx,
	total_types
};

struct mem_system_bandwidth_t{

	int core_id;
	long long epoch;
	long long bytes_tx;
	long long bytes_rx;
};

/*mem system stats*/
struct mem_system_stats_t{

	int first_mem_access_lat;
	long long fetch_lat_hist[HISTSIZE];
	long long load_lat_hist[HISTSIZE];
	long long store_lat_hist[HISTSIZE];

	long long cpu_total_fetch_requests;
	long long cpu_total_fetch_replys;
	long long fetch_l1_hits;
	long long l2_total_fetch_requests;
	long long fetch_l2_hits;
	long long l3_total_fetch_requests;
	long long fetch_l3_hits;
	long long fetch_memory;

	long long cpu_total_load_requests;
	long long cpu_total_load_replys;
	long long load_l1_hits;
	long long l2_total_load_requests;
	long long load_l2_hits;
	long long l3_total_load_requests;
	long long load_l3_hits;
	long long load_memory;
	long long load_get_fwd;
	long long l1_load_nack;
	long long l2_load_nack;
	long long l3_load_nack;

	long long cpu_total_store_requests;
	long long cpu_total_store_replys;
	long long store_l1_hits;
	long long l2_total_store_requests;
	long long store_l2_hits;
	long long l3_total_store_requests;
	long long store_l3_hits;
	long long store_memory;
	long long store_getx_fwd;
	long long store_upgrade;
	long long l1_store_nack;
	long long l2_store_nack;
	long long l3_store_nack;

	long long gpu_total_loads;
	long long gpu_total_stores;

	long long l3_loops;

	/*timing stats*/
	long long l1_i_hit_time;
	long long l1_d_load_hit_time;
	long long l1_d_store_hit_time;

};


#define SETROUTE(packet, source, dest)															\
								packet->src_name = source->name;								\
								packet->src_id = str_map_string(&node_strn_map, source->name);	\
								packet->dest_name = dest->name;									\
								packet->dest_id = str_map_string(&node_strn_map, dest->name);

extern struct mem_system_stats_t *mem_system_stats;

long long write_back_id;

extern int gpu_core_id;

struct  mem_system_bandwidth_t *init_bandwidth_container(void);
void store_stat_bandwidth(enum bandwidth_type_t type, int core_id, int transfer_time, int bus_width);
void dump_stat_bandwidth(void);



struct cgm_packet_t *packet_create(void);
void packet_destroy(struct cgm_packet_t *packet);
struct cgm_packet_status_t *status_packet_create(void);
void status_packet_destroy(struct cgm_packet_status_t *status_packet);
void init_write_back_packet(struct cache_t *cache, struct cgm_packet_t *write_back_packet, int set, int tag, int pending, enum cgm_cache_block_state_t cache_block_state);
void init_reply_packet(struct cgm_packet_t *reply_packet, unsigned int address);
void init_downgrade_packet(struct cgm_packet_t *downgrade_packet, unsigned int address);
/*void init_upgrade_inval_packet(struct cgm_packet_t *downgrade_packet, unsigned int address);*/
void init_getx_fwd_inval_packet(struct cgm_packet_t *downgrade_packet, unsigned int address);
void init_upgrade_request_packet(struct cgm_packet_t *upgrade_request_packet, unsigned int address);
void init_upgrade_inval_request_packet(struct cgm_packet_t *upgrade_request_packet, unsigned int address);
void init_upgrade_putx_n_inval_request_packet(struct cgm_packet_t *upgrade_request_packet, unsigned int address);
void init_downgrade_nack_packet(struct cgm_packet_t *nack_packet, unsigned int address);
void init_upgrade_ack_packet(struct cgm_packet_t *nack_packet, unsigned int address);
void init_downgrade_ack_packet(struct cgm_packet_t *reply_packet, unsigned int address);
void init_getx_fwd_nack_packet(struct cgm_packet_t *reply_packet, unsigned int address);
void init_getx_fwd_ack_packet(struct cgm_packet_t *reply_packet, unsigned int address);
void init_flush_packet(struct cache_t *cache, struct cgm_packet_t *inval_packet, int set, int way);
unsigned int get_block_address(unsigned int address, unsigned int cache_address_mask);
int is_writeback_present(struct cgm_packet_t *writeback_packet);


void mem_system_dump_stats(struct cgm_stats_t *cgm_stat_container);
void mem_system_reset_stats(void);
void mem_system_store_stats(struct cgm_stats_t *cgm_stat_container);

//////////////////////
/////CPU MESI protocol
//////////////////////

//implements a multilevel cache MESI protocol.
void cgm_mesi_fetch(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_i_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_load_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_store(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_store_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);

int cgm_mesi_cpu_flush(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_gpu_flush(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_cpu_fence(struct cache_t *cache, struct cgm_packet_t *message_packet);


int cgm_mesi_l1_d_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_flush_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l1_d_downgrade(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_getx_fwd_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_upgrade_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);


void cgm_mesi_l2_gets(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l2_getx(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l2_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_flush_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_flush_block_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_cpu_flush(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_cpu_flush_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_gpu_flush(struct cache_t *cache, struct cgm_packet_t *message_packet);


void cgm_mesi_l2_get_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_get_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet); /*note this need to be changed to get_fwd_ack and nack*/
void cgm_mesi_l2_downgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);


void cgm_mesi_l2_getx_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_getx_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_getx_fwd_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_getx_fwd_inval_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);


int cgm_mesi_l2_upgrade(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l2_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_upgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_upgrade_putx_n(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_upgrade_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);


void cgm_mesi_l3_gets(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_getx(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l3_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_flush_block_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_cpu_flush(struct cache_t *cache, struct cgm_packet_t *message_packet);
/*void cgm_mesi_l3_gpu_flush(struct cache_t *cache, struct cgm_packet_t *message_packet);*/
void cgm_mesi_l3_gpu_flush_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l3_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_downgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l3_getx_fwd_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_getx_fwd_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_getx_fwd_upgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l3_get_fwd_upgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l3_upgrade(struct cache_t *cache, struct cgm_packet_t *message_packet);



//////////////////////
//GPU MESI protocol
//////////////////////

void cgm_mesi_gpu_s_load(struct cache_t *cache, struct cgm_packet_t *message_packet);


void cgm_mesi_gpu_l1_v_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l1_v_store(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_gpu_l1_v_load_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l1_v_store_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);

int cgm_mesi_gpu_l1_v_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l1_v_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l1_v_flush_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l1_v_gpu_flush(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_gpu_l1_v_get_getx_fwd_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);


//void cgm_mesi_gpu_l1_v_downgrade(struct cache_t *cache, struct cgm_packet_t *message_packet);

//void cgm_mesi_gpu_l1_v_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cgm_mesi_gpu_l1_v_upgrade_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);


void cgm_mesi_gpu_l2_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_gpu_l2_getx(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_gpu_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_gpu_l2_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l2_flush_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l2_flush_block_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_gpu_l2_get_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l2_getx_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_gpu_l2_gpu_flush(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l2_gpu_flush_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_gpu_l2_get_getx_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l2_get_getx_fwd_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l2_get_getx_fwd_inval_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);


//void cgm_mesi_gpu_l2_get_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cgm_mesi_gpu_l2_get_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cgm_mesi_gpu_l2_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cgm_mesi_gpu_l2_downgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);

//void cgm_mesi_gpu_l2_getx_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cgm_mesi_gpu_l2_getx_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cgm_mesi_gpu_l2_getx_fwd_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cgm_mesi_gpu_l2_getx_fwd_inval_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);

//int cgm_mesi_gpu_l2_upgrade(struct cache_t *cache, struct cgm_packet_t *message_packet);
//int cgm_mesi_gpu_l2_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cgm_mesi_gpu_l2_upgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cgm_mesi_gpu_l2_upgrade_putx_n(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cgm_mesi_gpu_l2_upgrade_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);



/*OLD functions*/
/*void cgm_nc_gpu_s_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_nc_gpu_s_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_nc_gpu_v_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_nc_gpu_v_store(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_nc_gpu_v_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_nc_gpu_l2_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_nc_gpu_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_gpu_s_load(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_gpu_v_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_v_store(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_v_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_v_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_gpu_l2_getx(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);*/

#endif /*PROTOCOL_H_*/
