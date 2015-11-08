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
		{"cgm_access_inv_ack", cgm_access_inv_ack},
		{"cgm_access_upgrade", cgm_access_upgrade},
		{"cgm_access_upgrade_ack", cgm_access_upgrade_ack},
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

/*long long temp_access_id = 0;*/
long long write_back_id = 0;

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

void init_write_back_packet(struct cache_t *cache, struct cgm_packet_t *write_back_packet, int set, int way, int pending, enum cgm_cache_block_state_t victim_state){

	write_back_packet->access_type = cgm_access_write_back;
	write_back_packet->flush_pending = pending;
	write_back_packet->cache_block_state = victim_state;
	write_back_packet->write_back_id = write_back_id++;

	//reconstruct the address from the set and tag
	//write_back_packet->address = cache->sets[set].blocks[way].address;
	write_back_packet->address = cgm_cache_build_address(cache, cache->sets[set].id, cache->sets[set].blocks[way].tag);
	assert(write_back_packet->address != 0);
	assert(cache->sets[set].id >=0 && cache->sets[set].id < cache->num_sets);

	if(((write_back_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
	{
		printf("block 0x%08x %s downgrade_ack wb packet created ID %llu cycle %llu\n",
			(write_back_packet->address & cache->block_address_mask), cache->name, write_back_packet->write_back_id, P_TIME);
	}

	write_back_packet->set = cache->sets[set].id;
	write_back_packet->tag = cache->sets[set].blocks[way].tag;
	return;
}

void init_reply_packet(struct cgm_packet_t *reply_packet, unsigned int address){

	reply_packet->access_type = cgm_access_downgrade_ack;
	reply_packet->downgrade_ack = 1;
	reply_packet->size = 1;
	reply_packet->address = address;
	return;
}

void init_downgrade_ack_packet(struct cgm_packet_t *reply_packet, unsigned int address){

	reply_packet->access_type = cgm_access_downgrade_ack;
	reply_packet->downgrade_ack = 1;
	reply_packet->size = 1;
	reply_packet->address = address;
	return;
}

void init_getx_fwd_ack_packet(struct cgm_packet_t *reply_packet, unsigned int address){

	reply_packet->access_type = cgm_access_getx_fwd_ack;
	reply_packet->inval_ack = 1;
	reply_packet->size = 1;
	reply_packet->address = address;
	return;
}


void init_downgrade_packet(struct cgm_packet_t *downgrade_packet, unsigned int address){

	downgrade_packet->access_type = cgm_access_downgrade;
	downgrade_packet->downgrade = 1;
	downgrade_packet->size = 1;
	downgrade_packet->address = address;

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
	return;
}

void init_upgrade_inval_request_packet(struct cgm_packet_t *upgrade_request_packet, unsigned int address){

	upgrade_request_packet->access_type = cgm_access_upgrade_inval;
	upgrade_request_packet->upgrade = 1;
	upgrade_request_packet->size = 1;
	upgrade_request_packet->address = address;

	return;
}

void init_upgrade_putx_n_inval_request_packet(struct cgm_packet_t *upgrade_request_packet, unsigned int address){

	upgrade_request_packet->access_type = cgm_access_upgrade_inval;
	upgrade_request_packet->upgrade_putx_n = 1;
	upgrade_request_packet->size = 1;
	upgrade_request_packet->address = address;

	return;
}

void init_getx_fwd_inval_packet(struct cgm_packet_t *downgrade_packet, unsigned int address){

	downgrade_packet->access_type = cgm_access_getx_fwd_inval;
	downgrade_packet->inval = 1;
	downgrade_packet->size = 1;
	downgrade_packet->address = address;

	return;
}

void init_flush_packet(struct cache_t *cache, struct cgm_packet_t *inval_packet, int set, int way){

	inval_packet->access_type = cgm_access_inv;
	inval_packet->inval = 1;
	inval_packet->size = 1;

	//reconstruct the address from the set and tag
	inval_packet->address = cgm_cache_build_address(cache, cache->sets[set].id, cache->sets[set].blocks[way].tag);

	return;
}


////////////////////////////
//GPU non coherent functions
////////////////////////////
void cgm_nc_gpu_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	enum cgm_access_kind_t access_type;
	/*long long access_id = 0;*/
	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	access_type = message_packet->access_type;
	/*access_id = message_packet->access_id;*/

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_exclusive:
		case cgm_cache_block_shared:
		case cgm_cache_block_modified:
		case cgm_cache_block_owned:
			fatal("gpu_s_cache_ctrl(): Invalid block state on hit as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		//miss or invalid cache block states
		case cgm_cache_block_invalid:

			/*printf("I$ miss fetch\n");*/

			//stats
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//add some routing/status data to the packet
			message_packet->gpu_access_type = cgm_access_load;
			message_packet->gpu_cache_id = cache->id;

			message_packet->access_type = cgm_access_gets_s;
			message_packet->l1_access_type = cgm_access_gets_s;

			//find victim and evict on return l1_i_cache just drops the block on return
			message_packet->l1_victim_way = cgm_cache_replace_block(cache, message_packet->set);

			//charge delay
			P_PAUSE(cache->latency);

			//transmit to L2
			cache_put_io_down_queue(cache, message_packet);
			break;

		case cgm_cache_block_noncoherent:

			//stats
			cache->hits++;

			//set retry state and delay
			if(access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				P_PAUSE(cache->latency);

				//enter retry state.
				gpu_cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			/*printf("fetch id %llu set %d tag %d complete cycle %llu\n", message_packet->access_id, message_packet->tag, message_packet->set, P_TIME);*/
			message_packet->end_cycle = P_TIME;
			cache_gpu_S_return(cache, message_packet);
			break;
	}
	return;
}



void gpu_l1_cache_access_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//struct cgm_packet_t *ort_packet;
	//struct cgm_packet_t *miss_status_packet;
	enum cgm_access_kind_t access_type;
	unsigned int addr = 0;
	long long access_id = 0;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int state = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;

	int cache_status = 0;

	int i = 0;
	int row = 0;

	//stats
	cache->loads++;

	//access information
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	//store the decode
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;


	CGM_DEBUG(GPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//////testing
	/*if(gpu_l1_inf && (cache->cache_type == gpu_v_cache_t || cache->cache_type == gpu_s_cache_t))
	{
		if(cache->cache_type == gpu_v_cache_t || cache->cache_type == gpu_s_cache_t)
		{
		cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_noncoherent);
		}
	}*/
	//////testing

	//get the block and the state of the block and charge cycles
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(cache->latency);



	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{
		assert(*state_ptr != cgm_cache_block_invalid);
		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

		if(*state_ptr == cgm_cache_block_noncoherent) //*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
		{
			cache->hits++;

			//GPU L1 caches
			if(message_packet->access_type == cgm_access_load_s)
			{
				//remove packet from cache queue, global queue, and simulator memory
				(*message_packet->witness_ptr)++;
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);
			}
			else if(message_packet->access_type == cgm_access_load_v)
			{
				//remove packet from cache queue, global queue, and simulator memory
				(*message_packet->witness_ptr)++;
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);
			}
			else
			{
				fatal("gpu_l1_cache_access_load(): incorrect access type %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
			}
		}
		else
		{
			fatal("gpu_l1_cache_access_load(): incorrect block state set\n");
		}

		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu hit state %s\n", cache->name, access_id, P_TIME, str_map_value(&cgm_cache_block_state_map, *state_ptr));
	}
	//Cache miss or cache block is invalid
	else if(cache_status == 0 || *state_ptr == 0)
	{
		cache->misses++;

		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		//GPU L1 S cache
		if(message_packet->access_type == cgm_access_load_s)
		{
			message_packet->gpu_access_type = cgm_access_load_s;
			message_packet->gpu_cache_id = cache->id;

			message_packet->access_type = cgm_access_gets_s;
			message_packet->l1_access_type = cgm_access_gets_s;
		}
		//GPU L1 V cache
		else if(message_packet->access_type == cgm_access_load_v)
		{
			message_packet->gpu_access_type = cgm_access_load_v;
			message_packet->gpu_cache_id = cache->id;

			message_packet->access_type = cgm_access_gets_v;
			message_packet->l1_access_type = cgm_access_gets_v;
		}
		else
		{
			fatal("gpu_l1_cache_access_load(): invalid GPU l1 cache access type %s access_id %llu cycle %llu",
					str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), access_id, P_TIME);
		}

		//miss so check ORT status
		i = ort_search(cache, tag, set);

		//entry was not found
		if(i == cache->mshr_size)
		{

			//get an empty row and set the ORT values.
			row = get_ort_status(cache);
			assert(row < cache->mshr_size);
			ort_set(cache, row, tag, set);

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu l2 queue free size %d\n",
					cache->name, access_id, P_TIME, list_count(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].Rx_queue_top));

			P_PAUSE(cache->latency);
			//P_PAUSE(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].wire_latency);

			message_packet = list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			//list_enqueue(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].Rx_queue_top, message_packet);
			list_enqueue(cache->Tx_queue_bottom, message_packet);
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu %s as %s\n",
					cache->name, access_id, P_TIME, gpu_l2_caches[cgm_gpu_cache_map(cache->id)].name, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu %s Miss SEND %s %s\n",
					access_id, P_TIME, cache->name, gpu_l2_caches[cgm_gpu_cache_map(cache->id)].name, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			//advance the L2 cache adding some wire delay time.
			//future_advance(&gpu_l2_cache[cgm_gpu_cache_map(cache->id)], WIRE_DELAY(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].wire_latency));
			//advance(&gpu_l2_cache[cgm_gpu_cache_map(cache->id)]);
			advance(cache->cache_io_down_ec);

		}
		else if (i >= 0 && i < cache->mshr_size)
		{
			//entry found in ORT so coalesce access
			assert(cache->ort[i][0] == tag && cache->ort[i][1] == set && cache->ort[i][2] == 1);

			P_PAUSE(cache->latency);

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu coalesced\n",
					cache->name, access_id, P_TIME);

			list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->ort_list, message_packet);
		}
		else
		{
			fatal("gpu_l1_cache_access_load(): %s i outside of bounds\n", cache->name);
		}
	}
	return;
}

