/*
 * cache.h
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */

#ifndef CACHE_H_
#define CACHE_H_



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <lib/util/string.h>
#include <lib/util/misc.h>

#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>

#include <cgm/misc.h>
#include <cgm/tasking.h>
#include <cgm/packet.h>
//#include <cgm/cgm.h>

/*star todo fix this somehow. We shouldn't need to be included before all of
the #includes (cgm.h) is loading protocol.h before cache_block_state_t is defined*/
enum cgm_cache_block_state_t{

	cgm_cache_block_invalid = 0,
	cgm_cache_block_noncoherent,
	cgm_cache_block_modified,
	cgm_cache_block_owned,
	cgm_cache_block_exclusive,
	cgm_cache_block_shared,
	cgm_cache_block_transient,
	cgm_cache_block_null,
	cgm_cache_block_state_num
};

#include <cgm/directory.h>
#include <cgm/switch.h>
#include <cgm/hub-iommu.h>
#include <cgm/cgm.h>

#define WIRE_DELAY(wire_latency) (etime.count + (wire_latency *2))


//star todo integrate m2s prefetcher
extern struct str_map_t cgm_cache_policy_map;
extern struct str_map_t cgm_cache_block_state_map;
extern struct str_map_t cgm_mem_access_strn_map;

enum cache_type_enum{

	l1_i_cache_t,
	l1_d_cache_t,
	l2_cache_t,
	l3_cache_t,
	gpu_s_cache_t,
	gpu_v_cache_t,
	gpu_l2_cache_t
};

enum cache_waylist_enum{

	cache_waylist_head,
	cache_waylist_tail
};

enum cache_policy_t{

	cache_policy_invalid = 0,
	cache_policy_lru,
	cache_policy_fifo,
	cache_policy_random,
	cache_policy_num
};

struct cache_block_t{

	struct cache_block_t *way_next;
	struct cache_block_t *way_prev;

	int tag;
	int set;
	int transient_tag;
	int way;
	int prefetched;

	enum cgm_cache_block_state_t state;
	enum cgm_cache_block_state_t transient_state;

	//each block has it's own directory (unsigned char)
	union directory_t directory_entry;
	int data_type;

	//for error checking
	long long transient_access_id;
};

struct cache_set_t{

	int id;

	struct cache_block_t *way_head;
	struct cache_block_t *way_tail;
	struct cache_block_t *blocks;

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
	enum cache_policy_t policy;
	int policy_type;
	int slice_type;
	int bus_width;

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
	struct list_t *Tx_queue_top;
	struct list_t *Tx_queue_bottom;
	struct list_t *Coherance_Tx_queue;
	struct list_t *Coherance_Rx_queue;
	struct list_t *retry_queue;
	struct list_t *write_back_buffer;
	struct list_t *next_queue;
	struct list_t *last_queue;

	//io ctrl
	eventcount volatile *cache_io_up_ec;
	task *cache_io_up_tasks;

	eventcount volatile *cache_io_down_ec;
	task *cache_io_down_tasks;

	//physical characteristics
	unsigned int latency;
	unsigned int wire_latency;
	unsigned int directory_latency;

	//directory bit vectors for coherence
	unsigned int dir_latency;
	union directory_t **dir;

	//statistics
	long long fetches;
	long long loads;
	long long stores;
	long long hits;
	long long invalid_hits;
	long long misses;
	long long upgrade_misses;
	long long retries;
	long long coalesces;
	long long mshr_entires;
	long long stalls;
	unsigned int *fetch_address_history;
	unsigned int *load_address_history;
	unsigned int *store_address_history;

};

//global variables.
//star todo bring this into the cache struct
extern int QueueSize;
extern int l1_i_inf;
extern int l1_d_inf;
extern int l2_inf;
extern int l3_inf;
extern int l1_i_miss;
extern int l1_d_miss;
extern int l2_miss;
extern int l3_miss;
extern int gpu_l1_inf;
extern int gpu_l2_inf;

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

extern int l1_i_io_pid;
extern int l1_d_io_pid;
extern int l2_up_io_pid;
extern int l2_down_io_pid;
extern int l3_up_io_pid;
extern int l3_down_io_pid;
extern int gpu_v_io_pid;
extern int gpu_s_io_pid;
extern int gpu_l2_io_pid;
extern int gpu_lds_io_pid;

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


void run_array(void);


