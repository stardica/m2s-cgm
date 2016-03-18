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

#define HISTSIZE 20000

extern struct str_map_t protocol_kind_strn_map;
extern struct str_map_t cgm_mem_access_strn_map;

extern enum protocol_kind_t cgm_cache_protocol;
extern enum protocol_kind_t cgm_gpu_cache_protocol;

/*mem system stats*/
extern int first_mem_access_lat;
extern long long fetch_lat_hist[HISTSIZE];
extern long long load_lat_hist[HISTSIZE];
extern long long store_lat_hist[HISTSIZE];

extern long long fetch_l1_hits;
extern long long fetch_l2_hits;
extern long long fetch_l3_hits;
extern long long fetch_memory;

extern long long load_l1_hits;
extern long long load_l2_hits;
extern long long load_l3_hits;
extern long long load_memory;
extern long long load_get_fwd;

extern long long store_l1_hits;
extern long long store_l2_hits;
extern long long store_l2_upgrade;
extern long long store_l3_hits;
extern long long store_l3_upgrade;
extern long long store_memory;
extern long long store_getx_fwd;
extern long long store_upgrade;

extern long long cpu_total_fetches;
extern long long cpu_total_loads;
extern long long cpu_total_stores;

extern long long cpu_ls_stalls;

	/*gpu stats*/
extern long long gpu_total_loads;
extern long long gpu_total_stores;

long long write_back_id;

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










//////////////////////
/////CPU MESI protocol
//////////////////////

//implements a MESI protocol.
void cgm_mesi_fetch(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_i_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_store(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l1_d_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l1_d_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_downgrade(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_getx_fwd_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_upgrade_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_flush_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l2_gets(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l2_getx(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l2_get_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_getx_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l2_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_flush_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_flush_block_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_get_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_downgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_getx_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_getx_fwd_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_getx_fwd_inval_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l2_upgrade(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_upgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_upgrade_putx_n(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_upgrade_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);


void cgm_mesi_l3_gets(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_getx(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

int cgm_mesi_l3_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_flush_block_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l3_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_downgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l3_getx_fwd_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_getx_fwd_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l3_getx_fwd_upgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_get_fwd_upgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l3_upgrade(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cgm_mesi_l3_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);

/////////////////////
/////GPU NC protocol
/////////////////////
void cgm_nc_gpu_s_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_nc_gpu_s_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_nc_gpu_v_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_nc_gpu_v_store(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_nc_gpu_v_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_nc_gpu_l2_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_nc_gpu_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

//////////////////////
/////GPU MESI protocol
//////////////////////
void cgm_mesi_gpu_s_load(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_gpu_v_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_v_store(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_v_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_v_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_gpu_l2_getx(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_gpu_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

#endif /*PROTOCOL_H_*/
