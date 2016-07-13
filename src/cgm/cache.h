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

/*#include <lib/util/debug.h>*/
#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <lib/util/string.h>
#include <lib/util/misc.h>

#include <cgm/cgm-struct.h>
#include <cgm/protocol.h>
#include <cgm/tasking.h>
#include <cgm/directory.h>
#include <cgm/switch.h>
#include <cgm/hub-iommu.h>
#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>

#define WIRE_DELAY(wire_latency) (etime.count + (wire_latency *2))

//global variables.
extern long long temp_id;

//star todo integrate m2s prefetcher
extern struct str_map_t cgm_cache_policy_map;
extern struct str_map_t cgm_cache_block_state_map;
extern struct str_map_t cgm_mem_access_strn_map;

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
void cache_dump_stats(struct cgm_stats_t *cgm_stat_container);
void cache_reset_stats(void);
void cache_store_stats(struct cgm_stats_t *cgm_stat_container);

//cpu
void l1_i_cache_ctrl(void);
void l1_d_cache_ctrl(void);
void l2_cache_ctrl(void);
void l3_cache_ctrl(void);

void l1_i_cache_down_io_ctrl(void);
void l1_d_cache_down_io_ctrl(void);
void l2_cache_up_io_ctrl(void);
void l2_cache_down_io_ctrl(void);
void l3_cache_up_io_ctrl(void);
void l3_cache_down_io_ctrl(void);

//gpu
void gpu_s_cache_ctrl(void);
void gpu_v_cache_ctrl(void);
void gpu_l2_cache_ctrl(void);
void gpu_lds_unit_ctrl(void);

void gpu_s_cache_down_io_ctrl(void);
void gpu_v_cache_down_io_ctrl(void);
void gpu_l2_cache_up_io_ctrl(void);
void gpu_l2_cache_down_io_ctrl(void);


