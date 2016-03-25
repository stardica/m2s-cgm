/* protocol.c
 *
 *  Created on: Apr 6, 2015
 *      Author: stardica*/

#include <cgm/protocol.h>

struct str_map_t protocol_kind_strn_map =
{ num_cgm_protocol_types,
	{
	{"cgm_protocol_mesi", cgm_protocol_mesi},
	{"cgm_protocol_moesi", cgm_protocol_moesi}
	}
};

struct str_map_t cgm_mem_access_strn_map =
{ 	num_access_types,
		{
		{"cgm_access_invalid", cgm_access_invalid},
		{"cgm_access_fetch", cgm_access_fetch},
		{"cgm_access_load", cgm_access_load},
		{"cgm_access_store", cgm_access_store},//
		{"cgm_access_nc_store", cgm_access_nc_store},
		{"cgm_access_nc_load", cgm_access_nc_load},//
		{"cgm_access_store_v", cgm_access_store_v},
		{"cgm_access_load_s", cgm_access_load_s},
		{"cgm_access_load_v", cgm_access_load_v},
		{"cgm_access_prefetch", cgm_access_prefetch},
		{"cgm_access_gets", cgm_access_gets},
		{"cgm_access_gets_i", cgm_access_gets_i},
		{"cgm_access_get", cgm_access_get},
		{"cgm_access_get_nack", cgm_access_get_nack},
		{"cgm_access_get_fwd", cgm_access_get_fwd},
		{"cgm_access_get_fwd_nack", cgm_access_get_fwd_nack},
		{"cgm_access_get_fwd_upgrade_nack", cgm_access_get_fwd_upgrade_nack},
		{"cgm_access_getx_fwd", cgm_access_getx_fwd},
		{"cgm_access_getx_fwd_nack", cgm_access_getx_fwd_nack},
		{"cgm_access_getx_fwd_upgrade_nack", cgm_access_getx_fwd_upgrade_nack},
		{"cgm_access_getx_fwd_ack", cgm_access_getx_fwd_ack},
		{"cgm_access_getx_fwd_inval", cgm_access_getx_fwd_inval},
		{"cgm_access_getx_fwd_inval_ack", cgm_access_getx_fwd_inval_ack},
		{"cgm_access_gets_s", cgm_access_gets_s},
		{"cgm_access_gets_v", cgm_access_gets_v},
		{"cgm_access_getx", cgm_access_getx},
		{"cgm_access_getx_nack", cgm_access_getx_nack},
		{"cgm_access_inv", cgm_access_inv},
		{"cgm_access_flush_block", cgm_access_flush_block},
		{"cgm_access_flush_block_ack", cgm_access_flush_block_ack},
		{"cgm_access_inv_ack", cgm_access_inv_ack},
		{"cgm_access_upgrade", cgm_access_upgrade},
		{"cgm_access_upgrade_ack", cgm_access_upgrade_ack},
		{"cgm_access_upgrade_nack", cgm_access_upgrade_nack},
		{"cgm_access_upgrade_putx_n", cgm_access_upgrade_putx_n},
		{"cgm_access_upgrade_getx_fwd", cgm_access_upgrade_getx_fwd},
		{"cgm_access_upgrade_inval", cgm_access_upgrade_inval},
		{"cgm_access_upgrade_inval_ack", cgm_access_upgrade_inval_ack},
		{"cgm_access_upgrade_putx", cgm_access_upgrade_putx},
		{"cgm_access_downgrade", cgm_access_downgrade},
		{"cgm_access_downgrade_ack", cgm_access_downgrade_ack},
		{"cgm_access_downgrade_nack", cgm_access_downgrade_nack},
		{"cgm_access_mc_load", cgm_access_mc_load},
		{"cgm_access_mc_store", cgm_access_mc_store},
		{"cgm_access_mc_put", cgm_access_mc_put},
		{"cgm_access_put_clnx", cgm_access_put_clnx},
		{"cgm_access_putx", cgm_access_putx},
		{"cgm_access_puts", cgm_access_puts},
		{"cgm_access_puto", cgm_access_puto},
		{"cgm_access_puto_shared", cgm_access_puto_shared},
		{"cgm_access_unblock", cgm_access_unblock},
		{"cgm_access_retry", cgm_access_retry},
		{"cgm_access_fetch_retry", cgm_access_fetch_retry},
		{"cgm_access_load_retry", cgm_access_load_retry},
		{"cgm_access_store_retry", cgm_access_store_retry},
		{"cgm_access_write_back", cgm_access_write_back},
		{"cgm_access_retry_i" ,cgm_access_retry_i},
		{"num_access_types", num_access_types}
		}
};

