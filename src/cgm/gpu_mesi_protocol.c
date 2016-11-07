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


int gpu_core_id = 0;

void cgm_mesi_gpu_s_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*GPU S$ contains read only data that is established prior to kernel execution (during OS/driver configuration)
	it should be sufficient to charge a small latency and continue on for simulator purposes.*/

	GPU_PAUSE(cache->latency);
	message_packet = list_remove(cache->last_queue, message_packet);
	(*message_packet->witness_ptr)++;
	packet_destroy(message_packet);
	return;

}

void cgm_mesi_gpu_l1_v_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("access_id %llu\n",message_packet->access_id);*/

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

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	//charge delay
	GPU_PAUSE(cache->latency);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_owned:
		case cgm_cache_block_noncoherent:
			fatal("l1_d_cache_ctrl(): Invalid block state on load hit as \"%s\"\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		//miss or invalid cache block states
		case cgm_cache_block_invalid:

			//block is in write back !!!!
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/
				assert(*cache_block_hit_ptr == 0);
				assert(write_back_packet->cache_block_state == cgm_cache_block_modified);
				assert(message_packet->set == write_back_packet->set && message_packet->tag == write_back_packet->tag);
				assert(message_packet->access_type == cgm_access_load || message_packet->access_type == cgm_access_load_retry);

				//see if we can write it back into the cache.
				write_back_packet->l1_victim_way = cgm_cache_get_victim_for_wb(cache, write_back_packet->set);

				//if not then we must coalesce
				if(write_back_packet->l1_victim_way == -1)
				{
					//Set and ways are all transient must coalesce
					cache_check_ORT(cache, message_packet);

					/*stats*/
					cache->CoalescePut++;

					if(message_packet->coalesced == 1)
						return;
					else
						fatal("cgm_mesi_load(): write failed to coalesce when all ways are transient...\n");
				}

				//we are writing in a block so evict the victim
				assert(write_back_packet->l1_victim_way >= 0 && write_back_packet->l1_victim_way < cache->assoc);

				//first evict the old block if it isn't invalid already
				if(cgm_cache_get_block_state(cache, write_back_packet->set, write_back_packet->l1_victim_way) != cgm_cache_block_invalid)
					cgm_L1_cache_evict_block(cache, write_back_packet->set, write_back_packet->l1_victim_way);

				//now set the block
				cgm_cache_set_block(cache, write_back_packet->set, write_back_packet->l1_victim_way, write_back_packet->tag, write_back_packet->cache_block_state);

				//free the write back
				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s load wb hit id %llu state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);

				//check for retries on successful cache read...
				if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
				{
					cache->MergeRetries++;

					//enter retry state.
					cache_coalesed_retry(cache, message_packet->tag, message_packet->set, message_packet->access_id);
				}

				/*stats*/
				cache->WbMerges++;
				message_packet->end_cycle = P_TIME;
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L1_hit;

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				cache_gpu_v_return(cache, message_packet);
				return;
			}
			else
			{
				//block isn't in the cache or in write back.

				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
				{
					/*stats*/
					cache->CoalescePut++;

					DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s load miss coalesce ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);

					return;
				}

				//add some routing/status data to the packet
				message_packet->access_type = cgm_access_get;
				message_packet->l1_access_type = cgm_access_get;
				message_packet->l1_cache_id = cache->id;

				//find victim
				message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set, message_packet->tag);

				/*	message_packet->l1_victim_way = cgm_cache_replace_block(cache, message_packet->set);*/
				assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

				if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l1_victim_way) != cgm_cache_block_invalid)
					cgm_L1_cache_evict_block(cache, message_packet->set, message_packet->l1_victim_way);

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				//transmit to L2
				cache_put_io_down_queue(cache, message_packet);

				DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s load miss id %llu state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);

			}

			break;

		//hit states
		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_shared:


			//check for pending upgrade before finishing
			upgrade_pending = ort_search(cache, message_packet->tag, message_packet->set);

			/*star todo start separating out these kinds of things,
			this should be done in parallel with the cache access.*/
			if(upgrade_pending < cache->mshr_size)
			{
				/*there is a pending upgrade this means we have a valid block
				in the shared state, but an earlier store is waiting on an upgrade to modified.
				We must coalesce this access and wait for the earlier store to finish.*/

				if(*cache_block_state_ptr != cgm_cache_block_shared)
				{
					fatal("block 0x%08x %s load hit coalesce ID %llu type %d state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}

				assert(*cache_block_state_ptr == cgm_cache_block_shared);

				DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s load hit coalesce ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);

				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				/*stats*/
				cache->CoalescePut++;

				if(message_packet->coalesced == 1)
					return;

				//should always coalesce because we are waiting on an upgrade miss.
				fatal("cgm_mesi_load(): transient state with no load coalesce\n");
			}

			DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s load hit ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
					message_packet->access_type, *cache_block_state_ptr, P_TIME);

			//there are no pending accesses, we can continue and finish the load.

			//set the retry state and charge latency
			if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//enter retry state.

				cache_coalesed_retry(cache, message_packet->tag, message_packet->set, message_packet->access_id);
			}

			/*stats*/
			message_packet->end_cycle = P_TIME;
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L1_hit;

			/*reset mp flags*/
			message_packet->coalesced = 0;
			message_packet->assoc_conflict = 0;

			cache_gpu_v_return(cache,message_packet);

			break;
	}

	return;
}

void cgm_mesi_gpu_l1_v_load_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int ort_row = 0;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;

	//charge delay
	GPU_PAUSE(cache->latency);

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s load_nack ID %llu type %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, P_TIME);

	//store the decoded address
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);

	//do not set retry because this contains the coalesce set and tag.
	//check that there is an ORT entry for this address
	ort_row = ort_search(cache, message_packet->tag, message_packet->set);
	assert(ort_row < cache->mshr_size);

	//add some routing/status data to the packet
	message_packet->access_type = cgm_access_get;

	cache_put_io_down_queue(cache, message_packet);

	/*stats*/
	mem_system_stats->l1_load_nack++;

	return;
}

void cgm_mesi_gpu_l1_v_store_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int ort_row = 0;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;

	//charge delay
	GPU_PAUSE(cache->latency);

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s store_nack ID %llu type %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, P_TIME);

	//store the decoded address
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);

	//do not set retry because this contains the coalesce set and tag.
	//check that there is an ORT entry for this address
	ort_row = ort_search(cache, message_packet->tag, message_packet->set);
	assert(ort_row < cache->mshr_size);

	//add some routing/status data to the packet
	message_packet->access_type = cgm_access_getx;

	cache_put_io_down_queue(cache, message_packet);

	/*stats*/
	mem_system_stats->l1_store_nack++;

	return;
}


void cgm_mesi_gpu_l1_v_store(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("gpu v %d storing\n", cache->id);*/
	/*STOP;*/

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	//check this...
	if(write_back_packet)
	{
		assert(*cache_block_hit_ptr == 0);
		assert(write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive);

	}

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	//charge delay
	GPU_PAUSE(cache->latency);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_owned:
		case cgm_cache_block_noncoherent:
			fatal("l1_d_cache_ctrl(): Invalid block state on store hit %s \n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		//miss or invalid cache block state
		case cgm_cache_block_invalid:

			//lock in write back for the cache line.
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/
				assert(*cache_block_hit_ptr == 0);
				assert(write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive);
				assert(message_packet->set == write_back_packet->set && message_packet->tag == write_back_packet->tag);
				assert(message_packet->access_type == cgm_access_store || message_packet->access_type == cgm_access_store_retry);

				//see if we can write it back into the cache.
				write_back_packet->l1_victim_way = cgm_cache_get_victim_for_wb(cache, write_back_packet->set);

				//if not then we must coalesce
				if(write_back_packet->l1_victim_way == -1)
				{
					//Set and ways are all transient must coalesce
					cache_check_ORT(cache, message_packet);

					/*stats*/
					cache->CoalescePut++;

					if(message_packet->coalesced == 1)
						return;
					else
						fatal("cgm_mesi_load(): write failed to coalesce when all ways are transient...\n");
				}

				//we are writing in a block so evict the victim
				assert(write_back_packet->l1_victim_way >= 0 && write_back_packet->l1_victim_way < cache->assoc);

				//first evict the old block if it isn't invalid already
				if(cgm_cache_get_block_state(cache, write_back_packet->set, write_back_packet->l1_victim_way) != cgm_cache_block_invalid)
					cgm_L1_cache_evict_block(cache, write_back_packet->set, write_back_packet->l1_victim_way);

				cgm_cache_set_block(cache, write_back_packet->set, write_back_packet->l1_victim_way, write_back_packet->tag, cgm_cache_block_modified);

				//free the write back
				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s store wb hit id %llu state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);

				//check for retires on successful cache write...
				if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
				{
					cache->MergeRetries++;

					//enter retry state.
					cache_coalesed_retry(cache, message_packet->tag, message_packet->set, message_packet->access_id);
				}

				/*stats*/
				cache->WbMerges++;
				message_packet->end_cycle = P_TIME;
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L1_hit;

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				cache_gpu_v_return(cache,message_packet);
				return;
			}
			else
			{
				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
				{
					/*stats*/
					cache->CoalescePut++;

					DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s store miss coalesce ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type,
							*cache_block_state_ptr, P_TIME);

					return;
				}

				//add some routing/status data to the packet
				message_packet->access_type = cgm_access_getx;
				message_packet->l1_access_type = cgm_access_getx;
				message_packet->l1_cache_id = cache->id;

				//find victim
				message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set, message_packet->tag);

				/*message_packet->l1_victim_way = cgm_cache_replace_block(cache, message_packet->set);*/
				assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

				//evict the block if the data is valid
				if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l1_victim_way) != cgm_cache_block_invalid)
					cgm_L1_cache_evict_block(cache, message_packet->set, message_packet->l1_victim_way);

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				//transmit to L2
				cache_put_io_down_queue(cache, message_packet);

				DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s store miss ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
						message_packet->access_type, *cache_block_state_ptr, P_TIME);

			}

			break;

		case cgm_cache_block_shared:

			//this is an upgrade_miss

			fatal("cgm_mesi_gpu_l1_v_store(): shared state not possible in GPU v caches...\n");

			/*star todo find a better way to do this.
			this is for a special case where a coalesced store
			can be pulled from the ORT and is an upgrade miss here
			at this point we want the access to be treated as a new miss
			so set coalesced to 0. Older packets in the ORT will stay in the ORT
			preserving order until the missing access returns with the upgrade.*/
			if(message_packet->coalesced == 1)
			{
				message_packet->coalesced = 0;
			}

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
			{
				/*stats*/
				cache->CoalescePut++;

				DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s upgrade miss coalesce ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type,
						*cache_block_state_ptr, P_TIME);

				return;
			}

			//set block transient state, but don't evict because the block is valid and just needs to be upgraded
			cgm_cache_set_block_transient_state(cache, message_packet->set, message_packet->way, cgm_cache_block_transient);

			//keep the way of the block to upgrade (might come back as a putx in lieu of an upgrade ack)
			message_packet->l1_victim_way = message_packet->way;

			//set access type
			message_packet->access_type = cgm_access_upgrade;
			message_packet->l1_access_type = cgm_access_upgrade;

			/*stats*/
			cache->UpgradeMisses++;
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L1_upgrade;

			/*reset mp flags*/
			message_packet->coalesced = 0;
			message_packet->assoc_conflict = 0;

			//transmit upgrade request to L2
			cache_put_io_down_queue(cache, message_packet);

			DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s upgrade miss ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
					message_packet->access_type, *cache_block_state_ptr, P_TIME);

			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			//set modified if current block state is exclusive
			if(*cache_block_state_ptr == cgm_cache_block_exclusive)
			{
				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);
			}

			//check for retry state
			if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set, message_packet->access_id);
			}

			DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s store hit ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
					message_packet->access_type, *cache_block_state_ptr, P_TIME);

			/*stats*/
			message_packet->end_cycle = P_TIME;
			if(!message_packet->protocol_case)
					message_packet->protocol_case = L1_hit;

			/*reset mp flags*/
			message_packet->coalesced = 0;
			message_packet->assoc_conflict = 0;

			cache_gpu_v_return(cache,message_packet);

			break;
	}

	return;
}



void cgm_mesi_gpu_l1_v_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet){


	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;
	//struct cgm_packet_t *wb_packet;
	//enum cgm_cache_block_state_t block_trainsient_state;
	//int l3_map;
	//int error = 0;

	//charge the delay
	GPU_PAUSE(cache->latency);

	//we should only receive modified lines from L1 D cache
	assert(message_packet->cache_block_state == cgm_cache_block_modified);

	//get the state of the local cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//check for block transient state
	/*block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	if(block_trainsient_state == cgm_cache_block_transient)
	{
		if potentially merging in cache the block better not be transient, check that the tags don't match
		if they don't match the block is missing from both the cache and wb buffer when it should not be

		//check that the tags don't match. This should not happen as the request should have been coalesced at L1 D.
		assert(message_packet->tag != cache->sets[message_packet->set].blocks[message_packet->way].tag);
	}*/

	DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s wb sent id %llu state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id,
			message_packet->cache_block_state, P_TIME);

	/*better not be in my cache*/
	assert(*cache_block_hit_ptr == 0);
	assert(message_packet->flush_pending == 0 && message_packet->L3_flush_join == 0);

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);

	/*stats*/
	cache->TotalWriteBackSent++;

	message_packet->size = cache->block_size;

	cache_put_io_down_queue(cache, message_packet);

	return;
}

void cgm_mesi_gpu_l1_v_get_getx_fwd_inval(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*warning("l1 get getx inval\n");*/


	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	int ort_status = -1;

	//charge delay
	GPU_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s getx fwd inval ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
			message_packet->access_type, *cache_block_state_ptr, P_TIME);

	//check the ORT table is there an outstanding access for this block we are trying to evict?
	ort_status = ort_search(cache, message_packet->tag, message_packet->set);
	if(ort_status != cache->mshr_size)
	{
		fatal("cgm_mesi_gpu_l1_v_get_getx_fwd_inval(): shoudn't be pending\n");

		/*fatal("here\n");*/
		/*yep there is so set the bit in the ort table to 0.
		 * When the put/putx comes kill it and try again...*/
		ort_set_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);

		//warning("l1 conflict found ort set cycle %llu\n", P_TIME);
	}

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);


	/*if the block is in the cache it is not in the WB buffer
	if the block is dirty send down to L2 cache for merge*/
	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_owned:
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_shared:
		fatal("cgm_mesi_l1_d_getx_fwd_inval(): block 0x%08x Invalid block state on flush hit %s ID %llu cycle %llu\n",
				(message_packet->address & cache->block_address_mask), str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
				message_packet->access_id, P_TIME);

			break;

		case cgm_cache_block_invalid:

			//check write back buffer
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/
				assert((write_back_packet->cache_block_state == cgm_cache_block_modified
						|| write_back_packet->cache_block_state == cgm_cache_block_exclusive)
						&& *cache_block_state_ptr == cgm_cache_block_invalid);

				if(write_back_packet->cache_block_state == cgm_cache_block_modified)
				{
					message_packet->size = cache->block_size;
					message_packet->cache_block_state = cgm_cache_block_modified;
				}

				//clear the wb buffer
				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				message_packet->size = 1;
				message_packet->cache_block_state = cgm_cache_block_invalid;

				//set access type
				message_packet->l1_cache_id = cache->id;
				message_packet->access_type = cgm_access_getx_fwd_inval_ack;

				//reply to the L2 cache
				cache_put_io_down_queue(cache, message_packet);

			}
			else
			{
				//if invalid it was silently dropped
				message_packet->size = 1;
				message_packet->cache_block_state = cgm_cache_block_invalid;

				//set access type
				message_packet->l1_cache_id = cache->id;
				message_packet->access_type = cgm_access_getx_fwd_inval_ack;

				//reply to the L2 cache
				cache_put_io_down_queue(cache, message_packet);
			}
			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			assert(*cache_block_hit_ptr == 1);

			if(*cache_block_state_ptr == cgm_cache_block_exclusive)
			{
				//if E it is not dirty
				message_packet->size = 1;
				message_packet->cache_block_state = cgm_cache_block_invalid;
			}
			else if(*cache_block_state_ptr == cgm_cache_block_modified)
			{
				//hit and its dirty send the ack and block down (sharing write back) to the L2 cache.
				message_packet->size = cache->block_size;
				message_packet->cache_block_state = cgm_cache_block_modified;
			}

			//invalidate the local block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			//set access type
			message_packet->l1_cache_id = cache->id;
			message_packet->access_type = cgm_access_getx_fwd_inval_ack;

			//reply to the L2 cache
			cache_put_io_down_queue(cache, message_packet);

			break;
	}

	/*stats*/
	cache->TotalGetxFwdInvals++;

	return;
}