void gpu_l1_cache_access_store(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//struct cgm_packet_t *miss_status_packet;
	enum cgm_access_kind_t access_type;
	unsigned int addr = 0;
	long long access_id = 0;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int state = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;

	int cache_status = 0;
	//int mshr_status = 0;

	int i = 0;
	int row = 0;

	//stats
	cache->stores++;

	//access information
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, addr, set_ptr, tag_ptr, offset_ptr);
	//store the decode for later
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	//////testing
	/*if(gpu_l1_inf && cache->cache_type == gpu_v_cache_t)
	{
		if(cache->cache_type == gpu_v_cache_t)
		{
		cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_noncoherent);
		}
	}*/
	//////testing

	/*CGM_DEBUG(GPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);*/


	//get the block and the state of the block and charge cycles
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(cache->latency);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)// && *state_ptr != cache_block_shared)
	{
		//check state of the block
		//block is valid

		assert(*state_ptr != cgm_cache_block_invalid);

		//star todo this is wrong
		if(*state_ptr == cgm_cache_block_noncoherent) //*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
		{
			cache->hits++;

			if(message_packet->access_type == cgm_access_store_v || message_packet->access_type == cgm_access_nc_store)
			{
				(*message_packet->witness_ptr)++;
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);
			}
			else
			{
				fatal("gpu_l1_cache_access_store(): incorrect access type %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
			}
		}
		else
		{
			fatal("gpu_l1_cache_access_store(): incorrect block state set");
		}
	}
	//Cache Miss!
	else if(cache_status == 0 || *state_ptr == 0)
	{

		cache->misses++;

		//on both a miss and invalid hit the state_ptr should be zero

		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		//only the D$ stores
		message_packet->gpu_access_type = cgm_access_store_v;
		message_packet->gpu_cache_id = cache->id;

		message_packet->access_type = cgm_access_gets_v;
		message_packet->l1_access_type = cgm_access_gets_v;

		//access is unique in the MSHR so send forward
		//while the next level of cache's in queue is full stall

		//miss so check ORT status
		i = ort_search(cache, tag, set);

		//entry was not found
		if(i == cache->mshr_size)
		{
			//get an empty row and set the ORT values.
			row = get_ort_status(cache);
			assert(row < cache->mshr_size);
			ort_set(cache, row, tag, set);

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu l2 queue free size %d\n",
				cache->name, access_id, P_TIME, list_count(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].Rx_queue_top));

			P_PAUSE(cache->latency);
			//P_PAUSE(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].wire_latency);

			/*change the access type for the coherence protocol and drop into the L2's queue
			remove the access from the l1 cache queue and place it in the l2 cache ctrl queue*/

			message_packet = list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			//list_enqueue(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].Rx_queue_top, message_packet);
			list_enqueue(cache->Tx_queue_bottom, message_packet);
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu %s as %s\n",
					cache->name, access_id, P_TIME, l2_caches[cache->id].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
			CGM_DEBUG(protocol_debug_file, "%s Access_id %llu cycle %llu %s miss SEND %s %s\n",
					cache->name, access_id, P_TIME, cache->name, l2_caches[cache->id].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			//advance the L2 cache adding some wire delay time.
			//future_advance(&l2_cache[cache->id], WIRE_DELAY(l2_caches[cache->id].wire_latency));

			//advance(&gpu_l2_cache[cgm_gpu_cache_map(cache->id)]);
			advance(cache->cache_io_down_ec);
		}
		else if (i >= 0 && i < cache->mshr_size)
		{
			//entry found in ORT so coalesce access
			assert(cache->ort[i][0] == tag && cache->ort[i][1] == set && cache->ort[i][2] == 1);

			P_PAUSE(cache->latency);

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu coalesced\n",
					cache->name, access_id, P_TIME);

			list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->ort_list, message_packet);
		}
		else
		{
			fatal("gpu_l1_cache_access_store(): %s i outside of bounds\n", cache->name);
		}
	}
	return;
}


