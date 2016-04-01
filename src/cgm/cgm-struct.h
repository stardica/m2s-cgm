/*
 * cgm-struct.h
 *
 *  Created on: Sep 6, 2015
 *      Author: stardica
 */


#ifndef CGMSTRUCT_H_
#define CGMSTRUCT_H_

#include <lib/util/list.h>

#include <cgm/tasking.h>
#include <cgm/directory.h>

#define HISTSIZE 40000

typedef int bool;
enum {false, true};

enum protocol_case_kind_t{
	invalid = 0,
	L1_hit,
	L2_hit,
	L3_hit,
	memory,
	get_fwd,
	getx_fwd,
	L1_upgrade,
	L2_upgrade,
	L3_upgrade,
	number_cases
};

enum cgm_cache_block_state_t{

	cgm_cache_block_invalid = 0,
	cgm_cache_block_noncoherent,/*1*/
	cgm_cache_block_modified, /*2*/
	cgm_cache_block_owned, /*3*/
	cgm_cache_block_exclusive,/*4*/
	cgm_cache_block_shared, /*5*/
	cgm_cache_block_transient,/*6*/
	cgm_cache_block_flush,/*6*/
	cgm_cache_block_null,/*7*/
	cgm_cache_block_state_num
};

enum cache_waylist_enum{

	cache_waylist_head,
	cache_waylist_tail
};


enum protocol_kind_t {

	cgm_protocol_invalid = 0,
	cgm_protocol_mesi,
	cgm_protocol_bt,
	cgm_protocol_moesi,
	cgm_protocol_gmesi,
	cgm_protocol_non_coherent,
	num_cgm_protocol_types
};

enum cgm_access_kind_t {

	/*0*/	cgm_access_invalid = 0,
			cgm_access_fetch,
			cgm_access_load,
	/*3*/	cgm_access_store,
			cgm_access_nc_store,
			cgm_access_nc_load,
			cgm_access_store_v,
			cgm_access_load_s,
			cgm_access_load_v,
			cgm_access_prefetch,
	/*10*/	cgm_access_gets, //get shared
			cgm_access_gets_i,
			cgm_access_get, //get specific to d caches
			cgm_access_get_nack,
			cgm_access_get_fwd,
			cgm_access_get_fwd_nack,
			cgm_access_get_fwd_upgrade_nack,
			cgm_access_getx_fwd,
			cgm_access_getx_fwd_nack,
			cgm_access_getx_fwd_upgrade_nack,
	/*20*/	cgm_access_getx_fwd_ack,
			cgm_access_getx_fwd_inval,
			cgm_access_getx_fwd_inval_ack,
			cgm_access_gets_s, //get shared specific to s caches
			cgm_access_gets_v, //get shared specific to v caches
			cgm_access_getx, //get exclusive (or get with intent to write)
			cgm_access_getx_nack,
			cgm_access_inv,  //invalidation request
			cgm_access_flush_block,
			cgm_access_flush_block_ack,
			cgm_access_inv_ack,
	/*30*/		cgm_access_upgrade, //upgrade request
		cgm_access_upgrade_ack,
			cgm_access_upgrade_nack,
			cgm_access_upgrade_putx_n,
			cgm_access_upgrade_getx_fwd,
			cgm_access_upgrade_inval,
			cgm_access_upgrade_inval_ack,
			cgm_access_upgrade_putx,
			cgm_access_downgrade, //downgrade request
			cgm_access_downgrade_ack,
	/*40*/		cgm_access_downgrade_nack,
		cgm_access_mc_load,	//request sent to system agent/memory controller
			cgm_access_mc_store,	//request sent to system agent/memory controller
			cgm_access_mc_put,	//reply from system agent/memory controller
			cgm_access_put_clnx, //put block in clean exclusive state
			cgm_access_putx, //put block in modified state
		/*46*/	cgm_access_puts, //put block in shared state.
		cgm_access_puto, //put block in owned state.
			cgm_access_puto_shared, //request for write back of cache block in owned state but other sharers of the block exist.
			cgm_access_unblock, //message to unblock next cache level/directory for blocking protocols.
	/*50*/		cgm_access_retry,
		cgm_access_fetch_retry,
			cgm_access_load_retry,
			cgm_access_store_retry,
			cgm_access_loadx_retry, /*gpu mesi mode*/
			cgm_access_storex_retry, /*gpu mesi mode*/
			cgm_access_write_back,
			cgm_access_retry_i,//not used
			num_access_types
};

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
	char *gpu_cache_name;

	//access data
	long long access_id;
	long long write_back_id;
	long long evict_id;
	unsigned int address;
	unsigned int block_address;
	int set;
	int tag;
	int way;
	unsigned int offset;
	int size;
	int coalesced;
	int assoc_conflict;
	int translation_id;

	//for evictions, write backs, downgrades, upgrades
	int flush_pending;
	int L3_flush_join;
	int downgrade;
	int downgrade_pending;
	int downgrade_ack;
	int inval;
	int inval_pending;
	int inval_ack;
	int upgrade;
	int upgrade_putx_n;
	int upgrade_ack;
	int upgrade_pending;
	int upgrade_inval_ack_count;
	int upgrade_dirty;

	//for victims
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
	long long dram_start_cycle;
	enum protocol_case_kind_t protocol_case;
};

