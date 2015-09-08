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

#include <cgm/misc.h>

extern struct str_map_t protocol_kind_strn_map;
extern struct str_map_t cgm_mem_access_strn_map;

extern enum protocol_kind_t cgm_cache_protocol;
extern enum protocol_kind_t cgm_gpu_cache_protocol;

struct cgm_packet_t *packet_create(void);
void packet_destroy(struct cgm_packet_t *packet);
struct cgm_packet_status_t *status_packet_create(void);
void status_packet_destroy(struct cgm_packet_status_t *status_packet);
void init_write_back_packet(struct cache_t *cache, struct cgm_packet_t *write_back_packet, int set, int tag, int pending, enum cgm_cache_block_state_t cache_block_state);
void init_reply_packet(struct cgm_packet_t *reply_packet, unsigned int address);
void init_downgrade_packet(struct cgm_packet_t *downgrade_packet, unsigned int address);
void init_getx_fwd_inval_packet(struct cgm_packet_t *downgrade_packet, unsigned int address);
void init_downgrade_ack_packet(struct cgm_packet_t *reply_packet, unsigned int address);
void init_getx_fwd_ack_packet(struct cgm_packet_t *reply_packet, unsigned int address);
void init_flush_packet(struct cache_t *cache, struct cgm_packet_t *inval_packet, int set, int way);

//////////////////////
/////CPU MESI protocol
//////////////////////


//implements a MESI protocol.
void cgm_mesi_fetch(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_store(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l1_i_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l1_d_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_downgrade(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_getx_fwd_inval(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l2_gets(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_get_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_getx_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_inval_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_getx_fwd_inval_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);

void cgm_mesi_l3_gets(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_getx(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_downgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_getx_fwd_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_getx_fwd_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);


//////////////////
/////GPU protocol
//////////////////

void cgm_nc_gpu_load(struct cache_t *cache, struct cgm_packet_t *message_packet);


void gpu_l1_cache_access_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void gpu_l1_cache_access_store(struct cache_t *cache, struct cgm_packet_t *message_packet);
void gpu_cache_access_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
void gpu_cache_access_put(struct cache_t *cache, struct cgm_packet_t *message_packet);
void gpu_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet);

void gpu_cache_coalesced_retry(struct cache_t *cache, int *tag_ptr, int *set_ptr);

#endif /*PROTOCOL_H_*/
