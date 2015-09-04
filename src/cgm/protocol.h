/*
 * protocol.h
 *
 *  Created on: Apr 6, 2015
 *      Author: stardica
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_


#include <lib/util/linked-list.h>

#include <cgm/misc.h>
/*#include <cgm/cgm.h>*/
#include <cgm/cache.h>
#include <cgm/switch.h>
#include <cgm/hub-iommu.h>
#include <cgm/sys-agent.h>


enum protocol_kind_t {
	cgm_protocol_mesi = 0,
	cgm_protocol_moesi,
	cgm_protocol_gmesi,
	cgm_protocol_non_coherent,
	num_cgm_protocol_types
};


enum cgm_access_kind_t {
	cgm_access_invalid = 0,
	cgm_access_fetch,
	cgm_access_load,
	cgm_access_store,
	cgm_access_nc_store,
	cgm_access_nc_load,
	cgm_access_store_v,
	cgm_access_load_s,
	cgm_access_load_v,
	cgm_access_prefetch,
	cgm_access_gets, //get shared
	cgm_access_gets_i,
	cgm_access_get, //get specific to d caches
	cgm_access_get_fwd,
	cgm_access_gets_s, //get shared specific to s caches
	cgm_access_gets_v, //get shared specific to v caches
	cgm_access_getx, //get exclusive (or get with intent to write)
	cgm_access_inv,  //invalidation request
	cgm_access_inv_ack,
	cgm_access_upgrade, //upgrade request
	cgm_access_upgrade_ack,
	cgm_access_downgrade, //downgrade request
	cgm_access_downgrade_ack,
	cgm_access_downgrade_nack,
	cgm_access_mc_get,	//request sent to system agent/memory controller
	cgm_access_mc_put,	//reply from system agent/memory controller
	cgm_access_put_clnx, //put block in exclusive or modified state
	cgm_access_putx, //request for write back of cache block exclusive data.
	cgm_access_puts, //request for write back of cache block in shared state.
	cgm_access_puto, //request for write back of cache block in owned state.
	cgm_access_puto_shared, //request for write back of cache block in owned state but other sharers of the block exist.
	cgm_access_unblock, //message to unblock next cache level/directory for blocking protocols.
	cgm_access_retry,
	cgm_access_fetch_retry,
	cgm_access_load_retry,
	cgm_access_store_retry,
	cgm_access_write_back,
	cgm_access_retry_i,//not used
	num_access_types
};

extern struct str_map_t protocol_kind_strn_map;
extern struct str_map_t cgm_mem_access_strn_map;

extern enum protocol_kind_t cgm_cache_protocol;
extern enum protocol_kind_t cgm_gpu_cache_protocol;

struct cgm_packet_t{

	char *name;

	//star todo clean this up when the simulator is done.
	enum cgm_access_kind_t access_type;
	enum cgm_access_kind_t l1_access_type;
	enum cgm_access_kind_t cpu_access_type;
	enum cgm_access_kind_t gpu_access_type;

	int l1_cache_id;
	char *l2_cache_name;
	int l2_cache_id;
	int gpu_cache_id;

	//access data
	long long access_id;
	unsigned int address;
	int set;
	int tag;
	int way;
	unsigned int offset;
	int size;
	int coalesced;

	//for evictions, write backs, downgrades
	int flush_pending;
	int downgrade;
	int downgrade_pending;
	int downgrade_ack;
	int inval;
	int inval_pending;
	int inval_ack;


	int l1_victim_way;
	int l2_victim_way;
	int l3_victim_way;

	//for protocol messages
	enum cgm_cache_block_state_t cache_block_state;

	//for routing
	char *src_name;
	int src_id;
	char *dest_name;
	int dest_id;

	//for m2s CPU and GPU
	struct linked_list_t *event_queue;
	int *witness_ptr;
	void *data;

	//stats
	long long start_cycle;
	long long end_cycle;
};

struct cgm_packet_status_t{

	//used for global memory list
	enum cgm_access_kind_t access_type;
	unsigned int address;
	long long access_id;
	int in_flight;
};

/*struct cgm_packet_status_t;*/

struct cgm_packet_t *packet_create(void);
void packet_destroy(struct cgm_packet_t *packet);
struct cgm_packet_status_t *status_packet_create(void);
void status_packet_destroy(struct cgm_packet_status_t *status_packet);
void init_write_back_packet(struct cache_t *cache, struct cgm_packet_t *write_back_packet, int set, int tag, int pending, enum cgm_cache_block_state_t cache_block_state);
void init_reply_packet(struct cgm_packet_t *reply_packet, unsigned int address);
void init_downgrade_packet(struct cgm_packet_t *downgrade_packet, unsigned int address);
void init_downgrade_ack_packet(struct cgm_packet_t *reply_packet, unsigned int address);
void init_flush_packet(struct cache_t *cache, struct cgm_packet_t *inval_packet, int set, int way);

////////////////
/////CPU MESI protocol V2
////////////////



//implements a MESI protocol.
void cgm_mesi_fetch(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_store(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_i_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l1_d_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l1_d_downgrade(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_gets(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l2_get_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cgm_mesi_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_gets(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cgm_mesi_l3_downgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet);


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
