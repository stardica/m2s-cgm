/*
 * cache.h
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */


#ifndef CACHE_H_
#define CACHE_H_

//star todo add prefetching and coalescing

extern struct str_map_t cache_policy_map;
extern struct str_map_t cache_block_state_map;

enum cache_policy_t
{
	cache_policy_invalid = 0,
	cache_policy_lru,
	cache_policy_fifo,
	cache_policy_random
};

enum cache_block_state_t
{
	cache_block_invalid = 0,
	cache_block_noncoherent,
	cache_block_modified,
	cache_block_owned,
	cache_block_exclusive,
	cache_block_shared
};

struct cache_block_t
{
	struct cache_block_t *way_next;
	struct cache_block_t *way_prev;

	int tag;
	int transient_tag;
	int way;
	int prefetched;

	enum cache_block_state_t state;
};

struct cache_set_t
{
	struct cache_block_t *way_head;
	struct cache_block_t *way_tail;
	struct cache_block_t *blocks;
};


struct cache_t
{

	//star >> my added elements.
	char * name;

	//cache configuration settings
	unsigned int num_sets;
	unsigned int block_size;
	unsigned int assoc;
	unsigned int num_ports;

	//connections
	//cache list of queues
	//a cache maybe multi-ported i.e. a shared L2 cache.
	struct list_t *in_queues;
	struct list_t *out_queues;

	//still the old elements.
	//enum cache_policy_t policy;
	const char *policy;
	unsigned int latency;
	unsigned int mshr_size;
	unsigned int directory_latency;

	//cache data
	struct cache_set_t *sets;
	unsigned int block_mask;
	int log_block_size;

	//struct prefetcher_t *prefetcher;
};


//CPU caches
extern struct cache_t *l1_i_caches;
extern struct cache_t *l1_d_caches;
extern struct cache_t *l2_caches;
extern struct cache_t *l3_s0_cache;
extern struct cache_t *l3_s1_cache;
extern struct cache_t *l3_s2_cache;
extern struct cache_t *l3_s3_cache;

//GPU caches
extern struct cache_t *l1_v_caches;
extern struct cache_t *l1_s_caches;
extern struct cache_t *l2_caches;
extern struct cache_t *lds_units;


//star todo write functions for cache access, processing and reply.
//function prototypes
struct cache_t *cgm_cache_create(void);
void cgm_cache_configure(void);
void cache_init(void);
void connect_queue(struct list_t *queue);
void cache_poll_queues(void);
int cache_ctrl(struct list_t *queue);



#endif /*CACHE_H_*/