void cgm_mesi_gpu_l1_v_gpu_flush(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//CPU is flushing a block, flush and send down to L2 and L3/MC
	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *wb_packet;
	int ort_status = -1;

	//enum cgm_cache_block_state_t victim_trainsient_state;

	//charge the delay
	GPU_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s GPU flush block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id,
			message_packet->access_type, *cache_block_state_ptr, P_TIME);

	//check the ORT table is there an outstanding access for this block we are trying to flush?
	ort_status = ort_search(cache, message_packet->tag, message_packet->set);
	if(ort_status != cache->mshr_size)
	{
		fatal("cgm_mesi_gpu_l1_v_gpu_flush(): ort conflict set\n");
		/*CPU is flushing blocks, but we are still waiting on the memory system to bring the block in
		the CPU is now trying to flush, stall until the block is brought and the progress is made*/
		//fatal("not sure about this\n");
	}

	//remove the block from the cache....
	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_owned:
		case cgm_cache_block_noncoherent:
		fatal("cgm_mesi_l1_d_flush_block(): Invalid block state on flush hit %s \n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		//if invalid check the WB buffer
		case cgm_cache_block_invalid:

				wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

				//found the block in the WB buffer
				if(wb_packet)
				{
					assert(wb_packet->cache_block_state == cgm_cache_block_modified);

					message_packet->size = cache->block_size;
					message_packet->cache_block_state = cgm_cache_block_modified;

					//remove the block from the WB buffer
					wb_packet = list_remove(cache->write_back_buffer, wb_packet);
					packet_destroy(wb_packet);

					//set access type inval_ack
					message_packet->access_type = cgm_access_gpu_flush_ack;

					cache_put_io_down_queue(cache, message_packet);
				}
				else
				{

					/*Dropped if exclusive or shared
					 Also WB may be in the pipe between L1 and L2 if Modified This will flush it out*/
					message_packet->size = 1;
					message_packet->cache_block_state = cgm_cache_block_invalid;

					//set access type inval_ack
					message_packet->access_type = cgm_access_gpu_flush_ack;

					//reply to the L2 cache
					cache_put_io_down_queue(cache, message_packet);
				}

			break;

		case cgm_cache_block_exclusive:

			message_packet->size = 1;
			message_packet->cache_block_state = cgm_cache_block_invalid;

			//invalidate the local block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			//set access type inval_ack
			message_packet->access_type = cgm_access_gpu_flush_ack;

			//reply to the L2 cache
			cache_put_io_down_queue(cache, message_packet);

			break;

		case cgm_cache_block_modified:

			//hit and its dirty send the ack and block down to the L2 cache.
			message_packet->size = cache->block_size;
			message_packet->cache_block_state = cgm_cache_block_modified;

			//invalidate the local block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			//set access type inval_ack
			message_packet->access_type = cgm_access_gpu_flush_ack;

			//reply to the L2 cache
			cache_put_io_down_queue(cache, message_packet);

			break;

		case cgm_cache_block_shared:

			fatal("cgm_mesi_gpu_l1_v_gpu_flush(): shared in l1\n");

			break;

	}

	return;
}


void cgm_mesi_gpu_l1_v_flush_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//Invalidation/eviction request from L2 cache

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *wb_packet;
	int ort_status = -1;

	//enum cgm_cache_block_state_t victim_trainsient_state;

	//charge the delay
	GPU_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s GPU flush block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id,
			message_packet->access_type, *cache_block_state_ptr, P_TIME);

	//check the ORT table is there an outstanding access for this block we are trying to evict?
	ort_status = ort_search(cache, message_packet->tag, message_packet->set);
	if(ort_status != cache->mshr_size)
	{
		/*yep there is so set the bit in the ort table to 0.
		 * When the put/putx comes kill it and try again...*/
		ort_set_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);

		//warning("l1 conflict found ort set cycle %llu\n", P_TIME);
	}

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);

	//victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);

	//first check the cache for the block
	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_owned:
		case cgm_cache_block_noncoherent:
		fatal("cgm_mesi_l1_d_flush_block(): Invalid block state on flush hit %s \n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		//if invalid check the WB buffer
		case cgm_cache_block_invalid:

				wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

				//found the block in the WB buffer
				if(wb_packet)
				{
					assert(wb_packet->cache_block_state == cgm_cache_block_modified);

					message_packet->size = cache->block_size;
					message_packet->cache_block_state = cgm_cache_block_modified;

					//remove the block from the WB buffer
					wb_packet = list_remove(cache->write_back_buffer, wb_packet);
					packet_destroy(wb_packet);

					//set access type inval_ack
					message_packet->access_type = cgm_access_flush_block_ack;

					//reply to the L2 cache
					cache_put_io_down_queue(cache, message_packet);
				}
				else
				{

					/*Dropped if exclusive or shared
					 Also WB may be in the pipe between L1 and L2 if Modified This will flush it out*/
					message_packet->size = 1;
					message_packet->cache_block_state = cgm_cache_block_invalid;

					//set access type inval_ack
					message_packet->access_type = cgm_access_flush_block_ack;

					//reply to the L2 cache
					cache_put_io_down_queue(cache, message_packet);
				}

			break;

		case cgm_cache_block_exclusive:

			message_packet->size = 1;
			message_packet->cache_block_state = cgm_cache_block_invalid;

			//invalidate the local block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			//set access type inval_ack
			message_packet->access_type = cgm_access_flush_block_ack;

			//reply to the L2 cache
			cache_put_io_down_queue(cache, message_packet);
			break;

		case cgm_cache_block_modified:
			//hit and its dirty send the ack and block down (sharing writeback) to the L2 cache.
			message_packet->size = cache->block_size;
			message_packet->cache_block_state = cgm_cache_block_modified;

			//invalidate the local block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			//set access type inval_ack
			message_packet->access_type = cgm_access_flush_block_ack;

			//reply to the L2 cache
			cache_put_io_down_queue(cache, message_packet);

			break;


		case cgm_cache_block_shared:

			fatal("cgm_mesi_gpu_l1_v_flush_block(): block shared in GPU v caches\n");

			//invalidate the local block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			break;

	}

	/*if(message_packet->evict_id == 37134)
		printf("L1 flushed\n");*/

	/*stats*/
	cache->EvictInv++;

	return;
}


int cgm_mesi_gpu_l1_v_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	assert(cache->cache_type == gpu_v_cache_t);
	assert((message_packet->access_type == cgm_access_puts && message_packet->cache_block_state == cgm_cache_block_shared)
			|| (message_packet->access_type == cgm_access_put_clnx && message_packet->cache_block_state == cgm_cache_block_exclusive)
			|| (message_packet->access_type == cgm_access_putx && message_packet->cache_block_state == cgm_cache_block_modified));

	/*if(message_packet->access_id == 15509891)
		fatal("%s write block after flush cycle %llu\n", cache->name, P_TIME);*/


	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;
	enum cgm_cache_block_state_t victim_trainsient_state;

	int ort_status = -1;
	int ort_join_bit = -1;

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);

	cache_get_transient_block(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	if(*cache_block_hit_ptr != 1)
	{
		cgm_cache_dump_set(cache, message_packet->set);

		fatal("block 0x%08x %s write block didn't find transient block! ID %llu type %s state %d way %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
			str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->way, message_packet->cache_block_state, P_TIME);
		fflush(stderr);
	}

	assert(message_packet->size == 64);

	ort_status = ort_search(cache, message_packet->tag, message_packet->set);
	assert(ort_status < cache->mshr_size);

	//The block should be in the transient state.
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);

	//make sure victim way was correctly stored.
	assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

	if(victim_trainsient_state != cgm_cache_block_transient)
	{
		cgm_cache_dump_set(cache, message_packet->set);

		unsigned int temp = message_packet->address;
		temp = temp & cache->block_address_mask;

		fatal("cgm_mesi_l1_d_write_block(): %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d cycle %llu\n",
			cache->name, message_packet->access_id, message_packet->address, temp, message_packet->set, message_packet->tag, message_packet->l1_victim_way, P_TIME);
	}

	assert(victim_trainsient_state == cgm_cache_block_transient);

	/*special case check for a join
	if so kill this write block and retry the get/getx*/
	ort_join_bit = ort_get_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);
	assert(ort_join_bit == 1 || ort_join_bit == 0);

	if(ort_join_bit == 0)
	{

		DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s write block conflict found retrying access ID %llu type %d state %d cycle %llu\n",
				(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type,
				message_packet->cache_block_state, P_TIME);

		/*we know that the victim is still transient*/

		/*clear the pending bit in the ort and retry the access*/
		ort_clear_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);

		/*change the access type*/
		if(message_packet->l1_access_type == cgm_access_get)
		{
			message_packet->access_type = cgm_access_get;
		}
		else
		{
			//warning("l1_d_write_block %d\n", message_packet->l1_access_type);
			//assert(message_packet->l1_access_type == cgm_access_getx || message_packet->l1_access_type == cgm_access_upgrade_ack);
			message_packet->access_type = cgm_access_getx;
		}

		if(message_packet->coalesced == 1)
			message_packet->coalesced = 0;


		/*change the size*/
		message_packet->size = 1;

		//transmit to L2
		cache_put_io_down_queue(cache, message_packet);

		//warning("l1 write block caught conflict cycle %llu\n", P_TIME);

		return 0;
	}


	/*we are clear to write the block in*/
	ort_clear(cache, message_packet);


	DEBUG(LEVEL == 1 || LEVEL == 3, "block 0x%08x %s write block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
			message_packet->access_type, message_packet->cache_block_state, P_TIME);


	//write the block
	cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_cache_get_retry_state(message_packet->gpu_access_type);

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	/*stats*/
	cache->TotalWriteBlocks++;

	return 1;
}

