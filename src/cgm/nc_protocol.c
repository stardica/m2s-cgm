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

	/*printf("%s s load vtl_addr 0x%08x\n", cache->name, message_packet->address);*/
	/*STOP;*/

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	P_PAUSE(cache->latency);
	message_packet = list_remove(cache->last_queue, message_packet);
	(*message_packet->witness_ptr)++;
	packet_destroy(message_packet);
	return;

	/*switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_exclusive:
		case cgm_cache_block_shared:
		case cgm_cache_block_modified:
		case cgm_cache_block_owned:
			fatal("gpu_s_cache_ctrl(): Invalid block state on hit as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		case cgm_cache_block_invalid:

			//charge delay
			P_PAUSE(cache->latency);

			//stats
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
			{
				if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
				{
					printf("block 0x%08x %s fetch miss coalesce ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
				return;
			}

			//add some routing/status data to the packet
			message_packet->gpu_access_type = cgm_access_load_s;

			message_packet->gpu_cache_id = cache->id;
			message_packet->access_type = cgm_access_gets_s;
			message_packet->l1_access_type = cgm_access_gets_s;

			//find victim and evict
			message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			//evict the block
			assert(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l1_victim_way) == cgm_cache_block_invalid
					|| cgm_cache_get_block_state(cache, message_packet->set, message_packet->l1_victim_way) == cgm_cache_block_noncoherent);
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->l1_victim_way, cgm_cache_block_invalid);

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
			cache_gpu_s_return(cache, message_packet);
			break;
	}
	return;*/
}

void cgm_nc_gpu_v_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("%s v load\n", cache->name);*/
	/*STOP;*/

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	/*message_packet = list_remove(cache->last_queue, message_packet);
	(*message_packet->witness_ptr)++;
	packet_destroy(message_packet);
	return;*/

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

			if(message_packet->access_id == 1628128)
			{
				ort_dump(cache);
				printf("%s message %llu here.\n", cache->name, message_packet->access_id);
			}

			//add some routing/status data to the packet
			/*message_packet->gpu_access_type = cgm_access_load_v;*/

			message_packet->l1_cache_id = cache->id;
			message_packet->access_type = cgm_access_gets_v;
			message_packet->l1_access_type = cgm_access_gets_v;

			//find victim and evict
			message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			//evict the block
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

	/*printf("%s v store\n", cache->name);*/
	/*STOP;*/

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	/*message_packet = list_remove(cache->last_queue, message_packet);
	(*message_packet->witness_ptr)++;
	free(message_packet);
	return;*/

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
			message_packet->l1_cache_id = cache->id;
			message_packet->access_type = cgm_access_gets_v;
			message_packet->l1_access_type = cgm_access_gets_v;

			//find victim and evict
			message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			//evict the block
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
	//make sure victim way was correctly stored.
	assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);
	assert(message_packet->cache_block_state == cgm_cache_block_noncoherent);

	//find the access in the ORT table and clear it.
	ort_clear(cache, message_packet);

	enum cgm_cache_block_state_t victim_trainsient_state;
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);
	/*assert(victim_trainsient_state == cgm_cache_block_transient);*/

	if(victim_trainsient_state != cgm_cache_block_transient)
	{
		cgm_cache_dump_set(cache, message_packet->set);

		unsigned int temp = message_packet->address;
		temp = temp & cache->block_address_mask;

		fatal("cgm_mesi_l1_d_write_block(): %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d cycle %llu\n",
			cache->name, message_packet->access_id, message_packet->address, temp, message_packet->set, message_packet->tag, message_packet->l1_victim_way, P_TIME);
	}

	/*printf("%s write block id %llu cycle %llu\n", cache->name, message_packet->access_id, P_TIME);*/

	//set the block and retry the access in the cache.
	if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
	{
		printf("block 0x%08x %s write block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, message_packet->cache_block_state, P_TIME);
	}

	cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_gpu_cache_get_retry_state(message_packet->gpu_access_type);

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

	//find the access in the ORT table and clear it.
	ort_clear(cache, message_packet);

	enum cgm_cache_block_state_t victim_trainsient_state;
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);
	assert(victim_trainsient_state == cgm_cache_block_transient);

	//set the block and retry the access in the cache.
	if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
	{
		printf("block 0x%08x %s write block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, message_packet->cache_block_state, P_TIME);
	}

	cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_gpu_cache_get_retry_state(message_packet->gpu_access_type);

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

	/*printf("gpu l2 id %d made it here id %llu cycle %llu\n", message_packet->gpu_cache_id, message_packet->access_id, P_TIME);*/

	/*message_packet->access_type = cgm_access_puts;
	message_packet->size = 64;
	message_packet->cache_block_state = cgm_cache_block_noncoherent;

	cache_put_io_up_queue(cache, message_packet);

	return;
	fatal("gpu l2 running\n");*/

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
			message_packet->l2_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l2_victim_way >= 0 && message_packet->l2_victim_way < cache->assoc);

			//evict the block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->l2_victim_way, cgm_cache_block_invalid);

			//add some routing/status data to the packet
			//message_packet->gpu_access_type = cgm_access_mc_load;

			message_packet->access_type = cgm_access_mc_load;
			message_packet->cache_block_state = cgm_cache_block_noncoherent;

			message_packet->l2_cache_id = cache->id;
			message_packet->l2_cache_name = cache->name;

			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&gpu_l2_strn_map, cache->name);

			message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
			message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

			//transmit to system agent
			cache_put_io_down_queue(cache, message_packet);
			break;

		case cgm_cache_block_noncoherent:

			//stats
			cache->hits++;

			//set retry state and delay
			if(message_packet->access_type == cgm_access_store_retry || message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				gpu_cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			message_packet->access_type = cgm_access_puts;
			message_packet->size = 64;
			message_packet->cache_block_state = cgm_cache_block_noncoherent;

			cache_put_io_up_queue(cache, message_packet);
			break;
	}
	return;

}

void cgm_nc_gpu_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//check the packet for integrity
	assert(cache->cache_type == gpu_l2_cache_t);
	assert(message_packet->cache_block_state == cgm_cache_block_noncoherent || message_packet->cache_block_state == cgm_cache_block_modified);
	//make sure victim way was correctly stored.
	assert(message_packet->l2_victim_way >= 0 && message_packet->l2_victim_way < cache->assoc);

	//star todo fix this
	message_packet->cache_block_state = cgm_cache_block_noncoherent;

	//find the access in the ORT table and clear it.
	ort_clear(cache, message_packet);

	enum cgm_cache_block_state_t victim_trainsient_state;
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l2_victim_way);
	assert(victim_trainsient_state == cgm_cache_block_transient);

	//set the block and retry the access in the cache.
	if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
	{
		printf("block 0x%08x %s write block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, message_packet->cache_block_state, P_TIME);
	}

	cgm_cache_set_block(cache, message_packet->set, message_packet->l2_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_gpu_cache_get_retry_state(message_packet->gpu_access_type);

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	return;
}