void gpu_cache_access_get(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//struct cgm_packet_status_t *miss_status_packet;
	enum cgm_access_kind_t access_type;
	unsigned int addr = 0;
	long long access_id = 0;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int state = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;

	int cache_status;
	//int mshr_status = 0;
	//int l3_map = -1;

	/*int i = 0;*/
	int row = 0;

	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;

	//stats
	cache->loads++;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(GPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//////testing
	/*if(gpu_l2_inf && cache->cache_type == gpu_l2_cache_t)
	{
		if(cache->cache_type == gpu_l2_cache_t)
		{
		cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_noncoherent);
		}
	}*/
	//////testing

	//look up, and charge a cycle.
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(cache->latency);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{

		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

		assert(*state_ptr != cgm_cache_block_invalid);

		if(*state_ptr == cgm_cache_block_noncoherent)// *state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
		{
			cache->hits++;

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			if(cache->cache_type == gpu_l2_cache_t)
			{

				if (message_packet->access_type == cgm_access_gets_s)
				{
					//while the next level of cache's in queue is full stall
					while(!cache_can_access_bottom(&gpu_s_caches[message_packet->gpu_cache_id]))
					{
						printf("%s stalled cycle %llu\n", cache->name, P_TIME);
						P_PAUSE(1);
					}

					CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu L1 bottom queue free size %d\n",
							cache->name, access_id, P_TIME, list_count(gpu_s_caches[message_packet->gpu_cache_id].Rx_queue_bottom));

					P_PAUSE(cache->latency);
					//P_PAUSE(gpu_s_caches[message_packet->gpu_cache_id].wire_latency);

					message_packet->access_type = cgm_access_puts;
					message_packet = list_remove(cache->last_queue, message_packet);

					list_enqueue(cache->Tx_queue_top, message_packet);
					advance(cache->cache_io_up_ec);
					//list_enqueue(gpu_s_caches[message_packet->gpu_cache_id].Rx_queue_bottom, message_packet);
					//advance(&gpu_s_cache[message_packet->gpu_cache_id]);
					//future_advance(&gpu_s_cache[message_packet->gpu_cache_id], WIRE_DELAY(gpu_s_caches[message_packet->gpu_cache_id].wire_latency));

				}
				else if (message_packet->access_type == cgm_access_gets_v)
				{
					//while the next level of cache's in queue is full stall
					while(!cache_can_access_bottom(&gpu_v_caches[message_packet->gpu_cache_id]))
					{
						printf("%s stalled cycle %llu\n", cache->name, P_TIME);
						P_PAUSE(1);
					}

					CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu %s free size %d\n",
							cache->name, access_id, P_TIME, gpu_v_caches[message_packet->gpu_cache_id].Rx_queue_bottom->name,
							list_count(gpu_v_caches[message_packet->gpu_cache_id].Rx_queue_bottom));


					P_PAUSE(cache->latency);
					//P_PAUSE(gpu_v_caches[message_packet->gpu_cache_id].wire_latency);

					message_packet->access_type = cgm_access_puts;
					message_packet = list_remove(cache->last_queue, message_packet);
					//list_enqueue(gpu_v_caches[message_packet->gpu_cache_id].Rx_queue_bottom, message_packet);
					//future_advance(&gpu_v_cache[message_packet->gpu_cache_id], WIRE_DELAY(gpu_v_caches[message_packet->gpu_cache_id].wire_latency));
					//advance(&gpu_v_cache[message_packet->gpu_cache_id]);
					list_enqueue(cache->Tx_queue_top, message_packet);
					advance(cache->cache_io_up_ec);

					CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu %s Hit SEND %s to %s\n",
							access_id, P_TIME, cache->name, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), l1_i_caches[cache->id].name);
				}
				else
				{
					fatal("gpu_cache_access_get(): %s access_id %llu cycle %llu incorrect access type %s\n",
							cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
				}
			}
			else
			{
					fatal("gpu_cache_access_get(): hit bad cache type %s access_id %llu cycle %llu type %s\n",
							cache->name, access_id, P_TIME, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
			}
		}
		else
		{
			fatal("gpu_cache_access_get(): bad cache block type\n");
		}

	}
	//Cache Miss!
	else if(cache_status == 0 || *state_ptr == 0)
	{
		cache->misses++;

		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		if(cache->cache_type == gpu_l2_cache_t)
		{
			//get an empty row and set the ORT values.
			row = get_ort_status(cache);
			assert(row < cache->mshr_size);
			ort_set(cache, row, tag, set);

			/*while(!hub_iommu_can_access(hub_iommu->Rx_queue_top[cache->id]))
			{
				printf("%s stalled cycle %llu\n", cache->name, P_TIME);
				P_PAUSE(1);
			}*/

			/*CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu miss %s size %d\n",
					cache->name, access_id, P_TIME, hub_iommu->Rx_queue_top[cache->id]->name, list_count(hub_iommu->Rx_queue_top[cache->id]));

			P_PAUSE(cache->latency);
			//P_PAUSE(hub_iommu->wire_latency);*/

			//message_packet->l1_access_type = message_packet->access_type;

			//changes start here
			//message_packet->access_type = cgm_access_gets;
			message_packet->access_type = cgm_access_mc_load;

			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&gpu_l2_strn_map, cache->name);

			message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
			message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

			message_packet = list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			//list_enqueue(hub_iommu->Rx_queue_top[cache->id], message_packet);
			//list_enqueue(switches[cache->id].north_queue, message_packet);
			list_enqueue(cache->Tx_queue_bottom, message_packet);

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu %s send %s\n",
					cache->name, access_id, P_TIME, cache->name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			//star todo figure out what to do with this.
			CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu %s Miss SEND %s %s\n",
					access_id, P_TIME, cache->name, gpu_l2_caches[cache->id].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));


			//advance the L2 cache adding some wire delay time.
			//future_advance(hub_iommu_ec, WIRE_DELAY(hub_iommu->wire_latency));
			//advance(hub_iommu_ec);
			advance(cache->cache_io_down_ec);

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, row);

		}
		else
		{
			fatal("gpu_cache_access_get(): miss bad cache type %s access_id %llu cycle %llu type %s\n",
					cache->name, access_id, P_TIME, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
		}

	}
	return;
}