void cgm_mesi_gpu_l2_get(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int ort_row = -1;

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;
	struct cgm_packet_t *pending_join = NULL;

	int sharers, owning_core, pending_bit;

	int num_cus = si_gpu_num_compute_units;

	struct cgm_packet_t *flush_packet = NULL;

	//charge delay
	GPU_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(gpu, cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l1_cache_id);
	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);
	//get block transient state
	//block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	//assert(victim_trainsient_state == cgm_cache_block_transient);

	/*assumption block is never shared in GPU v caches*/
	if(sharers > 1)
		warning("cgm_mesi_gpu_l2_get(): sharers = %d\n", sharers);

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	if(pending_bit == 1 && *cache_block_hit_ptr == 1)
	{
		/*its possible for a get to come in from an owing core to and for the block to be pending
		this occurs if the owning core silently dropped the block and a get_fwd was processed
		just before the owning core's request comes in send the block back to the owning core
		a previously sent get_fwd will be joined with this put/putx*/
		if(owning_core == 1)
		{

			assert(message_packet->coalesced == 0);

			DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s pending at L2 owning core PUT back to L2 id %llu state %d cycle %llu\n",
				(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);

			/*there should be only be the owning core in the directory and the pending bit set*/
			assert(sharers == 1 && owning_core == 1);

			/*The block MUST be in the E or M state*/
			assert(*cache_block_state_ptr == cgm_cache_block_exclusive || *cache_block_state_ptr == cgm_cache_block_modified);

			//update message status
			if(*cache_block_state_ptr == cgm_cache_block_exclusive)
			{
				message_packet->access_type = cgm_access_put_clnx;
			}
			else if(*cache_block_state_ptr == cgm_cache_block_modified)
			{
				message_packet->access_type = cgm_access_putx;
			}

			//get the cache block state
			message_packet->cache_block_state = *cache_block_state_ptr;

			//set message package size
			message_packet->size = gpu_v_caches[message_packet->l1_cache_id].block_size;

			//don't change the directory entries, the downgrade ack will come back and clean things up.

			/*reset mp flags*/
			assert(message_packet->coalesced == 0);
			assert(message_packet->assoc_conflict == 0);

			//send the cache block out
			cache_put_io_up_queue(cache, message_packet);

			/*stats*/
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L2_hit;

		}
		else
		{

			DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s pending at L2 get nacked back to L2 id %llu state %d cycle %llu\n",
				(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);

			/*Thrid party is looking for access to the block, but it is busy nack and let node retry later*/


			//check if the packet has coalesced accesses.
			if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set, message_packet->access_id);

				/*if(message_packet->access_id == 71992322)
					warning("l3 checking retries ID %llu\n", message_packet->access_id);*/
			}

			/*star todo find a better way to do this.
			this is for a special case where a coalesced access was pulled
			and is going to be nacked at this point we want the access to be
			treated as a new miss so set coalesced to 0*/
			if(message_packet->coalesced == 1)
			{
				message_packet->coalesced = 0;
			}


			//send the reply up as a NACK!
			message_packet->access_type = cgm_access_get_nack;

			//set message package size
			message_packet->size = 1;

			/*reset mp flags*/
			assert(message_packet->coalesced == 0);
			assert(message_packet->assoc_conflict == 0);

			//send the reply
			cache_put_io_up_queue(cache, message_packet);

			/*stats*/
			mem_system_stats->l2_load_nack++;
		}

		return;

	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
			fatal("cgm_mesi_l2_get(): L2 id %d Invalid block state on get as %s cycle %llu\n",
					cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), P_TIME);
			break;

		case cgm_cache_block_invalid:

			//ZMG block is in the write back !!!!
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/
				assert(*cache_block_hit_ptr == 0);
				assert(write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive);
				assert(message_packet->set == write_back_packet->set && message_packet->tag == write_back_packet->tag);
				assert(message_packet->access_type == cgm_access_get || message_packet->access_type == cgm_access_load_retry);

				//see if we can write it back into the cache.
				write_back_packet->l2_victim_way = cgm_cache_get_victim_for_wb(cache, write_back_packet->set);

				/*special case nack the request back to L1 because L2 is still waiting on a a flush block ack*/
				if(write_back_packet->flush_pending == 1)
				{
					/*flush is pending, but we have a request for the block, nack it back to L1*/
					//send the reply up as a NACK!
					message_packet->access_type = cgm_access_get_nack;

					//set message package size
					message_packet->size = 1;

					/*stats*/
					/*star todo add a stat for this*/

					DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s load nack wb pending flush ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type,
							*cache_block_state_ptr, P_TIME);

					/*reset mp flags*/
					message_packet->coalesced = 0;
					message_packet->assoc_conflict = 0;

					cache_put_io_up_queue(cache, message_packet);

					//warning("nacking load back to L1, flush still pending\n");

					return;
				}


				//if not then we must coalesce
				if(write_back_packet->l2_victim_way == -1)
				{
					//Set and ways are all transient must coalesce
					cache_check_ORT(cache, message_packet);

					/*stats*/
					cache->CoalescePut++;

					if(message_packet->coalesced == 1)
						return;
					else
						fatal("cgm_mesi_l2_get(): write failed to coalesce when all ways are transient...\n");
				}

				assert(write_back_packet->L3_flush_join == 0);
				assert(write_back_packet->flush_pending == 0);

				//we are bringing a new block so evict the victim and flush the L1 copies
				assert(write_back_packet->l2_victim_way >= 0 && write_back_packet->l2_victim_way < cache->assoc);

				//first evict the old block if it isn't invalid already
				if(cgm_cache_get_block_state(cache, write_back_packet->set, write_back_packet->l2_victim_way) != cgm_cache_block_invalid)
					cgm_L2_cache_evict_block(cache, write_back_packet->set, write_back_packet->l2_victim_way,
							cgm_cache_get_num_shares(gpu, cache, write_back_packet->set, write_back_packet->l2_victim_way), 0);

				//clear the old directory entry
				cgm_cache_clear_dir(cache,  write_back_packet->set, write_back_packet->l2_victim_way);

				//set the new directory entry
				cgm_cache_set_dir(cache, write_back_packet->set, write_back_packet->l2_victim_way, message_packet->l1_cache_id);

				//now set the block
				assert(write_back_packet->cache_block_state == cgm_cache_block_exclusive || write_back_packet->cache_block_state == cgm_cache_block_modified);
				cgm_cache_set_block(cache, write_back_packet->set, write_back_packet->l2_victim_way, write_back_packet->tag, write_back_packet->cache_block_state);

				//check for retries on successful cache read...
				if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
				{
					cache->MergeRetries++;
					//enter retry state process all coalesced accesses
					cache_coalesed_retry(cache, message_packet->tag, message_packet->set, message_packet->access_id);
				}

				//set message size
				message_packet->size = l1_d_caches[cache->id].block_size; //this can be either L1 I or L1 D cache block size.
				message_packet->cache_block_state = write_back_packet->cache_block_state;

				//update message status
				if(message_packet->cache_block_state == cgm_cache_block_modified)
				{
					message_packet->access_type = cgm_access_putx;
				}
				else if(message_packet->cache_block_state == cgm_cache_block_exclusive)
				{
					message_packet->access_type = cgm_access_put_clnx;
				}
				else
				{
					fatal("cgm_mesi_l2_get(): invalid write back block state\n");
				}

				//free the write back
				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s load wb hit id %llu state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);

				/*stats*/
				cache->WbMerges++;
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L2_hit;

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				cache_put_io_up_queue(cache, message_packet);
				return;
			}
			else
			{
				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
				{
					/*stats*/
					cache->CoalescePut++;

					DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s load miss coalesce ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
							message_packet->access_type, *cache_block_state_ptr, P_TIME);

					return;
				}

				//we are bringing a new block so evict the victim and flush the L1 copies
				message_packet->l2_victim_way = cgm_cache_get_victim(cache, message_packet->set, message_packet->tag);
				assert(message_packet->l2_victim_way >= 0 && message_packet->l2_victim_way < cache->assoc);

				if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l2_victim_way) != cgm_cache_block_invalid)
					cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->l2_victim_way,
							cgm_cache_get_num_shares(gpu, cache, message_packet->set, message_packet->l2_victim_way), 0);

				//clear the directory entry
				cgm_cache_clear_dir(cache, message_packet->set, message_packet->l2_victim_way);

				ort_row = ort_search(cache, message_packet->tag, message_packet->set);
				assert(ort_row < cache->mshr_size);

				if(cgm_cache_get_block_upgrade_pending_bit(cache, message_packet->set, message_packet->l2_victim_way) == 1
						|| ort_get_pending_join_bit(cache, ort_row, message_packet->tag, message_packet->set) == 0)
				{
					printf("\n");
					cgm_cache_dump_set(cache, message_packet->set);

					printf("\n");
					cache_dump_queue(cache->pending_request_buffer);

					printf("\n");

					ort_dump(cache);
					printf("\n");

					fatal("block 0x%08x %s load miss coalesce ID %llu type %d state %d set %d tag %d way %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
							message_packet->access_type, *cache_block_state_ptr, message_packet->set, message_packet->tag, message_packet->l2_victim_way, P_TIME);

				}


				assert(cgm_cache_get_block_upgrade_pending_bit(cache, message_packet->set, message_packet->l2_victim_way) == 0);

				//add some routing/status data to the packet
				assert(message_packet->access_type == cgm_access_get);

				message_packet->size = 1;

				if(hub_iommu_connection_type == hub_to_mc)
				{
					//message is going down to mc so its and mc_load
					message_packet->access_type = cgm_access_mc_load;
				}
				else
				{
					//message is going down to L3 so its a getx
					message_packet->access_type = cgm_access_getx;
					message_packet->cpu_access_type = cgm_access_load;
				}

				message_packet->cache_block_state = cgm_cache_block_exclusive;

				//L3 should see the entire GPU as a single core.
				message_packet->l2_cache_id = gpu_core_id;
				message_packet->l2_cache_name = str_map_value(&l2_strn_map, gpu_core_id);

				DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s load miss ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
						message_packet->access_type, *cache_block_state_ptr, P_TIME);

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				//transmit to L3
				cache_put_io_down_queue(cache, message_packet);
			}

			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_shared:


			assert(sharers >= 0 && sharers <= num_cus);
			assert(owning_core >= 0 && owning_core <= 1);
			assert(*cache_block_state_ptr != cgm_cache_block_shared);

			if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set, message_packet->access_id);

				/*check for a pending get_fwd or getx_fwd join*/
				pending_join = cache_search_pending_request_buffer(cache, (message_packet->address & cache->block_address_mask));

				if(pending_join)
				{
					pending_join->downgrade_pending--;

					if (pending_join->downgrade_pending == 0)
					{
						/*printf("pending get/getx (load)_fwd request joined id %llu\n", pending_join->access_id);
						getchar();*/

						pending_join = list_remove(cache->pending_request_buffer, pending_join);
						list_enqueue(cache->retry_queue, pending_join);
						advance(cache->ec_ptr);
					}
				}
			}

			//if it is a new access (L3 retry) or a repeat access from an already owning core.
			if(sharers == 0 || owning_core == 1)
			{
				/*there should be only 1 core with the block*/
				if(owning_core == 1)
					assert(sharers == 1);

				DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s load hit zero or single shared ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);

				//set the presence bit in the directory for the requesting core.
				cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

				cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l1_cache_id);

				//set message size
				message_packet->size = gpu_v_caches[cache->id].block_size; //this can be either L1 I or L1 D cache block size.

				//update message status
				if(*cache_block_state_ptr == cgm_cache_block_modified)
				{
					message_packet->access_type = cgm_access_putx;
				}
				else if(*cache_block_state_ptr == cgm_cache_block_exclusive)
				{
					message_packet->access_type = cgm_access_put_clnx;
				}
				else if(*cache_block_state_ptr == cgm_cache_block_shared)
				{
					message_packet->access_type = cgm_access_puts;
				}

				/*this will send the block and block state up to the higher level cache.*/
				message_packet->cache_block_state = *cache_block_state_ptr;

				/*DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s load hit ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
						message_packet->access_type, *cache_block_state_ptr, P_TIME);*/

				/*stats*/
				if(!message_packet->protocol_case)
						message_packet->protocol_case = L2_hit;

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				cache_put_io_up_queue(cache, message_packet);

			}
			else if (sharers >= 1)
			{

				/*The connection between GPU L1 and L2 caches is a cross bar
				hold onto this request, flush the block out of the compute unit,
				join on the ack, in the mean time nack all other L1 request for this block.*/

				//there better be only 1 sharer
				assert(sharers == 1 && pending_bit == 0);

				//flush the block out of the pending core...
				DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s load hit multi share (CU flush) ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);


				//set the directory pending bit.
				cgm_cache_set_dir_pending_bit(cache, message_packet->set, message_packet->way);

				//get the id of the owning core L2
				owning_core = cgm_cache_get_xown_core(gpu, cache, message_packet->set, message_packet->way);

				//flush the block out of the core...
				flush_packet = packet_create();
				init_flush_packet(cache, flush_packet, message_packet->set, message_packet->way);

				flush_packet->gpu_access_type = cgm_access_store;
				flush_packet->l1_cache_id = owning_core;


				/*printf("l2 flushing block id %llu cycle %llu\n", flush_packet->evict_id, P_TIME);*/
				list_enqueue(cache->Tx_queue_top, flush_packet);
				advance(cache->cache_io_up_ec);

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				//drop the message packet into the pending request buffer
				message_packet = list_remove(cache->last_queue, message_packet);
				list_enqueue(cache->pending_request_buffer, message_packet);

			}
			else
			{
				fatal("cgm_mesi_gpu_l2_get(): invalid sharer/owning_core state\n");
			}

			break;
	}

	return;
}



/*void cgm_mesi_gpu_l2_get_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int ort_row = 0;
	int conflict_bit = -1;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int l3_map = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;

	struct cache_t *l3_cache_ptr = NULL;


	//charge delay
	P_PAUSE(cache->latency);

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s get_nack ID %llu type %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
			message_packet->access_type, P_TIME);

	//store the decoded address
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	//do not set retry because this contains the coalesce set and tag.
	//check that there is an ORT entry for this address
	ort_row = ort_search(cache, message_packet->tag, message_packet->set);
	assert(ort_row < cache->mshr_size);
	conflict_bit = cache->ort[ort_row][2];


	//if the conflict bit is set in the ort reset it because this is the nack



	if(conflict_bit == 0)
	{
		cache->ort[ort_row][2] = 1;

		DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s get_nack caught the conflict bit in the ORT table ID %llu type %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
			message_packet->access_type, P_TIME);
	}




	//add some routing/status data to the packet
	message_packet->access_type = cgm_access_get;

	l3_cache_ptr = cgm_l3_cache_map(message_packet->set);
	message_packet->l2_cache_id = cache->id;
	message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

	SETROUTE(message_packet, cache, l3_cache_ptr)

	//transmit to L3
	cache_put_io_down_queue(cache, message_packet);

	stats
	mem_system_stats->l2_load_nack++;

	return;
}*/