struct cgm_packet_status_t{

	//used for global memory list
	enum cgm_access_kind_t access_type;
	unsigned int address;
	long long access_id;
	int in_flight;
};

struct cache_block_t{

	struct cache_block_t *way_next;
	struct cache_block_t *way_prev;

	int tag;
	int set;
	int transient_tag;
	int way;
	int prefetched;
	int flush_pending;
	int upgrade_pending;
	unsigned int address;
	int written;

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

enum cache_type_enum{

	l1_i_cache_t,
	l1_d_cache_t,
	l2_cache_t,
	l3_cache_t,
	gpu_s_cache_t,
	gpu_v_cache_t,
	gpu_l2_cache_t
};

enum cache_policy_t{

	cache_policy_invalid = 0,
	cache_policy_lru,
	cache_policy_fifo,
	cache_policy_random,
	cache_policy_first_available,
	cache_policy_num
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
	unsigned int addr_range_base;
	unsigned int addr_range_top;
	enum cache_policy_t policy;
	char * policy_type;
	int slice_type;
	int bus_width;


	//cache data
	struct cache_set_t *sets;
	unsigned int block_mask;
	unsigned int block_address_mask;
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
	//star todo rewrite all of this queues should be in-boxes
	//buffers are internal buffers
	struct list_t *Rx_queue_top;
	struct list_t *Rx_queue_bottom;
	struct list_t *Tx_queue_top;
	struct list_t *Tx_queue_bottom;
	struct list_t *Coherance_Tx_queue;
	struct list_t *Coherance_Rx_queue;
	struct list_t *retry_queue;
	struct list_t *write_back_buffer;
	struct list_t *pending_request_buffer;
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
	unsigned int share_mask;

	//L1 I cache protocol virtual functions
	void (*l1_i_fetch)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l1_i_write_block)(struct cache_t *cache, struct cgm_packet_t *message_packet);