//Cache Access Functions
struct cache_t *cgm_l3_cache_map(int set);
int cache_can_access_top(struct cache_t *cache);
int cache_can_access_bottom(struct cache_t *cache);
int cache_can_access_Tx_bottom(struct cache_t *cache);
int cache_can_access_Tx_top(struct cache_t *cache);
int cgm_gpu_cache_map(struct cache_t *cache, unsigned int addr);
/*int cgm_gpu_cache_map(int cache_id);*/
void cache_l1_i_return(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_l1_d_return(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_put_io_up_queue(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_put_io_down_queue(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_gpu_s_return(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_gpu_v_return(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_gpu_lds_return(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_access_stats(struct cache_t *cache, int hit, int state);
void cgm_cache_set_route(struct cgm_packet_t * message_packet, struct cache_t * cache);

//Scheduler functions
struct cgm_packet_t *cache_get_message(struct cache_t *cache);
void cache_dump_queue(struct list_t *queue);

//Address Manipulations
void cgm_cache_probe_address(struct cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr);
void cgm_cache_print_set_tag(struct cache_t *cache, unsigned int addr);
unsigned int cgm_cache_build_address(struct cache_t *cache, int set, int tag);

//Directory Manipulations
void cgm_cache_clear_dir(struct cache_t *cache, int set, int way);
int cgm_cache_get_dir_dirty_bit(struct cache_t *cache, int set, int way);
void cgm_cache_set_dir_pending_bit(struct cache_t *cache, int set, int way);
int cgm_cache_get_dir_pending_bit(struct cache_t *cache, int set, int way);
void cgm_cache_clear_dir_pending_bit(struct cache_t *cache, int set, int way);
void cgm_cache_set_dir(struct cache_t *cache, int set, int way, int l2_cache_id);
int cgm_cache_get_num_shares(enum cgm_processor_kind_t processor, struct cache_t *cache, int set, int way);
int cgm_cache_get_xown_core(enum cgm_processor_kind_t processor, struct cache_t *cache, int set, int way);
//int cgm_cache_get_sown_core(struct cache_t *cache, int set, int way);
int cgm_cache_is_owning_core(struct cache_t *cache, int set, int way, int l2_cache_id);

//Write Back Buffer Manipulations
struct cgm_packet_t *cache_search_wb(struct cache_t *cache, int tag, int set);
int cache_search_wb_dup_packets(struct cache_t *cache, int tag, int set);
struct cgm_packet_t *cache_search_wb_not_pending_flush(struct cache_t *cache);
void cache_dump_write_back(struct cache_t *cache);

//Pending Request Buffer Manipulations
void cgm_cache_insert_pending_request_buffer(struct cache_t *cache, struct cgm_packet_t *message_packet);
struct cgm_packet_t *cache_search_pending_request_buffer(struct cache_t *cache, unsigned int address);
int cache_search_pending_request_get_getx_fwd(struct cache_t *cache, unsigned int address);

//block Manipulations
long long cgm_cache_get_block_transient_state_id(struct cache_t *cache, int set, int way);
enum cgm_cache_block_state_t cgm_cache_get_block_transient_state(struct cache_t *cache, int set, int way);
void cgm_cache_set_block_transient_state(struct cache_t *cache, int set, int way, enum cgm_cache_block_state_t t_state);
void cgm_cache_set_block_address(struct cache_t *cache, int set, int way, unsigned int address);
void cache_put_block(struct cache_t *cache, struct cgm_packet_t *message_packet);
void cache_get_block_status(struct cache_t *cache, struct cgm_packet_t *message_packet, int *cache_block_hit_ptr, int *cache_block_state_ptr);
int cgm_cache_find_block(struct cache_t *cache, int *tag_ptr, int *set_ptr, unsigned int *offset_ptr, int *way_ptr, int *state_ptr);
void cache_get_transient_block(struct cache_t *cache, struct cgm_packet_t *message_packet, int *cache_block_hit_ptr, int *cache_block_state_ptr);
int cgm_cache_find_transient_block(struct cache_t *cache, int *tag_ptr, int *set_ptr, unsigned int *offset_ptr, int *way_ptr, int *state_ptr);
int cgm_cache_get_way(struct cache_t *cache, int tag, int set);
void cgm_cache_set_block(struct cache_t *cache, int set, int way, int tag, int state);
void cgm_cache_set_block_type(struct cache_t *cache, int type, int set, int way);
void cgm_cache_update_waylist(struct cache_set_t *set, struct cache_block_t *blk, enum cache_waylist_enum where);
void cgm_L1_cache_evict_block(struct cache_t *cache, int set, int way);
void cgm_L2_cache_evict_block(struct cache_t *cache, int set, int way, int sharers, int victim_way);
void cgm_L3_cache_evict_block(struct cache_t *cache, int set, int way, int sharers, int victim_way);
int cgm_cache_get_block_type(struct cache_t *cache, int set, int way, int tag);
void cgm_cache_set_block_state(struct cache_t *cache, int set, int way, enum cgm_cache_block_state_t state);
enum cgm_cache_block_state_t cgm_cache_get_block_state(struct cache_t *cache, int set, int way);
void cgm_cache_set_block_flush_pending_bit(struct cache_t *cache, int set, int way);
void cgm_cache_set_block_upgrade_pending_bit(struct cache_t *cache, int set, int way);
void cgm_cache_clear_block_upgrade_pending_bit(struct cache_t *cache, int set, int way);
int cgm_cache_get_block_upgrade_pending_bit(struct cache_t *cache, int set, int way);
void cgm_cache_clear_block_flush_pending_bit(struct cache_t *cache, int set, int way);
int cgm_cache_get_block_flush_pending_bit(struct cache_t *cache, int set, int way);
/*enum cgm_cache_block_state_t cgm_cache_get_block_state(struct cache_t *cache, int set, int way);*/
void cgm_cache_get_block(struct cache_t *cache, int set, int way, int *tag_ptr, int *state_ptr);
void cgm_cache_access_block(struct cache_t *cache, int set, int way);
void cgm_cache_update_block_order(struct cache_t *cache, int set);

int cgm_cache_get_victim_for_wb(struct cache_t *cache, int set);
int cgm_cache_get_victim(struct cache_t *cache, int set, int tran_tag);
int cgm_cache_replace_block(struct cache_t *cache, int set);
void cgm_cache_dump_set(struct cache_t *cache, int set);
int cgm_cache_get_block_usage(struct cache_t *cache);

enum cgm_access_kind_t cgm_gpu_cache_get_retry_state(enum cgm_access_kind_t r_state);
enum cgm_access_kind_t cgm_cache_get_retry_state(enum cgm_access_kind_t r_state);
void gpu_cache_coalesed_retry(struct cache_t *cache, int tag, int set);
void cache_coalesed_retry(struct cache_t *cache, int tag_ptr, int set_ptr, long long access_id);

//ORT Manipulations
long long get_oldest_packet(struct cache_t *cache, int set);
int get_ort_status(struct cache_t *cache);
void cache_check_ORT(struct cache_t *cache, struct cgm_packet_t *message_packet);
int cache_get_ORT_size(struct cache_t *cache);
int ort_search(struct cache_t *cache, int tag, int set);
void ort_set_pending_join_bit(struct cache_t *cache, int row, int tag, int set);
int ort_get_pending_join_bit(struct cache_t *cache, int row, int tag, int set);
void ort_clear_pending_join_bit(struct cache_t *cache, int row, int tag, int set);
void ort_set(struct cache_t *cache, int entry, int tag, int set);
void ort_clear(struct cache_t *cache, struct cgm_packet_t *message_packet);
void ort_dump(struct cache_t *cache);
void ort_get_row_sets_size(struct cache_t *cache, int tag, int set, int *hit_row_ptr, int *num_sets_ptr, int *ort_size_ptr);
void ort_set_row(struct cache_t *cache, int tag, int set);
int ort_get_num_coal(struct cache_t *cache, int tag, int set);

/*error checking*/
int cache_validate_block_flushed_from_core(int core_id, unsigned int addr);
int cache_validate_block_flushed_from_l1(struct cache_t *cache, int core_id, unsigned int addr);

#endif /*CACHE_H_*/