int cgm_mesi_gpu_l2_getx(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;
	struct cgm_packet_t *pending_join = NULL;

	int sharers, owning_core, pending_bit;

	int num_cus = si_gpu_num_compute_units;

	struct cgm_packet_t *flush_packet = NULL;

	//charge delay
	GPU_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(gpu, cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l1_cache_id);
	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	/*assumption block is never shared in GPU v caches*/
	if(sharers > 1)
		warning("cgm_mesi_gpu_l2_get(): sharers = %d\n", sharers);

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	if(pending_bit == 1 && *cache_block_hit_ptr == 1)
	{

		if(owning_core == 1)
		{
			assert(message_packet->coalesced == 0);

			DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s pending at L2 getx owning core PUTX back to L2 id %llu state %d cycle %llu\n",
				(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);

			/*there should be only be the owning core in the directory and the pending bit set*/
			assert(sharers == 1 && owning_core == 1);

			/*The block MUST be in the E or M state*/
			/*if(*cache_block_state_ptr != cgm_cache_block_exclusive || *cache_block_state_ptr != cgm_cache_block_modified)
				printf("block 0x%08x %s pending at L2 getx owning core (BUG HERE?) PUTX back to L2 id %llu state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);*/

			assert(*cache_block_state_ptr == cgm_cache_block_exclusive || *cache_block_state_ptr == cgm_cache_block_modified);

			//update message status
			message_packet->access_type = cgm_access_putx;

			//set cache block state modified
			message_packet->cache_block_state = cgm_cache_block_modified;

			// update message packet size
			message_packet->size = gpu_v_caches[message_packet->l1_cache_id].block_size;

			//don't change the directory entries, the downgrade ack will come back and clean things up.

			/*reset mp flags*/
			message_packet->coalesced = 0;
			message_packet->assoc_conflict = 0;

			cache_put_io_up_queue(cache, message_packet);

			/*stats*/
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L2_hit;

		}
		else
		{
			DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s pending at L2 getx nacked back to L2 id %llu state %d cycle %llu\n",
				(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);

			/*Third party is looking for access to the block, but it is busy nack and let node retry later*/

			//check if the packet has coalesced accesses.
			if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set, message_packet->access_id);
			}


			/*star todo find a better way to do this.
			this is for a special case where a coalesced access was pulled
			and is going to be nacked at this point we want the access to be
			treated as a new miss so set coalesced to 0*/
			if(message_packet->coalesced == 1)
			{
				message_packet->coalesced = 0;
			}

			//send the reply up as a NACK!
			message_packet->access_type = cgm_access_getx_nack;

			//set message package size
			message_packet->size = 1;

			/*reset mp flags*/
			message_packet->coalesced = 0;
			message_packet->assoc_conflict = 0;

			//send the reply
			cache_put_io_up_queue(cache, message_packet);

			/*stats*//*stats*/
			mem_system_stats->l2_store_nack++;
		}

		return 1;
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
			fatal("l2_cache_ctrl(): Invalid block state on store hit assss %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		case cgm_cache_block_invalid:

			//stats
			//cache->misses++;

			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/
				assert(*cache_block_hit_ptr == 0);
				assert(write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive);
				assert(message_packet->set == write_back_packet->set && message_packet->tag == write_back_packet->tag);
				assert(message_packet->access_type == cgm_access_getx || message_packet->access_type == cgm_access_store_retry);

				//see if we can write it back into the cache.
				write_back_packet->l2_victim_way = cgm_cache_get_victim_for_wb(cache, write_back_packet->set);

				/*special case nack the request back to L1 because L2 is still waiting on a a flush block ack*/
				if(write_back_packet->flush_pending == 1)
				{
					/*flush is pending, but we have a request for the block, nack it back to L1*/
					//send the reply up as a NACK!
					message_packet->access_type = cgm_access_getx_nack;

					//set message package size
					message_packet->size = 1;

					/*stats*/
					/*star todo add a stat for this*/

					DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s store nack wb pending flush ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type,
							*cache_block_state_ptr, P_TIME);

					/*reset mp flags*/
					message_packet->coalesced = 0;
					message_packet->assoc_conflict = 0;

					cache_put_io_up_queue(cache, message_packet);

					//warning("nacking store back to L1, flush still pending\n");

					return 1;
				}

				//if not then we must coalesce
				if(write_back_packet->l2_victim_way == -1)
				{
					//Set and ways are all transient must coalesce
					cache_check_ORT(cache, message_packet);

					/*stats*/
					cache->CoalescePut++;

					if(message_packet->coalesced == 1)
						return 1;
					else
						fatal("cgm_mesi_l2_getx(): write failed to coalesce when all ways are transient...\n");
				}

				assert(write_back_packet->L3_flush_join == 0);
				assert(write_back_packet->flush_pending == 0);

				//we are bringing a new block so evict the victim and flush the L1 copies
				assert(write_back_packet->l2_victim_way >= 0 && write_back_packet->l2_victim_way < cache->assoc);

				//first evict the old block if it isn't invalid already
				if(cgm_cache_get_block_state(cache, write_back_packet->set, write_back_packet->l2_victim_way) != cgm_cache_block_invalid)
					cgm_L2_cache_evict_block(cache, write_back_packet->set, write_back_packet->l2_victim_way,
							cgm_cache_get_num_shares(gpu, cache, write_back_packet->set, write_back_packet->l2_victim_way), 0);

				//clear the old directory entry
				cgm_cache_clear_dir(cache,  write_back_packet->set, write_back_packet->l2_victim_way);
				//set the new directory entry
				cgm_cache_set_dir(cache, write_back_packet->set, write_back_packet->l2_victim_way, message_packet->l1_cache_id);

				//now set the new block
				assert(write_back_packet->cache_block_state == cgm_cache_block_exclusive || write_back_packet->cache_block_state == cgm_cache_block_modified);
				cgm_cache_set_block(cache, write_back_packet->set, write_back_packet->l2_victim_way, write_back_packet->tag, write_back_packet->cache_block_state);

				//check for retries on successful cache write...
				if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
				{
					cache->MergeRetries++;
					//enter retry state.
					cache_coalesed_retry(cache, message_packet->tag, message_packet->set, message_packet->access_id);
				}

				//set message size
				message_packet->size = l1_d_caches[cache->id].block_size; //this can be either L1 I or L1 D cache block size.
				message_packet->cache_block_state = write_back_packet->cache_block_state;

				//update message status
				if(message_packet->cache_block_state == cgm_cache_block_modified)
				{
					message_packet->access_type = cgm_access_putx;
				}
				else if(message_packet->cache_block_state == cgm_cache_block_exclusive)
				{
					message_packet->access_type = cgm_access_put_clnx;
				}
				else
				{
					fatal("cgm_mesi_l2_getx(): invalid write back block state\n");
				}

				assert(write_back_packet->flush_pending == 0);

				//free the write back
				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s store wb hit id %llu state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);

				/*stats*/
				cache->WbMerges++;
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L2_hit;

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				cache_put_io_up_queue(cache, message_packet);
			}
			else
			{

				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
				{
					/*stats*/
					cache->CoalescePut++;

					DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s store miss coalesce ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);

					return 1;
				}

				//find victim
				message_packet->l2_victim_way = cgm_cache_get_victim(cache, message_packet->set, message_packet->tag);
				assert(message_packet->l2_victim_way >= 0 && message_packet->l2_victim_way < cache->assoc);

				//evict the victim
				if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l2_victim_way) != cgm_cache_block_invalid)
					cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->l2_victim_way,
							cgm_cache_get_num_shares(gpu, cache, message_packet->set, message_packet->l2_victim_way), 0);

				//clear the directory entry
				cgm_cache_clear_dir(cache, message_packet->set, message_packet->l2_victim_way);

				assert(cgm_cache_get_block_upgrade_pending_bit(cache, message_packet->set, message_packet->l2_victim_way) == 0);

				//set access type
				assert(message_packet->access_type == cgm_access_getx);

				message_packet->size = 1;

				if(hub_iommu_connection_type == hub_to_mc)
				{
					//message is going down to mc so its and mc_load
					message_packet->access_type = cgm_access_mc_load;
				}
				else
				{
					//message is going down to L3 so its a getx
					message_packet->access_type = cgm_access_getx;
					message_packet->cpu_access_type = cgm_access_store;
				}

				message_packet->cache_block_state = cgm_cache_block_modified;

				message_packet->l2_cache_id = gpu_core_id;
				message_packet->l2_cache_name = str_map_value(&l2_strn_map, gpu_core_id);

				DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s store miss ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
						message_packet->access_type, *cache_block_state_ptr, P_TIME);

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				cache_put_io_down_queue(cache, message_packet);
			}

			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			assert(sharers >= 0 && sharers <= num_cus);
			assert(owning_core >= 0 && owning_core <= 1);
			//assert(*cache_block_state_ptr != cgm_cache_block_shared);

			//set retry state
			if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set, message_packet->access_id);

				/*check for a pending get_fwd or getx_fwd join*/
				pending_join = cache_search_pending_request_buffer(cache, (message_packet->address & cache->block_address_mask));

				if(pending_join)
				{
					pending_join->downgrade_pending--;

					if (pending_join->downgrade_pending == 0)
					{
						/*printf("pending get/getx_fwd request joined id %llu\n", pending_join->access_id);
						getchar();*/

						pending_join = list_remove(cache->pending_request_buffer, pending_join);
						list_enqueue(cache->retry_queue, pending_join);
						advance(cache->ec_ptr);
					}
				}
			}


			//if it is a new access (l2 retry) or a repeat access from an already owning core.
			if(sharers == 0 || owning_core == 1)
			{
				//if the block is in the E state set M before sending up
				if(*cache_block_state_ptr == cgm_cache_block_exclusive)
				{
					cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);
				}

				DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s store hit single shared ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);

				//update message status
				message_packet->access_type = cgm_access_putx;

				//set cache block state modified
				message_packet->cache_block_state = cgm_cache_block_modified;

				//set message status and size
				message_packet->size = gpu_v_caches[message_packet->l1_cache_id].block_size; //this should be L1 D cache block size.

				//update directory
				cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

				cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l1_cache_id);


				/*stats*/
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L2_hit;

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				//send up to L1 D cache
				cache_put_io_up_queue(cache, message_packet);

			}
			else if(sharers >= 1)
			{

				/*The connection between GPU L1 and L2 caches is a cross bar
				hold onto this request, flush the block out of the compute unit,
				join on the ack, in the mean time nack all other L1 request for this block.*/

				//there better be only 1 sharer
				assert(sharers == 1 && pending_bit == 0);

				//flush the block out of the pending core...
				DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s store hit multi share (CU flush) ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);


				//set the directory pending bit.
				cgm_cache_set_dir_pending_bit(cache, message_packet->set, message_packet->way);

				//get the id of the owning core L2
				owning_core = cgm_cache_get_xown_core(gpu, cache, message_packet->set, message_packet->way);

				//flush the block out of the core...
				flush_packet = packet_create();
				init_flush_packet(cache, flush_packet, message_packet->set, message_packet->way);

				flush_packet->gpu_access_type = cgm_access_store;
				flush_packet->l1_cache_id = owning_core;

				if(message_packet->access_id == 6535548 || message_packet->access_id == 6310108)
					printf("owning core is %d pending bit is %d evict is %llu core access type %d\n",
							owning_core, cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way), flush_packet->evict_id, message_packet->access_type);


				list_enqueue(cache->Tx_queue_top, flush_packet);
				advance(cache->cache_io_up_ec);

				/*reset mp flags*/
				message_packet->coalesced = 0;
				message_packet->assoc_conflict = 0;

				//drop the message packet into the pending request buffer
				message_packet = list_remove(cache->last_queue, message_packet);
				list_enqueue(cache->pending_request_buffer, message_packet);

			}
			else
			{
				fatal("cgm_mesi_gpu_l2_getx(): invalid sharer/owning_core state\n");
			}

			break;

		case cgm_cache_block_shared:

			fatal("cgm_mesi_gpu_l2_getx(): block is shared\n");

			DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s upgrade miss ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
					message_packet->access_type, *cache_block_state_ptr, P_TIME);

			//access was a miss in L1 D but a hit in the shared state at the L2 level, set upgrade and run again.
			message_packet->access_type = cgm_access_upgrade;

			/*stats*/
			cache->UpgradeMisses++;
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L2_upgrade;

			//return 0 to process as an upgrade.
			return 0;

			break;
	}

	return 1;
}




void cgm_mesi_gpu_l2_get_getx_fwd_inval_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *get_getx_fwd_reply_packet = NULL;
	struct cgm_packet_t *pending_get_getx_fwd_request = NULL;
	struct cgm_packet_t *write_back_packet = NULL;
	struct cache_t *l3_cache_ptr = NULL;

	//int sharers, owning_core, pending_bit;

	//int num_cus = si_gpu_num_compute_units;

	//int l3_map;
	int error = 0;

	//charge delay
	GPU_PAUSE(cache->latency);

	//L1 D cache has been flushed

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	//get number of sharers
	//sharers = cgm_cache_get_num_shares(gpu, cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	//owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l1_cache_id);
	//check pending state
	//pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);

	DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s getx_fwd_inval_ack ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
			fatal("cgm_mesi_l2_getx_fwd_inval_ack(): L2 id %d invalid block state on getx_fwd inval ask as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			fatal("cgm_mesi_gpu_l2_get_getx_fwd_inval_ack(): assume not going to happen yet\n");

			//check WB for line...
			if(write_back_packet)
			{
				/*inval is complete for this write back so it should not be up in L1 D*/
				error = cache_validate_block_flushed_from_gpu_l1(gpu_v_caches, message_packet->address);
				assert(error == 0);

				assert(write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive);

				//////////
				//GETX_FWD
				//////////

				//forward block to requesting core

				//pull the GETX_FWD from the pending request buffer
				pending_get_getx_fwd_request = cache_search_pending_request_buffer(cache, message_packet->address);
				/*if not found uh-oh...*/
				assert(pending_get_getx_fwd_request);
				/*the address better be the same too...*/
				assert(pending_get_getx_fwd_request->address == message_packet->address);
				assert(pending_get_getx_fwd_request->start_cycle != 0);

				//prepare to forward the block
				//set access type
				pending_get_getx_fwd_request->access_type = cgm_access_putx;

				//set the block state
				pending_get_getx_fwd_request->cache_block_state = cgm_cache_block_modified;

				//set message package size if modified in L2/L1.
				/*if(message_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_modified)
				{*/
				pending_get_getx_fwd_request->size = l2_caches[str_map_string(&l2_strn_map, pending_get_getx_fwd_request->l2_cache_name)].block_size;
				/*}
				else
				{
				pending_get_getx_fwd_request->size = 1;
				}*/

				//fwd block to requesting core
				//update routing headers swap dest and src
				//requesting node
				pending_get_getx_fwd_request->dest_name = str_map_value(&node_strn_map, pending_get_getx_fwd_request->src_id);
				pending_get_getx_fwd_request->dest_id = str_map_string(&node_strn_map, pending_get_getx_fwd_request->src_name);

				//owning node L2
				pending_get_getx_fwd_request->src_name = cache->name;
				pending_get_getx_fwd_request->src_id = str_map_string(&node_strn_map, cache->name);

				//transmit block to requesting node
				pending_get_getx_fwd_request = list_remove(cache->pending_request_buffer, pending_get_getx_fwd_request);
				list_enqueue(cache->Tx_queue_bottom, pending_get_getx_fwd_request);
				advance(cache->cache_io_down_ec);

				///////////////
				//getx_fwd_ack
				///////////////

				//send the getx_fwd_ack to L3 cache.

				//create getx_fwd_ack packet
				get_getx_fwd_reply_packet = packet_create();
				assert(get_getx_fwd_reply_packet);

				init_getx_fwd_ack_packet(get_getx_fwd_reply_packet, message_packet->address);
				get_getx_fwd_reply_packet->access_id = message_packet->access_id;

				//set message package size if modified in L2/L1.
				if(message_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_modified)
				{
					get_getx_fwd_reply_packet->size = l2_caches[str_map_string(&l2_strn_map, pending_get_getx_fwd_request->l2_cache_name)].block_size;
					get_getx_fwd_reply_packet->cache_block_state = cgm_cache_block_modified;
				}
				else
				{
					get_getx_fwd_reply_packet->size = 1;
					get_getx_fwd_reply_packet->cache_block_state = cgm_cache_block_invalid;
				}

				//fwd reply (getx_fwd_ack) to L3
				l3_cache_ptr = cgm_l3_cache_map(message_packet->set);

				//fakes src as the requester
				get_getx_fwd_reply_packet->l2_cache_id = pending_get_getx_fwd_request->l2_cache_id;
				get_getx_fwd_reply_packet->l2_cache_name = pending_get_getx_fwd_request->src_name;

				SETROUTE(get_getx_fwd_reply_packet, cache, l3_cache_ptr)

				//transmit getx_fwd_ack to L3 (home)
				list_enqueue(cache->Tx_queue_bottom, get_getx_fwd_reply_packet);
				advance(cache->cache_io_down_ec);

				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				//destroy the L1 D getx_fwd_inval_ack message because we don't need it anymore.
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);

			}
			else
			{

				fatal("cgm_mesi_l2_getx_fwd_inval_ack(): get_getx_fwd should no longer get this far\n");
				/*unsigned int temp = message_packet->address;
				temp = temp & cache->block_address_mask;
				fatal("cgm_mesi_l2_getx_fwd_inval_ack(): line missing in L2 after downgrade block addr 0x%08x\n", temp);

				//pull the GET_FWD from the pending request buffer
				pending_getx_fwd_request = cache_search_pending_request_buffer(cache, message_packet->address);
				if not found uh-oh...
				assert(pending_getx_fwd_request);
				the address better be the same too...
				assert(pending_getx_fwd_request->address == message_packet->address);
				assert(pending_getx_fwd_request->start_cycle != 0);

				//downgrade the local block
				assert(pending_getx_fwd_request->set == message_packet->set && pending_getx_fwd_request->way == message_packet->way);

				//set cgm_access_getx_fwd_nack
				pending_getx_fwd_request->access_type = cgm_access_getx_fwd_nack;

				//fwd reply (downgrade_nack) to L3
				l3_map = cgm_l3_cache_map(pending_getx_fwd_request->set);

				here send the nack down to the L3
				don't change any of the source information

				message_packet->l2_cache_id = l2_caches[my_pid].id;
				message_packet->l2_cache_name = str_map_value(&l2_strn_map, l2_caches[my_pid].id);
				reply_packet->src_name = l2_caches[my_pid].name;
				reply_packet->src_id = str_map_string(&node_strn_map, l2_caches[my_pid].name);

				pending_getx_fwd_request->dest_name = l3_caches[l3_map].name;
				pending_getx_fwd_request->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

				//transmit back to L3
				pending_getx_fwd_request = list_remove(cache->pending_request_buffer, pending_getx_fwd_request);
				list_enqueue(cache->Tx_queue_bottom, pending_getx_fwd_request);
				advance(cache->cache_io_down_ec);

				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);*/

			}

			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			//block still present in L2 cache

			//////////
			//GETX_FWD
			//////////

			//forward block to requesting core

			//pull the GETX_FWD from the pending request buffer
			pending_get_getx_fwd_request = cache_search_pending_request_buffer(cache, message_packet->address);
			/*if not found uh-oh...*/
			assert(pending_get_getx_fwd_request);
			/*the address better be the same too...*/
			assert(pending_get_getx_fwd_request->address == message_packet->address);
			assert(pending_get_getx_fwd_request->start_cycle != 0);
			assert(pending_get_getx_fwd_request->access_type == cgm_access_get_fwd || pending_get_getx_fwd_request->access_type == cgm_access_getx_fwd);

			//invalidate the local block
			assert(pending_get_getx_fwd_request->set == message_packet->set && pending_get_getx_fwd_request->way == message_packet->way);
			cgm_cache_set_block_state(cache, pending_get_getx_fwd_request->set, pending_get_getx_fwd_request->way, cgm_cache_block_invalid);

			//make sure the directory is clear...
			cgm_cache_clear_dir(cache, pending_get_getx_fwd_request->set, pending_get_getx_fwd_request->way);

			//prepare to forward the block
			pending_get_getx_fwd_request->size = l2_caches[str_map_string(&l2_strn_map, pending_get_getx_fwd_request->l2_cache_name)].block_size;

			//set message package size if modified in L2/L1.
			if(*cache_block_state_ptr == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_modified)
			{
				//set access type
				pending_get_getx_fwd_request->access_type = cgm_access_putx;

				//set the block state
				pending_get_getx_fwd_request->cache_block_state = cgm_cache_block_modified;
			}
			else
			{
				//set access type
				pending_get_getx_fwd_request->access_type = cgm_access_put_clnx;

				//set the block state
				pending_get_getx_fwd_request->cache_block_state = cgm_cache_block_exclusive;
			}

			//fwd block to requesting core
			//update routing headers swap dest and src
			//requesting node
			pending_get_getx_fwd_request->dest_name = str_map_value(&node_strn_map, pending_get_getx_fwd_request->src_id);
			pending_get_getx_fwd_request->dest_id = str_map_string(&node_strn_map, pending_get_getx_fwd_request->src_name);

			//owning node L2
			pending_get_getx_fwd_request->src_name = cache->name;
			pending_get_getx_fwd_request->src_id = str_map_string(&node_strn_map, cache->name);

			//transmit block to requesting node
			pending_get_getx_fwd_request = list_remove(cache->pending_request_buffer, pending_get_getx_fwd_request);
			list_enqueue(cache->Tx_queue_bottom, pending_get_getx_fwd_request);
			advance(cache->cache_io_down_ec);

			///////////////
			//getx_fwd_ack
			///////////////

			//send the get_getx_fwd_ack to L3 cache.

			//create get_getx_fwd_ack packet
			get_getx_fwd_reply_packet = packet_create();
			assert(get_getx_fwd_reply_packet);

			init_getx_fwd_ack_packet(get_getx_fwd_reply_packet, message_packet->address);

			//set message package size if modified in L2/L1.
			if(*cache_block_state_ptr == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_modified)
			{
				get_getx_fwd_reply_packet->size = l2_caches[str_map_string(&l2_strn_map, pending_get_getx_fwd_request->l2_cache_name)].block_size;
				get_getx_fwd_reply_packet->cache_block_state = cgm_cache_block_modified;
			}
			else
			{
				get_getx_fwd_reply_packet->size = 1;
				get_getx_fwd_reply_packet->cache_block_state = cgm_cache_block_exclusive;
			}

			//fwd reply (getx_fwd_ack) to L3
			//l3_cache_ptr = cgm_l3_cache_map(message_packet->set);

			//fakes src as the requester
			get_getx_fwd_reply_packet->l2_cache_id = pending_get_getx_fwd_request->l2_cache_id;
			get_getx_fwd_reply_packet->l2_cache_name = pending_get_getx_fwd_request->src_name;

			//SETROUTE(get_getx_fwd_reply_packet, cache, l3_cache_ptr)

			//transmit getx_fwd_ack to L3 (home)
			list_enqueue(cache->Tx_queue_bottom, get_getx_fwd_reply_packet);
			advance(cache->cache_io_down_ec);

			//destroy the L1 D getx_fwd_inval_ack message because we don't need it anymore.
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			break;
	}

	return;
}