	//L1 D cache protocol virtual functions
	void (*l1_d_load)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l1_d_store)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	int (*l1_d_write_block)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l1_d_downgrade)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l1_d_getx_fwd_inval)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l1_d_upgrade_inval)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l1_d_upgrade_ack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l1_d_write_back)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l1_d_flush_block)(struct cache_t *cache, struct cgm_packet_t *message_packet);

	//L2 cache protocol virtual functions
	void (*l2_gets)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_get)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_get_nack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	int (*l2_getx)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_getx_nack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_downgrade_ack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_get_fwd)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_downgrade_nack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_getx_fwd)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_getx_fwd_nack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_getx_fwd_inval_ack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_flush_block_ack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	int (*l2_upgrade)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_upgrade_ack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_upgrade_nack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_upgrade_putx_n)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_upgrade_inval)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	int (*l2_write_block)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	int (*l2_write_back)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l2_flush_block)(struct cache_t *cache, struct cgm_packet_t *message_packet);

	//L3 cache protocol virtual functions
	void (*l3_gets)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l3_get)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l3_getx)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l3_downgrade_ack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l3_downgrade_nack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l3_getx_fwd_nack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l3_getx_fwd_upgrade_nack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l3_get_fwd_upgrade_nack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l3_getx_fwd_ack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	int (*l3_upgrade)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	int (*l3_upgrade_ack)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l3_write_block)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	int (*l3_write_back)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*l3_flush_block_ack)(struct cache_t *cache, struct cgm_packet_t *message_packet);


	//GPU S cache protocol virtual functions
	void (*gpu_s_load)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*gpu_s_write_block)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	/*void (*gpu_s_put)(struct cache_t *cache, struct cgm_packet_t *message_packet);*/
	/*void (*gpu_s_retry)(struct cache_t *cache, struct cgm_packet_t *message_packet);*/

	//GPU V cache protocol virtual functions
	void (*gpu_v_load)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*gpu_v_store)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*gpu_v_write_block)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*gpu_v_inval)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	/*void (*gpu_v_put)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*gpu_v_retry)(struct cache_t *cache, struct cgm_packet_t *message_packet);*/

	//GPU L2 cache protocol virtual functions
	void (*gpu_l2_getx)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*gpu_l2_get)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*gpu_l2_write_block)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	/*void (*gpu_l2_get)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*gpu_l2_put)(struct cache_t *cache, struct cgm_packet_t *message_packet);
	void (*gpu_l2_retry)(struct cache_t *cache, struct cgm_packet_t *message_packet);*/

	//watch dog
	/*unsigned int *outstanding_addresses;*/

	//statistics
	long long occupancy;
	long long coalesces;
	long long WbMerges;
	long long TotalHits;
	long long TotalMisses;
	long long UpgradeMisses;
	long long EvictInv;
	long long TotalWriteBackRecieved;
	long long TotalWriteBackSent;
	long long TotalWriteBackDropped;
	long long stalls;
	long long TotalDowngrades;
	long long TotalGetxFwdInvals;
	long long TotalUpgradeAcks;
	long long TotalUpgradeInvals;
	long long TotalWriteBlocks;



	long long Totalfetches;
	long long TotalAdvances; //this is an error check...
	long long TotalAcesses;
	long long TotalReads;
	long long TotalWrites;
	long long TotalGets;
	long long TotalGet;
	long long TotalGetx;
	long long TotalUpgrades;
	long long TotalReadMisses;
	long long TotalWriteMisses;
	//long long TotalWriteBacks;
	long long invalid_hits;
	long long assoc_conflict;

	long long retries;

	long long mshr_entries;


	unsigned int *fetch_address_history;
	unsigned int *load_address_history;
	unsigned int *store_address_history;
};

//for general stats
struct cgm_stats_t{

	bool execution_success;

	enum container_type_t{
		startupSection = 0,
		parallelSection,
		wrapupSection,
		systemStats
	}stats_type;

	char *stat_file_name;
	char *date_time_file;
	char *date_time_pretty;

	/*benchmark related*/
	char *benchmark_name;
	char *args;
	double start_wall_time;
	double end_wall_time;
	double sim_time;
	long long start_startup_section_cycle;
	long long end_startup_section_cycle;
	long long total_startup_section_cycles;
	long long start_parallel_section_cycle;
	long long end_parallel_section_cycle;
	long long total_parallel_section_cycles;
	long long start_wrapup_section_cycle;
	long long end_wrapup_section_cycle;
	long long total_wrapup_section_cycles;

	enum parallel_section_t{
		not_present = 0,
		present
	}parallel_section;

	enum state_t{
		not_consolidated = 0,
		consolidated
	}state;

	/*redundant stats, this so we can save away our stats at a certain point in the benchmark's run.*/
	long long *core_num_syscalls;
	long long *core_syscall_stalls;
	long long *core_rob_stalls;
	long long *core_rob_stall_load;
	long long *core_rob_stall_store;
	long long *core_rob_stall_other;
	long long *core_first_fetch_cycle;
	long long *core_fetch_stalls;
	long long *core_last_commit_cycle;
	long long *core_issued_memory_insts;
	long long *core_commited_memory_insts;

	//memory system at large
	int first_mem_access_lat;
	long long fetch_lat_hist[HISTSIZE];
	long long load_lat_hist[HISTSIZE];
	long long store_lat_hist[HISTSIZE];
	long long cpu_total_fetch_requests;
	long long cpu_total_fetch_replys;
	long long fetch_l1_hits;
	long long fetch_l2_hits;
	long long l2_total_fetch_requests;
	long long fetch_l3_hits;
	long long l3_total_fetch_requests;
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
	long long l2_store_nack;
	long long l3_store_nack;


