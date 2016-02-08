/*
 * gpu_mesi_protocol.c
 *
 *  Created on: Jan 29, 2016
 *      Author: stardica
 */

//////////////////////
/////GPU MESI protocol
//////////////////////

#include <cgm/protocol.h>

void cgm_mesi_gpu_s_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*GPU S$ contains read only data that is established prior to kernel execution (during OS/driver configuration)
	it should be sufficient to charge a small latency and continue on for simulator purposes.*/

	P_PAUSE(cache->latency);
	message_packet = list_remove(cache->last_queue, message_packet);
	(*message_packet->witness_ptr)++;
	packet_destroy(message_packet);
	return;

}

void cgm_mesi_gpu_v_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("%s v load\n", cache->name);
	STOP;*/

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	int upgrade_pending = 0;

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data if in WB the block is either in the E or M state so return
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if(write_back_packet)
	{
		/*found the packet in the write back buffer
		data should not be in the rest of the cache*/

		assert((write_back_packet->cache_block_state == cgm_cache_block_modified
				|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == cgm_cache_block_invalid);


		if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
		{
			printf("block 0x%08x %s load wb hit id %llu state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);
		}


		message_packet->end_cycle = P_TIME;
		cache_gpu_v_return(cache, message_packet);
		return;
	}

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_owned:
		case cgm_cache_block_noncoherent:
			fatal("cgm_mesi_gpu_v_load(): Invalid block state on hit as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		//miss or invalid cache block states
		case cgm_cache_block_invalid:

			//charge delay
			P_PAUSE(cache->latency);

			//stats
			//cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
			{
				if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
				{
					printf("block 0x%08x %s load miss coalesce ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}

				return;
			}

			//add some routing/status data to the packet
			message_packet->gpu_access_type = cgm_access_load_v;

			//this is mesi hsa stuff baby. GETX!
			message_packet->l1_cache_id = cache->id;
			message_packet->l1_access_type = cgm_access_getx;
			message_packet->access_type = cgm_access_getx;


			//find victim and evict
			message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			//evict the block
			cgm_L1_cache_evict_block(cache, message_packet->set, message_packet->l1_victim_way);
			/*cgm_cache_set_block_state(cache, message_packet->set, message_packet->l1_victim_way, cgm_cache_block_invalid);*/

			//transmit to L2
			cache_put_io_down_queue(cache, message_packet);

			if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
			{
				printf("block 0x%08x %s load miss id %llu state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);
			}

			break;

			case cgm_cache_block_exclusive:
			case cgm_cache_block_shared:
				fatal("cgm_mesi_gpu_v_load(): %s %s state not handled\n", cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
				break;

		case cgm_cache_block_modified:

			//stats
			//cache->hits++;

			//check for pending upgrade before finishing
			upgrade_pending = ort_search(cache, message_packet->tag, message_packet->set);


			/*star todo start separating out these kinds of things,
			this should be done in parallel with the cache access.*/
			if(upgrade_pending < cache->mshr_size)
			{
				/*there is a pending upgrade this means we have a valid block
				in the shared state, but an earlier store is waiting on an upgrade to modified.
				We must coalesce this access and wait for the earlier store to finish.*/
				assert(*cache_block_state_ptr == cgm_cache_block_shared);

				//charge delay
				P_PAUSE(cache->latency);

				if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
				{
					printf("block 0x%08x %s load hit coalesce ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}

				//stats
				//cache->transient_misses++;

				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
					return;

				//should always coalesce because we are waiting on an upgrade miss.
				fatal("cgm_mesi_load(): transient state with no load coalesce\n");
			}

			if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
			{
				printf("block 0x%08x %s load hit ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
			}

			//there are no pending accesses, we can continue and finish the load.

			//set the retry state and charge latency
			if(message_packet->access_type == cgm_access_load_retry || message_packet->access_type == cgm_access_loadx_retry || message_packet->coalesced == 1)
			{
				//charge a delay only on retry state.
				P_PAUSE(cache->latency);

				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			message_packet->end_cycle = P_TIME;
			cache_gpu_v_return(cache, message_packet);

			break;

	}
	return;
}

/*what did the pirate say on his eightieth birthday????
 *
 *AYE MATEY!
 *
 *lol...
 */

void cgm_mesi_gpu_v_store(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("%s v store\n", cache->name);
	STOP;*/

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	//lock in write back for the cache line.
	if(write_back_packet)
	{
		/*found the packet in the write back buffer
		data should not be in the rest of the cache*/

		assert((write_back_packet->cache_block_state == cgm_cache_block_modified
				|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

		assert(message_packet->access_type != cgm_access_store_retry || message_packet->coalesced != 1);

		if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
		{
			printf("block 0x%08x %s store wb hit id %llu state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);
		}

		write_back_packet->cache_block_state = cgm_cache_block_modified;

		message_packet->end_cycle = P_TIME;
		cache_gpu_v_return(cache, message_packet);
		return;
	}

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_owned:
		case cgm_cache_block_noncoherent:
			fatal("cgm_mesi_gpu_v_store(): Invalid block state on hit as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		//miss or invalid cache block states
		case cgm_cache_block_invalid:

			//charge delay
			P_PAUSE(cache->latency);

			//stats
			//cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
			{
				if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
				{
					printf("block 0x%08x %s store miss coalesce ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}

				return;
			}

			//add some routing/status data to the packet
			message_packet->l1_cache_id = cache->id;
			message_packet->l1_access_type = cgm_access_getx;
			message_packet->access_type = cgm_access_getx;

			//find victim and evict
			message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			//evict the block is the data is valid
			cgm_L1_cache_evict_block(cache, message_packet->set, message_packet->l1_victim_way);
			/*cgm_cache_set_block_state(cache, message_packet->set, message_packet->l1_victim_way, cgm_cache_block_invalid);*/

			//transmit to L2
			cache_put_io_down_queue(cache, message_packet);

			if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
			{
				printf("block 0x%08x %s store miss ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
			}
			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_shared:
		case cgm_cache_block_modified:
			fatal("cgm_mesi_gpu_v_store(): %s %s state not handled\n", cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

	}
	return;
}



void cgm_mesi_gpu_v_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//check the packet for integrity
	assert(cache->cache_type == gpu_v_cache_t);
	//assert(message_packet->access_type == cgm_access_puts && message_packet->cache_block_state == cgm_cache_block_shared);
	//make sure victim way was correctly stored.
	assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

	//find the access in the ORT table and clear it.
	ort_clear(cache, message_packet);

	if(message_packet->access_id == 1627758)
	{
		printf("%s access_id %llu write block\n", cache->name, message_packet->access_id);
	}

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

void cgm_mesi_gpu_v_inval(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//Invalidation/eviction request from L2 cache

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *wb_packet;

	enum cgm_cache_block_state_t victim_trainsient_state;

	//charge the delay
	P_PAUSE(cache->latency);

	fatal("cgm_mesi_gpu_v_inval()\n");

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
	{
		printf("block 0x%08x %s invalidated ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
	}

	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);


	if(victim_trainsient_state == cgm_cache_block_transient)
	{
		//block is transient and has already been evicted.
		message_packet->size = 1;
		message_packet->cache_block_state = cgm_cache_block_invalid;

		//set access type inval_ack
		message_packet->access_type = cgm_access_inv_ack;

		//reply to the L2 cache
		cache_put_io_down_queue(cache, message_packet);

	}
	else
	{
		//first check the cache for the block
		switch(*cache_block_state_ptr)
		{
			case cgm_cache_block_owned:
			case cgm_cache_block_noncoherent:
			fatal("l1_d_cache_ctrl(): Invalid block state on flush hit %s \n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
				break;

			//if invalid check the WB buffer
			case cgm_cache_block_invalid:

					wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

					//found the block in the WB buffer
					if(wb_packet)
					{

						assert(wb_packet->cache_block_state != cgm_cache_block_invalid
								|| wb_packet->cache_block_state != cgm_cache_block_shared);

						//if modified send inval_ack with data
						if(wb_packet->cache_block_state == cgm_cache_block_modified)
						{
							message_packet->size = cache->block_size;
							message_packet->cache_block_state = cgm_cache_block_modified;
						}
						else
						{
							message_packet->size = 1;
							message_packet->cache_block_state = cgm_cache_block_invalid;
						}

						//remove the block from the WB buffer
						wb_packet = list_remove(cache->write_back_buffer, wb_packet);
						free(wb_packet);
					}
					//block isn't in the cache or WB send inval_acl without data (empty inval_ack)
					else
					{
						message_packet->size = 1;
						message_packet->cache_block_state = cgm_cache_block_invalid;
					}

				break;
			case cgm_cache_block_exclusive:
			case cgm_cache_block_shared:
				//hit and its NOT dirty send the ack down to the L2 cache.
				message_packet->size = 1;
				message_packet->cache_block_state = cgm_cache_block_invalid;

				//invalidate the local block
				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);
				break;

			case cgm_cache_block_modified:
				//hit and its dirty send the ack and block down (sharing writeback) to the L2 cache.
				message_packet->size = cache->block_size;
				message_packet->cache_block_state = cgm_cache_block_modified;

				//invalidate the local block
				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

				break;
		}

		//set access type inval_ack
		message_packet->access_type = cgm_access_inv_ack;

		//reply to the L2 cache
		cache_put_io_down_queue(cache, message_packet);
	}

	return;
}

void cgm_mesi_gpu_l2_getx(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int l3_map;

	struct cgm_packet_t *write_back_packet = NULL;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if(write_back_packet)
	{
		/*printf("l2 getx WB id %llu type %d cycle %llu\n", message_packet->access_id, message_packet->access_type, P_TIME);*/

		/*found the packet in the write back buffer
		data should not be in the rest of the cache*/
		assert((write_back_packet->cache_block_state == cgm_cache_block_modified
				|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

		assert(message_packet->access_type != cgm_access_store_retry || message_packet->coalesced != 1);

		if(write_back_packet->cache_block_state == cgm_cache_block_exclusive)
		{
			//drop the WB and send request to L3
			write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
			packet_destroy(write_back_packet);
		}
		else
		{
			//block is modified flush to L3 and send request
			/*printf("l2 getx WB id %llu type %d cycle %llu\n", message_packet->access_id, message_packet->access_type, P_TIME);*/
			fatal("cgm_mesi_gpu_l2_getx(): writeback here\n");

			//add routing/status data to the packet
			write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);

			write_back_packet->access_id = message_packet->access_id;

			l3_map = cgm_l3_cache_map(write_back_packet->set);
			write_back_packet->l2_cache_id = cache->id;
			write_back_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

			write_back_packet->src_name = cache->name;
			write_back_packet->src_id = str_map_string(&node_strn_map, cache->name);
			write_back_packet->dest_name = l3_caches[l3_map].name;
			write_back_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

			//transmit to SA/MC
			list_enqueue(cache->Tx_queue_bottom, write_back_packet);
			advance(cache->cache_io_down_ec);
		}
	}

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_owned:
		case cgm_cache_block_noncoherent:
			fatal("cgm_mesi_gpu_l2_getx): Invalid block state on hit as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		//miss or invalid cache block states
		case cgm_cache_block_invalid:

			//stats
		//	cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
			{
				if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
				{
					printf("block 0x%08x %s store miss coalesce ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}

				return;
			}

			//find victim and evict on return
			message_packet->l2_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l2_victim_way >= 0 && message_packet->l2_victim_way < cache->assoc);

			//evict the victim
			cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->l2_victim_way, message_packet->l1_cache_id);

			//set access type
			message_packet->access_type = cgm_access_getx;

			message_packet->l2_cache_id = cache->id;
			message_packet->l2_cache_name = cache->name;

			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&gpu_l2_strn_map, cache->name);

			message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
			message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

			//transmit to system agent
			cache_put_io_down_queue(cache, message_packet);

			if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
			{
				printf("block 0x%08x %s store miss ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
			}
			break;

		case cgm_cache_block_shared:
		case cgm_cache_block_exclusive:
			fatal("cgm_mesi_gpu_l2_getx(): %s %s state not handled\n", cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		case cgm_cache_block_modified:

			//stats;
			//cache->hits++;

			//set retry state
			if(message_packet->access_type == cgm_access_storex_retry || message_packet->access_type == cgm_access_loadx_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			if(*cache_block_state_ptr == cgm_cache_block_exclusive)
			{
				/*if the block is in the E state set M because the message is a store
				a flush will bring the modified line down later
				the block remains in the E state at L3*/
				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);
			}

			//set message status and size
			message_packet->size = gpu_v_caches[cache->id].block_size; //this should be L1 D cache block size.
			message_packet->access_type = cgm_access_putx;
			message_packet->cache_block_state = cgm_cache_block_modified;

			printf("L2 cache hit gpu type %s \n", str_map_value(&cgm_mem_access_strn_map, message_packet->gpu_access_type));

			//send up to gpu v cache
			cache_put_io_up_queue(cache, message_packet);

			if(((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
			{
				printf("block 0x%08x %s store hit ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
			}
			break;

	}
	return;

}

void cgm_mesi_gpu_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//check the packet for integrity
	assert(cache->cache_type == gpu_l2_cache_t);
	assert(message_packet->cache_block_state == cgm_cache_block_modified);
	//make sure victim way was correctly stored.
	assert(message_packet->l2_victim_way >= 0 && message_packet->l2_victim_way < cache->assoc);

	if(message_packet->access_id == 1627758)
	{
		printf("%s access_id %llu write block\n", cache->name, message_packet->access_id);
	}

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