void cgm_mesi_gpu_l2_get_getx_fwd_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	warning("%s received get getx_fwd nack cycle %llu\n", cache->name, P_TIME);

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	// *way_ptr = &way;

	//int l3_map = 0;
	int ort_status = 0;
	//struct cgm_packet_t *pending_packet;
	enum cgm_cache_block_state_t victim_trainsient_state;

	//struct cgm_packet_t *pending_get_getx_fwd_request = NULL;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	//store the decode in the packet for now.
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;
	message_packet->way = way;

	//charge delay
	GPU_PAUSE(cache->latency);

	/*our load (getx_fwd) request has been nacked by the owning L2*/

	/*verify status of cache blk*/
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l2_victim_way);
	assert(victim_trainsient_state == cgm_cache_block_transient); //there is a transient block
	if(victim_trainsient_state != cgm_cache_block_transient)
	{
		ort_dump(cache);
		cgm_cache_dump_set(cache, message_packet->set);

		unsigned int temp = message_packet->address;
		temp = temp & cache->block_address_mask;

		fatal("cgm_mesi_l2_getx_fwd_nack(): %s block not in transient state access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d cycle %llu\n",
			cache->name, message_packet->access_id, message_packet->address, temp,
			message_packet->set, message_packet->tag, message_packet->way, P_TIME);
	}


	ort_status = ort_search(cache, message_packet->tag, message_packet->set); // there is an outstanding request.
	assert(ort_status <= cache->mshr_size);

	DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s getx_fwd_nack nack ID %llu type %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, P_TIME);

	//change to a get
	message_packet->access_type = cgm_access_getx;
	message_packet->cpu_access_type = cgm_access_load;
	message_packet->cache_block_state = cgm_cache_block_exclusive;

	//L3 should see the entire GPU as a single core.
	message_packet->l2_cache_id = gpu_core_id;
	message_packet->l2_cache_name = str_map_value(&l2_strn_map, gpu_core_id);

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);

	cache_put_io_down_queue(cache, message_packet);

	return;
}



void cgm_mesi_gpu_l2_get_getx_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//fatal("cgm_mesi_gpu_l2_get_getx_fwd(): BOOM!\n");


	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *inval_packet = NULL;
	struct cgm_packet_t *write_back_packet = NULL;
	struct cgm_packet_t *nack_packet = NULL;
	struct cgm_packet_t *reply_packet = NULL;
	struct cache_t *l3_cache_ptr = NULL;

	int sharers, pending_bit;//, owning_core;

	int error = 0;
	int ort_status = 0;
	//int l3_map;

	//charge delay
	GPU_PAUSE(cache->latency);

	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);




	//get number of sharers
	sharers = cgm_cache_get_num_shares(gpu, cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	//owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l1_cache_id);
	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);


	DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s getx_fwd ID %llu type %d state %d wb? %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
			message_packet->access_type, *cache_block_state_ptr, is_writeback_present(write_back_packet), P_TIME);

	/*assumption block is never shared in GPU v caches*/
	if(sharers > 1)
		warning("cgm_mesi_gpu_l2_get(): sharers = %d\n", sharers);

	assert(pending_bit == 0);

	/*look for an access conflict, this can happen if a get_fwd beats a putx/putx_n/upgrade(putx)*/
	ort_status = ort_search(cache, message_packet->tag, message_packet->set);
	if(ort_status != cache->mshr_size)
	{

		fatal("cgm_mesi_gpu_l2_get_getx_fwd(): access conflict, but shouldn't have one of these yet...\n");

		/*if there is a pending access int the ORT there better not be a block or a write back*/
		if(*cache_block_state_ptr == cgm_cache_block_invalid)
			assert(!write_back_packet);

		/*if(cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way) == cgm_cache_block_transient)
			assert(*cache_block_state_ptr == cgm_cache_block_shared && *cache_block_hit_ptr == 1);*/
	}

	/*if(message_packet->src_id == 0)
		fatal("%s getx_fwd blk addr 0x%08x  cache id %d\n", cache->name, message_packet->address & cache->block_address_mask, message_packet->l2_cache_id);*/

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:

			cgm_cache_dump_set(cache, message_packet->set);

			fatal("cgm_mesi_l2_getx_fwd(): %s invalid block state on getx_fwd as %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
				cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
				message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
				message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);
			break;

		case cgm_cache_block_invalid:

			//if((message_packet->address & cache->block_address_mask) == 0x081c7e40)
				//fatal("%s blk 0x%08x in in gpu cycle %llu\n", cache->name, (message_packet->address & cache->block_address_mask), P_TIME);

			warning("************cgm_mesi_gpu_l2_get_getx_fwd(): block isn't in the cache id %llu blk addr 0x%08x\n",
					message_packet->access_id,  message_packet->address & cache->block_address_mask);

			//check the WB buffer
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/

				assert(*cache_block_hit_ptr == 0);
				assert(write_back_packet->cache_block_state == cgm_cache_block_modified
						|| write_back_packet->cache_block_state == cgm_cache_block_exclusive);

				/*check state of write back for flush*/
				if(write_back_packet->flush_pending == 0)
				{
					//a flush has completed and the WB is still here so it better be modified
					assert(write_back_packet->cache_block_state == cgm_cache_block_modified);

					/*the flush is complete finish the getx_fwd now*/

					/*flush is complete for this write back so it should not be up in L1 D*/
					error = cache_validate_block_flushed_from_gpu_l1(gpu_v_caches, message_packet->address);
					assert(error == 0);

					//////////
					//GETX_FWD
					//////////

					/*for now we handle both get and getx fwd in the same function on the GPU*/

					//forward block to requesting core

					//set access type
					message_packet->access_type = cgm_access_putx;

					message_packet->cache_block_state = cgm_cache_block_modified;

					//prepare to forward the block
					message_packet->size = l2_caches[str_map_string(&l2_strn_map, message_packet->l2_cache_name)].block_size;

					//set message package size if modified in L2/L1.
					/*if(write_back_packet->cache_block_state == cgm_cache_block_modified)
					{
						//prepare to forward the block
						message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

						//set access type
						message_packet->access_type = cgm_access_putx;

						//set the block state
						message_packet->cache_block_state = cgm_cache_block_modified;
					}
					else
					{
						//prepare to forward the block
						message_packet->size = 1;

						//set access type
						message_packet->access_type = cgm_access_put_clnx;

						//set the block state
						message_packet->cache_block_state = cgm_cache_block_exclusive;
					}*/

					//--------------------------------

					//fwd block to requesting core
					//update routing headers swap dest and src
					//requesting node
					message_packet->dest_name = str_map_value(&node_strn_map, message_packet->src_id);
					message_packet->dest_id = str_map_string(&node_strn_map, message_packet->src_name);

					//owning node L2
					message_packet->src_name = cache->name;
					message_packet->src_id = str_map_string(&node_strn_map, cache->name);

					//transmit block to requesting node
					message_packet = list_remove(cache->last_queue, message_packet);
					list_enqueue(cache->Tx_queue_bottom, message_packet);
					advance(cache->cache_io_down_ec);

					///////////////
					//getx_fwd_ack
					///////////////

					//send the getx_fwd_ack to L3 cache.

					//create getx_fwd_ack packet
					reply_packet = packet_create();
					assert(reply_packet);

					init_getx_fwd_ack_packet(reply_packet, message_packet->address);

					reply_packet->size = l2_caches[str_map_string(&l2_strn_map, message_packet->l2_cache_name)].block_size;
					reply_packet->cache_block_state = cgm_cache_block_modified;

					//fakes src as the requester
					reply_packet->l2_cache_id = message_packet->l2_cache_id;
					reply_packet->l2_cache_name = message_packet->src_name;

					//set message package size if modified in L2/L1.
					/*if(write_back_packet->cache_block_state == cgm_cache_block_modified)
					{
						reply_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;
						reply_packet->cache_block_state = cgm_cache_block_modified;
					}
					else
					{
						reply_packet->size = 1;
						reply_packet->cache_block_state = cgm_cache_block_invalid;
					}*/

					//fwd reply (getx_fwd_ack) to L3
					//l3_cache_ptr = cgm_l3_cache_map(message_packet->set);

					//fakes src as the requester
					//reply_packet->l2_cache_id = message_packet->l2_cache_id;
					//reply_packet->l2_cache_name = message_packet->src_name;

					//SETROUTE(reply_packet, cache, l3_cache_ptr)

					//transmit getx_fwd_ack to L3 (home)
					list_enqueue(cache->Tx_queue_bottom, reply_packet);
					advance(cache->cache_io_down_ec);

					write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
					packet_destroy(write_back_packet);

				}
				else
				{

					fatal("cgm_mesi_gpu_l2_get_getx_fwd(): shouldn't have pending flush yet\n");

					/*if the wb is flush pending we have to wait for the flush to complete and then join there*/

					/*write back is in the process of being flushed by L1*/
					assert(write_back_packet->flush_pending == 1);

					//message_packet->downgrade_pending = 1;
					message_packet->L3_flush_join = 1;
					cgm_cache_insert_pending_request_buffer(cache, message_packet);
				}
			}
			else
			{

				fatal("cgm_mesi_gpu_l2_get_getx_fwd(): blk miss in l2 shouldn't happen yet??\n");

				//printf("\tcgm_mesi_l2_getx_fwd(): no wb blk addr 0x%08x \n", message_packet->address & cache->block_address_mask);
				//fatal("Getx_fwd %s access id %llu blk_addr 0x%08x\n", cache->name, message_packet->access_id, message_packet->address & cache->block_address_mask);

				if(ort_status < cache->mshr_size)
				{

					fatal("cgm_mesi_gpu_l2_get_getx_fwd(): shouldn't be an ORT issue in the GPU\n");

					DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s getx_fwd conflict pending join ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name,message_packet->access_id,
							message_packet->access_type, *cache_block_state_ptr, P_TIME);

					/*set the bit in the ort table to 0*/
					ort_set_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);

					message_packet->downgrade_pending = 1;

					/*drop into the pending request buffer*/
					message_packet =  list_remove(cache->last_queue, message_packet);
					list_enqueue(cache->pending_request_buffer, message_packet);
				}
				else
				{

					/* The block was evicted silently and should not be in the L1 cache.
					 * However, the block may be in L1 D's write back or in the pipe between L1 D and L2.
					 * We have to send a flush to L1 D to make sure the block is really out of there before proceeding.*/
					error = cache_validate_block_flushed_from_gpu_l1(gpu_v_caches, message_packet->address);
					assert(error == 0);

					/*two part reply (1) send nack to L3 and (2) send nack to requesting L2*/
					//set access type
					message_packet->access_type = cgm_access_getx_fwd_nack;

					//set the block state
					message_packet->cache_block_state = cgm_cache_block_invalid;

					//set message package size
					message_packet->size = 1;

					//fwd nack to requesting core
					//update routing headers swap dest and src
					//requesting node
					message_packet->dest_name = str_map_value(&node_strn_map, message_packet->src_id);
					message_packet->dest_id = str_map_string(&node_strn_map, message_packet->src_name);

					//owning node L2
					message_packet->src_name = cache->name;
					message_packet->src_id = str_map_string(&node_strn_map, cache->name);

					//transmit nack to L2
					cache_put_io_down_queue(cache, message_packet);

					////////
					//part 2
					////////

					//create downgrade_nack
					nack_packet = packet_create();
					assert(nack_packet);

					init_getx_fwd_nack_packet(nack_packet, message_packet->address);
					nack_packet->access_id = message_packet->access_id;

					//fwd reply (downgrade_nack) to L3
					l3_cache_ptr = cgm_l3_cache_map(message_packet->set);

					SETROUTE(nack_packet, cache, l3_cache_ptr)

					//transmit block to L3
					list_enqueue(cache->Tx_queue_bottom, nack_packet);
					advance(cache->cache_io_down_ec);
				}

			}

			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			//a GET/GETX_FWD means the block is E/M in this core. The block will be E/M in the L1

			/*if((message_packet->address & cache->block_address_mask) == 0x081c7e40)
				fatal("%s blk 0x%08x in in gpu cycle %llu\n", cache->name, (message_packet->address & cache->block_address_mask), P_TIME);*/

			//store the getx_fwd in the pending request buffer
			message_packet->inval_pending = 1;
			cgm_cache_insert_pending_request_buffer(cache, message_packet);

			//set the flush_pending bit to 1 in the block
			cgm_cache_set_block_flush_pending_bit(cache, message_packet->set, message_packet->way);


			//flush the L1 cache because the line may be dirty in L1
			inval_packet = packet_create();
			assert(inval_packet);
			init_getx_fwd_inval_packet(inval_packet, message_packet->address);

			unsigned long long bit_vector;

			//get the presence bits from the directory
			bit_vector = cache->sets[message_packet->set].blocks[message_packet->way].directory_entry.entry;
			bit_vector = bit_vector & cache->share_mask;


			//send the L1 D cache the inval message

			inval_packet->l1_cache_id = LOG2(bit_vector);

			/*warning("bit vector value %llu owning core %d\n", bit_vector, inval_packet->l1_cache_id);*/


			inval_packet->gpu_access_type = cgm_access_store;
			inval_packet->access_id = message_packet->access_id;
			list_enqueue(cache->Tx_queue_top, inval_packet);
			advance(cache->cache_io_up_ec);
			break;
	}



	return;
}

