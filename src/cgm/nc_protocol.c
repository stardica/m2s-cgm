/*
 * nc_protocol.c
 *
 *  Created on: Nov 8, 2015
 *      Author: stardica
 */


//////////////////////
/////GPU NC protocol
//////////////////////

#include <cgm/protocol.h>

void cgm_nc_gpu_s_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

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
			message_packet->gpu_access_type = cgm_access_load_s;
			message_packet->gpu_cache_id = cache->id;

			message_packet->access_type = cgm_access_gets_s;
			message_packet->l1_access_type = cgm_access_gets_s;

			//find victim and evict on return l1_i_cache just drops the block on return
			message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			//set the victim's transient state.
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->l1_victim_way, cgm_cache_block_invalid);

			//charge delay
			P_PAUSE(cache->latency);

			//transmit to L2
			cache_put_io_down_queue(cache, message_packet);
			break;

		case cgm_cache_block_noncoherent:

			//stats
			cache->hits++;

			//set retry state and delay
			if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//charge delay
				P_PAUSE(cache->latency);

				//enter retry state.
				gpu_cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			message_packet->end_cycle = P_TIME;
			cache_gpu_S_return(cache, message_packet);
			break;
	}
	return;
}

void cgm_nc_gpu_v_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

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

			//stats
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//add some routing/status data to the packet
			message_packet->gpu_access_type = cgm_access_load_v;
			message_packet->gpu_cache_id = cache->id;

			message_packet->access_type = cgm_access_gets_v;
			message_packet->l1_access_type = cgm_access_gets_v;

			//find victim and evict on return l1_i_cache just drops the block on return
			message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			//set the victim's transient state.
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->l1_victim_way, cgm_cache_block_invalid);

			//charge delay
			P_PAUSE(cache->latency);

			//transmit to L2
			cache_put_io_down_queue(cache, message_packet);
			break;

		case cgm_cache_block_noncoherent:

			//stats
			cache->hits++;

			//set retry state and delay
			if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//charge delay
				P_PAUSE(cache->latency);

				//enter retry state.
				gpu_cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			message_packet->end_cycle = P_TIME;
			cache_gpu_v_return(cache, message_packet);
			break;
	}
	return;
}

void cgm_nc_gpu_v_store(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

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

			//stats
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//add some routing/status data to the packet
			message_packet->gpu_access_type = cgm_access_store_v;
			message_packet->gpu_cache_id = cache->id;

			//star todo this is wrong this should be get_v NOT gets_v
			message_packet->access_type = cgm_access_gets_v;
			message_packet->l1_access_type = cgm_access_gets_v;

			//find victim and evict on return l1_d_cache just drops the block on return
			message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			//set the victim's transient state.
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->l1_victim_way, cgm_cache_block_invalid);

			//charge delay
			P_PAUSE(cache->latency);

			//transmit to L2
			cache_put_io_down_queue(cache, message_packet);
			break;

		case cgm_cache_block_noncoherent:

			//stats
			cache->hits++;

			//set retry state and delay
			if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
			{
				//charge delay
				P_PAUSE(cache->latency);

				//enter retry state.
				gpu_cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			message_packet->end_cycle = P_TIME;
			cache_gpu_v_return(cache, message_packet);
			break;
	}
	return;
}

void cgm_nc_gpu_s_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//check the packet for integrity
	assert(cache->cache_type == gpu_s_cache_t);
	//assert(message_packet->access_type == cgm_access_puts && message_packet->cache_block_state == cgm_cache_block_shared);
	//make sure victim way was correctly stored.
	assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

	enum cgm_cache_block_state_t victim_trainsient_state;
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);
	assert(victim_trainsient_state == cgm_cache_block_transient);

	//find the access in the ORT table and clear it.
	ort_clear(cache, message_packet);

	//set the block and retry the access in the cache.
	if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
	{
		printf("block 0x%08x %s write block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, message_packet->cache_block_state, P_TIME);
	}

	cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_gpu_cache_get_retry_state(message_packet->cpu_access_type);

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	return;
}

void cgm_nc_gpu_v_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//check the packet for integrity
	assert(cache->cache_type == gpu_v_cache_t);
	//assert(message_packet->access_type == cgm_access_puts && message_packet->cache_block_state == cgm_cache_block_shared);
	//make sure victim way was correctly stored.
	assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

	enum cgm_cache_block_state_t victim_trainsient_state;
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);
	assert(victim_trainsient_state == cgm_cache_block_transient);

	//find the access in the ORT table and clear it.
	ort_clear(cache, message_packet);

	//set the block and retry the access in the cache.
	if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
	{
		printf("block 0x%08x %s write block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, message_packet->cache_block_state, P_TIME);
	}

	cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_gpu_cache_get_retry_state(message_packet->cpu_access_type);

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	return;
}

void cgm_nc_gpu_l2_get(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//charge delay
	P_PAUSE(cache->latency);

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

			//stats
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//find victim and evict on return
			message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			//add some routing/status data to the packet
			message_packet->gpu_access_type = cgm_access_mc_load;

			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&gpu_l2_strn_map, cache->name);

			message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
			message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

			//transmit to L2
			cache_put_io_down_queue(cache, message_packet);
			break;

		case cgm_cache_block_noncoherent:

			//stats
			cache->hits++;

			//set retry state and delay
			if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				gpu_cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			message_packet->access_type = cgm_access_puts;
			cache_put_io_up_queue(cache, message_packet);
			break;
	}
	return;

}

void cgm_nc_gpu_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//check the packet for integrity
	assert(cache->cache_type == gpu_l2_cache_t);
	//assert(message_packet->access_type == cgm_access_puts && message_packet->cache_block_state == cgm_cache_block_shared);
	//make sure victim way was correctly stored.
	assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

	enum cgm_cache_block_state_t victim_trainsient_state;
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);
	assert(victim_trainsient_state == cgm_cache_block_transient);

	//find the access in the ORT table and clear it.
	ort_clear(cache, message_packet);

	//set the block and retry the access in the cache.
	if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
	{
		printf("block 0x%08x %s write block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, message_packet->cache_block_state, P_TIME);
	}

	cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_gpu_cache_get_retry_state(message_packet->cpu_access_type);

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	return;
}