struct mem_system_stats_t *mem_system_stats;

/*long long temp_access_id = 0;*/
long long write_back_id = 1;
long long evict_id = 1;

enum protocol_kind_t cgm_cache_protocol;
enum protocol_kind_t cgm_gpu_cache_protocol;


//CPU will call create packet and load into correct queue.
struct cgm_packet_t *packet_create(void){

	struct cgm_packet_t *new_packet;

	new_packet = (void *) calloc(1, sizeof(struct cgm_packet_t));

	return new_packet;
}

void packet_destroy(struct cgm_packet_t *packet){

	free(packet->name);
	free(packet);
	//dont' need to free these because we never malloc these.
	//free(packet->l2_cache_name);
	//free(packet->src_name);
	//free(packet->dest_name);

	return;
}

struct cgm_packet_status_t *status_packet_create(void){

	struct cgm_packet_status_t *new_packet;

	new_packet = (void *) calloc(1, sizeof(struct cgm_packet_status_t));


	return new_packet;
}

void status_packet_destroy(struct cgm_packet_status_t *status_packet){

	free(status_packet);

	return;
}

void mem_system_store_stats(struct cgm_stats_t *cgm_stat_container){

	int i = 0;

	//memory system at large
	cgm_stat_container->first_mem_access_lat = mem_system_stats->first_mem_access_lat;

	for(i = 0; i < HISTSIZE; i++)
	{
		cgm_stat_container->fetch_lat_hist[i] = mem_system_stats->fetch_lat_hist[i];
		cgm_stat_container->load_lat_hist[i] = mem_system_stats->load_lat_hist[i];
		cgm_stat_container->store_lat_hist[i] = mem_system_stats->store_lat_hist[i];
	}

	cgm_stat_container->cpu_total_fetches = mem_system_stats->cpu_total_fetches;
	cgm_stat_container->fetch_l1_hits = mem_system_stats->fetch_l1_hits;
	cgm_stat_container->fetch_l2_hits = mem_system_stats->fetch_l2_hits;
	cgm_stat_container->fetch_l3_hits = mem_system_stats->fetch_l3_hits;
	cgm_stat_container->fetch_memory = mem_system_stats->fetch_memory;
	cgm_stat_container->cpu_total_loads = mem_system_stats->cpu_total_loads;
	cgm_stat_container->load_l1_hits = mem_system_stats->load_l1_hits;
	cgm_stat_container->load_l2_hits = mem_system_stats->load_l2_hits;
	cgm_stat_container->load_l3_hits = mem_system_stats->load_l3_hits;
	cgm_stat_container->load_memory = mem_system_stats->load_memory;
	cgm_stat_container->load_get_fwd = mem_system_stats->load_get_fwd;
	cgm_stat_container->cpu_total_stores = mem_system_stats->cpu_total_stores;
	cgm_stat_container->store_l1_hits = mem_system_stats->store_l1_hits;
	cgm_stat_container->store_l2_hits = mem_system_stats->store_l2_hits;
	cgm_stat_container->store_l3_hits = mem_system_stats->store_l3_hits;
	cgm_stat_container->store_memory = mem_system_stats->store_memory;
	cgm_stat_container->store_getx_fwd = mem_system_stats->store_getx_fwd;
	cgm_stat_container->store_upgrade = mem_system_stats->store_upgrade;

	return;
}