void cgm_mesi_gpu_l2_gpu_flush_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//this is the ack from L1, flush it down from here now...

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//get number of sharers
	int sharers; //, owning_core, pending_bit;

	//int l3_map = 0;
	int ort_status = -1;

	enum cgm_cache_block_state_t victim_trainsient_state;

	//struct cgm_packet_t *wb_packet = NULL;
	//struct cache_t *l3_cache_ptr = NULL;

	//unsigned long long bit_vector;

	//charge delay
	GPU_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(gpu, cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	//owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l1_cache_id);
	//check pending state
	//pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	//wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	DEBUG(LEVEL == 2 || LEVEL == 3, "cgm_mesi_gpu_l2_gpu_flush_ack(): block 0x%08x %s GPU flush block ack ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id,
			message_packet->access_type, *cache_block_state_ptr, P_TIME);

	/*check the ORT table for an outstanding access
	check the ORT table is there an outstanding access for this block we are trying to evict?*/
	ort_status = ort_search(cache, message_packet->tag, message_packet->set);
	if(ort_status != cache->mshr_size)
	{
		fatal("cgm_mesi_gpu_l2_gpu_flush(): shouldn't have this\n");
		/*yep there is so set the bit in the ort table to 0.
		 * When the put/putx comes kill it and try again...*/
		//ort_set_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);
	}

	//this is an ack so there better still be a sharer
	assert(sharers == 1);



	//search the WB buffer for the data
	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
			fatal("cgm_mesi_gpu_l2_gpu_flush(): L2 id %d invalid block state on inval as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			fatal("cgm_mesi_gpu_l2_gpu_flush_ack(): block should NOT be invalid\n");

			/*wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

			//found the block in the WB buffer
			if(wb_packet)
			{
				if pending is 0 the L1 cache has been flushed process now
				if(wb_packet->flush_pending == 0)
				{
					assert(wb_packet->cache_block_state == cgm_cache_block_modified);

					message_packet->size = cache->block_size;
					message_packet->cache_block_state = cgm_cache_block_modified;

					//remove the block from the WB buffer
					wb_packet = list_remove(cache->write_back_buffer, wb_packet);
					packet_destroy(wb_packet);

					//set access type inval_ack
					message_packet->access_type = cgm_access_gpu_flush_ack;

					cache_put_io_down_queue(cache, message_packet);

				}
				else if(wb_packet->flush_pending == 1)
				{

					fatal("cgm_mesi_gpu_l2_gpu_flush(): hope this doesn't happen!\n");

					//waiting on flush to finish insert into pending request buffer
					assert(wb_packet->cache_block_state == cgm_cache_block_exclusive || wb_packet->cache_block_state == cgm_cache_block_modified);

					set flush_join bit
					wb_packet->L3_flush_join = 1;

					//put the message packet into the pending request buffer
					message_packet = list_remove(cache->last_queue, message_packet);
					list_enqueue(cache->pending_request_buffer, message_packet);
				}
				else
				{
					fatal("cgm_mesi_gpu_l2_gpu_flush(): wb_packet has invalid flush_pending value\n");
				}
			}
			else
			{

				//star todo somehow check and make sure these are modified
				if here the L2 cache has already written back, send down so the flush can complete
				message_packet->size = 1;
				message_packet->cache_block_state = cgm_cache_block_invalid;

				//set access type inval_ack
				message_packet->access_type = cgm_access_gpu_flush_ack;

				cache_put_io_down_queue(cache, message_packet);
			}*/

			break;


		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			/*The block is in L2 but not up in one of the compute units (sharers == 0)
			flush it from the L2 cache and move on.*/
			assert(victim_trainsient_state != cgm_cache_block_transient);

			//set the block state to invalid
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			//clear the directory entry
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

			if(*cache_block_state_ptr == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_modified)
			{
				message_packet->size = cache->block_size;
				message_packet->cache_block_state = cgm_cache_block_modified;
			}
			else
			{
				message_packet->size = 1;
				message_packet->cache_block_state = cgm_cache_block_invalid;
			}

				//set access type inval_ack
				message_packet->access_type = cgm_access_gpu_flush_ack;

				cache_put_io_down_queue(cache, message_packet);

			break;

		case cgm_cache_block_shared:

			fatal("cgm_mesi_gpu_l2_gpu_flush(): shouldn't be a shared state\n");

			break;
	}

	return;
}


void cgm_mesi_gpu_l2_gpu_flush(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//get number of sharers
	int sharers; //, owning_core, pending_bit;

	//int l3_map = 0;
	int ort_status = -1;

	enum cgm_cache_block_state_t victim_trainsient_state;

	struct cgm_packet_t *wb_packet = NULL;
	//struct cache_t *l3_cache_ptr = NULL;

	unsigned long long bit_vector;

	//charge delay
	GPU_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(gpu, cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	//owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l1_cache_id);
	//check pending state
	//pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	DEBUG(LEVEL == 2 || LEVEL == 3, "cgm_mesi_gpu_l2_gpu_flush(): (from system) block 0x%08x %s GPU flush block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id,
			message_packet->access_type, *cache_block_state_ptr, P_TIME);

	/*check the ORT table for an outstanding access
	check the ORT table is there an outstanding access for this block we are trying to evict?*/
	ort_status = ort_search(cache, message_packet->tag, message_packet->set);
	if(ort_status != cache->mshr_size)
	{
		fatal("cgm_mesi_gpu_l2_gpu_flush(): shouldn't have this\n");
		/*yep there is so set the bit in the ort table to 0.
		 * When the put/putx comes kill it and try again...*/
		//ort_set_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);
	}

	//first check and see if there are any sharers for the block

	if(sharers >= 1 && *cache_block_hit_ptr == 1)
	{
		/*should only be one compute unit with the block*/
		assert(sharers == 1);

		/*just forward the flush request on to l1 cache*/
		//get the presence bits from the directory
		bit_vector = cache->sets[message_packet->set].blocks[message_packet->way].directory_entry.entry;
		assert(bit_vector > 0);
		bit_vector = bit_vector & cache->share_mask;

		message_packet->l1_cache_id = LOG2(bit_vector);
		message_packet->gpu_access_type = cgm_access_store;

		//making sure something didn't happen to these fields.
		message_packet->size = 1;
		message_packet->access_type = cgm_access_gpu_flush;

		//transmit to SA
		cache_put_io_up_queue(cache, message_packet);
	}
	else
	{
		//no GPU compute unit has the block, so finish the flush and send down

		//search the WB buffer for the data
		switch(*cache_block_state_ptr)
		{
			case cgm_cache_block_noncoherent:
			case cgm_cache_block_owned:
				fatal("cgm_mesi_gpu_l2_gpu_flush(): L2 id %d invalid block state on inval as %s address %u\n",
					cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
				break;

			case cgm_cache_block_invalid:

				wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

				//found the block in the WB buffer
				if(wb_packet)
				{
					/*if pending is 0 the L1 cache has been flushed process now*/
					if(wb_packet->flush_pending == 0)
					{
						assert(wb_packet->cache_block_state == cgm_cache_block_modified);

						message_packet->size = cache->block_size;
						message_packet->cache_block_state = cgm_cache_block_modified;

						//remove the block from the WB buffer
						wb_packet = list_remove(cache->write_back_buffer, wb_packet);
						packet_destroy(wb_packet);

						//set access type inval_ack
						message_packet->access_type = cgm_access_gpu_flush_ack;

						cache_put_io_down_queue(cache, message_packet);

					}
					else if(wb_packet->flush_pending == 1)
					{

						fatal("cgm_mesi_gpu_l2_gpu_flush(): hope this doesn't happen!\n");

						//waiting on flush to finish insert into pending request buffer
						assert(wb_packet->cache_block_state == cgm_cache_block_exclusive || wb_packet->cache_block_state == cgm_cache_block_modified);

						/*set flush_join bit*/
						wb_packet->L3_flush_join = 1;

						//put the message packet into the pending request buffer
						message_packet = list_remove(cache->last_queue, message_packet);
						list_enqueue(cache->pending_request_buffer, message_packet);
					}
					else
					{
						fatal("cgm_mesi_gpu_l2_gpu_flush(): wb_packet has invalid flush_pending value\n");
					}
				}
				else
				{

					//star todo somehow check and make sure these are modified
					/*if here the L2 cache may have already written back, send down so the flush can complete*/
					message_packet->size = 1;
					message_packet->cache_block_state = cgm_cache_block_invalid;

					//set access type inval_ack
					message_packet->access_type = cgm_access_gpu_flush_ack;

					cache_put_io_down_queue(cache, message_packet);
				}

				break;


			case cgm_cache_block_exclusive:
			case cgm_cache_block_modified:

				assert(sharers == 0);

				/*The block is in L2 but not up in one of the compute units (sharers == 0)
				flush it from the L2 cache and move on.*/
				assert(victim_trainsient_state != cgm_cache_block_transient);

				assert(cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way) == 0);

				//set the block state to invalid
				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

				//clear the directory entry
				cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

				if(*cache_block_state_ptr == cgm_cache_block_modified)
				{
					message_packet->size = cache->block_size;
					message_packet->cache_block_state = cgm_cache_block_modified;
				}
				else
				{
					message_packet->size = 1;
					message_packet->cache_block_state = cgm_cache_block_exclusive;
				}

				//set access type inval_ack
				message_packet->access_type = cgm_access_gpu_flush_ack;

				cache_put_io_down_queue(cache, message_packet);

				break;

			case cgm_cache_block_shared:

				fatal("cgm_mesi_gpu_l2_gpu_flush(): shouldn't be a shared state\n");

				break;
		}

	}

	return;
}

void cgm_mesi_gpu_l2_flush_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//Invalidation/eviction request from L3 cache

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;
	//int l3_map = 0;
	int ort_status = -1;

	struct cgm_packet_t *pending_request_packet = NULL;

	enum cgm_cache_block_state_t victim_trainsient_state;

	int sharers = -1;//, owning_core, pending_bit;

	struct cgm_packet_t *wb_packet = NULL;
	struct cache_t *l3_cache_ptr = NULL;

	//charge delay
	GPU_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//check transient state
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	//check writeback
	wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);
	//get number of sharers
	sharers = cgm_cache_get_num_shares(gpu, cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	//owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l1_cache_id);
	//check pending state
	//pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	DEBUG(LEVEL == 2 || LEVEL == 3, "cgm_mesi_gpu_l2_flush_block(): (from L3) block 0x%08x %s GPU flush block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id,
			message_packet->access_type, *cache_block_state_ptr, P_TIME);

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);


	assert(message_packet->access_type != cgm_access_gpu_flush);

	/*check the ORT table for an outstanding access*/
	//check the ORT table is there an outstanding access for this block we are trying to evict?
	ort_status = ort_search(cache, message_packet->tag, message_packet->set);
	if(ort_status != cache->mshr_size)
	{
		fatal("block 0x%08x %s flush block conflict found ort set cycle %llu\n", (message_packet->address & cache->block_address_mask), cache->name, P_TIME);
		/*yep there is so set the bit in the ort table to 0.
		 * When the put/putx comes kill it and try again...*/
		ort_set_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);
	}

	/*//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);*/
	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
			fatal("cgm_mesi_l2_inval(): L2 id %d invalid block state on inval as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			//fatal("cgm_mesi_gpu_l2_flush_block(): %s blk not in cache 0x%08x\n", cache->name, (message_packet->address & cache->block_address_mask));
			/*wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);*/

			//found the block in the WB buffer
			if(wb_packet)
			{

				/*if pending is 0 the L1 cache has been flushed process now*/
				if(wb_packet->flush_pending == 0)
				{

					assert(wb_packet->cache_block_state == cgm_cache_block_modified);

					message_packet->size = cache->block_size;
					message_packet->cache_block_state = cgm_cache_block_modified;

					//remove the block from the WB buffer
					wb_packet = list_remove(cache->write_back_buffer, wb_packet);
					packet_destroy(wb_packet);

					//set access type inval_ack
					message_packet->access_type = cgm_access_flush_block_ack;

					cache_put_io_down_queue(cache, message_packet);

				}
				else if(wb_packet->flush_pending == 1)
				{

					//fatal("cgm_mesi_l2_flush_block(): flush pending blk 0x%08x this is probably ok, just need to make sure its working right\n",
					//		message_packet->address & cache->block_address_mask);

					//waiting on flush to finish insert into pending request buffer
					assert(wb_packet->cache_block_state == cgm_cache_block_exclusive || wb_packet->cache_block_state == cgm_cache_block_modified);

					/*set flush_join bit*/
					wb_packet->L3_flush_join = 1;

					//put the message packet into the pending request buffer
					message_packet = list_remove(cache->last_queue, message_packet);
					list_enqueue(cache->pending_request_buffer, message_packet);
				}
				else
				{
					fatal("cgm_mesi_l2_flush_block(): wb_packet has invalid flush_pending value\n");
				}
			}
			else
			{

				//star todo somehow check and make sure these are modified
				/*if here the L2 cache has already written back, send down so the flush can complete*/
				message_packet->size = 1;
				message_packet->cache_block_state = cgm_cache_block_invalid;

				//set access type inval_ack
				message_packet->access_type = cgm_access_flush_block_ack;

				//reply to the L3 cache
				cache_put_io_down_queue(cache, message_packet);
			}
			break;


		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			/*if the block is found in the L2 it may or may not be in the L1 cache
			we must invalidate here and send an invalidation to the L1 D cache*/
			assert(victim_trainsient_state != cgm_cache_block_transient);

			if(sharers == 1)
			{
				/*assert(cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way) == 0);*/
				if(cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way) == 1)
				{
					int num_pending_requests = 0;

					do
					{	/*There are pending request for this block, we need to nack them now then evict.*/
						pending_request_packet = cache_search_pending_request_buffer(cache, message_packet->address);

						/*nack the packet*/
						if(pending_request_packet)
						{
							assert(pending_request_packet->access_type == cgm_access_get || pending_request_packet->access_type == cgm_access_getx);

							num_pending_requests++;

							//send the reply up as a NACK!
							if(pending_request_packet->access_type == cgm_access_get)
								pending_request_packet->access_type = cgm_access_get_nack;
							else
								pending_request_packet->access_type = cgm_access_getx_nack;

							//set message package size
							pending_request_packet->size = 1;

							/*reset mp flags*/
							assert(pending_request_packet->coalesced == 0);
							assert(pending_request_packet->assoc_conflict == 0);

							pending_request_packet = list_remove(cache->pending_request_buffer, pending_request_packet);

							list_enqueue(cache->Tx_queue_top, pending_request_packet);
							advance(cache->cache_io_up_ec);

						}

					} while(pending_request_packet);

					assert(num_pending_requests == 1);
				}

				/*evict block here*/
				cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->way,
						cgm_cache_get_num_shares(gpu, cache, message_packet->set, message_packet->way), 0);

				//clear the directory entry
				cgm_cache_clear_dir(cache,  message_packet->set, message_packet->way); /*NOTE CLEARS THE PENDING BIT*/

				//search WB again, because the evict would drop this cache line into the WB buffer..
				//star todo find a better way to do this...
				wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

				assert(wb_packet);
				wb_packet->L3_flush_join = 1;

				message_packet = list_remove(cache->last_queue, message_packet);
				list_enqueue(cache->pending_request_buffer, message_packet);
			}
			else
			{
				//block is not up in one of the vector caches...
				assert(sharers == 0);

				assert(cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way) == 0);

				/*evict block here*/
				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

				//clear the directory entry
				cgm_cache_clear_dir(cache,  message_packet->set, message_packet->way);

				if(*cache_block_state_ptr == cgm_cache_block_modified)
				{
					message_packet->size = cache->block_size;
					message_packet->cache_block_state = cgm_cache_block_modified;
				}
				else
				{
					message_packet->size = 1;
					message_packet->cache_block_state = cgm_cache_block_invalid;
				}

				message_packet->access_type = cgm_access_flush_block_ack;

				//reply to the L3 cache
				cache_put_io_down_queue(cache, message_packet);

			}

			break;

		case cgm_cache_block_shared:

			fatal("cgm_mesi_gpu_l2_flush_block(): blk is shared\n");

			//special case...
			if(victim_trainsient_state == cgm_cache_block_transient
					&& ort_get_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set) == 0
					&& cgm_cache_get_block_upgrade_pending_bit(cache, message_packet->set, message_packet->way) == 1)
			{

				warning("block 0x%08x %s flush block is shared and transient ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);

				/*There is an outstanding request to L3 and it is an upgrade
				depending on the state, L3 may show that the block is modified
				send ack to L3*/

				assert(victim_trainsient_state == cgm_cache_block_transient);

				message_packet->size = 1;
				message_packet->cache_block_state = cgm_cache_block_invalid;

				//set access type inval_ack
				message_packet->access_type = cgm_access_flush_block_ack;

				l3_cache_ptr = cgm_l3_cache_map(message_packet->set);
				message_packet->l2_cache_id = cache->id;
				message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

				SETROUTE(message_packet, cache, l3_cache_ptr)

				//reply to the L3 cache
				cache_put_io_down_queue(cache, message_packet);

				DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s flush block is shared and transient ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);

				/*evict block here*/
				cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->way, 0, message_packet->way);

				/*block should "still" be transient*/
				assert(victim_trainsient_state == cgm_cache_block_transient);

			}
			else
			{
				/*block is shared drop it no need to send ack to L3 as there is no pending flush in WB*/
				/*evict block here*/

				cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->way, 0, message_packet->way);
				message_packet->l2_victim_way = message_packet->way;

				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);
			}

			break;
	}

	return;
}


