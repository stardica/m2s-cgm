/*
 * cache.h
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */


#ifndef CACHE_H_
#define CACHE_H_

#include <cgm/cgm.h>
#include <cgm/tasking.h>
#include <cgm/packet.h>
#include <cgm/mshr.h>

#include <lib/util/string.h>


#define WIRE_DELAY(wire_latency) (etime.count + (wire_latency *2))


//star todo prefetching, and coalescing
extern struct str_map_t cache_policy_map;
extern struct str_map_t cache_block_state_map;
extern struct str_map_t cgm_mem_access_strn_map;


enum cache_type_enum
{
	l1_i_cache_t,
	l1_d_cache_t,
	l2_cache_t,
	l3_cache_t
};


enum cache_waylist_enum
{
	cache_waylist_head,
	cache_waylist_tail
};

enum cache_policy_t{

	cache_policy_invalid = 0,
	cache_policy_lru,
	cache_policy_fifo,
	cache_policy_random
};

enum cache_block_state_t{

	cache_block_invalid = 0,
	cache_block_noncoherent,
	cache_block_modified,
	cache_block_owned,
	cache_block_exclusive,
	cache_block_shared
};

struct cache_block_t{

	struct cache_block_t *way_next;
	struct cache_block_t *way_prev;

	int tag;
	int transient_tag;
	int way;
	int prefetched;

	enum cache_block_state_t state;
};

struct cache_set_t{

	int id;

	struct cache_block_t *way_head;
	struct cache_block_t *way_tail;
	struct cache_block_t *blocks;

	//directory
	unsigned int *state;

};


struct cache_t{

	//star >> my added elements.
	char *name;
	int id;

	enum cache_type_enum cache_type;

	//this is so the cache can advance itself
	eventcount *ec_ptr;

	//cache configuration settings
	unsigned int num_slices;
	unsigned int num_sets;
	unsigned int block_size;
	unsigned int assoc;
	unsigned int num_ports;
	//enum cache_policy_t policy;
	const char *policy;

	//cache data
	struct cache_set_t *sets;
	unsigned int block_mask;
	int log_block_size;
	unsigned int set_mask;
	int log_set_size;

	//mshr control links
	int mshr_size;
	struct mshr_t *mshrs;

	//outstanding request table
	int **ort;
	struct list_t *ort_list;
	int max_coal;


	//cache queues
	struct list_t *Rx_queue_top;
	struct list_t *Rx_queue_bottom;
	struct list_t *next_queue;
	struct list_t *last_queue;
	struct list_t *retry_queue;

	//physical characteristics
	unsigned int latency;
	unsigned int wire_latency;
	unsigned int directory_latency;

	//directory bit vectors for coherence
	unsigned int dir_latency;

	//statistics
	long long fetches;
	long long loads;
	long long stores;
	long long hits;
	long long invalid_hits;
	long long misses;
	long long retries;
	long long coalesces;
	long long mshr_entires;

};

//global variables.
//star todo bring this into the cache struct
extern int QueueSize;
extern int gpu_l2_qty;
int mem_miss;

//CPU caches
extern struct cache_t *l1_i_caches;
extern struct cache_t *l1_d_caches;
extern struct cache_t *l2_caches;
extern struct cache_t *l3_caches;

//GPU caches
extern struct cache_t *gpu_v_caches;
extern struct cache_t *gpu_s_caches;
extern struct cache_t *gpu_l2_caches;
extern struct cache_t *gpu_lds_units;

//tasking related
extern int l1_i_pid;
extern int l1_d_pid;
extern int l2_pid;
extern int l3_pid;
extern int gpu_v_pid;
extern int gpu_s_pid;
extern int gpu_l2_pid;
extern int gpu_lds_pid;

//event counts
extern eventcount volatile *l1_i_cache;
extern eventcount volatile *l1_d_cache;
extern eventcount volatile *l2_cache;
extern eventcount volatile *l3_cache;
extern eventcount volatile *gpu_l2_cache;
extern eventcount volatile *gpu_v_cache;
extern eventcount volatile *gpu_s_cache;
extern eventcount volatile *gpu_lds_unit;

//tasks
extern task *l1_i_cache_tasks;
extern task *l1_d_cache_tasks;
extern task *l2_cache_tasks;
extern task *l3_cache_tasks;
extern task *gpu_l2_cache_tasks;
extern task *gpu_v_cache_tasks;
extern task *gpu_s_cache_tasks;
extern task *gpu_lds_tasks;

//simulator functions
void cache_init(void);
void cache_create(void);
void cache_create_tasks(void);
void cache_dump_stats(void);

//tasks
void l1_i_cache_ctrl(void);
void l1_d_cache_ctrl(void);
void l2_cache_ctrl(void);
void l3_cache_ctrl(void);
void gpu_s_cache_ctrl(void);
void gpu_v_cache_ctrl(void);
void gpu_l2_cache_ctrl(void);
void gpu_lds_unit_ctrl(void);



//cache functions
struct cgm_packet_t *cache_get_message(struct cache_t *cache);
void cpu_l1_cache_access_load(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cpu_l1_cache_access_store(struct cache_t *cache, struct cgm_packet_t *message_packet);
//void cpu_l1_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cpu_cache_access_get(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cpu_cache_access_put(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cpu_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet);


void cpu_cache_coalesced_retry(struct cache_t *cache, int *tag_ptr, int *set_ptr);

int cgm_l3_cache_map(int *set);
int cgm_l2_cache_map(int src_id);

int cache_can_access_top(struct cache_t *cache);
int cache_can_access_bottom(struct cache_t *cache);


int cgm_gpu_cache_map(int cache_id);

//borrowed from m2s mem-system and tweaked a bit
void cgm_cache_decode_address(struct cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr);
int cgm_cache_find_block(struct cache_t *cache, int *tag_ptr, int *set_ptr, unsigned int *offset_ptr, int *way_ptr, int *state_ptr);
void cgm_cache_set_block(struct cache_t *cache, int set, int way, int tag, int state);
//void cache_get_block(struct cache_t *cache, int set, int way, int *tag_ptr, int *state_ptr);
//void cache_access_block(struct cache_t *cache, int set, int way);
//int cache_replace_block(struct cache_t *cache, int set);
//void cache_set_transient_tag(struct cache_t *cache, int set, int way, int tag);
void cgm_cache_update_waylist(struct cache_set_t *set, struct cache_block_t *blk, enum cache_waylist_enum where);


#endif /*CACHE_H_*/