void gpu_cache_access_put(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*struct cgm_packet_t *ort_packet;
	enum cgm_access_kind_t access_type;*/
	unsigned int addr = 0;
	long long access_id = 0;
	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int state = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;

	/*int mshr_row = -1;

	int i = 0;
	int adv = 0;*/
	int row = 0;


	assert(message_packet != NULL);

	//the packet is from the L2 cache
	/*access_type = message_packet->access_type;*/
	addr = message_packet->address;
	access_id = message_packet->access_id;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	/*CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu puts\n", cache->name, access_id, P_TIME);*/

	//block is returned so find it in the ORT
	//clear the entry from the ort
	row = ort_search(cache, tag, set);
	assert(row < cache->mshr_size);
	ort_clear(cache, message_packet);

	if(cache->cache_type == gpu_s_cache_t)
	{
		//Evict, get the LRU
		*(way_ptr) = cgm_cache_replace_block(cache, set);

		//i cache blocks should either be in the shared or invalid state (initialized).
		if(*state_ptr == cgm_cache_block_noncoherent || *state_ptr == cgm_cache_block_invalid) //*state_ptr == cache_block_shared)
		{
			//with s cache it is ok to evict and just drop the block
			cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_noncoherent);
		}
		else
		{
			fatal("gpu_cache_access_put(): s cache invalid block state\n");
		}
	}
	else if(cache->cache_type == gpu_v_cache_t)
	{
		//Evict, get the LRU
		*(way_ptr) = cgm_cache_replace_block(cache, set);

		//get the state of the block
		cgm_cache_get_block(cache, set, way, NULL, state_ptr);

		//v cache blocks can either be in the shared, exclusive, modified or invalid state (initialized and invalidated).
		//for a single core machine the d cache line will always be exclusive, modified, or invalid.
		if(*state_ptr == cgm_cache_block_noncoherent || *state_ptr == cgm_cache_block_invalid) //*state_ptr == cache_block_shared || *state_ptr == cache_block_exclusive)
		{
			//if the block is invalid it is ok to drop the line.
			cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_noncoherent);
		}
		else
		{
			fatal("gpu_cache_access_put(): v cache invalid block state\n");
		}
	}
	else if(cache->cache_type == gpu_l2_cache_t)
	{
		//Evict, get the LRU
		*(way_ptr) = cgm_cache_replace_block(cache, set);

		//get the state of the block
		cgm_cache_get_block(cache, set, way, NULL, state_ptr);

		if(*state_ptr == cgm_cache_block_noncoherent || *state_ptr == cgm_cache_block_invalid) //*state_ptr == cache_block_shared || *state_ptr == cache_block_exclusive)
		{
			//if the block is invalid it is ok to drop the line.
			cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_noncoherent);
		}
		else
		{
			fatal("gpu_cache_access_put(): l2 cache invalid block state %s\n", str_map_value(&cgm_cache_block_state_map, message_packet->access_type));
		}
	}

	//set retry
	message_packet->access_type = cgm_access_retry;
	list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	advance(cache->ec_ptr);

	return;
}