void cgm_mesi_gpu_l2_flush_block_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *wb_packet = NULL;
	struct cgm_packet_t *pending_request_packet = NULL;
	struct cgm_packet_t *reply_packet = NULL;
	struct cache_t *l3_cache_ptr = NULL;
	//int l3_map = 0;
	int error = 0;
	int pending_bit = 0;

	//charge delay
	GPU_PAUSE(cache->latency);

	//flush block ack from L1 D cache...

	//get the address set and tag
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	if(*cache_block_hit_ptr == 1)
		assert(pending_bit == 1);

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);


	error = cache_validate_block_flushed_from_gpu_l1(gpu_v_caches, message_packet->address);
	if(error != 0)
	{
		struct cgm_packet_t *L1_wb_packet = cache_search_wb(&gpu_v_caches[message_packet->l1_cache_id], message_packet->tag, message_packet->set);

		if(L1_wb_packet)
			fatal("wbp found %llu\n", L1_wb_packet->evict_id);


		fatal("cgm_mesi_gpu_l2_flush_block_ack(): %s error %d as %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
			cache->name, error, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
			message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
			message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);
	}

	assert(error == 0);

	//state should be either invalid of modified.
	assert(message_packet->cache_block_state == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_invalid);

	//find the block in the local WB cache_get_block_statusbuffer
	wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);


	if(message_packet->cache_block_state == cgm_cache_block_modified)
		assert(message_packet->size == 64);

	if(*cache_block_state_ptr == 1 && wb_packet)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			cgm_cache_dump_set(cache, message_packet->set);
			printf("\n");

			ort_dump(cache);
			printf("\n");

			cache_dump_queue(cache->pending_request_buffer);
			printf("\n");

			cache_dump_queue(cache->write_back_buffer);
			printf("\n");

			fatal("block 0x%08x %s *cache_block_state_ptr != 0 ID %llu type %d state %d set %d tag %d way %d cycle %llu\n",
				(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id, message_packet->access_type, *cache_block_state_ptr,
				message_packet->set, message_packet->tag, message_packet->way, P_TIME);
		}
	}


	/*if a L1 flush check write back*/
	if(wb_packet)
	{

		DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s flush blk ack with wb in l2 flush_join %d pending bit %d ID %llu type %d state %d cycle %llu\n",
				(message_packet->address & cache->block_address_mask), cache->name, wb_packet->L3_flush_join, pending_bit, message_packet->evict_id,
				message_packet->access_type, *cache_block_state_ptr, P_TIME);


		if(wb_packet->L3_flush_join == 0)
		{
			/*if there is a pending get_fwd or getx_fwd request join here*/
			pending_request_packet = cache_search_pending_request_buffer(cache, message_packet->address);

			if(pending_request_packet)
			{

				fatal("here 0!!!?\n");

				//printf("%s processing get/getx_fwd join after evict access id %llu access type %d\n", cache->name, pending_request_packet->access_id, pending_request_packet->access_type);

				assert(pending_request_packet->access_type == cgm_access_get_fwd || pending_request_packet->access_type == cgm_access_getx_fwd);
				assert((pending_request_packet->address & cache->block_address_mask) == (message_packet->address & cache->block_address_mask));
				assert(pending_request_packet->start_cycle != 0);
				assert(pending_request_packet->set == message_packet->set);

				//prepare to forward the block
				if(pending_request_packet->access_type == cgm_access_get_fwd)
				{	//set access type
					pending_request_packet->access_type = cgm_access_puts;
					//set the block state
					pending_request_packet->cache_block_state = cgm_cache_block_shared;

				}
				else
				{
					/*OK to putx and modified because the getx means intent to store.*/
					//set access type
					pending_request_packet->access_type = cgm_access_putx;
					//set the block state
					pending_request_packet->cache_block_state = cgm_cache_block_modified;
				}

				//set message package size
				pending_request_packet->size = l2_caches[str_map_string(&l2_strn_map, pending_request_packet->l2_cache_name)].block_size;

				//fwd block to requesting core
				//update routing headers swap dest and src
				//requesting node

				struct cache_t *requesting_ptr = &l2_caches[str_map_string(&l2_strn_map, pending_request_packet->src_name)];

				SETROUTE(pending_request_packet, cache, requesting_ptr)

				//transmit block to requesting node
				pending_request_packet = list_remove(cache->pending_request_buffer, pending_request_packet);
				list_enqueue(cache->Tx_queue_bottom, pending_request_packet);
				advance(cache->cache_io_down_ec);

				///////////////
				//downgrade/getx_inval_ack
				///////////////

				//send the downgrade/getx_inval ack to L3 cache.

				//create downgrade_ack
				reply_packet = packet_create();
				assert(reply_packet);

				/*NOTE access type changed above!!!*/
				if(pending_request_packet->access_type ==  cgm_access_puts)
				{
					init_downgrade_ack_packet(reply_packet, pending_request_packet->address);
				}
				else
				{
					init_getx_fwd_ack_packet(reply_packet, pending_request_packet->address);
				}

				//determine if we need to send dirty data to L3

				/*if(message_packet->cache_block_state != cgm_cache_block_modified || wb_packet->cache_block_state != cgm_cache_block_modified)
				{
					warning("bug is here block 0x%08x %s downgrade ID %llu type %d mp state %d wb state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name,
							message_packet->access_id, message_packet->access_type, message_packet->cache_block_state, wb_packet->cache_block_state, P_TIME);

				}*/

				//assert(message_packet->cache_block_state == cgm_cache_block_modified || wb_packet->cache_block_state == cgm_cache_block_modified);
				if(message_packet->cache_block_state == cgm_cache_block_modified || wb_packet->cache_block_state == cgm_cache_block_modified)
				{
					reply_packet->cache_block_state = cgm_cache_block_modified;
					reply_packet->size = l2_caches[str_map_string(&l2_strn_map, pending_request_packet->l2_cache_name)].block_size;
				}
				else
				{
					reply_packet->cache_block_state = cgm_cache_block_shared;
					reply_packet->size = 1;
				}

				//fwd reply (downgrade_ack) to L3
				l3_cache_ptr = cgm_l3_cache_map(pending_request_packet->set);

				//fakes src as the requester
				/*reply_packet->l2_cache_id = l2_caches[my_pid].id;*/
				reply_packet->l2_cache_id = pending_request_packet->l2_cache_id;
				reply_packet->l2_cache_name = pending_request_packet->src_name;

				SETROUTE(reply_packet, cache, l3_cache_ptr)

				//transmit downgrad_ack to L3 (home)
				list_enqueue(cache->Tx_queue_bottom, reply_packet);
				advance(cache->cache_io_down_ec);

				wb_packet = list_remove(cache->write_back_buffer, wb_packet);
				packet_destroy(wb_packet);

				//destroy the downgrade message because we don't need it anymore.
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);

			}
			else
			{
				//incoming data from L1 is dirty
				if(wb_packet->cache_block_state == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_modified)
				{
					//merge the block.
					wb_packet->cache_block_state = cgm_cache_block_modified;

					//clear the pending bit and leave the wb in the buffer
					wb_packet->flush_pending = 0;
				}
				else
				{
					//Neither the l1 line or L2 line are dirty clear the wb from the buffer
					assert(wb_packet->cache_block_state == cgm_cache_block_exclusive);
					wb_packet = list_remove(cache->write_back_buffer, wb_packet);
					free(wb_packet);
				}

				//free the flush message packet
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);
			}
		}
		else
		{

			assert(wb_packet->L3_flush_join == 1); /*pull the join if there is one waiting*/

			/*its's possible for L3 to evict a block during the epoch of the L2's block movement.*/
			pending_request_packet = cache_search_pending_request_buffer(cache, message_packet->address);
			assert(pending_request_packet);

			assert(pending_request_packet->access_type == cgm_access_gpu_flush
					|| pending_request_packet->access_type == cgm_access_flush_block);


			//error check should not be a pending get/getx_fwd in the buffer
			error = cache_search_pending_request_get_getx_fwd(cache, (message_packet->address & cache->block_address_mask));
			assert(error == 0);

			wb_packet->flush_pending = 0;
			wb_packet->L3_flush_join = 0;

			if(wb_packet->cache_block_state == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_modified)
			{
				pending_request_packet->size = cache->block_size;
				pending_request_packet->cache_block_state = cgm_cache_block_modified;
			}
			else
			{
				pending_request_packet->size = 1;
				pending_request_packet->cache_block_state = cgm_cache_block_invalid;
			}

			if(pending_request_packet->access_type == cgm_access_gpu_flush)
			{
				pending_request_packet->access_type = cgm_access_gpu_flush_ack;
			}
			else
			{
				pending_request_packet->access_type = cgm_access_flush_block_ack;
			}


			//reply to the L3 cache
			/*fatal("l2_flush_block_ack_here id %llu dest %d cycle %llu\n",
					pending_request_packet->evict_id, pending_request_packet->dest_id, P_TIME);*/

			pending_request_packet = list_remove(cache->pending_request_buffer, pending_request_packet);
			list_enqueue(cache->Tx_queue_bottom, pending_request_packet);
			advance(cache->cache_io_down_ec);

			//free the write back
			wb_packet = list_remove(cache->write_back_buffer, wb_packet);
			packet_destroy(wb_packet);

			//free the message packet
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

		}
	}
	else
	{

		DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s flush blk ack without wb in l2 ID %llu type %d state %d cycle %llu\n",
				(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);

		/*if the eviction goes up and finds the block shared its dropped in L1.
		if the block is not in cache or WB at L1 we don't know if there is a M line coming down
		because the block was exclusive in L2. So Let's see what we got*/

		/*case L3 flushed L2 then L2 flushed L1 and now back to L2 and there is no WB waiting at L2*/

		/*case GPU only, GPU caches are mapped many to 1. its possible that L2 is flushing a compute unit
		 to service the request of another compute unit. check the pending bit and pending request buffer */


		/*GPU CASE*/
		if(pending_bit == 1)
		{
			//check for a pending request....
			pending_request_packet = cache_search_pending_request_buffer(cache, message_packet->address);

			assert(*cache_block_hit_ptr == 1);
			assert(*cache_block_state_ptr == cgm_cache_block_modified || *cache_block_state_ptr == cgm_cache_block_exclusive);
			assert(pending_request_packet);
			assert(pending_request_packet->set == message_packet->set && pending_request_packet->way == message_packet->way);

			if(message_packet->cache_block_state == cgm_cache_block_modified)
			{
				cgm_cache_set_block_state(cache, pending_request_packet->set, pending_request_packet->way, cgm_cache_block_modified);
			}

			if(*cache_block_state_ptr == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_modified)
			{
				pending_request_packet->access_type = cgm_access_putx;
				pending_request_packet->cache_block_state = cgm_cache_block_modified;
			}
			else
			{
				pending_request_packet->access_type = cgm_access_put_clnx;
				pending_request_packet->cache_block_state = cgm_cache_block_exclusive;
			}

			pending_request_packet->size = gpu_v_caches[pending_request_packet->l1_cache_id].block_size;

			//clear the old directory entry NOTE this clears the pending bit
			cgm_cache_clear_dir(cache, pending_request_packet->set, pending_request_packet->way);

			//set the new directory entry
			cgm_cache_set_dir(cache, pending_request_packet->set, pending_request_packet->way, pending_request_packet->l1_cache_id);

			/*pull the pending request and send to L1 cache*/
			pending_request_packet = list_remove(cache->pending_request_buffer, pending_request_packet);

			list_enqueue(cache->Tx_queue_top, pending_request_packet);
			advance(cache->cache_io_up_ec);

			/*printf("joined after flush evict id %llu cycle %llu\n", message_packet->evict_id, P_TIME);*/
		}
		else
		{
			//check for a pending request....
			pending_request_packet = cache_search_pending_request_buffer(cache, message_packet->address);

			if(pending_request_packet)
				fatal("block 0x%08x %s flush blk ack pending packet l2 mp_evict_id %llu type %d state %d pending id %llu type %d l1_id %d cycle %llu\n",
				(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id, message_packet->access_type, *cache_block_state_ptr,
				pending_request_packet->access_id, pending_request_packet->access_type, pending_request_packet->l1_cache_id, P_TIME);

			assert(!pending_request_packet);
		}

		//free the message packet
		message_packet = list_remove(cache->last_queue, message_packet);
		packet_destroy(message_packet);
	}

	/*stats*/
	cache->EvictInv++;

	return;
}