	//caches
	long long *l1_i_occupancy;
	long long *l1_i_coalesces;
	long long *l1_i_TotalMisses;
	long long *l1_i_TotalHits;
	long long *l1_i_WbMerges;
	long long *l1_i_UpgradeMisses;
	long long *l1_i_EvictInv;
	long long *l1_i_TotalWriteBackSent;
	long long *l1_i_TotalWriteBackRecieved;
	long long *l1_i_TotalWriteBackDropped;
	long long *l1_i_stalls;
	long long *l1_i_TotalDowngrades;
	long long *l1_i_TotalGetxFwdInvals;
	long long *l1_i_TotalUpgradeAcks;
	long long *l1_i_TotalUpgradeInvals;
	long long *l1_i_TotalWriteBlocks;



	long long *l1_i_TotalAdvances;
	long long *l1_i_TotalAcesses;
	long long *l1_i_TotalReads;
	long long *l1_i_TotalWrites;
	long long *l1_i_TotalGets;
	long long *l1_i_TotalGet;
	long long *l1_i_TotalGetx;
	long long *l1_i_TotalUpgrades;
	long long *l1_i_TotalReadMisses;
	long long *l1_i_TotalWriteMisses;
	long long *l1_i_TotalWriteBacks;
	long long *l1_i_invalid_hits;
	long long *l1_i_assoc_conflict;
	long long *l1_i_retries;
	long long *l1_i_mshr_entries;


	long long *l1_d_occupancy;
	long long *l1_d_coalesces;
	long long *l1_d_TotalHits;
	long long *l1_d_TotalMisses;
	long long *l1_d_WbMerges;
	long long *l1_d_UpgradeMisses;
	long long *l1_d_EvictInv;
	long long *l1_d_TotalWriteBackSent;
	long long *l1_d_TotalWriteBackRecieved;
	long long *l1_d_TotalWriteBackDropped;
	long long *l1_d_stalls;
	long long *l1_d_TotalDowngrades;
	long long *l1_d_TotalGetxFwdInvals;
	long long *l1_d_TotalUpgradeAcks;
	long long *l1_d_TotalUpgradeInvals;
	long long *l1_d_TotalWriteBlocks;

	long long *l1_d_TotalAdvances;
	long long *l1_d_TotalAcesses;
	long long *l1_d_TotalReads;
	long long *l1_d_TotalWrites;
	long long *l1_d_TotalGets;
	long long *l1_d_TotalGet;
	long long *l1_d_TotalGetx;
	long long *l1_d_TotalUpgrades;
	long long *l1_d_TotalReadMisses;
	long long *l1_d_TotalWriteMisses;
	long long *l1_d_TotalWriteBacks;
	long long *l1_d_invalid_hits;
	long long *l1_d_assoc_conflict;
	long long *l1_d_retries;
	long long *l1_d_mshr_entries;

	long long *l2_occupancy;
	long long *l2_TotalAdvances;
	long long *l2_TotalAcesses;
	long long *l2_TotalMisses;
	long long *l2_WbMerges;
	long long *l2_UpgradeMisses;
	long long *l2_EvictInv;
	long long *l2_TotalWriteBackSent;
	long long *l2_TotalWriteBackRecieved;
	long long *l2_TotalWriteBackDropped;
	long long *l2_stalls;
	long long *l2_TotalDowngrades;
	long long *l2_TotalGetxFwdInvals;
	long long *l2_TotalUpgradeAcks;
	long long *l2_TotalUpgradeInvals;
	long long *l2_TotalWriteBlocks;

	long long *l2_TotalHits;
	long long *l2_TotalReads;
	long long *l2_TotalWrites;
	long long *l2_TotalGets;
	long long *l2_TotalGet;
	long long *l2_TotalGetx;
	long long *l2_TotalUpgrades;
	long long *l2_TotalReadMisses;
	long long *l2_TotalWriteMisses;
	long long *l2_TotalWriteBacks;
	long long *l2_invalid_hits;
	long long *l2_assoc_conflict;
	long long *l2_retries;
	long long *l2_coalesces;
	long long *l2_mshr_entries;


	long long *l3_occupancy;
	long long *l3_TotalAdvances;
	long long *l3_TotalAcesses;
	long long *l3_TotalMisses;
	long long *l3_WbMerges;
	long long *l3_UpgradeMisses;
	long long *l3_EvictInv;
	long long *l3_TotalWriteBackSent;
	long long *l3_TotalWriteBackRecieved;
	long long *l3_TotalWriteBackDropped;
	long long *l3_stalls;
	long long *l3_TotalDowngrades;
	long long *l3_TotalGetxFwdInvals;
	long long *l3_TotalUpgradeAcks;
	long long *l3_TotalUpgradeInvals;
	long long *l3_TotalWriteBlocks;