void gpu_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet){

	enum cgm_access_kind_t access_type;
	unsigned int addr = 0;
	long long access_id = 0;
	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int state = 0;
	int cache_status;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;

	/*int i = 0;*/

	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;

	//stats
	cache->retries++;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(GPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
		cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//look up, and charge a cycle.
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	//P_PAUSE(cache->latency);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{
		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu retry hit\n", cache->name, access_id, P_TIME);

		if(*state_ptr == cgm_cache_block_noncoherent) //*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
		{

			//list_remove(cache->retry_queue, message_packet);
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
				cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			//send to correct l1 cache and change access type
			if(cache->cache_type == gpu_s_cache_t || cache->cache_type == gpu_v_cache_t)
			{
				//GPU L1 S Cache
				if(message_packet->gpu_access_type == cgm_access_load_s)
				{
					//clear out the returned memory request
					//remove packet from cache retry queue and global queue
					P_PAUSE(cache->latency);

					(*message_packet->witness_ptr)++;
					message_packet = list_remove(cache->last_queue, message_packet);
					packet_destroy(message_packet);

					//retry coalesced packets.
					gpu_cache_coalesced_retry(cache, tag_ptr, set_ptr);

				}
				//GPU L1 V Cache
				else if(message_packet->gpu_access_type == cgm_access_load_v ||
						message_packet->gpu_access_type == cgm_access_store_v ||
						message_packet->gpu_access_type == cgm_access_nc_store)
				{

					P_PAUSE(cache->latency);

					(*message_packet->witness_ptr)++;
					message_packet = list_remove(cache->last_queue, message_packet);
					packet_destroy(message_packet);

					//retry coalesced packets.
					gpu_cache_coalesced_retry(cache, tag_ptr, set_ptr);
				}

			}
			else if(cache->cache_type == gpu_l2_cache_t)
			{

				if (message_packet->l1_access_type == cgm_access_gets_s)
				{
					//while the next level of cache's in queue is full stall
					while(!cache_can_access_bottom(&gpu_s_caches[message_packet->gpu_cache_id]))
					{
						printf("%s stalled cycle %llu\n", cache->name, P_TIME);
						P_PAUSE(1);
					}

						CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu %s free size %d\n",
							cache->name, access_id, P_TIME, gpu_s_caches[message_packet->gpu_cache_id].Rx_queue_bottom->name,
							list_count(gpu_s_caches[message_packet->gpu_cache_id].Rx_queue_bottom));

						P_PAUSE(cache->latency);
						//P_PAUSE(gpu_s_caches[message_packet->gpu_cache_id].wire_latency);

						message_packet->access_type = cgm_access_puts;
						message_packet = list_remove(cache->last_queue, message_packet);

						//list_enqueue(gpu_s_caches[message_packet->gpu_cache_id].Rx_queue_bottom, message_packet);
						//advance(&gpu_s_cache[message_packet->gpu_cache_id]);
						list_enqueue(cache->Tx_queue_top, message_packet);
						advance(cache->cache_io_up_ec);

						//future_advance(&gpu_s_cache[cache->id], WIRE_DELAY(gpu_s_caches[cache->id].wire_latency));


						//retry coalesced packets.
						//cpu_cache_coalesced_retry(cache, tag_ptr, set_ptr);

				}
				else if (message_packet->l1_access_type == cgm_access_gets_v)
				{
					//while the next level of cache's in queue is full stall
					while(!cache_can_access_bottom(&gpu_v_caches[message_packet->gpu_cache_id]))
					{
						printf("%s stalled cycle %llu \n", cache->name, P_TIME);
						P_PAUSE(1);
					}

					P_PAUSE(cache->latency);
					//P_PAUSE(gpu_v_caches[message_packet->gpu_cache_id].wire_latency);

					CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu %s free size %d\n",
							cache->name, access_id, P_TIME, gpu_v_caches[message_packet->gpu_cache_id].Rx_queue_bottom->name,
							list_count(gpu_v_caches[message_packet->gpu_cache_id].Rx_queue_bottom));

					message_packet->access_type = cgm_access_puts;
					message_packet = list_remove(cache->last_queue, message_packet);

					//list_enqueue(gpu_v_caches[message_packet->gpu_cache_id].Rx_queue_bottom, message_packet);

					//future_advance(&gpu_v_cache[cache->id], WIRE_DELAY(gpu_v_caches[cache->id].wire_latency));
					//advance(&gpu_v_cache[message_packet->gpu_cache_id]);

					list_enqueue(cache->Tx_queue_top, message_packet);
					advance(cache->cache_io_up_ec);

					//retry coalesced packets.
					//cpu_cache_coalesced_retry(cache, tag_ptr, set_ptr);
				}
				else
				{
					fatal("gpu_cache_access_retry(): %s access_id %llu cycle %llu incorrect l1 access type %s\n",
							cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->l1_access_type));
				}
			}
			else
			{
				fatal("gpu_cache_access_retry(): bad cache type\n");
			}
		}
		else
		{
			fatal("gpu_cache_access_retry(): incorrect block state set");
		}
	}
	else
	{
		fatal("gpu_cache_access_retry(): miss on retry");
	}

	//CGM_DEBUG(GPU_cache_debug_file, "%s last put cycle %llu\n", cache->name, P_TIME);

	return;
}

void gpu_cache_coalesced_retry(struct cache_t *cache, int *tag_ptr, int *set_ptr){

	struct cgm_packet_t *ort_packet;
	int i = 0;
	int tag = *tag_ptr;
	int set = *set_ptr;

	LIST_FOR_EACH(cache->ort_list, i)
	{
		//get pointer to access in queue and check it's status.
		ort_packet = list_get(cache->ort_list, i);

		if(ort_packet->tag == tag && ort_packet->set == set)
		{
			ort_packet = list_remove_at(cache->ort_list, i);
			ort_packet->access_type = cgm_access_retry;
			list_enqueue(cache->retry_queue, ort_packet);
			advance(cache->ec_ptr);

			//this may cause problems the intent is to run one coalesced packet per iteration of the retry state.
			return;
		}
	}
	//no coalesced packets remaining.
	return;
}