int cgm_mesi_gpu_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int row = 0;
	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	enum cgm_cache_block_state_t victim_trainsient_state;
	//struct cgm_packet_t *pending_get_fwd_getx_fwd_request = NULL;
	int pending_join_bit = -1;
	int pending_upgrade_bit = -1;

	struct cgm_packet_t *pending_upgrade = NULL;
	struct cgm_packet_t *pending_get_getx_fwd = NULL;
	struct cgm_packet_t *pending_request = NULL;

	cache_get_transient_block(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	if(*cache_block_hit_ptr != 1)
	{
		cgm_cache_dump_set(cache, message_packet->set);

		warning("block 0x%08x %s write block ID %llu type %s state %d way %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
			str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->way, message_packet->cache_block_state, P_TIME);
		fflush(stderr);
	}

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);

	//victim should have been in transient state
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	assert(victim_trainsient_state == cgm_cache_block_transient);

	/*assert((message_packet->access_type == cgm_access_puts && message_packet->cache_block_state == cgm_cache_block_shared)
			|| (message_packet->access_type == cgm_access_put_clnx && message_packet->cache_block_state == cgm_cache_block_exclusive)
			|| (message_packet->access_type == cgm_access_putx && message_packet->cache_block_state == cgm_cache_block_modified));*/

	DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s write block ID %llu type %d state %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
			message_packet->access_type, message_packet->cache_block_state, P_TIME);

	/*check the ort table for a pending join get/getx_fwd*/
	row = ort_search(cache, message_packet->tag, message_packet->set);
	assert(row != cache->mshr_size);
	pending_join_bit = cache->ort[row][2];

	pending_upgrade_bit = cgm_cache_get_block_upgrade_pending_bit(cache, message_packet->set, message_packet->way);

	/*reply has returned for a previously sent gets/get/getx*/
	if(pending_join_bit == 1 && pending_upgrade_bit == 0)
	{
		ort_clear(cache, message_packet);

		cgm_cache_set_block(cache, message_packet->set, message_packet->way, message_packet->tag, message_packet->cache_block_state);

		//set retry state
		message_packet->access_type = cgm_cache_get_retry_state(message_packet->gpu_access_type);

		message_packet = list_remove(cache->last_queue, message_packet);
		list_enqueue(cache->retry_queue, message_packet);
	}

	/*We have a pending upgrade packet*/
	else if(pending_join_bit == 1 && pending_upgrade_bit == 1)
	{
		//pull the pending request from the buffer
		pending_upgrade = cache_search_pending_request_buffer(cache, (message_packet->address & cache->block_address_mask));

		/*OK this is a corner case. we sent an upgrade but received a putx clear the pending upgrade data before proceeding.*/
		if(!pending_upgrade)
		{
			printf("pending_upgrade_bit %d pending queue size %d\n", pending_upgrade_bit, list_count(cache->pending_request_buffer));

			cgm_cache_dump_set(cache, message_packet->set);

			cache_dump_queue(cache->pending_request_buffer);

			warning("block 0x%08x %s write block packet failure ID %llu type %s set %d tag %d way %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
					str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->set, message_packet->tag,
					message_packet->way, message_packet->cache_block_state, P_TIME);

		}

		assert(pending_upgrade);
		assert(pending_upgrade->access_type == cgm_access_upgrade);
		assert(pending_upgrade->address == message_packet->address); //address should match to the byte because this is the reply to the upgrade

		//clear the cache block pending bit
		cgm_cache_clear_block_upgrade_pending_bit(cache, message_packet->set, message_packet->way);

		//write the block
		cgm_cache_set_block(cache, message_packet->set, message_packet->way, message_packet->tag, message_packet->cache_block_state);

		//set retry state
		pending_upgrade->access_type = cgm_cache_get_retry_state(pending_upgrade->gpu_access_type);
		assert(pending_upgrade->data && pending_upgrade->event_queue);

		//remove the pending request (upgrade)
		pending_upgrade = list_remove(cache->pending_request_buffer, pending_upgrade);
		list_enqueue(cache->retry_queue, pending_upgrade);

		ort_clear(cache, message_packet);

		//destroy the putx
		message_packet = list_remove(cache->last_queue, message_packet);
		packet_destroy(message_packet);

	}

	//We have a eviction conflict OR a pending get/getx_fwd packet
	else if(pending_join_bit == 0 && pending_upgrade_bit == 0)
	{
		/*ORT is set, this means either a pending request is in OR L3 evicted the line*/
		//first look for a pending request in the buffer
		pending_get_getx_fwd = cache_search_pending_request_buffer(cache, (message_packet->address & cache->block_address_mask));

		if(!pending_get_getx_fwd)
		{
			/*printf("\n");

			ort_dump(cache);

			printf("\n");
			cgm_cache_dump_set(cache, message_packet->set);

			printf("\n");
			cache_dump_queue(cache->pending_request_buffer);*/

			/*printf("\n");*/
			/*warning("block 0x%08x %s write block case two ID %llu cpu_access type %d type %d state %d set %d tag %d way %d pj_bit %d pu_bit %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->cpu_access_type,
			message_packet->access_type, message_packet->cache_block_state,
			message_packet->set, message_packet->tag, message_packet->way,
			pending_join_bit, pending_upgrade_bit, P_TIME);*/

		}

		if(pending_get_getx_fwd)
		{
			assert(pending_get_getx_fwd->access_type == cgm_access_get_fwd || pending_get_getx_fwd->access_type == cgm_access_getx_fwd);
			assert((pending_get_getx_fwd->address & cache->block_address_mask) == (message_packet->address & cache->block_address_mask));
			assert(pending_get_getx_fwd->downgrade_pending == 1);

			/*set the number of coalesced accesses*/
			pending_get_getx_fwd->downgrade_pending = (ort_get_num_coal(cache, message_packet->tag, message_packet->set) + 1); // + 1 account for the packet that was not coalesced and went to L3
		}
		else
		{
			assert(message_packet->access_type == cgm_access_put_clnx || message_packet->access_type == cgm_access_putx
					|| message_packet->access_type == cgm_access_puts);
		}

		//write the block
		cgm_cache_set_block(cache, message_packet->set, message_packet->way, message_packet->tag, message_packet->cache_block_state);

		//set retry state
		message_packet->access_type = cgm_cache_get_retry_state(message_packet->gpu_access_type);

		//clear the ort
		ort_clear(cache, message_packet);

		//remove the pending request (upgrade)
		message_packet = list_remove(cache->last_queue, message_packet);
		list_enqueue(cache->retry_queue, message_packet);

	}

	//We have both a pending upgrade packet and a pending get/getx_fwd packet
	else if(pending_join_bit == 0 && pending_upgrade_bit == 1)
	{
		/*ort_dump(cache);
		cgm_cache_dump_set(cache, message_packet->set);
		cache_dump_queue(cache->pending_request_buffer);*/

		/*warning("CASE FOUR: block 0x%08x %s write block case four ID %llu type %d state %d set %d tag %d way %d pj_bit %d pu_bit %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
			message_packet->access_type, message_packet->cache_block_state,
			message_packet->set, message_packet->tag, message_packet->way,
			pending_join_bit, pending_upgrade_bit, P_TIME);*/


		//there are TWO pending requests in the buffer or there is an upgrade and a conflict (L3 evicted).
		do{

			pending_request = cache_search_pending_request_buffer(cache, (message_packet->address & cache->block_address_mask));

			assert(pending_request);
			assert(pending_request->access_type == cgm_access_upgrade || pending_request->access_type == cgm_access_get_fwd || pending_request->access_type == cgm_access_getx_fwd);
			assert((pending_request->address & cache->block_address_mask) == (message_packet->address & cache->block_address_mask));

			if(pending_request->access_type == cgm_access_upgrade)
			{
				pending_upgrade = list_remove(cache->pending_request_buffer, pending_request);
			}
			else if(pending_request->access_type == cgm_access_get_fwd || pending_request->access_type == cgm_access_getx_fwd)
			{
				pending_get_getx_fwd = list_remove(cache->pending_request_buffer, pending_request);
			}
			else
			{
				fatal("cgm_mesi_l2_write_block(): case four pending request something other than upgrade or get/getx_fwd\n");
			}

			/*loop++;

			if(loop > 4)
			{
				ort_dump(cache);
				cgm_cache_dump_set(cache, message_packet->set);
				cache_dump_queue(cache->pending_request_buffer);
				fatal("here the address is 0x%08x\n", (message_packet->address & cache->block_address_mask));
			}

			printf("inf loop\n");*/

		}while(pending_upgrade == NULL || pending_get_getx_fwd == NULL);

		//just for my sanity
		assert(pending_upgrade->access_type == cgm_access_upgrade);
		assert(pending_get_getx_fwd->access_type == cgm_access_get_fwd || pending_get_getx_fwd->access_type == cgm_access_getx_fwd);

		///////////////
		//get_getx_fwd
		///////////////

		//process the pending get/getx_fwd first
		assert(pending_get_getx_fwd->downgrade_pending == 1);

		/*set the number of coalesced accesses and leave in buffer*/
		pending_get_getx_fwd->downgrade_pending = (ort_get_num_coal(cache, message_packet->tag, message_packet->set) + 1); // + 1 account for the packet that was not coalesced and went to L3

		/*star todo fix this, we have to remove and insert here because the
		cache_search_pending_request_buffer() looks for address and then blk*/
		list_enqueue(cache->pending_request_buffer, pending_get_getx_fwd);

		///////////////
		//join upgrade
		///////////////


		//clear the cache block pending bit
		cgm_cache_clear_block_upgrade_pending_bit(cache, message_packet->set, message_packet->way);

		//write the block
		cgm_cache_set_block(cache, message_packet->set, message_packet->way, message_packet->tag, message_packet->cache_block_state);

		//set retry state
		pending_upgrade->access_type = cgm_cache_get_retry_state(pending_upgrade->gpu_access_type);
		assert(pending_upgrade->data && pending_upgrade->event_queue);

		list_enqueue(cache->retry_queue, pending_upgrade);

		ort_clear(cache, message_packet);

		//destroy the putx
		message_packet = list_remove(cache->last_queue, message_packet);
		packet_destroy(message_packet);

	}
	else
	{
		warning("block 0x%08x %s write block bad case ID %llu type %d state %d set %d tag %d way %d pj_bit %d pu_bit %d cycle %llu\n",
			(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
			message_packet->access_type, message_packet->cache_block_state,
			message_packet->set, message_packet->tag, message_packet->way,
			pending_join_bit, pending_upgrade_bit, P_TIME);

		fatal("cgm_mesi_l2_write_block(): bad case\n");

	}

	/*stats*/
	cache->TotalWriteBlocks++;

	return 1;
}



int cgm_mesi_gpu_l2_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;
	struct cgm_packet_t *wb_packet;
	//enum cgm_cache_block_state_t block_trainsient_state;
	//int l3_map;
	int error = 0;
	struct cache_t *l3_cache_ptr = NULL;

	//charge the delay
	GPU_PAUSE(cache->latency);

	//we should only receive modified lines from L1 cache
	assert(message_packet->cache_block_state == cgm_cache_block_modified);

	//get the state of the local cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	/*reset mp flags*/
	assert(message_packet->coalesced == 0);
	assert(message_packet->assoc_conflict == 0);

	//check for block transient state
	/*block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	if(block_trainsient_state == cgm_cache_block_transient)
	{
		if potentially merging in cache the block better not be transient, check that the tags don't match
		if they don't match the block is missing from both the cache and wb buffer when it should not be

		//check that the tags don't match. This should not happen as the request should have been coalesced at L1 D.
		assert(message_packet->tag != cache->sets[message_packet->set].blocks[message_packet->way].tag);
	}*/

	/*on a write back with inclusive caches L2 Merges the line
	if the write back is a surprise the block will be exclusive and old in the L2 cache.*/

	//WB from L1 D cache
	if(cache->last_queue == cache->Rx_queue_top)
	{

		assert(message_packet->size == 64);

		switch(*cache_block_state_ptr)
		{
			case cgm_cache_block_noncoherent:
			case cgm_cache_block_owned:
			case cgm_cache_block_shared:
				cgm_cache_dump_set(cache, message_packet->set);

				fatal("cgm_mesi_l2_write_back(): %s invalid block state on write back as %s wb_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
					cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
					message_packet->write_back_id, message_packet->address, message_packet->address & cache->block_address_mask,
					message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);
				break;

			case cgm_cache_block_invalid:

				//check WB buffer
				wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

				if(wb_packet)
				{
					DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s write back - write back merge ID %llu type %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);

					//cache block found in the WB buffer merge the change here
					//set modified if the line was exclusive
					wb_packet->cache_block_state = cgm_cache_block_modified;
					/*if(wb_packet->flush_pending != 1)
						fatal("l2 block address 0x%08x\n", message_packet->address & cache->block_address_mask);*/

					//assert(wb_packet->flush_pending == 1);

					//destroy the L1 D WB packet
					message_packet = list_remove(cache->last_queue, message_packet);
					packet_destroy(message_packet);
				}
				else
				{

					fatal("cgm_mesi_l2_write_back(): miss in L2 write back should no longer happen??\n");

					/*this case shouldn't happen any longer with the new changes.*/
					//cgm_cache_dump_set(cache, message_packet->set);
					/* cache_dump_queue(cache->write_back_buffer);


					fatal("cgm_mesi_l2_write_back(): %s write back missing in cache %s writeback_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
						cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
						message_packet->write_back_id, message_packet->address, message_packet->address & cache->block_address_mask,
						message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);*/

					//fatal("cgm_mesi_l2_write_back(): block not in cache or wb at L2 on L1 WB. this should not be happening anymore\n");

					/*it is possible for the WB from L1 D to miss at the L2. This means there was a recent L2 eviction of the block*/

				}
				break;

			case cgm_cache_block_exclusive:
			case cgm_cache_block_modified:

				//hit in cache merge write back here.

				DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s write back - cache merge ID %llu type %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);


				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);

				//clear the directory for another core to pull, but only if not already pending...
				if(cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way) != 1)
					cgm_cache_clear_dir(cache,  message_packet->set, message_packet->way);

				error = cache_validate_block_flushed_from_gpu_l1(gpu_v_caches, message_packet->address);
				if(error != 0)
				{
					struct cgm_packet_t *L1_wb_packet = cache_search_wb(&gpu_v_caches[cache->id], message_packet->tag, message_packet->set);

					if(L1_wb_packet)
						warning("wbp found %llu\n", L1_wb_packet->evict_id);


					fatal("cgm_mesi_gpu_l2_write_back(): %s error %d as %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
						cache->name, error, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
						message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
						message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);

				}
				assert(error == 0);

				//destroy the L1 D WB message. L2 will clear its WB at an opportune time.
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);
				break;
		}

		/*stats*/
		cache->TotalWriteBackRecieved++;

	}
	//if here the L2 generated it's own write back.
	else if(cache->last_queue == cache->write_back_buffer)
	{
		//the wb should not be waiting on a flush to finish.
		assert(message_packet->flush_pending == 0); //verify that the wb has completed it's flush.
		assert(*cache_block_hit_ptr == 0); //verify block is not in cache.

		//verify that the block is out of L1
		error = cache_validate_block_flushed_from_gpu_l1(gpu_v_caches, message_packet->address);
		assert(error == 0);

		//verify that there is only one wb in L2 for this block.
		error = cache_search_wb_dup_packets(cache, message_packet->tag, message_packet->set);
		assert(error == 1); //error == 1 i.e only one wb packet and we are about to send it.

		//if the line is still in the exclusive state at this point drop it.
		if(message_packet->cache_block_state == cgm_cache_block_exclusive)
		{

			DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s write back destroy ID %llu type %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);


			/*stats*/
			cache->TotalWriteBackDropped++;

			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);
		}
		else if (message_packet->cache_block_state == cgm_cache_block_modified)
		{
			DEBUG(LEVEL == 2 || LEVEL == 3, "block 0x%08x %s write back sent (to L3) %llu type %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);

			message_packet->size = cache->block_size;

			if(hub_iommu_connection_type == hub_to_mc)
			{
				//message is going down to mc so its and mc_load
				message_packet->access_type = cgm_access_mc_store;

				message_packet->l2_cache_id = cache->id;
				message_packet->l2_cache_name = cache->name;

				SETROUTE(message_packet, cache, system_agent)

			}
			else
			{

				l3_cache_ptr = cgm_l3_cache_map(message_packet->set);
				message_packet->l2_cache_id = cache->id;
				message_packet->l2_cache_name = cache->name;

				SETROUTE(message_packet, cache, l3_cache_ptr)
			}


			//send the write back on.
			cache_put_io_down_queue(cache, message_packet);

			/*stats*/
			cache->TotalWriteBackSent++;
		}
		else
		{
			fatal("cgm_mesi_l2_write_back(): Invalid block state in write back buffer cycle %llu\n", P_TIME);
		}

		return 0;
	}
	else
	{
		fatal("cgm_mesi_l2_write_back(): Invalid queue cycle %llu\n", P_TIME);
	}

	return 1;
}