	long long *l3_TotalHits;
	long long *l3_TotalReads;
	long long *l3_TotalWrites;
	long long *l3_TotalGets;
	long long *l3_TotalGet;
	long long *l3_TotalGetx;
	long long *l3_TotalUpgrades;
	long long *l3_TotalReadMisses;
	long long *l3_TotalWriteMisses;
	long long *l3_TotalWriteBacks;
	long long *l3_invalid_hits;
	long long *l3_assoc_conflict;
	long long *l3_retries;
	long long *l3_coalesces;
	long long *l3_mshr_entries;



	long long *switch_total_links;
	int *switch_max_links;
	long long *switch_total_wakes;
	long long *switch_north_io_transfers;
	long long *switch_north_io_transfer_cycles;
	long long *switch_north_io_bytes_transfered;
	long long *switch_east_io_transfers;
	long long *switch_east_io_transfer_cycles;
	long long *switch_east_io_bytes_transfered;
	long long *switch_south_io_transfers;
	long long *switch_south_io_transfer_cycles;
	long long *switch_south_io_bytes_transfered;
	long long *switch_west_io_transfers;
	long long *switch_west_io_transfer_cycles;
	long long *switch_west_io_bytes_transfered;
	long long *switch_north_txqueue_max_depth;
	double *switch_north_txqueue_ave_depth;
	long long *switch_east_txqueue_max_depth;
	double *switch_east_txqueue_ave_depth;
	long long *switch_south_txqueue_max_depth;
	double *switch_south_txqueue_ave_depth;
	long long *switch_west_txqueue_max_depth;
	double *switch_west_txqueue_ave_depth;

	long long *switch_north_tx_inserts;
	long long *switch_east_tx_inserts;
	long long *switch_south_tx_inserts;
	long long *switch_west_tx_inserts;

	long long *switch_north_rxqueue_max_depth;
	double *switch_north_rxqueue_ave_depth;
	long long *switch_east_rxqueue_max_depth;
	double *switch_east_rxqueue_ave_depth;
	long long *switch_south_rxqueue_max_depth;
	double *switch_south_rxqueue_ave_depth;
	long long *switch_west_rxqueue_max_depth;
	double *switch_west_rxqueue_ave_depth;

	long long *switch_north_rx_inserts;
	long long *switch_east_rx_inserts;
	long long *switch_south_rx_inserts;
	long long *switch_west_rx_inserts;

	//system agent
	long long system_agent_busy_cycles;
	long long system_agent_north_io_busy_cycles;
	long long system_agent_south_io_busy_cycles;
	long long system_agent_mc_loads;
	long long system_agent_mc_stores;
	long long system_agent_mc_returns;
	int system_agent_max_north_rxqueue_depth;
	double system_agent_ave_north_rxqueue_depth;
	int system_agent_max_south_rxqueue_depth;
	double system_agent_ave_south_rxqueue_depth;
	int system_agent_max_north_txqueue_depth;
	double system_agent_ave_north_txqueue_depth;
	int system_agent_max_south_txqueue_depth;
	double system_agent_ave_south_txqueue_depth;
	/*long long system_agent_north_gets;
	long long system_agent_south_gets;
	long long system_agent_north_puts;
	long long system_agent_south_puts;*/

	//Memory controller and DRAMSim
	long long mem_ctrl_busy_cycles;
	long long mem_ctrl_num_reads;
	long long mem_ctrl_num_writes;
	double mem_ctrl_ave_dram_read_lat;
	double mem_ctrl_ave_dram_write_lat;
	double mem_ctrl_ave_dram_total_lat;
	long long mem_ctrl_read_min;
	long long mem_ctrl_read_max;
	long long mem_ctrl_write_min;
	long long mem_ctrl_write_max;
	long long mem_ctrl_dram_max_queue_depth;
	double mem_ctrl_dram_ave_queue_depth;
	long long mem_ctrl_dram_busy_cycles;
	long long mem_ctrl_rx_max;
	long long mem_ctrl_tx_max;
	long long mem_ctrl_bytes_read;
	long long mem_ctrl_bytes_wrote;
	long long mem_ctrl_io_busy_cycles;

};

#endif /* CGMSTRUCT_H_ */