void mem_system_reset_stats(void){

	int i = 0;

	//memory system at large
	mem_system_stats->first_mem_access_lat = 0;

	for(i = 0; i < HISTSIZE; i++)
	{
		mem_system_stats->fetch_lat_hist[i] = 0;
		mem_system_stats->load_lat_hist[i] = 0;
		mem_system_stats->store_lat_hist[i] = 0;
	}

	mem_system_stats->cpu_total_fetches = 0;
	mem_system_stats->fetch_l1_hits = 0;
	mem_system_stats->fetch_l2_hits = 0;
	mem_system_stats->fetch_l3_hits = 0;
	mem_system_stats->fetch_memory = 0;
	mem_system_stats->cpu_total_loads = 0;
	mem_system_stats->load_l1_hits = 0;
	mem_system_stats->load_l2_hits = 0;
	mem_system_stats->load_l3_hits = 0;
	mem_system_stats->load_memory = 0;
	mem_system_stats->load_get_fwd = 0;
	mem_system_stats->cpu_total_stores = 0;
	mem_system_stats->store_l1_hits = 0;
	mem_system_stats->store_l2_hits = 0;
	mem_system_stats->store_l3_hits = 0;
	mem_system_stats->store_memory = 0;
	mem_system_stats->store_getx_fwd = 0;
	mem_system_stats->store_upgrade = 0;

	return;
}