//cache tasks
void l1_i_cache_ctrl(void);
void l1_d_cache_ctrl(void);
void l2_cache_ctrl(void);
void l3_cache_ctrl(void);
void gpu_s_cache_ctrl(void);
void gpu_v_cache_ctrl(void);
void gpu_l2_cache_ctrl(void);
void gpu_lds_unit_ctrl(void);

//cpu
void l1_i_cache_down_io_ctrl(void);
void l1_d_cache_down_io_ctrl(void);
void l2_cache_up_io_ctrl(void);
void l2_cache_down_io_ctrl(void);
void l3_cache_up_io_ctrl(void);
void l3_cache_down_io_ctrl(void);

//gpu
void gpu_s_cache_down_io_ctrl(void);
void gpu_v_cache_down_io_ctrl(void);
void gpu_l2_cache_up_io_ctrl(void);
void gpu_l2_cache_down_io_ctrl(void);


////cache cntrl functions
struct cgm_packet_t *cache_get_message(struct cache_t *cache);
int cgm_l3_cache_map(int set);
int cache_can_access_top(struct cache_t *cache);
int cache_can_access_bottom(struct cache_t *cache);
int cache_can_access_Tx_bottom(struct cache_t *cache);
int cache_can_access_Tx_top(struct cache_t *cache);
int cgm_gpu_cache_map(int cache_id);
void cache_get_block_status(struct cache_t *cache, struct cgm_packet_t *message_packet, int *cache_block_hit_ptr, int *cache_block_state_ptr);
void cache_l1_i_return(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_l1_d_return(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_check_ORT(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_put_io_up_queue(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_put_io_down_queue(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_put_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_coalesed_retry(struct cache_t *cache, int tag_ptr, int set_ptr);
void cgm_cache_set_dir(struct cache_t *cache, int set, int way, int l2_cache_id);
void cgm_cache_clear_dir(struct cache_t *cache, int set, int way);
int cgm_cache_get_dir_dirty_bit(struct cache_t *cache, int set, int way);
void cgm_cache_set_block_transient_state(struct cache_t *cache, int set, int way, long long id, enum cgm_cache_block_state_t t_state);
enum cgm_cache_block_state_t cgm_cache_get_block_transient_state(struct cache_t *cache, int set, int way);
long long cgm_cache_get_block_transient_state_id(struct cache_t *cache, int set, int way);
enum cgm_access_kind_t cgm_cache_get_retry_state(enum cgm_access_kind_t r_state);



//lower level cache functions
void cgm_cache_probe_address(struct cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr);
unsigned int cgm_cache_build_address(struct cache_t *cache, int set, int tag);
int cgm_cache_find_block(struct cache_t *cache, int *tag_ptr, int *set_ptr, unsigned int *offset_ptr, int *way_ptr, int *state_ptr);
int cgm_cache_get_way(struct cache_t *cache, int tag, int set);
void cgm_cache_evict_block(struct cache_t *cache, int set, int way);
void cgm_cache_inval_block(struct cache_t *cache, int set, int way);
void cgm_cache_set_block(struct cache_t *cache, int set, int way, int tag, int state);
void cgm_cache_set_block_type(struct cache_t *cache, int type, int set, int way);
int cgm_cache_get_block_type(struct cache_t *cache, int set, int way, int tag);
void cgm_cache_set_block_state(struct cache_t *cache, int set, int way, enum cgm_cache_block_state_t state);
enum cgm_cache_block_state_t cgm_cache_get_block_state(struct cache_t *cache, int set, int way);
void cgm_cache_get_block(struct cache_t *cache, int set, int way, int *tag_ptr, int *state_ptr);
void cgm_cache_access_block(struct cache_t *cache, int set, int way);
int cgm_cache_replace_block(struct cache_t *cache, int set);
//void cgm_cache_set_transient_tag(struct cache_t *cache, int set, int way, int tag);
void cgm_cache_update_waylist(struct cache_set_t *set, struct cache_block_t *blk, enum cache_waylist_enum where);

//lower level ORT functions
int get_ort_status(struct cache_t *cache);
int get_ort_num_coalesced(struct cache_t *cache, int entry, int tag, int set);
int ort_search(struct cache_t *cache, int tag, int set);
void ort_clear(struct cache_t *cache, struct cgm_packet_t *message_packet);
void ort_set(struct cache_t *cache, int entry, int tag, int set);
void ort_dump(struct cache_t *cache);


#endif /*CACHE_H_*/