void init_write_back_packet(struct cache_t *cache, struct cgm_packet_t *write_back_packet, int set, int way, int pending, enum cgm_cache_block_state_t victim_state){

	//int l1_error = 0;
	//int l2_error = 0;
	//int l3_error = 0;

	write_back_packet->access_type = cgm_access_write_back;
	write_back_packet->flush_pending = pending;
	write_back_packet->cache_block_state = victim_state;
	write_back_packet->write_back_id = write_back_id++;
	write_back_packet->start_cycle = P_TIME;

	//reconstruct the address from the set and tag
	//write_back_packet->address = cache->sets[set].blocks[way].address;
	write_back_packet->address = cgm_cache_build_address(cache, cache->sets[set].id, cache->sets[set].blocks[way].tag);
	assert(write_back_packet->address != 0);
	assert(cache->sets[set].id >=0 && cache->sets[set].id < cache->num_sets);

	if((((write_back_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		/*//verify that there is only one wb in L2 for this block.
		l1_error = cache_search_wb_dup_packets(cache, cache->sets[set].blocks[way].tag, set);
		l2_error = cache_search_wb_dup_packets(&l2_caches[cache->id], cache->sets[set].blocks[way].tag, set);
		l3_error = cache_search_wb_dup_packets(&l3_caches[cgm_l3_cache_map(set)], cache->sets[set].blocks[way].tag, set);*/

		if((LEVEL == 1 || LEVEL == 3) && (cache->cache_type == l1_i_cache_t || cache->cache_type == l1_d_cache_t))
		{
			printf("block 0x%08x %s wb packet created ID %llu cycle %llu\n",
					(write_back_packet->address & cache->block_address_mask), cache->name, write_back_packet->write_back_id, P_TIME);
		}
		else if((LEVEL == 2 || LEVEL == 3) && (cache->cache_type == l2_cache_t || cache->cache_type == l3_cache_t))
		{
			printf("block 0x%08x %s wb packet created ID %llu cycle %llu\n",
					(write_back_packet->address & cache->block_address_mask), cache->name, write_back_packet->write_back_id, P_TIME);
		}
	}

	write_back_packet->set = cache->sets[set].id;
	write_back_packet->tag = cache->sets[set].blocks[way].tag;
	return;
}

/*void init_reply_packet(struct cgm_packet_t *reply_packet, unsigned int address){

	reply_packet->access_type = cgm_access_downgrade_ack;
	reply_packet->downgrade_ack = 1;
	reply_packet->size = 1;
	reply_packet->address = address;
	return;
}*/

void init_downgrade_nack_packet(struct cgm_packet_t *nack_packet, unsigned int address){

	nack_packet->access_type = cgm_access_downgrade_nack;
	nack_packet->downgrade_ack = 1;
	nack_packet->size = 1;
	nack_packet->address = address;
	nack_packet->start_cycle = P_TIME;
	return;

}

void init_upgrade_ack_packet(struct cgm_packet_t *ack_packet, unsigned int address){

	ack_packet->access_type = cgm_access_upgrade_ack;
	ack_packet->downgrade_ack = 1;
	ack_packet->size = 1;
	ack_packet->address = address;
	ack_packet->start_cycle = P_TIME;
	return;

}

void init_downgrade_ack_packet(struct cgm_packet_t *reply_packet, unsigned int address){

	reply_packet->access_type = cgm_access_downgrade_ack;
	reply_packet->downgrade_ack = 1;
	reply_packet->size = 1;
	reply_packet->address = address;
	reply_packet->start_cycle = P_TIME;
	return;
}

void init_getx_fwd_ack_packet(struct cgm_packet_t *reply_packet, unsigned int address){

	reply_packet->access_type = cgm_access_getx_fwd_ack;
	reply_packet->inval_ack = 1;
	reply_packet->size = 1;
	reply_packet->address = address;
	reply_packet->start_cycle = P_TIME;
	return;
}

void init_getx_fwd_nack_packet(struct cgm_packet_t *reply_packet, unsigned int address){

	reply_packet->access_type = cgm_access_getx_fwd_nack;
	reply_packet->inval_ack = 1;
	reply_packet->size = 1;
	reply_packet->address = address;
	reply_packet->start_cycle = P_TIME;
	return;
}


void init_downgrade_packet(struct cgm_packet_t *downgrade_packet, unsigned int address){

	downgrade_packet->access_type = cgm_access_downgrade;
	downgrade_packet->downgrade = 1;
	downgrade_packet->size = 1;
	downgrade_packet->address = address;
	downgrade_packet->start_cycle = P_TIME;
	return;
}

/*void init_upgrade_inval_packet(struct cgm_packet_t *inval_packet, unsigned int address){

	inval_packet->access_type = cgm_access_upgrade;
	inval_packet->inval = 1;
	inval_packet->size = 1;
	inval_packet->address = address;

	return;
}*/

void init_upgrade_request_packet(struct cgm_packet_t *upgrade_request_packet, unsigned int address){

	upgrade_request_packet->access_type = cgm_access_upgrade;
	upgrade_request_packet->upgrade = 1;
	upgrade_request_packet->size = 1;
	upgrade_request_packet->address = address;
	upgrade_request_packet->start_cycle = P_TIME;
	return;
}

void init_upgrade_inval_request_packet(struct cgm_packet_t *upgrade_request_packet, unsigned int address){

	upgrade_request_packet->access_type = cgm_access_upgrade_inval;
	upgrade_request_packet->upgrade = 1;
	upgrade_request_packet->size = 1;
	upgrade_request_packet->address = address;
	upgrade_request_packet->start_cycle = P_TIME;
	return;
}

void init_upgrade_putx_n_inval_request_packet(struct cgm_packet_t *upgrade_request_packet, unsigned int address){

	upgrade_request_packet->access_type = cgm_access_upgrade_inval;
	upgrade_request_packet->upgrade_putx_n = 1;
	upgrade_request_packet->size = 1;
	upgrade_request_packet->address = address;
	upgrade_request_packet->start_cycle = P_TIME;
	return;
}

void init_getx_fwd_inval_packet(struct cgm_packet_t *downgrade_packet, unsigned int address){

	downgrade_packet->access_type = cgm_access_getx_fwd_inval;
	downgrade_packet->inval = 1;
	downgrade_packet->size = 1;
	downgrade_packet->address = address;
	downgrade_packet->start_cycle = P_TIME;
	return;
}

void init_flush_packet(struct cache_t *cache, struct cgm_packet_t *inval_packet, int set, int way){

	inval_packet->access_type = cgm_access_flush_block;
	inval_packet->inval = 1;
	inval_packet->size = 1;
	inval_packet->evict_id = evict_id++;
	inval_packet->write_back_id = write_back_id;
	inval_packet->start_cycle = P_TIME;

	//reconstruct the address from the set and tag
	inval_packet->address = cgm_cache_build_address(cache, cache->sets[set].id, cache->sets[set].blocks[way].tag);

	return;
}
