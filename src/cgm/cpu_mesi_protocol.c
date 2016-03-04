/*
 * mesi_protocol.c
 *
 *  Created on: Oct 22, 2015
 *      Author: stardica
 */

//////////////////////
/////CPU MESI protocol
//////////////////////

#include <cgm/protocol.h>

unsigned int get_block_address(unsigned int address, unsigned int cache_address_mask){

	return address & cache_address_mask;
}

int is_writeback_present(struct cgm_packet_t *writeback_packet){

	int return_val = 0;
	(writeback_packet == NULL) ? (return_val = 0) : (return_val = 1);
	return return_val;
}


void cgm_mesi_fetch(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("l1 i %d fetching\n", cache->id);
	STOP;*/

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;
	struct cgm_packet_t *write_back_packet = NULL;

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);
	assert(!write_back_packet);

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	/*stats*/
	if(*cache_block_hit_ptr == 0)
	{
		cache->TotalMisses++;
		cache->TotalReadMisses++;
		cache->TotalGets++;
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_modified:
		case cgm_cache_block_owned:
		case cgm_cache_block_exclusive:
			fatal("l1_i_cache_ctrl(): Invalid block state on hit as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		//miss or invalid cache block states
		case cgm_cache_block_invalid:

			//charge delay on a miss
			P_PAUSE(cache->latency);

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
			{
				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 1 || LEVEL == 3)
					{
						printf("block 0x%08x %s fetch miss coalesce ID %llu type %d state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
					}
				}

				return;
			}

			//add some routing/status data to the packet
			message_packet->access_type = cgm_access_gets;
			message_packet->l1_access_type = cgm_access_gets;

			//find victim and evict on return l1_i_cache just drops the block on return
			//message_packet->l1_victim_way = cgm_cache_replace_block(cache, message_packet->set);
			message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 1 || LEVEL == 3)
				{
					printf("block 0x%08x %s fetch miss ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}

			//evict the block
			assert(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l1_victim_way) == cgm_cache_block_shared
					|| cgm_cache_get_block_state(cache, message_packet->set, message_packet->l1_victim_way) == cgm_cache_block_invalid);
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->l1_victim_way, cgm_cache_block_invalid);

			//transmit to L2
			cache_put_io_down_queue(cache, message_packet);
			break;

		case cgm_cache_block_shared:

			/*stats*/

			//set retry state and delay
			if(message_packet->access_type == cgm_access_fetch_retry || message_packet->coalesced == 1)
			{
				//charge a delay only in the event of a retry.
				P_PAUSE(cache->latency);

				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 1 || LEVEL == 3)
				{
					printf("block 0x%08x %s fetch hit ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}

			/*stats*/
			message_packet->end_cycle = P_TIME;
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L1_hit;
			cache_l1_i_return(cache,message_packet);
			break;

	}

	return;
}

void cgm_mesi_l1_i_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//check the packet for integrity
	assert(cache->cache_type == l1_i_cache_t);
	assert(message_packet->access_type == cgm_access_puts && message_packet->cache_block_state == cgm_cache_block_shared);

	//find the access in the ORT table and clear it.
	ort_clear(cache, message_packet);

	//set the block and retry the access in the cache.
	/*cache_put_block(cache, message_packet);*/

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 1 || LEVEL == 3)
		{
			printf("block 0x%08x %s fetch miss ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, message_packet->cache_block_state, P_TIME);
		}
	}

	cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	return;
}


void cgm_mesi_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("l1 d %d load\n", cache->id);
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


	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	/*stats*/
	if(*cache_block_hit_ptr == 0)
	{
		cache->TotalMisses++;
		cache->TotalReadMisses++;
		cache->TotalGet++;
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_owned:
		case cgm_cache_block_noncoherent:
			fatal("l1_d_cache_ctrl(): Invalid block state on load hit as \"%s\"\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		//miss or invalid cache block states
		case cgm_cache_block_invalid:

			//charge delay
			P_PAUSE(cache->latency);

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

					assert(message_packet->coalesced == 1);
					if(message_packet->coalesced == 1)
					{
						return;
					}
					else
					{
						fatal("cgm_mesi_load(): write failed to coalesce when all ways are transient...\n");
					}
				}

				/*stats star todo find a better way to take this stat...*/
				cache->TotalMisses--;
				cache->TotalReadMisses--;
				cache->TotalGet--;

				//we are writting in a block so evict the victim
				assert(write_back_packet->l1_victim_way >= 0 && write_back_packet->l1_victim_way < cache->assoc);

				//first evict the old block if it isn't invalid already
				if(cgm_cache_get_block_state(cache, write_back_packet->set, write_back_packet->l1_victim_way) != cgm_cache_block_invalid)
					cgm_L1_cache_evict_block(cache, write_back_packet->set, write_back_packet->l1_victim_way);

				//now set the block
				cgm_cache_set_block(cache, write_back_packet->set, write_back_packet->l1_victim_way, write_back_packet->tag, write_back_packet->cache_block_state);

				//free the write back
				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 1 || LEVEL == 3)
					{
						printf("block 0x%08x %s load wb hit id %llu state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);
					}
				}

				//check for retries on successful cache read...
				if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
				{
					//enter retry state.
					cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
				}

				/*stats*/
				message_packet->end_cycle = P_TIME;
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L1_hit;

				cache_l1_d_return(cache, message_packet);
				return;
			}
			else
			{
				//block isn't in the cache or in write back.

				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
				{
					if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
					{
						if(LEVEL == 1 || LEVEL == 3)
						{
							printf("block 0x%08x %s load miss coalesce ID %llu type %d state %d cycle %llu\n",
									(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
						}
					}
					return;
				}

				//add some routing/status data to the packet
				message_packet->access_type = cgm_access_get;
				message_packet->l1_access_type = cgm_access_get;

				//find victim
				message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);

				/*	message_packet->l1_victim_way = cgm_cache_replace_block(cache, message_packet->set);*/
				assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

				if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l1_victim_way) != cgm_cache_block_invalid)
					cgm_L1_cache_evict_block(cache, message_packet->set, message_packet->l1_victim_way);

				//transmit to L2
				cache_put_io_down_queue(cache, message_packet);

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 1 || LEVEL == 3)
					{
						printf("block 0x%08x %s load miss id %llu state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);
					}
				}
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
				assert(*cache_block_state_ptr == cgm_cache_block_shared);

				//charge delay
				P_PAUSE(cache->latency);

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 1 || LEVEL == 3)
					{
						printf("block 0x%08x %s load hit coalesce ID %llu type %d state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
					}
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

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 1 || LEVEL == 3)
				{
					printf("block 0x%08x %s load hit ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}

			//there are no pending accesses, we can continue and finish the load.

			//set the retry state and charge latency
			if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//charge a delay only on retry state.
				P_PAUSE(cache->latency);

				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			/*stats*/
			message_packet->end_cycle = P_TIME;
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L1_hit;

			cache_l1_d_return(cache,message_packet);

			break;
	}

	return;
}

void cgm_mesi_store(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("l1 d %d storing\n", cache->id);
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

	//check this...
	if(write_back_packet)
	{
		assert(*cache_block_hit_ptr == 0);
		assert((write_back_packet->cache_block_state == cgm_cache_block_modified
				|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == cgm_cache_block_invalid);

	}

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	/*stats*/
	if(*cache_block_hit_ptr == 0)
	{
		cache->TotalMisses++;
		cache->TotalWriteMisses++;
		cache->TotalGetx++;
	}

	if (*cache_block_state_ptr == cgm_cache_block_shared)
	{
		cache->TotalMisses++;
		cache->TotalWriteMisses++;
		cache->TotalUpgrades++;
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_owned:
		case cgm_cache_block_noncoherent:
			fatal("l1_d_cache_ctrl(): Invalid block state on store hit %s \n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		//miss or invalid cache block state
		case cgm_cache_block_invalid:

			//charge delay
			P_PAUSE(cache->latency);

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

					assert(message_packet->coalesced == 1);
					if(message_packet->coalesced == 1)
					{
						return;
					}
					else
					{
						fatal("cgm_mesi_load(): write failed to coalesce when all ways are transient...\n");
					}
				}

				/*stats*/ //actually a hit...
				cache->TotalMisses--;
				cache->TotalWriteMisses--;
				cache->TotalGetx--;

				//we are writing in a block so evict the victim
				assert(write_back_packet->l1_victim_way >= 0 && write_back_packet->l1_victim_way < cache->assoc);

				//first evict the old block if it isn't invalid already
				if(cgm_cache_get_block_state(cache, write_back_packet->set, write_back_packet->l1_victim_way) != cgm_cache_block_invalid)
					cgm_L1_cache_evict_block(cache, write_back_packet->set, write_back_packet->l1_victim_way);

				cgm_cache_set_block(cache, write_back_packet->set, write_back_packet->l1_victim_way, write_back_packet->tag, cgm_cache_block_modified);

				//free the write back
				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 1 || LEVEL == 3)
					{
						printf("block 0x%08x %s store wb hit id %llu state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);
					}
				}

				//check for retires on successful cache write...
				if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
				{
					//charge a delay only on retry state.
					P_PAUSE(cache->latency);

					//enter retry state.
					cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
				}

				/*stats*/
				message_packet->end_cycle = P_TIME;
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L1_hit;

				cache_l1_d_return(cache,message_packet);
				return;
			}
			else
			{

				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
				{
					if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
					{
						if(LEVEL == 1 || LEVEL == 3)
						{
							printf("block 0x%08x %s store miss coalesce ID %llu type %d state %d cycle %llu\n",
									(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
						}
					}

					return;
				}

				//add some routing/status data to the packet
				message_packet->access_type = cgm_access_getx;
				message_packet->l1_access_type = cgm_access_getx;

				//find victim
				message_packet->l1_victim_way = cgm_cache_get_victim(cache, message_packet->set);

				/*message_packet->l1_victim_way = cgm_cache_replace_block(cache, message_packet->set);*/
				assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

				//evict the block if the data is valid
				if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l1_victim_way) != cgm_cache_block_invalid)
					cgm_L1_cache_evict_block(cache, message_packet->set, message_packet->l1_victim_way);

				//transmit to L2
				cache_put_io_down_queue(cache, message_packet);

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 1 || LEVEL == 3)
					{
						printf("block 0x%08x %s store miss ID %llu type %d state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
					}
				}
			}

			break;

		case cgm_cache_block_shared:

			//this is an upgrade_miss

			//charge delay
			P_PAUSE(cache->latency);

			//put back on the core event queue to end memory system access.
			/*cache_l1_d_return(cache, message_packet);
			return;*/

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
				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 1 || LEVEL == 3)
					{
						printf("block 0x%08x %s upgrade miss coalesce ID %llu type %d state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
					}
				}

				return;
			}

			/*if(message_packet->access_id == 87630)
			{
				cgm_cache_dump_set(cache, message_packet->set);
				printf("%s id %llu upgrade miss set %d way %d tag %d cycle %llu\n",
						cache->name, message_packet->access_id, message_packet->set, message_packet->way, message_packet->tag, P_TIME);
				getchar();
			}*/

			//set block transient state, but don't evict because the block is valid and just needs to be upgraded
			cgm_cache_set_block_transient_state(cache, message_packet->set, message_packet->way, cgm_cache_block_transient);

			//keep the way of the block to upgrade (might come back as a putx in lieu of an upgrade ack)
			message_packet->l1_victim_way = message_packet->way;

			//set access type
			message_packet->access_type = cgm_access_upgrade;

			//transmit upgrade request to L2
			cache_put_io_down_queue(cache, message_packet);

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 1 || LEVEL == 3)
				{
					printf("block 0x%08x %s upgrade miss ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}
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
				//charge the delay only if the retry state is set.
				P_PAUSE(cache->latency);

				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 1 || LEVEL == 3)
				{
					printf("block 0x%08x %s store hit ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}

			/*stats*/
			message_packet->end_cycle = P_TIME;
			if(!message_packet->protocol_case)
					message_packet->protocol_case = L1_hit;

			cache_l1_d_return(cache,message_packet);

			break;
	}

	return;
}



void cgm_mesi_l1_d_downgrade(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	enum cgm_cache_block_state_t block_trainsient_state;

	struct cgm_packet_t *write_back_packet = NULL;

	//charge the delay
	P_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);

	/*unsigned int temp = message_packet->address;
	temp = temp & cache->block_address_mask;
	printf("%s id %llu addr 0x%08x blk addr 0x%08x hit_ptr %d\n", cache->name, message_packet->access_id, message_packet->address, temp, *cache_block_hit_ptr);*/
	assert((*cache_block_hit_ptr == 1 && block_trainsient_state != cgm_cache_block_transient) || (*cache_block_hit_ptr == 0));

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 1 || LEVEL == 3)
		{
			printf("block 0x%08x %s downgrade ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
			cgm_cache_dump_set(cache, message_packet->set);

			unsigned int temp = (unsigned int) 0x000422e4;
			temp = temp & cache->block_address_mask;

			fatal("cgm_mesi_l1_d_downgrade(): L1 id %d invalid block state on downgrade as %s set %d wat %d tag %d address 0x%08x\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->set, message_packet->way, message_packet->tag, temp);
			break;

		case cgm_cache_block_invalid:

			assert(*cache_block_hit_ptr == 0);

			//check write back buffer
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/
				assert((write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive)
						&& *cache_block_state_ptr == cgm_cache_block_invalid);

				if(write_back_packet->cache_block_state == cgm_cache_block_modified)
				{
					message_packet->size = cache->block_size;
					message_packet->cache_block_state = cgm_cache_block_modified;
				}

				//clear the WB from the buffer
				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				message_packet->l1_cache_id = cache->id;

				//set the access type
				message_packet->access_type = cgm_access_downgrade_ack;

				//reply to the L2 cache
				cache_put_io_down_queue(cache, message_packet);

			}
			else
			{
				//if invalid and not in write back it was silently dropped
				message_packet->size = 1;
				message_packet->cache_block_state = cgm_cache_block_invalid;

				//set the access type
				message_packet->access_type = cgm_access_downgrade_ack;

				//reply to the L2 cache
				cache_put_io_down_queue(cache, message_packet);
			}

			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			assert(!write_back_packet);
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

			//set the access type
			message_packet->access_type = cgm_access_downgrade_ack;

			//downgrade the local block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_shared);

			//reply to the L2 cache
			cache_put_io_down_queue(cache, message_packet);
			break;
	}

	return;
}

void cgm_mesi_l1_d_getx_fwd_inval(struct cache_t *cache, struct cgm_packet_t *message_packet){


	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	//charge delay
	P_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 1 || LEVEL == 3)
		{
			printf("block 0x%08x %s getx fwd inval ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	/*if the block is in the cache it is not in the WB buffer
	if the block is dirty send down to L2 cache for merge*/
	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_owned:
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_shared:
		fatal("l1_d_cache_ctrl(): Invalid block state on flush hit %s \n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		case cgm_cache_block_invalid:

			//check write back buffer
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/
				assert((write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive)
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

	return;
}

void cgm_mesi_l1_d_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet){


	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;
	//struct cgm_packet_t *wb_packet;
	//enum cgm_cache_block_state_t block_trainsient_state;
	//int l3_map;
	//int error = 0;

	//charge the delay
	P_PAUSE(cache->latency);

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

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 1 || LEVEL == 3)
		{
			printf("block 0x%08x %s wb sent id %llu state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->cache_block_state, P_TIME);
		}
	}

	/*better not be in my cache*/
	assert(*cache_block_hit_ptr == 0);
	assert(message_packet->flush_pending == 0 && message_packet->L3_flush_join == 0);

	/*stats*/
	cache->TotalWriteBacks++;

	cache_put_io_down_queue(cache, message_packet);

	return;
}

void cgm_mesi_l1_d_flush_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//Invalidation/eviction request from L2 cache

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *wb_packet;

	//enum cgm_cache_block_state_t victim_trainsient_state;

	//charge the delay
	P_PAUSE(cache->latency);


	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 1 || LEVEL == 3)
		{
			printf("block 0x%08x %s flush block ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}



	//victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);


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



			//invalidate the local block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			break;

	}

	return;
}


int cgm_mesi_l1_d_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	assert(cache->cache_type == l1_d_cache_t);
	assert((message_packet->access_type == cgm_access_puts && message_packet->cache_block_state == cgm_cache_block_shared)
			|| (message_packet->access_type == cgm_access_put_clnx && message_packet->cache_block_state == cgm_cache_block_exclusive)
			|| (message_packet->access_type == cgm_access_putx && message_packet->cache_block_state == cgm_cache_block_modified));

	enum cgm_cache_block_state_t victim_trainsient_state;

	//check the transient state of the victim
	//if the state is set, an earlier access is bringing the block
	//if it is not set the victim is clear to evict
	P_PAUSE(cache->latency);

	ort_clear(cache, message_packet);

	//make sure victim way was correctly stored.
	assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);
	//The block should be in the transient state.

	if(victim_trainsient_state != cgm_cache_block_transient)
	{
		cgm_cache_dump_set(cache, message_packet->set);

		unsigned int temp = message_packet->address;
		temp = temp & cache->block_address_mask;

		fatal("cgm_mesi_l1_d_write_block(): %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d cycle %llu\n",
			cache->name, message_packet->access_id, message_packet->address, temp, message_packet->set, message_packet->tag, message_packet->l1_victim_way, P_TIME);
		getchar();
	}

	assert(victim_trainsient_state == cgm_cache_block_transient);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 1 || LEVEL == 3)
		{
			printf("block 0x%08x %s write block ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, message_packet->cache_block_state, P_TIME);
		}
	}

	//write the block
	cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	return 1;
}

void cgm_mesi_l2_gets(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	int l3_map;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	/*stats*/
	if(*cache_block_hit_ptr == 0)
	{
		cache->TotalMisses++;
		cache->TotalGets++;
	}

	/*on gets there should never be a wb waiting for
	this block because the block should have .text only*/
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);
	assert(!write_back_packet);

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_modified:
		case cgm_cache_block_owned:
		case cgm_cache_block_exclusive:
			fatal("cgm_mesi_l2_gets(): L2 id %d Invalid block state on gets as %s address 0x%08x cycle %llu\n",
					cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address, P_TIME);
			break;

		case cgm_cache_block_invalid:

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//find victim, on return OK to just drop the block this is I$ traffic

			message_packet->l2_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l2_victim_way >= 0 && message_packet->l2_victim_way < cache->assoc);

			/*if the block isn't already invalid evict it*/
			if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l2_victim_way) != cgm_cache_block_invalid)
				cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->l2_victim_way, 0, NULL);

			//add some routing/status data to the packet
			message_packet->access_type = cgm_access_gets;

			l3_map = cgm_l3_cache_map(message_packet->set);
			message_packet->l2_cache_id = cache->id;
			message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);
			message_packet->dest_name = l3_caches[l3_map].name;
			message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

			//transmit to L3

			/*printf("l2_gets\n");*/
			cache_put_io_down_queue(cache, message_packet);

			break;

		case cgm_cache_block_shared:

			//check if the packet has coalesced accesses.
			if(message_packet->access_type == cgm_access_fetch_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			//set block state and message size
			message_packet->cache_block_state = *cache_block_state_ptr;
			message_packet->size = l1_i_caches[cache->id].block_size; //this can be either L1 I or L1 D cache block size.
			message_packet->access_type = cgm_access_puts;

			/*stats*/
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L2_hit;

			cache_put_io_up_queue(cache, message_packet);

			break;
	}
	return;
}

void cgm_mesi_l2_get(struct cache_t *cache, struct cgm_packet_t *message_packet){


	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int l3_map;

	struct cgm_packet_t *write_back_packet = NULL;
	struct cgm_packet_t *pending_join = NULL;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	/*stats*/
	if(*cache_block_hit_ptr == 0)
	{
		cache->TotalMisses++;
		cache->TotalReadMisses++;
		cache->TotalGet++;
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
			fatal("cgm_mesi_l2_get(): L2 id %d Invalid block state on get as %s cycle %llu\n",
					cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), P_TIME);
			break;

		case cgm_cache_block_invalid:

			//stats
			//cache->misses++;

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

				//if not then we must coalesce
				if(write_back_packet->l2_victim_way == -1)
				{
					//Set and ways are all transient must coalesce
					cache_check_ORT(cache, message_packet);

					assert(message_packet->coalesced == 1);
					if(message_packet->coalesced == 1)
					{
						return;
					}
					else
					{
						fatal("cgm_mesi_l2_get(): write failed to coalesce when all ways are transient...\n");
					}
				}

				/*stats*/
				cache->TotalMisses--;
				cache->TotalReadMisses--;
				cache->TotalGet--;

				//we are bringing a new block so evict the victim and flush the L1 copies
				assert(write_back_packet->l2_victim_way >= 0 && write_back_packet->l2_victim_way < cache->assoc);


				/*if(cache->sets[write_back_packet->set].blocks[write_back_packet->l2_victim_way].state != 0)
				{
					printf("cgm_mesi_l2_get(): overwriting a valid block must evict first; set %d tag %d way %d cycle %llu\n",
							write_back_packet->set, write_back_packet->tag, write_back_packet->l2_victim_way, P_TIME);
					getchar();
				}*/

				//first evict the old block if it isn't invalid already
				if(cgm_cache_get_block_state(cache, write_back_packet->set, write_back_packet->l2_victim_way) != cgm_cache_block_invalid)
					cgm_L2_cache_evict_block(cache, write_back_packet->set, write_back_packet->l2_victim_way, 0, NULL);


				//now set the block
				assert(write_back_packet->cache_block_state == cgm_cache_block_exclusive || write_back_packet->cache_block_state == cgm_cache_block_modified);
				cgm_cache_set_block(cache, write_back_packet->set, write_back_packet->l2_victim_way, write_back_packet->tag, write_back_packet->cache_block_state);

				//cgm_cache_dump_set(cache, message_packet->set);

				/*printf("mp set %d mp way %d mp tag %d wb set %d wb way %d wb tag %d\n",
						message_packet->set, message_packet->way, message_packet->tag, write_back_packet->set, write_back_packet->l2_victim_way, write_back_packet->tag);*/

				//check for retries on successful cache read...
				if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
				{
					//enter retry state process all coalesced accesses
					cache_coalesed_retry(cache, message_packet->tag, message_packet->set);

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

				/*stats*/
				cache->TotalReads++;

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s load wb hit id %llu state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);
					}
				}

				/*stats*/
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L2_hit;

				cache_put_io_up_queue(cache, message_packet);
				return;
			}
			else
			{
				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
				{
					if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
					{
						if(LEVEL == 2 || LEVEL == 3)
						{
							printf("block 0x%08x %s load miss coalesce ID %llu type %d state %d cycle %llu\n",
									(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
						}
					}
					return;
				}

				//we are bringing a new block so evict the victim and flush the L1 copies
				message_packet->l2_victim_way = cgm_cache_get_victim(cache, message_packet->set);
				assert(message_packet->l2_victim_way >= 0 && message_packet->l2_victim_way < cache->assoc);

				if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l2_victim_way) != cgm_cache_block_invalid)
					cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->l2_victim_way, 0, NULL);

				//add some routing/status data to the packet
				message_packet->access_type = cgm_access_get;

				l3_map = cgm_l3_cache_map(message_packet->set);
				message_packet->l2_cache_id = cache->id;
				message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);
				message_packet->dest_name = l3_caches[l3_map].name;
				message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s load miss ID %llu type %d state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
					}
				}

				//transmit to L3
				/*printf("l2_get\n");*/
				cache_put_io_down_queue(cache, message_packet);

				//debug
				/*CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu miss changed (%s) cycle %llu\n",
						cache->name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);*/
			}

			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_shared:

			/*stats*/
			cache->TotalReads++;

			/*if(cache->sets[message_packet->set].blocks[message_packet->way].flush_pending == 1)
				printf("************ %s get while flush is going up blk_address 0x%08x cycle %llu\n", cache->name, (message_packet->address & ~cache->block_mask), P_TIME);*/


			if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);

				/*check for a pending get_fwd or getx_fwd join*/
				pending_join = cache_search_pending_request_buffer(cache, (message_packet->address & cache->block_address_mask));

				if(pending_join)
				{
					pending_join->downgrade_pending--;

					if (pending_join->downgrade_pending == 0)
					{
						printf("pending get/getx (load)_fwd request joined id %llu\n", pending_join->access_id);
						//getchar();

						pending_join = list_remove(cache->pending_request_buffer, pending_join);
						list_enqueue(cache->retry_queue, pending_join);
						advance(cache->ec_ptr);
					}
				}
			}

			//set message size
			message_packet->size = l1_d_caches[cache->id].block_size; //this can be either L1 I or L1 D cache block size.

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

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s load hit ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}

			/*stats*/
			if(!message_packet->protocol_case)
					message_packet->protocol_case = L2_hit;

			cache_put_io_up_queue(cache, message_packet);

			break;
	}
}

void cgm_mesi_l2_get_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int ort_row = 0;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	/*int way = 0;*/
	int l3_map = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	/*int *way_ptr = &way;*/

	//charge delay
	P_PAUSE(cache->latency);

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s get_nack ID %llu type %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, P_TIME);
		}
	}

	//store the decoded address
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	//do not set retry because this contains the coalesce set and tag.
	//check that there is an ORT entry for this address
	ort_row = ort_search(cache, message_packet->tag, message_packet->set);
	assert(ort_row < cache->mshr_size);

	//add some routing/status data to the packet
	message_packet->access_type = cgm_access_get;

	l3_map = cgm_l3_cache_map(message_packet->set);
	message_packet->l2_cache_id = cache->id;
	message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

	message_packet->src_name = cache->name;
	message_packet->src_id = str_map_string(&node_strn_map, cache->name);
	message_packet->dest_name = l3_caches[l3_map].name;
	message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

	//we are bringing a new block so evict the victim and flush the L1 copies
	//find victim

	//transmit to L3
	/*printf("l2_get nack\n");*/
	cache_put_io_down_queue(cache, message_packet);

	return;
}


int cgm_mesi_l2_getx(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int l3_map;

	struct cgm_packet_t *write_back_packet = NULL;
	struct cgm_packet_t *pending_join = NULL;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	/*stats*/
	if(*cache_block_hit_ptr == 0)
	{
		cache->TotalMisses++;
		cache->TotalWriteMisses++;
		cache->TotalGetx++;
	}

	if (*cache_block_state_ptr == cgm_cache_block_shared)
	{
		cache->TotalMisses++;
		cache->TotalWriteMisses++;
		cache->TotalUpgrades++;
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

				//printf("mp set %d mp way %d mp tag %d wb set %d wb way %d wb tag %d\n",
				//		message_packet->set, message_packet->way, message_packet->tag, write_back_packet->set, write_back_packet->l2_victim_way, write_back_packet->tag);

				//see if we can write it back into the cache.
				write_back_packet->l2_victim_way = cgm_cache_get_victim_for_wb(cache, write_back_packet->set);

				//if not then we must coalesce
				if(write_back_packet->l2_victim_way == -1)
				{
					//Set and ways are all transient must coalesce
					cache_check_ORT(cache, message_packet);

					assert(message_packet->coalesced == 1);
					if(message_packet->coalesced == 1)
					{
						return 1;
					}
					else
					{
						fatal("cgm_mesi_l2_getx(): write failed to coalesce when all ways are transient...\n");
					}
				}

				cache->TotalMisses--;
				cache->TotalWriteMisses--;

				//we are bringing a new block so evict the victim and flush the L1 copies
				assert(write_back_packet->l2_victim_way >= 0 && write_back_packet->l2_victim_way < cache->assoc);


				//first evict the old block if it isn't invalid already
				if(cgm_cache_get_block_state(cache, write_back_packet->set, write_back_packet->l2_victim_way) != cgm_cache_block_invalid)
					cgm_L2_cache_evict_block(cache, write_back_packet->set, write_back_packet->l2_victim_way, 0, NULL);

				//now set the new block
				cgm_cache_set_block(cache, write_back_packet->set, write_back_packet->l2_victim_way, write_back_packet->tag, write_back_packet->cache_block_state);

				/*if(write_back_packet->set == 23)
					run_watch_dog = 1;*/

				/*cgm_cache_dump_set(cache, write_back_packet->set);*/

				//check for retries on successful cache write...
				if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
				{
					//enter retry state.
					cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
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



				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s store wb hit id %llu state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);
					}
				}

				/*stats*/
				cache->TotalReads++;
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L2_hit;

				cache_put_io_up_queue(cache, message_packet);
			}
			else
			{

				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
				{
					if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
					{
						if(LEVEL == 2 || LEVEL == 3)
						{
							printf("block 0x%08x %s store miss coalesce ID %llu type %d state %d cycle %llu\n",
									(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
						}
					}

					return 1;
				}

				//find victim
				message_packet->l2_victim_way = cgm_cache_get_victim(cache, message_packet->set);
				assert(message_packet->l2_victim_way >= 0 && message_packet->l2_victim_way < cache->assoc);

				//evict the victim
				if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l2_victim_way) != cgm_cache_block_invalid)
					cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->l2_victim_way, 0, NULL);

				//set access type
				message_packet->access_type = cgm_access_getx;

				//update routing headers for the packet
				l3_map = cgm_l3_cache_map(message_packet->set);
				message_packet->l2_cache_id = cache->id;
				message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);
				message_packet->dest_name = l3_caches[l3_map].name;
				message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s store miss ID %llu type %d state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
					}
				}

				//transmit to L3
				/*printf("l2_getx\n");*/
				cache_put_io_down_queue(cache, message_packet);
			}

			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			//set retry state
			if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);

				/*check for a pending get_fwd or getx_fwd join*/
				pending_join = cache_search_pending_request_buffer(cache, (message_packet->address & cache->block_address_mask));

				if(pending_join)
				{
					pending_join->downgrade_pending--;

					if (pending_join->downgrade_pending == 0)
					{
						//printf("pending get/getx_fwd request joined id %llu\n", pending_join->access_id);
						//getchar();

						pending_join = list_remove(cache->pending_request_buffer, pending_join);
						list_enqueue(cache->retry_queue, pending_join);
						advance(cache->ec_ptr);
					}
				}
			}

			if(*cache_block_state_ptr == cgm_cache_block_exclusive)
			{
				/*if the block is in the E state set M because the message is a store
				a flush will bring the modified line down later
				the block remains in the E state at L3*/
				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);
			}

			//set message status and size
			message_packet->size = l1_d_caches[cache->id].block_size; //this should be L1 D cache block size.
			message_packet->access_type = cgm_access_putx;
			message_packet->cache_block_state = cgm_cache_block_modified;

			/*stats*/
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L2_hit;

			//send up to L1 D cache
			cache_put_io_up_queue(cache, message_packet);

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s store hit ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}
			break;

		case cgm_cache_block_shared:

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s upgrade miss ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}

			//access was a miss in L1 D but a hit in the shared state at the L2 level, set upgrade and run again.
			message_packet->access_type = cgm_access_upgrade;

			/*stats*/
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L2_upgrade;

			//return 0 to process as an upgrade.
			return 0;

			break;
	}

	return 1;
}

void cgm_mesi_l2_getx_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int ort_row = 0;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	/*int way = 0;*/
	int l3_map = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	/*int *way_ptr = &way;*/

	//charge delay
	P_PAUSE(cache->latency);

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s getx_nack ID %llu type %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, P_TIME);
		}
	}

	//store the decoded address
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	//do not set retry because this contains the coalesce set and tag.
	//check that there is an ORT entry for this address
	ort_row = ort_search(cache, message_packet->tag, message_packet->set);

	/*printf("here\n");
	getchar();*/
	if(ort_row >= cache->mshr_size)
	{

		ort_dump(cache);

		printf("problem set %d tag %d block 0x%08x cycle %llu\n",
			message_packet->set, message_packet->tag, (message_packet->address & cache->block_address_mask), P_TIME);

		assert(ort_row < cache->mshr_size);
	}

	//add some routing/status data to the packet
	message_packet->access_type = cgm_access_getx;

	l3_map = cgm_l3_cache_map(message_packet->set);
	message_packet->l2_cache_id = cache->id;
	message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

	message_packet->src_name = cache->name;
	message_packet->src_id = str_map_string(&node_strn_map, cache->name);
	message_packet->dest_name = l3_caches[l3_map].name;
	message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

	//transmit to L3
	/*printf("l2_getx_nack\n");*/
	cache_put_io_down_queue(cache, message_packet);

	return;
}

void cgm_mesi_l2_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){


	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *reply_packet;
	struct cgm_packet_t *pending_request;
	struct cgm_packet_t *write_back_packet = NULL;

	int l3_map;
	int error = 0;

	//charge delay
	P_PAUSE(cache->latency);

	//L1 D cache flush complete

	//get the status of the cache block and try to find it in either the cache or WB buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//make sure the block isn't in the transient state
	//victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	//assert(victim_trainsient_state != cgm_cache_block_transient);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s downgrade ack ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
		fatal("cgm_mesi_l2_downgrade_ack(): L2 id %d invalid block state on downgrade_ack as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			//check WB for line...
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/

				/*downgrade is complete for this write back so it should not be up in L1 D*/
				error = cache_validate_block_flushed_from_l1(cache->id, message_packet->address);
				assert(error == 0);

				assert(write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive);
				/*assert(write_back_packet->flush_pending == 0);*/

				/////////
				//GET_FWD
				/////////

				//pull the GET_FWD from the pending request buffer
				pending_request = cache_search_pending_request_buffer(cache, message_packet->address);
				/*if not found uh-oh...*/
				assert(pending_request);
				/*the address better be the same too...*/
				assert(pending_request->address == message_packet->address);
				assert(pending_request->start_cycle != 0);
				/*if (pending_request->way != message_packet->way)
				{
					cgm_cache_dump_set(cache, message_packet->set);

					fatal("cgm_mesi_l2_downgrade_ack(): %s access_id %llu address 0x%08x blk_addr 0x%08x mp set %d mp tag %d mp way %d blk state %d pr set %d sr tag %d pr way %d cycle %llu\n",
						cache->name, message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
						message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, pending_request->set, pending_request->tag, pending_request->way, P_TIME);
				}*/
				assert(pending_request->set == message_packet->set);// && pending_request->way == message_packet->way);

				//the line is invalid in the cache so don't set the line shared.
				/*printf("%s downgrade_ack id %llu cycle %llu\n", cache->name, pending_request->access_id, P_TIME);*/

				//prepare to forward the block
				//set access type
				pending_request->access_type = cgm_access_puts;

				//set the block state
				pending_request->cache_block_state = cgm_cache_block_shared;

				//set message package size
				pending_request->size = l2_caches[str_map_string(&node_strn_map, pending_request->l2_cache_name)].block_size;

				//fwd block to requesting core
				//update routing headers swap dest and src
				//requesting node
				pending_request->dest_name = str_map_value(&node_strn_map, pending_request->src_id);
				pending_request->dest_id = str_map_string(&node_strn_map, pending_request->src_name);

				//owning node L2
				pending_request->src_name = cache->name;
				pending_request->src_id = str_map_string(&node_strn_map, cache->name);

				//transmit block to requesting node
				pending_request = list_remove(cache->pending_request_buffer, pending_request);
				list_enqueue(cache->Tx_queue_bottom, pending_request);
				advance(cache->cache_io_down_ec);

				///////////////
				//downgrade_ack
				///////////////

				//send the downgrade ack to L3 cache.

				//create downgrade_ack
				reply_packet = packet_create();
				assert(reply_packet);

				init_downgrade_ack_packet(reply_packet, message_packet->address);
				reply_packet->access_id = message_packet->access_id;

				//determine if this is a sharing WB
				assert(message_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_modified);
				if(message_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_modified)
				{
					reply_packet->cache_block_state = cgm_cache_block_modified;
				}
				else
				{
					reply_packet->cache_block_state = cgm_cache_block_shared;
				}

				//fwd reply (downgrade_ack) to L3
				l3_map = cgm_l3_cache_map(message_packet->set);

				//fakes src as the requester
				/*reply_packet->l2_cache_id = l2_caches[my_pid].id;*/
				reply_packet->l2_cache_id = pending_request->l2_cache_id;
				reply_packet->l2_cache_name = pending_request->src_name;

				reply_packet->src_name = cache->name;
				reply_packet->src_id = str_map_string(&node_strn_map, cache->name);
				reply_packet->dest_name = l3_caches[l3_map].name;
				reply_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

				//transmit downgrad_ack to L3 (home)
				list_enqueue(cache->Tx_queue_bottom, reply_packet);
				advance(cache->cache_io_down_ec);

				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);


				//destroy the downgrade message because we don't need it anymore.
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);

			}
			else
			{

				fatal("cgm_mesi_l2_downgrade_ack(): get_fwd should no longer see this case\n");

				/*//block was locally dropped

				//pull the GET_FWD from the pending request buffer
				pending_request = cache_search_pending_request_buffer(cache, message_packet->address);
				if not found uh-oh...
				assert(pending_request);
				the address better be the same too...
				assert(pending_request->address == message_packet->address);
				assert(pending_request->start_cycle != 0);

				//downgrade the local block
				assert(pending_request->set == message_packet->set && pending_request->way == message_packet->way);

				//set cgm_access_getx_fwd_nack
				pending_request->access_type = cgm_access_getx_fwd_nack;

				//fwd reply (downgrade_nack) to L3
				l3_map = cgm_l3_cache_map(pending_request->set);

				here send the nack down to the L3
				don't change any of the source information

				message_packet->l2_cache_id = l2_caches[my_pid].id;
				message_packet->l2_cache_name = str_map_value(&l2_strn_map, l2_caches[my_pid].id);
				reply_packet->src_name = l2_caches[my_pid].name;
				reply_packet->src_id = str_map_string(&node_strn_map, l2_caches[my_pid].name);

				pending_request->dest_name = l3_caches[l3_map].name;
				pending_request->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

				//transmit back to L3
				pending_request = list_remove(cache->pending_request_buffer, pending_request);
				list_enqueue(cache->Tx_queue_bottom, pending_request);
				advance(cache->cache_io_down_ec);

				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);*/
			}
			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			/////////
			//GET_FWD
			/////////

			//pull the GET_FWD from the pending request buffer
			pending_request = cache_search_pending_request_buffer(cache, message_packet->address);

			/*if not found uh-oh...*/
			assert(pending_request);
			/*the address better be the same too...*/
			assert(pending_request->address == message_packet->address);

			//downgrade the local block
			if(pending_request->set != message_packet->set || pending_request->way != message_packet->way)
			{
				ort_dump(cache);

				cgm_cache_dump_set(cache, message_packet->set);

				printf("pr set %d tag %d pr way %d mp set %d mp tag %d mp way %d\n",
						pending_request->set, pending_request->tag, pending_request->way, message_packet->set, message_packet->tag, message_packet->way);

				fatal("cgm_mesi_l2_downgrade_ack(): %s access id %llu blk_addr 0x%08x type %d start_cycle %llu end_cycle %llu\n",
						cache->name, message_packet->access_id, message_packet->address & cache->block_address_mask, message_packet->access_type,
						message_packet->start_cycle, message_packet->end_cycle);

			}

			assert(pending_request->set == message_packet->set && pending_request->way == message_packet->way);
			cgm_cache_set_block_state(cache, pending_request->set, pending_request->way, cgm_cache_block_shared);
			assert(cache->sets[pending_request->set].blocks[pending_request->way].flush_pending == 1);
			cgm_cache_clear_block_flush_pending_bit(cache, pending_request->set, pending_request->way);

			//prepare to forward the block
			//set access type
			pending_request->access_type = cgm_access_puts;

			//set the block state
			pending_request->cache_block_state = cgm_cache_block_shared;
			//end uncomment here

			//set message package size
			pending_request->size = l2_caches[str_map_string(&node_strn_map, pending_request->l2_cache_name)].block_size;

			//fwd block to requesting core
			//update routing headers swap dest and src
			//requesting node
			pending_request->dest_name = str_map_value(&node_strn_map, pending_request->src_id);
			pending_request->dest_id = str_map_string(&node_strn_map, pending_request->src_name);

			//owning node L2
			pending_request->src_name = cache->name;
			pending_request->src_id = str_map_string(&node_strn_map, cache->name);

			//transmit block to requesting node
			pending_request = list_remove(cache->pending_request_buffer, pending_request);
			list_enqueue(cache->Tx_queue_bottom, pending_request);
			advance(cache->cache_io_down_ec);

			///////////////
			//downgrade_ack
			///////////////

			//send the downgrade ack to L3 cache.

			//create downgrade_ack
			reply_packet = packet_create();
			assert(reply_packet);

			init_downgrade_ack_packet(reply_packet, message_packet->address);
			reply_packet->access_id = message_packet->access_id;

			//determine if this is a sharing WB
			assert(message_packet->cache_block_state == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_invalid);

			if(message_packet->cache_block_state == cgm_cache_block_modified || *cache_block_state_ptr == cgm_cache_block_modified)
			{
				reply_packet->cache_block_state = cgm_cache_block_modified;
			}
			else
			{
				reply_packet->cache_block_state = cgm_cache_block_shared;
			}

			//fwd reply (downgrade_ack) to L3
			l3_map = cgm_l3_cache_map(message_packet->set);

			//fakes src as the requester
			//this is important sot hat L3 will set the right sharers in the directory (should be two now).
			reply_packet->l2_cache_id = pending_request->l2_cache_id;
			reply_packet->l2_cache_name = pending_request->src_name;

			reply_packet->src_name = cache->name;
			reply_packet->src_id = str_map_string(&node_strn_map, cache->name);
			reply_packet->dest_name = l3_caches[l3_map].name;
			reply_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

			//transmit downgrad_ack to L3 (home)
			list_enqueue(cache->Tx_queue_bottom, reply_packet);
			advance(cache->cache_io_down_ec);

			//destroy the downgrade message because we don't need it anymore.
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			break;
	}
	return;
}

void cgm_mesi_l2_getx_fwd_inval_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *getx_fwd_reply_packet;
	struct cgm_packet_t *pending_getx_fwd_request;
	struct cgm_packet_t *write_back_packet = NULL;

	int l3_map;
	int error = 0;

	//charge delay
	P_PAUSE(cache->latency);

	//L1 D cache has been flushed

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s getx_fwd_inval_ack ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
			fatal("cgm_mesi_l2_getx_fwd_inval_ack(): L2 id %d invalid block state on getx_fwd inval ask as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			//check WB for line...
			if(write_back_packet)
			{
				/*inval is complete for this write back so it should not be up in L1 D*/
				error = cache_validate_block_flushed_from_l1(cache->id, message_packet->address);
				assert(error == 0);

				assert(write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive);

				//////////
				//GETX_FWD
				//////////

				//forward block to requesting core

				//pull the GETX_FWD from the pending request buffer
				pending_getx_fwd_request = cache_search_pending_request_buffer(cache, message_packet->address);
				/*if not found uh-oh...*/
				assert(pending_getx_fwd_request);
				/*the address better be the same too...*/
				assert(pending_getx_fwd_request->address == message_packet->address);
				assert(pending_getx_fwd_request->start_cycle != 0);

				//prepare to forward the block
				//set access type
				pending_getx_fwd_request->access_type = cgm_access_putx;

				//set the block state
				pending_getx_fwd_request->cache_block_state = cgm_cache_block_modified;

				//set message package size if modified in L2/L1.
				if(message_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_modified)
				{
					pending_getx_fwd_request->size = l2_caches[str_map_string(&node_strn_map, pending_getx_fwd_request->l2_cache_name)].block_size;
				}
				else
				{
					pending_getx_fwd_request->size = 1;
				}

				//fwd block to requesting core
				//update routing headers swap dest and src
				//requesting node
				pending_getx_fwd_request->dest_name = str_map_value(&node_strn_map, pending_getx_fwd_request->src_id);
				pending_getx_fwd_request->dest_id = str_map_string(&node_strn_map, pending_getx_fwd_request->src_name);

				//owning node L2
				pending_getx_fwd_request->src_name = cache->name;
				pending_getx_fwd_request->src_id = str_map_string(&node_strn_map, cache->name);

				//transmit block to requesting node
				pending_getx_fwd_request = list_remove(cache->pending_request_buffer, pending_getx_fwd_request);
				list_enqueue(cache->Tx_queue_bottom, pending_getx_fwd_request);
				advance(cache->cache_io_down_ec);

				///////////////
				//getx_fwd_ack
				///////////////

				//send the getx_fwd_ack to L3 cache.

				//create getx_fwd_ack packet
				getx_fwd_reply_packet = packet_create();
				assert(getx_fwd_reply_packet);

				init_getx_fwd_ack_packet(getx_fwd_reply_packet, message_packet->address);
				getx_fwd_reply_packet->access_id = message_packet->access_id;

				//set message package size if modified in L2/L1.
				if(message_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_modified)
				{
					getx_fwd_reply_packet->size = l2_caches[str_map_string(&node_strn_map, pending_getx_fwd_request->l2_cache_name)].block_size;
					getx_fwd_reply_packet->cache_block_state = cgm_cache_block_modified;
				}
				else
				{
					getx_fwd_reply_packet->size = 1;
					getx_fwd_reply_packet->cache_block_state = cgm_cache_block_invalid;
				}

				//fwd reply (getx_fwd_ack) to L3
				l3_map = cgm_l3_cache_map(message_packet->set);

				//fakes src as the requester
				getx_fwd_reply_packet->l2_cache_id = pending_getx_fwd_request->l2_cache_id;
				getx_fwd_reply_packet->l2_cache_name = pending_getx_fwd_request->src_name;

				getx_fwd_reply_packet->src_name = cache->name;
				getx_fwd_reply_packet->src_id = str_map_string(&node_strn_map, cache->name);
				getx_fwd_reply_packet->dest_name = l3_caches[l3_map].name;
				getx_fwd_reply_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

				//transmit getx_fwd_ack to L3 (home)
				list_enqueue(cache->Tx_queue_bottom, getx_fwd_reply_packet);
				advance(cache->cache_io_down_ec);

				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				//destroy the L1 D getx_fwd_inval_ack message because we don't need it anymore.
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);

			}
			else
			{

				fatal("cgm_mesi_l2_getx_fwd_inval_ack(): get_fwd should no longer get this far\n");
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
			pending_getx_fwd_request = cache_search_pending_request_buffer(cache, message_packet->address);
			/*if not found uh-oh...*/
			assert(pending_getx_fwd_request);
			/*the address better be the same too...*/
			assert(pending_getx_fwd_request->address == message_packet->address);
			assert(pending_getx_fwd_request->start_cycle != 0);

			//invalidate the local block
			assert(pending_getx_fwd_request->set == message_packet->set && pending_getx_fwd_request->way == message_packet->way);
			cgm_cache_set_block_state(cache, pending_getx_fwd_request->set, pending_getx_fwd_request->way, cgm_cache_block_invalid);

			//prepare to forward the block
			//set access type
			pending_getx_fwd_request->access_type = cgm_access_putx;

			//set the block state
			pending_getx_fwd_request->cache_block_state = cgm_cache_block_modified;

			//set message package size if modified in L2/L1.
			if(*cache_block_state_ptr == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_modified)
			{
				pending_getx_fwd_request->size = l2_caches[str_map_string(&node_strn_map, pending_getx_fwd_request->l2_cache_name)].block_size;
			}
			else
			{
				pending_getx_fwd_request->size = 1;
			}

			//fwd block to requesting core
			//update routing headers swap dest and src
			//requesting node
			pending_getx_fwd_request->dest_name = str_map_value(&node_strn_map, pending_getx_fwd_request->src_id);
			pending_getx_fwd_request->dest_id = str_map_string(&node_strn_map, pending_getx_fwd_request->src_name);

			//owning node L2
			pending_getx_fwd_request->src_name = cache->name;
			pending_getx_fwd_request->src_id = str_map_string(&node_strn_map, cache->name);

			//transmit block to requesting node
			pending_getx_fwd_request = list_remove(cache->pending_request_buffer, pending_getx_fwd_request);
			list_enqueue(cache->Tx_queue_bottom, pending_getx_fwd_request);
			advance(cache->cache_io_down_ec);

			///////////////
			//getx_fwd_ack
			///////////////

			//send the getx_fwd_ack to L3 cache.

			//create getx_fwd_ack packet
			getx_fwd_reply_packet = packet_create();
			assert(getx_fwd_reply_packet);

			init_getx_fwd_ack_packet(getx_fwd_reply_packet, message_packet->address);

			//set message package size if modified in L2/L1.
			if(*cache_block_state_ptr == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_modified)
			{
				getx_fwd_reply_packet->size = l2_caches[str_map_string(&node_strn_map, pending_getx_fwd_request->l2_cache_name)].block_size;
				getx_fwd_reply_packet->cache_block_state = cgm_cache_block_modified;
			}
			else
			{
				getx_fwd_reply_packet->size = 1;
				getx_fwd_reply_packet->cache_block_state = cgm_cache_block_invalid;
			}

			//fwd reply (getx_fwd_ack) to L3
			l3_map = cgm_l3_cache_map(message_packet->set);

			//fakes src as the requester
			getx_fwd_reply_packet->l2_cache_id = pending_getx_fwd_request->l2_cache_id;
			getx_fwd_reply_packet->l2_cache_name = pending_getx_fwd_request->src_name;

			getx_fwd_reply_packet->src_name = cache->name;
			getx_fwd_reply_packet->src_id = str_map_string(&node_strn_map, cache->name);
			getx_fwd_reply_packet->dest_name = l3_caches[l3_map].name;
			getx_fwd_reply_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

			//transmit getx_fwd_ack to L3 (home)
			list_enqueue(cache->Tx_queue_bottom, getx_fwd_reply_packet);
			advance(cache->cache_io_down_ec);

			//destroy the L1 D getx_fwd_inval_ack message because we don't need it anymore.
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			break;
	}

	return;
}

void cgm_mesi_l2_flush_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//Invalidation/eviction request from L3 cache

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;
	int l3_map = 0;

	enum cgm_cache_block_state_t victim_trainsient_state;

	struct cgm_packet_t *wb_packet = NULL;

	//charge delay
	P_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s flush block ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	/*if(message_packet->evict_id == 3624)
		fatal("caught 3624\n");*/


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
					message_packet->access_type = cgm_access_flush_block_ack;

					l3_map = cgm_l3_cache_map(message_packet->set);
					message_packet->l2_cache_id = cache->id;
					message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

					message_packet->src_name = cache->name;
					message_packet->src_id = str_map_string(&node_strn_map, cache->name);
					message_packet->dest_name = l3_caches[l3_map].name;
					message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

					//reply to the L3 cache
					/*printf("l2_flush_block_here_2 id %llu dest_id %d\n", message_packet->evict_id, message_packet->dest_id);*/
					cache_put_io_down_queue(cache, message_packet);

				}
				else if(wb_packet->flush_pending == 1)
				{
					//waiting on flush to finish insert into pending request buffer
					assert(wb_packet->cache_block_state == cgm_cache_block_exclusive || wb_packet->cache_block_state != cgm_cache_block_modified);

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

				l3_map = cgm_l3_cache_map(message_packet->set);
				message_packet->l2_cache_id = cache->id;
				message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);
				message_packet->dest_name = l3_caches[l3_map].name;
				message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

				//reply to the L3 cache
				/*printf("l2_flush_block_here_3 id %llu dest_id %d cycle %llu\n", message_packet->evict_id, message_packet->dest_id, P_TIME);*/
				cache_put_io_down_queue(cache, message_packet);
			}
			break;


		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			/*if the block is found in the L2 it may or may not be in the L1 cache
			we must invalidate here and send an invalidation to the L1 D cache*/
			assert(victim_trainsient_state != cgm_cache_block_transient);

			/*evict block here*/
			cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->way, 0, message_packet->way);

			//star todo find a better way to do this...
			wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

			assert(wb_packet);
			wb_packet->L3_flush_join = 1;

			message_packet = list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->pending_request_buffer, message_packet);

			break;

		case cgm_cache_block_shared:

			assert(victim_trainsient_state != cgm_cache_block_transient);
			/*block is shared drop it no need to send ack to L3 as there is no pending flush in WB*/
			/*evict block here*/
			cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->way, 0, message_packet->way);

			message_packet->l2_victim_way = message_packet->way;

			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			break;
	}



	return;
}

void cgm_mesi_l2_flush_block_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *wb_packet = NULL;
	struct cgm_packet_t *pending_request_packet = NULL;
	struct cgm_packet_t *reply_packet = NULL;
	int l3_map = 0;
	int error =0;

	//charge delay
	P_PAUSE(cache->latency);

	//flush block ack from L1 D cache...

	//get the address set and tag
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);
	//assert(*cache_block_hit_ptr == 0);

	error = cache_validate_block_flushed_from_l1(cache->id, message_packet->address);
	assert(error == 0);

	/*block should not be in L3 cache either*/
	assert(*cache_block_state_ptr == 0);

	//state should be either invalid of modified.
	assert(message_packet->cache_block_state == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_invalid);

	//find the block in the local WB buffer
	wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	/*if and L1 flush we better have a wb packet in write back*/
	if(wb_packet)
	{

		if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
		{
			if(LEVEL == 2 || LEVEL == 3)
			{
				printf("block 0x%08x %s flush blk ack with wb in l2 flush_join %d ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, wb_packet->L3_flush_join, message_packet->evict_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
			}
		}

		if(wb_packet->L3_flush_join == 0)
		{


			/*if there is a pending get_fwd or getx_fwd request join here*/
			pending_request_packet = cache_search_pending_request_buffer(cache, message_packet->address);

			if(pending_request_packet)
			{

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
				pending_request_packet->size = l2_caches[str_map_string(&node_strn_map, pending_request_packet->l2_cache_name)].block_size;

				//fwd block to requesting core
				//update routing headers swap dest and src
				//requesting node
				pending_request_packet->dest_name = str_map_value(&node_strn_map, pending_request_packet->src_id);
				pending_request_packet->dest_id = str_map_string(&node_strn_map, pending_request_packet->src_name);

				//owning node L2
				pending_request_packet->src_name = cache->name;
				pending_request_packet->src_id = str_map_string(&node_strn_map, cache->name);

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
				assert(message_packet->cache_block_state == cgm_cache_block_modified || wb_packet->cache_block_state == cgm_cache_block_modified);
				if(message_packet->cache_block_state == cgm_cache_block_modified || wb_packet->cache_block_state == cgm_cache_block_modified)
				{
					reply_packet->cache_block_state = cgm_cache_block_modified;
				}
				else
				{
					reply_packet->cache_block_state = cgm_cache_block_shared;
				}

				//fwd reply (downgrade_ack) to L3
				l3_map = cgm_l3_cache_map(pending_request_packet->set);

				//fakes src as the requester
				/*reply_packet->l2_cache_id = l2_caches[my_pid].id;*/
				reply_packet->l2_cache_id = pending_request_packet->l2_cache_id;
				reply_packet->l2_cache_name = pending_request_packet->src_name;

				reply_packet->src_name = cache->name;
				reply_packet->src_id = str_map_string(&node_strn_map, cache->name);
				reply_packet->dest_name = l3_caches[l3_map].name;
				reply_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

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

			pending_request_packet = cache_search_pending_request_buffer(cache, message_packet->address);
			assert(pending_request_packet);

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

			pending_request_packet->access_type = cgm_access_flush_block_ack;

			l3_map = cgm_l3_cache_map(pending_request_packet->set);
			pending_request_packet->l2_cache_id = cache->id;
			pending_request_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

			pending_request_packet->src_name = cache->name;
			pending_request_packet->src_id = str_map_string(&node_strn_map, cache->name);
			pending_request_packet->dest_name = l3_caches[l3_map].name;
			pending_request_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

			//reply to the L3 cache
			/*printf("l2_flush_block_ack_here id %llu dest %d cycle %llu\n", pending_request_packet->evict_id, pending_request_packet->dest_id, P_TIME);*/
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

		if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
		{
			if(LEVEL == 2 || LEVEL == 3)
			{
				printf("block 0x%08x %s flush blk ack without wb in l2 ID %llu type %d state %d cycle %llu\n",
						(message_packet->address & cache->block_address_mask), cache->name, message_packet->evict_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
			}
		}

		/*case L3 flushed L2 flushed L1 and now back to L2 and there is no WB waiting at L2*/

		/*if the eviction goes up and finds the block shared its dropped in L1.
		if the block is not in cache or WB at L1 we don't know if there is a M line coming down
		because the block was exclusive in L2. So Let's see what we got*/

		pending_request_packet = cache_search_pending_request_buffer(cache, message_packet->address);
		assert(!pending_request_packet);

		//printf("here 0x%08x\n", message_packet->address & cache->block_address_mask);

		//free the message packet
		message_packet = list_remove(cache->last_queue, message_packet);
		packet_destroy(message_packet);
	}

	return;
}


void cgm_mesi_l2_downgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;*/

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	//int *way_ptr = &way;

	int l3_map = 0;
	int ort_status = 0;
	//struct cgm_packet_t *pending_packet;
	enum cgm_cache_block_state_t victim_trainsient_state;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	//store the decode in the packet for now.
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;
	message_packet->way = way;

	//charge delay
	P_PAUSE(cache->latency);

	/*our load (get_fwd) request has been nacked by the owning L2*/

	/*verify status of cache blk*/
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l2_victim_way);
	assert(victim_trainsient_state == cgm_cache_block_transient); //there is a transient block
	if(victim_trainsient_state != cgm_cache_block_transient)
	{
		ort_dump(cache);
		cgm_cache_dump_set(cache, message_packet->set);

		unsigned int temp = message_packet->address;
		temp = temp & cache->block_address_mask;

		fatal("cgm_mesi_l2_downgrade_nack(): %s block not in transient state access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d cycle %llu\n",
			cache->name, message_packet->access_id, message_packet->address, temp,
			message_packet->set, message_packet->tag, message_packet->way, P_TIME);
	}


	ort_status = ort_search(cache, message_packet->tag, message_packet->set); // there is an outstanding request.
	assert(ort_status <= cache->mshr_size);


	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s downgrade nack ID %llu type %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, P_TIME);
		}
	}

	//change to a get
	message_packet->access_type = cgm_access_get;

	l3_map = cgm_l3_cache_map(message_packet->set);
	message_packet->l2_cache_id = cache->id;
	message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

	message_packet->src_name = cache->name;
	message_packet->src_id = str_map_string(&node_strn_map, cache->name);
	message_packet->dest_name = l3_caches[l3_map].name;
	message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

	cache_put_io_down_queue(cache, message_packet);

	return;
}


void cgm_mesi_l2_get_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *downgrade_packet = NULL;
	struct cgm_packet_t *write_back_packet = NULL;
	struct cgm_packet_t *nack_packet = NULL;
	struct cgm_packet_t *reply_packet = NULL;

	int error = 0;
	int ort_status = 0;
	int l3_map;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s get_fwd ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name,
					message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	/*look for an access conflict, this can happen if a get_fwd beats a put, put_clnx, putx*/
	ort_status = ort_search(cache, message_packet->tag, message_packet->set);
	if(ort_status != cache->mshr_size)
	{
		/*if there is a pending access int the ORT there better not be a block or a write back*/
		if(*cache_block_state_ptr == cgm_cache_block_invalid)
			assert(!write_back_packet);

		if(cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way) == cgm_cache_block_transient)
			assert(*cache_block_state_ptr == cgm_cache_block_shared && *cache_block_hit_ptr == 1);
	}


	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:

			cgm_cache_dump_set(cache, message_packet->set);

			fatal("cgm_mesi_l2_get_fwd(): %s invalid block state on get_fwd as %s set %d way %d tag %d address 0x%08x, blk_address 0x%08x\n",
				cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->set, message_packet->way, message_packet->tag,
				message_packet->address, message_packet->address & cache->block_address_mask);
			break;


		case cgm_cache_block_shared:

			/*star its possible to find the block in the shared state on the receipt of a get_fwd
			if this core is waiting on a putx/putx_n/upgrade(putx) join to finish. wait for this access
			to complete and join the getx_fwd with this access.*/

			/*block should be transient*/
			assert(cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way) == cgm_cache_block_transient);

			/*there should be an entry in the ORT for this set and tag in this cache*/
			assert(ort_status < cache->mshr_size);

			/*set the bit in the ort table to 0*/
			ort_set_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s inserting pending get_fwd (putx/putx_n join) ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name,
							message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}

			//getchar();

			message_packet->downgrade_pending = 1;

			/*drop into the pending request buffer*/
			message_packet =  list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->pending_request_buffer, message_packet);




			break;

		case cgm_cache_block_invalid:

			//check the WB buffer
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/
				assert(*cache_block_hit_ptr == 0);
				assert(write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive);


				/*check state of write back for flush*/
				if(write_back_packet->flush_pending == 0)
				{
					/*flush is complete for this write back so it should not be up in L1 D*/
					error = cache_validate_block_flushed_from_l1(cache->id, message_packet->address);
					assert(error == 0);

					/*if the flush is complete finish the get_fwd*/

					/*found the packet in the write back buffer
					data should not be in the rest of the cache*/

					/////////
					//GET_FWD
					/////////

					//set message package size
					message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

					//fwd block to requesting core
					//update routing headers swap dest and src
					//requesting node
					message_packet->access_type = cgm_access_puts;

					//set the block state
					message_packet->cache_block_state = cgm_cache_block_shared;

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
					//downgrade_ack
					///////////////

					//send the downgrade ack to L3 cache.

					//create downgrade_ack
					reply_packet = packet_create();
					assert(reply_packet);

					init_downgrade_ack_packet(reply_packet, message_packet->address);

					//determine if this is a sharing WB
					assert(write_back_packet->cache_block_state == cgm_cache_block_modified);
					if(write_back_packet->cache_block_state == cgm_cache_block_modified)
					{
						reply_packet->cache_block_state = cgm_cache_block_modified;
					}
					else
					{
						reply_packet->cache_block_state = cgm_cache_block_shared;
					}

					//fwd reply (downgrade_ack) to L3
					l3_map = cgm_l3_cache_map(message_packet->set);

					//fakes src as the requester
					/*reply_packet->l2_cache_id = l2_caches[my_pid].id;*/
					reply_packet->l2_cache_id = message_packet->l2_cache_id;
					reply_packet->l2_cache_name = message_packet->src_name;

					reply_packet->src_name = cache->name;
					reply_packet->src_id = str_map_string(&node_strn_map, cache->name);
					reply_packet->dest_name = l3_caches[l3_map].name;
					reply_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

					write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
					packet_destroy(write_back_packet);

					//transmit downgrad_ack to L3 (home)
					list_enqueue(cache->Tx_queue_bottom, reply_packet);
					advance(cache->cache_io_down_ec);

				}
				else
				{
					/*if the wb is flush pending we have to wait for the flush to complete and then join there*/
					/*fatal("get_fwd with wb pending fulsh\n");*/

					/*writeback is in the process of being flushed by L1*/
					assert(write_back_packet->flush_pending == 1);

					//message_packet->downgrade_pending = 1;
					message_packet->L3_flush_join = 1;
					cgm_cache_insert_pending_request_buffer(cache, message_packet);
				}
			}
			else
			{
				/*its possible that a get_fwd can come in for a block while the cache is waiting for the blk after sending a request
				if the cache is waiting for the block there will be in entry in the ORT table. L3 has already serviced the request
				and the core has the block M/E but the packet will arrive after the get_fwd. Perform a join to prevent deadlock*/
				if(ort_status < cache->mshr_size)
				{
					/*set the bit in the ort table to 0*/
					ort_set_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);

					//printf("inserting pending get_fwd blk_addr 0x%08x\n", message_packet->address & cache->block_address_mask);
					//getchar();

					message_packet->downgrade_pending = 1;

					/*drop into the pending request buffer*/
					message_packet =  list_remove(cache->last_queue, message_packet);
					list_enqueue(cache->pending_request_buffer, message_packet);

				}
				else
				{
					/*if no request is pending the block was either silently dropped OR was written back and is on its way to L3
					We can either let the WB service the get request (not implemented yet) or just nack and let the requesting core
					retry.*/

					/* The block is not in L2 cache it should not be in L1 cache either.*/
					error = cache_validate_block_flushed_from_l1(cache->id, message_packet->address);
					assert(error == 0);

					/*two part reply (1) send nack to L3 and (2) send nack to requesting L2*/
					//set access type
					message_packet->access_type = cgm_access_downgrade_nack;

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

					//create downgrade_ack
					nack_packet = packet_create();
					assert(nack_packet);

					init_downgrade_nack_packet(nack_packet, message_packet->address);
					nack_packet->access_id = message_packet->access_id;

					//fwd reply (downgrade_nack) to L3
					l3_map = cgm_l3_cache_map(message_packet->set);

					nack_packet->src_id = str_map_string(&node_strn_map, cache->name);
					nack_packet->src_name = cache->name;
					nack_packet->dest_name = l3_caches[l3_map].name;
					nack_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

					//transmit block to L3
					list_enqueue(cache->Tx_queue_bottom, nack_packet);
					advance(cache->cache_io_down_ec);
				}

			}
			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			//a GET_FWD means the block is exclusive in this core, but could also be modified

			//store the get_fwd in the pending request buffer
			message_packet->downgrade_pending = 1;
			cgm_cache_insert_pending_request_buffer(cache, message_packet);

			//set the flush_pending bit to 1 in the block
			cgm_cache_set_block_flush_pending_bit(cache, message_packet->set, message_packet->way);

			//flush the L1 cache because the line may be dirty in L1
			downgrade_packet = packet_create();
			assert(downgrade_packet);
			init_downgrade_packet(downgrade_packet, message_packet->address);

			//send the L1 D cache the downgrade message
			downgrade_packet->cpu_access_type = cgm_access_load;
			downgrade_packet->access_id = message_packet->access_id;
			list_enqueue(cache->Tx_queue_top, downgrade_packet);
			advance(cache->cache_io_up_ec);

			break;
	}

	return;
}

void cgm_mesi_l2_getx_fwd_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	// *way_ptr = &way;

	int l3_map = 0;
	int ort_status = 0;
	//struct cgm_packet_t *pending_packet;
	enum cgm_cache_block_state_t victim_trainsient_state;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	//store the decode in the packet for now.
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;
	message_packet->way = way;

	//charge delay
	P_PAUSE(cache->latency);

	/*our load (get_fwd) request has been nacked by the owning L2*/

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

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s getx_fwd_nack nack ID %llu type %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, P_TIME);
		}
	}

	//change to a get
	message_packet->access_type = cgm_access_getx;

	l3_map = cgm_l3_cache_map(message_packet->set);
	message_packet->l2_cache_id = cache->id;
	message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

	message_packet->src_name = cache->name;
	message_packet->src_id = str_map_string(&node_strn_map, cache->name);
	message_packet->dest_name = l3_caches[l3_map].name;
	message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

	cache_put_io_down_queue(cache, message_packet);

	return;
}


void cgm_mesi_l2_getx_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *inval_packet;
	struct cgm_packet_t *write_back_packet = NULL;
	struct cgm_packet_t *nack_packet = NULL;
	struct cgm_packet_t *reply_packet = NULL;

	int error = 0;
	int ort_status = 0;
	int l3_map;

	//charge delay
	P_PAUSE(cache->latency);

	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s getx_fwd ID %llu type %d state %d wb? %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, is_writeback_present(write_back_packet), P_TIME);
		}
	}

	/*look for an access conflict, this can happen if a get_fwd beats a putx/putx_n/upgrade(putx)*/
	ort_status = ort_search(cache, message_packet->tag, message_packet->set);
	if(ort_status != cache->mshr_size)
	{
		/*if there is a pending access int the ORT there better not be a block or a write back*/
		if(*cache_block_state_ptr == cgm_cache_block_invalid)
			assert(!write_back_packet);

		if(cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way) == cgm_cache_block_transient)
			assert(*cache_block_state_ptr == cgm_cache_block_shared && *cache_block_hit_ptr == 1);
	}


	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:

			cgm_cache_dump_set(cache, message_packet->set);

			fatal("cgm_mesi_l2_getx_fwd(): %s invalid block state on getx_fwd as %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
				cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
				message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
				message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);
			break;


		case cgm_cache_block_shared:

			/*star its possible to find the block in the shared state on the receipt of a getx_fwd
			if this core is waiting on a putx/putx_n/upgrade(putx) join to finish. wait for this access
			to complete and join the getx_fwd with this access.*/

			/*block should be transient*/
			assert(cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way) == cgm_cache_block_transient);

			/*there should be an entry in the ORT for this set and tag*/
			assert(ort_status < cache->mshr_size);

			/*set the bit in the ort table to 0*/
			ort_set_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s inserting pending getx_fwd (putx/putx_n join) ID %llu type %d state %d wb? %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, is_writeback_present(write_back_packet), P_TIME);
				}
			}

			message_packet->downgrade_pending = 1;

			/*drop into the pending request buffer*/
			message_packet =  list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->pending_request_buffer, message_packet);

			break;

		case cgm_cache_block_invalid:

			//check the WB buffer
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/

				assert(*cache_block_hit_ptr == 0);
				assert(write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive);

				/*check state of write back for flush*/
				if(write_back_packet->flush_pending == 0)
				{

					//printf("\tcgm_mesi_l2_getx_fwd(): wb_f = 0 blk addr 0x%08x cycle %llu\n", message_packet->address & cache->block_address_mask, P_TIME);

					/*if the flush is complete finish the getx_fwd now*/

					/*flush is complete for this write back so it should not be up in L1 D*/
					error = cache_validate_block_flushed_from_l1(cache->id, message_packet->address);
					assert(error == 0);

					//////////
					//GETX_FWD
					//////////

					//forward block to requesting core
					//set message package size
					message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;
					//set access type
					message_packet->access_type = cgm_access_putx;

					message_packet->cache_block_state = cgm_cache_block_modified;

					//set message package size if modified in L2/L1.
					if(write_back_packet->cache_block_state == cgm_cache_block_modified)
					{
						message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;
					}
					else
					{
						message_packet->size = 1;

					}

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

					//set message package size if modified in L2/L1.
					if(write_back_packet->cache_block_state == cgm_cache_block_modified)
					{
						reply_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;
						reply_packet->cache_block_state = cgm_cache_block_modified;
					}
					else
					{
						reply_packet->size = 1;
						reply_packet->cache_block_state = cgm_cache_block_invalid;
					}

					//fwd reply (getx_fwd_ack) to L3
					l3_map = cgm_l3_cache_map(message_packet->set);

					//fakes src as the requester
					reply_packet->l2_cache_id = message_packet->l2_cache_id;
					reply_packet->l2_cache_name = message_packet->src_name;

					reply_packet->src_name = cache->name;
					reply_packet->src_id = str_map_string(&node_strn_map, cache->name);
					reply_packet->dest_name = l3_caches[l3_map].name;
					reply_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

					write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
					packet_destroy(write_back_packet);

					//transmit getx_fwd_ack to L3 (home)
					list_enqueue(cache->Tx_queue_bottom, reply_packet);
					advance(cache->cache_io_down_ec);

				}
				else
				{

					//printf("\tcgm_mesi_l2_getx_fwd(): wb_f = 1\n");

					/*if the wb is flush pending we have to wait for the flush to complete and then join there*/
					/*fatal("get_fwd with wb pending fulsh\n");*/

					/*write back is in the process of being flushed by L1*/
					assert(write_back_packet->flush_pending == 1);

					//message_packet->downgrade_pending = 1;
					message_packet->L3_flush_join = 1;
					cgm_cache_insert_pending_request_buffer(cache, message_packet);
				}
			}
			else
			{

				//printf("\tcgm_mesi_l2_getx_fwd(): no wb blk addr 0x%08x \n", message_packet->address & cache->block_address_mask);
				//fatal("Getx_fwd %s access id %llu blk_addr 0x%08x\n", cache->name, message_packet->access_id, message_packet->address & cache->block_address_mask);

				if(ort_status < cache->mshr_size)
				{
					/*set the bit in the ort table to 0*/
					ort_set_pending_join_bit(cache, ort_status, message_packet->tag, message_packet->set);

					/*printf("inserting pending getx_fwd addr 0x%08x\n", message_packet->address);
					getchar();*/

					message_packet->downgrade_pending = 1;

					/*drop into the pending request buffer*/
					message_packet =  list_remove(cache->last_queue, message_packet);
					list_enqueue(cache->pending_request_buffer, message_packet);
				}
				else
				{

					/* The block was evicted silently and should not be L1 D's cache.
					 * However, the block may be in L1 D's write back or in the pipe between L1 D and L2.
					 * We have to send a flush to L1 D to make sure the block is really out of there before proceeding.*/
					error = cache_validate_block_flushed_from_l1(cache->id, message_packet->address);
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

					//create downgrade_ack
					nack_packet = packet_create();
					assert(nack_packet);

					init_getx_fwd_nack_packet(nack_packet, message_packet->address);
					nack_packet->access_id = message_packet->access_id;

					//fwd reply (downgrade_nack) to L3
					l3_map = cgm_l3_cache_map(message_packet->set);

					nack_packet->src_id = str_map_string(&node_strn_map, cache->name);
					nack_packet->src_name = cache->name;
					nack_packet->dest_name = l3_caches[l3_map].name;
					nack_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

					//transmit block to L3
					list_enqueue(cache->Tx_queue_bottom, nack_packet);
					advance(cache->cache_io_down_ec);
				}

			}
			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			//a GETX_FWD means the block is E/M in this core. The block will be E/M in the L1

			//printf("\tcgm_mesi_l2_getx_fwd(): in cache blk addr 0x%08x cycle %llu\n", message_packet->address & cache->block_address_mask, P_TIME);

			//store the getx_fwd in the pending request buffer
			message_packet->inval_pending = 1;
			cgm_cache_insert_pending_request_buffer(cache, message_packet);

			//set the flush_pending bit to 1 in the block
			cgm_cache_set_block_flush_pending_bit(cache, message_packet->set, message_packet->way);

			//flush the L1 cache because the line may be dirty in L1
			inval_packet = packet_create();
			assert(inval_packet);
			init_getx_fwd_inval_packet(inval_packet, message_packet->address);


			//send the L1 D cache the inval message
			inval_packet->cpu_access_type = cgm_access_store;
			inval_packet->access_id = message_packet->access_id;
			list_enqueue(cache->Tx_queue_top, inval_packet);
			advance(cache->cache_io_up_ec);
			break;
	}

	return;
}

int cgm_mesi_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int row = 0;
	int set = 0;
	int tag = 0;
	unsigned int offset = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;

	enum cgm_cache_block_state_t victim_trainsient_state;
	struct cgm_packet_t *pending_get_fwd_getx_fwd_request = NULL;
	int pending_join = 1;

	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	//store the decode
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	P_PAUSE(cache->latency);


	assert(cache->cache_type == l2_cache_t);
	assert((message_packet->access_type == cgm_access_puts && message_packet->cache_block_state == cgm_cache_block_shared)
			|| (message_packet->access_type == cgm_access_put_clnx && message_packet->cache_block_state == cgm_cache_block_exclusive)
			|| (message_packet->access_type == cgm_access_putx && message_packet->cache_block_state == cgm_cache_block_modified));

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s write block ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, message_packet->cache_block_state, P_TIME);
		}
	}

	//check the ort table for a pending join (get_fwd) (getx_fwd)
	row = ort_search(cache, message_packet->tag, message_packet->set);
	/*if(row == cache->mshr_size)
	{
		printf("write block access id %llu\n", message_packet->access_id);

		ort_dump(cache);

		cgm_cache_dump_set(cache, message_packet->set);

		fatal("cgm_mesi_l2_write_block(): %s access_id %llu address 0x%08x blk_addr 0x%08x mp set %d mp tag %d mp way %d cycle %llu\n",
			cache->name, message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
			message_packet->set, message_packet->tag, message_packet->way, P_TIME);
	}*/

	//assert(row < cache->mshr_size);
	if(row < cache->mshr_size)
		pending_join = cache->ort[row][2];



	/*order matters here*/

	/*look for a matching pending request.
	This could happen if L3 changed an upgrade to a getx_fwd/putx*/

	//pull the pending request from the buffer
	pending_get_fwd_getx_fwd_request = cache_search_pending_request_buffer(cache, (message_packet->address & cache->block_address_mask));

	if(pending_get_fwd_getx_fwd_request && pending_join == 1)
	{

		assert(pending_get_fwd_getx_fwd_request->upgrade_pending == 1);

		//fatal("cgm_mesi_l2_write_block(): %s shouldn't be here anymore blk_addr 0x%08x.\n", cache->name, pending_get_fwd_getx_fwd_request->address & cache->block_address_mask);
		/*OK this is a corner case. we sent an upgrade but received a getx_fwd clear the pending upgrade data before proceeding.
		i.e. another core beat us to it*/
		assert(pending_get_fwd_getx_fwd_request->address == message_packet->address);

		//clear the cache block pending bit
		cgm_cache_clear_block_upgrade_pending_bit(cache, pending_get_fwd_getx_fwd_request->set, pending_get_fwd_getx_fwd_request->way);

		//victim should have been in transient state
		victim_trainsient_state = cgm_cache_get_block_transient_state(cache, pending_get_fwd_getx_fwd_request->set, pending_get_fwd_getx_fwd_request->way);
		assert(victim_trainsient_state == cgm_cache_block_transient);

		//write the block
		cgm_cache_set_block(cache, pending_get_fwd_getx_fwd_request->set, pending_get_fwd_getx_fwd_request->way, pending_get_fwd_getx_fwd_request->tag, message_packet->cache_block_state);

		//set retry state
		pending_get_fwd_getx_fwd_request->access_type = cgm_cache_get_retry_state(pending_get_fwd_getx_fwd_request->cpu_access_type);

		//remove the pending request
		pending_get_fwd_getx_fwd_request = list_remove(cache->pending_request_buffer, pending_get_fwd_getx_fwd_request);
		list_enqueue(cache->retry_queue, pending_get_fwd_getx_fwd_request);

		//destroy the getx_fwd (putx)
		message_packet = list_remove(cache->last_queue, message_packet);
		packet_destroy(message_packet);

		ort_clear(cache, pending_get_fwd_getx_fwd_request);

	}
	else
	{
		assert((pending_join == 1 && !pending_get_fwd_getx_fwd_request) || (pending_join == 0 && pending_get_fwd_getx_fwd_request));

		if (pending_join == 0 && pending_get_fwd_getx_fwd_request)
		{
			/*get_fwd getx_fwd should be pending*/
			assert(pending_get_fwd_getx_fwd_request->downgrade_pending == 1);

			/*set the number of coalesced accesses*/
			pending_get_fwd_getx_fwd_request->downgrade_pending = (ort_get_num_coal(cache, message_packet->tag, message_packet->set) + 1); // + 1 account for the packet that was not coalesced and went to L3

			//printf("L2 write block found pending get_fwd/getx_fwd join blk addr 0x%08x number accesses coal %d\n", pending_get_fwd_getx_fwd_request->address & cache->block_address_mask, pending_get_fwd_getx_fwd_request->downgrade_pending);
			//getchar();

			/*cache_dump_request_queue(cache->pending_request_buffer);
			cache_dump_request_queue(cache->retry_queue);
			cache_dump_request_queue(cache->ort_list);
			("got pending retry blk addr 0x%08x number accesses coal %d\n", pending_get_fwd_getx_fwd_request->address & cache->block_address_mask, pending_get_fwd_getx_fwd_request->downgrade_pending);*/
		}

		ort_clear(cache, message_packet);

		//victim should have transient state set
		victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l2_victim_way);
		assert(victim_trainsient_state == cgm_cache_block_transient);

		//write the block
		/*if (cache->cache_type == l2_cache_t && message_packet->set == 23 && message_packet->way == 0)
			printf("\tmp access_id %llu cycle %llu\n", message_packet->access_id, P_TIME);*/
		cgm_cache_set_block(cache, message_packet->set, message_packet->l2_victim_way, message_packet->tag, message_packet->cache_block_state);

		//set retry state
		message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);

		message_packet = list_remove(cache->last_queue, message_packet);
		list_enqueue(cache->retry_queue, message_packet);

	}

	return 1;
}

void cgm_mesi_l3_gets(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	/*int num_cores = x86_cpu_num_cores;*/
	int sharers, owning_core, pending_bit;
	struct cgm_packet_t *write_back_packet = NULL;

	//charge the delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//should never find a wb with matching set and tag. This would mean a .text block was written to.
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);
	assert(!write_back_packet);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);
	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	/*checck the pending bit state... the block should never be pending,
	however if it is make sure the cache line ins't a hit*/
	assert(pending_bit == 0 || (pending_bit == 1 && *cache_block_state_ptr == 0));

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	/*stats*/
	if(*cache_block_hit_ptr == 0)
	{
		cache->TotalMisses++;
		cache->TotalGets++;
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:
			fatal("l3_cache_ctrl(): L3 id %d GetS invalid block state as %s cycle %llu\n", cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), P_TIME);
			break;

		case cgm_cache_block_invalid:

			//stats;
			//cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//find the victim.
			message_packet->l3_victim_way = cgm_cache_get_victim(cache, message_packet->set);
			assert(message_packet->l3_victim_way >= 0 && message_packet->l3_victim_way < cache->assoc);
			assert(cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->l3_victim_way) == 0);

			//evict the block
			if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l3_victim_way) != cgm_cache_block_invalid)
				cgm_L3_cache_evict_block(cache, message_packet->set, message_packet->l3_victim_way,
						cgm_cache_get_num_shares(cache, message_packet->set, message_packet->l3_victim_way), NULL);

			//clear the directory entry
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->l3_victim_way);

			//add some routing/status data to the packet
			message_packet->access_type = cgm_access_mc_load;

			//set return cache block state
			//star todo look into this, this should work for I$ requests
			message_packet->cache_block_state = cgm_cache_block_shared;

			assert(message_packet->cpu_access_type == cgm_access_fetch);

			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);
			message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
			message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

			/*stats*/
			if(!message_packet->protocol_case)
				message_packet->protocol_case = memory;

			//transmit to SA/MC
			cache_put_io_down_queue(cache, message_packet);

			break;

		case cgm_cache_block_shared:

			//stats;
			//cache->hits++;

			assert(message_packet->cpu_access_type == cgm_access_fetch);

			//check if the packet has coalesced accesses.
			if(message_packet->access_type == cgm_access_fetch_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			//set the presence bit in the directory for the requesting core.
			cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

			//update message packet status
			message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

			message_packet->cache_block_state = *cache_block_state_ptr;
			message_packet->access_type = cgm_access_puts;

			message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
			message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);

			/*stats*/
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L3_hit;

			cache_put_io_up_queue(cache, message_packet);

			break;

	}
	return;
}

int cgm_mesi_l2_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;
	struct cgm_packet_t *wb_packet;
	enum cgm_cache_block_state_t block_trainsient_state;
	int l3_map;
	int error = 0;

	//charge the delay
	P_PAUSE(cache->latency);

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

	/*on a write back with inclusive caches L2 Merges the line
	if the write back is a surprise the block will be exclusive and old in the L2 cache.*/

	//WB from L1 D cache
	if(cache->last_queue == cache->Coherance_Rx_queue)
	{
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
					if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
					{
						if(LEVEL == 2 || LEVEL == 3)
						{
							printf("block 0x%08x %s write back - write back merge ID %llu type %d cycle %llu\n",
									(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);
						}
					}

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

					//fatal("cgm_mesi_l2_write_back(): miss in L2 write back should no longer happen??\n");

					/*this case shouldn't happen any longer with the new changes.*/
					//cgm_cache_dump_set(cache, message_packet->set);
					 cache_dump_queue(cache->write_back_buffer);


					fatal("cgm_mesi_l2_write_back(): %s write back missing in cache %s writeback_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
						cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
						message_packet->write_back_id, message_packet->address, message_packet->address & cache->block_address_mask,
						message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);

					//fatal("cgm_mesi_l2_write_back(): block not in cache or wb at L2 on L1 WB. this should not be happening anymore\n");

					/*it is possible for the WB from L1 D to miss at the L2. This means there was a recent L2 eviction of the block
					if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
					{
						if(LEVEL == 2 || LEVEL == 3)
						{
							printf("block 0x%08x %s write back fwd (to L3) ID %llu type %d cycle %llu\n",
									(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);
						}
					}*/

				}
				break;

			case cgm_cache_block_exclusive:
			case cgm_cache_block_modified:

				//hit in cache merge write back here.
				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s write back - cache merge ID %llu type %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);
					}
				}

				error = cache_validate_block_flushed_from_l1(cache->id, message_packet->address);
				assert(error == 0);

				//destroy the L1 D WB message. L2 will clear its WB at an opportune time.
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);
				break;
		}
	}
	//if here the L2 generated it's own write back.
	else if(cache->last_queue == cache->write_back_buffer)
	{
		//the wb should not be waiting on a flush to finish.
		assert(message_packet->flush_pending == 0); //verify that the wb has completed it's flush.
		assert(*cache_block_hit_ptr == 0); //verify block is not in cache.

		//verify that the block is out of L1
		error = cache_validate_block_flushed_from_l1(cache->id, message_packet->address);
		assert(error == 0);

		//verify that there is only one wb in L2 for this block.
		error = cache_search_wb_dup_packets(cache, message_packet->tag, message_packet->set);
		assert(error == 1); //error == 1 i.e only one wb packet and we are about to send it.

		//if the line is still in the exclusive state at this point drop it.
		if(message_packet->cache_block_state == cgm_cache_block_exclusive)
		{

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s write back destroy ID %llu type %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);
				}
			}

			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);
		}
		else if (message_packet->cache_block_state == cgm_cache_block_modified)
		{
			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s write back sent (to L3) %llu type %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);
				}
			}

			//block is dirty send the write back down to the L3 cache.
			l3_map = cgm_l3_cache_map(message_packet->set);
			message_packet->l2_cache_id = cache->id;
			message_packet->l2_cache_name = cache->name;

			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);
			message_packet->dest_name = l3_caches[l3_map].name;
			message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

			//send the write back to the L3 cache.
			cache_put_io_down_queue(cache, message_packet);


			cache->TotalWriteBacks++;
		}
		else
		{
			fatal("cgm_mesi_l2_write_back(): Invalid block state in write back buffer cycle %llu\n", P_TIME);
		}

		return 0;
	}

	return 1;
}

void cgm_mesi_l3_get(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int num_cores = x86_cpu_num_cores;
	int sharers, owning_core, pending_bit;

	struct cgm_packet_t *write_back_packet = NULL;

	enum cgm_cache_block_state_t block_trainsient_state;


	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);
	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);
	//get block transient state
	block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	//assert(victim_trainsient_state == cgm_cache_block_transient);

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	/*stats*/
	if(*cache_block_hit_ptr == 0)
	{
		cache->TotalMisses++;
		cache->TotalReadMisses++;
		cache->TotalGet++;
	}

	//if access to the block is pending send nack back to requesting core.
	if(pending_bit == 1 && *cache_block_hit_ptr == 1)
	{
		/*there should be at least 1 or more sharers
		and the requester should not be the owning core
		because the access should be coalesced.*/
		//assert(sharers >= 1 &&  owning_core == 0);

		/*stats*/
		cache->TotalMisses++;
		cache->TotalReadMisses++;

		/*printf("access id %llu hit_ptr %d address 0x%08x\n", message_packet->access_id, *cache_block_hit_ptr, message_packet->address);
		getchar();*/

		//send the reply up as a NACK!
		message_packet->access_type = cgm_access_get_nack;

		//set message package size
		message_packet->size = 1;

		//update routing headers
		message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
		message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
		message_packet->src_name = cache->name;
		message_packet->src_id = str_map_string(&node_strn_map, cache->name);

		//send the reply
		cache_put_io_up_queue(cache, message_packet);

		return;
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		fatal("l3_cache_ctrl(): Get invalid block state on hit as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
			break;

		case cgm_cache_block_invalid:

			//stats;
			//cache->misses++;

			/*write back is chilling out in the wb buffer!!!*/
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/
				assert(*cache_block_hit_ptr == 0);
				assert(write_back_packet->cache_block_state == cgm_cache_block_modified || write_back_packet->cache_block_state == cgm_cache_block_exclusive);
				assert(message_packet->set == write_back_packet->set && message_packet->tag == write_back_packet->tag);
				assert(message_packet->access_type == cgm_access_get || message_packet->access_type == cgm_access_load_retry);

				/*we have a request for a block that is in L3 WB and not in the cache.
				if the number of transient ways is less than the cache's assoc we
				can write the block into the cache and service the request as modified.
				If not we coalesce, because all ways will be transient.*/

				//see if we can write it back into the cache.
				write_back_packet->l3_victim_way = cgm_cache_get_victim_for_wb(cache, write_back_packet->set);

				//if not then we must coalesce
				if(write_back_packet->l3_victim_way == -1)
				{
					//Set and ways are all transient must coalesce
					cache_check_ORT(cache, message_packet);

					assert(message_packet->coalesced == 1);

					if(message_packet->coalesced == 1)
					{
						return;
					}
					else
					{
						fatal("cgm_mesi_l3_get(): write failed to coalesce when all ways are transient...\n");
					}
				}

				cache->TotalMisses--;
				cache->TotalReadMisses--;
				cache->TotalGet--;

				/*we are writing the block in so evict the victim and flush the copies in the core.*/
				assert(write_back_packet->l3_victim_way >= 0 && write_back_packet->l3_victim_way < cache->assoc);

				//first evict the old block if it isn't invalid already
				if(cgm_cache_get_block_state(cache, write_back_packet->set, write_back_packet->l3_victim_way) != cgm_cache_block_invalid)
					cgm_L3_cache_evict_block(cache, write_back_packet->set, write_back_packet->l3_victim_way,
							cgm_cache_get_num_shares(cache, message_packet->set, message_packet->l3_victim_way), NULL);


				cgm_cache_set_block(cache, write_back_packet->set, write_back_packet->l3_victim_way, write_back_packet->tag, write_back_packet->cache_block_state);
				//clear the old directory entry
				cgm_cache_clear_dir(cache,  write_back_packet->set, write_back_packet->l3_victim_way);
				//set the new directory entry
				cgm_cache_set_dir(cache, write_back_packet->set, write_back_packet->l3_victim_way, message_packet->l2_cache_id);

				//check for retries on successful cache read...
				if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
				{
					//enter retry state.
					cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
				}

				//set message size
				message_packet->size = l2_caches[message_packet->l2_cache_id].block_size;
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
					fatal("cgm_mesi_l3_get(): invalid write back block state\n");
				}

				//free the write back
				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				/*stats*/
				cache->TotalReads++;

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s load wb hit (get) id %llu state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);
					}
				}

				//update routing headers
				message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
				message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);

				/*stats*/
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L3_hit;

				cache_put_io_up_queue(cache, message_packet);
			}
			else
			{

				assert(message_packet->cpu_access_type == cgm_access_load);

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s load miss ID %llu type %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, P_TIME);
					}
				}

				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
				{
					if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
					{
						if(LEVEL == 2 || LEVEL == 3)
						{
							printf("block 0x%08x %s load miss coalesce ID %llu type %d state %d cycle %llu\n",
									(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
						}
					}

					return;
				}

				//find victim .
				message_packet->l3_victim_way = cgm_cache_get_victim(cache, message_packet->set);
				assert(message_packet->l3_victim_way >= 0 && message_packet->l3_victim_way < cache->assoc);

				//evict the block
				if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l3_victim_way) != cgm_cache_block_invalid)
					cgm_L3_cache_evict_block(cache, message_packet->set, message_packet->l3_victim_way,
							cgm_cache_get_num_shares(cache, message_packet->set, message_packet->l3_victim_way), NULL);

				//clear the directory entry
				cgm_cache_clear_dir(cache, message_packet->set, message_packet->l3_victim_way);

				//add some routing/status data to the packet
				message_packet->access_type = cgm_access_mc_load;

				message_packet->cache_block_state = cgm_cache_block_exclusive;

				//set dest and src
				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);
				message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
				message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

				//transmit to SA/MC
				if(!message_packet->protocol_case)
					message_packet->protocol_case = memory;

				cache_put_io_down_queue(cache, message_packet);

			}

			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			/*on the first GET the block should have been brought in as exclusive.
			Then it will be a hit on retry with no presence bits set (exclusive).
			On a subsequent access (by either the requesting core or a different core) the block will be here as exclusive,
			if the request comes from the original core the block can be sent as exclusive again.
			if the request comes from a different core the block will need to be downgraded to shared before sending to requesting core.
			Once the block is downgraded to shared it will be in both cores and L3 as shared*/

			assert(sharers >= 0 && sharers <= num_cores);
			assert(owning_core >= 0 && owning_core <= 1);

			//check if the packet has coalesced accesses.
			if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			//if it is a new access (L3 retry) or a repeat access from an already owning core.
			if(sharers == 0 || owning_core == 1)
			{
				if(owning_core == 1)
				{
					/*there should be only 1 core with the block*/
					assert(sharers == 1);
				}

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s load hit single shared ID %llu type %d state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
					}
				}

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

				//set the presence bit in the directory for the requesting core.
				cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

				cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

				//set message package size
				message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

				//update routing headers
				message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
				message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);

				/*stats*/
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L3_hit;

				//send the cache block out
				cache_put_io_up_queue(cache, message_packet);

				/*if(message_packet->access_id == 4449325 || message_packet->access_id == 4449322)
				{
					printf("\tL3 received access id %llu cycle %llu\n", message_packet->access_id, P_TIME);
					cache_dump_request_queue(cache->Tx_queue_top);
				}*/

			}
			else if (sharers >= 1)
			{
				/*if it is a new access from another core(s).
				We need to downgrade the owning core.
				also, the owning core may have the block dirty
				so we may need to process a sharing write back*/

				/*in the exclusive/modified state there should only be one core with the cache block
				and there are no outstanding accesses to this block*/

				assert(sharers == 1 && pending_bit == 0);

				//forward the GET to the owning core*/

				//change the access type
				message_packet->access_type = cgm_access_get_fwd;

				//don't set the block state (yet)

				//don't set the presence bit in the directory for the requesting core (yet).

				//don't change the message package size (yet).

				//pending bit should be zero

				//set the directory pending bit.
				cgm_cache_set_dir_pending_bit(cache, message_packet->set, message_packet->way);

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s load hit multi share (get_fwd) ID %llu type %d state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
					}
				}

				/*update the routing headers.
				set src as requesting cache and dest as owning cache.
				We can derive the home (directory) later from the original access address.*/

				//get the id of the owning core L2
				owning_core = cgm_cache_get_xown_core(cache, message_packet->set, message_packet->way);

				//owning node
				message_packet->dest_name = str_map_value(&l2_strn_map, owning_core);
				message_packet->dest_id = str_map_string(&node_strn_map, message_packet->dest_name);

				//requesting node L2
				message_packet->src_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
				message_packet->src_name = str_map_value(&node_strn_map, message_packet->src_id);

				/*stats*/
				if(!message_packet->protocol_case)
					message_packet->protocol_case = get_fwd;

				cache_put_io_up_queue(cache, message_packet);

				/*if(message_packet->access_id == 4449325 || message_packet->access_id == 4449322)
				{
					printf("\tL3 received access id %llu cycle %llu\n", message_packet->access_id, P_TIME);
					cache_dump_request_queue(cache->Tx_queue_top);
				}*/

			}
			else
			{
				fatal("cgm_mesi_l3_get(): invalid sharer/owning_core state\n");
			}
			break;

		case cgm_cache_block_shared:

			//check if the packet has coalesced accesses.
			if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s load hit ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}

			//update message status
			message_packet->access_type = cgm_access_puts;

			//get the cache block state
			message_packet->cache_block_state = *cache_block_state_ptr;

			//set the presence bit in the directory for the requesting core.
			cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

			//set message package size
			message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

			//update routing
			message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
			message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);

			/*stats*/
			if(!message_packet->protocol_case)
				message_packet->protocol_case = L3_hit;

			cache_put_io_up_queue(cache, message_packet);

			break;
	}

	return;
}

void cgm_mesi_l3_getx(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*fatal("L3 getx\n");*/

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int num_cores = x86_cpu_num_cores;
	int sharers, owning_core, xowning_core, pending_bit;

	struct cgm_packet_t *upgrade_putx_n_inval_request_packet;
	enum cgm_cache_block_state_t block_trainsient_state;
	struct cgm_packet_t *write_back_packet = NULL;

	int i = 0;
	int l2_src_id;
	char *l2_name;

	//charge latency
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	//get the status of the cache block AGAIN, because we may have merged a WB.
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);
	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);
	//get block transient state
	block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);

	//update cache way list for cache replacement policies.
	if(*cache_block_hit_ptr == 1)
	{
		//make this block the MRU
		cgm_cache_update_waylist(&cache->sets[message_packet->set], cache->sets[message_packet->set].way_tail, cache_waylist_head);
	}

	/*stats*/
	if(*cache_block_hit_ptr == 0)
	{
		cache->TotalMisses++;
		cache->TotalWriteMisses++;
		cache->TotalGetx++;
	}

	if (*cache_block_state_ptr == cgm_cache_block_shared)
	{
		cache->TotalMisses++;
		cache->TotalWriteMisses++;
		cache->TotalUpgrades++;
	}

	if(pending_bit == 1 && *cache_block_hit_ptr == 1)
	{
		/*there should be at least 1 or more sharers
		and the requester should not be the owning core
		because the access should be coalesced.*/
		//assert(num_sharers >= 1 &&  owning_core == 0);

		/*stats*/
		cache->TotalMisses++;
		cache->TotalWriteMisses++;

		//send the reply up as a NACK!
		message_packet->access_type = cgm_access_getx_nack;

		//set message package size
		message_packet->size = 1;

		//update routing headers
		message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
		message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
		message_packet->src_name = cache->name;
		message_packet->src_id = str_map_string(&node_strn_map, cache->name);

		//send the reply
		cache_put_io_up_queue(cache, message_packet);
		return;
	}


	switch(*cache_block_state_ptr)
	{

		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
			fatal("l3_cache_ctrl(): Invalid block state on hit\n");
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
				write_back_packet->l3_victim_way = cgm_cache_get_victim_for_wb(cache, write_back_packet->set);

				//if not then we must coalesce
				if(write_back_packet->l3_victim_way == -1)
				{
					//Set and ways are all transient must coalesce
					cache_check_ORT(cache, message_packet);

					assert(message_packet->coalesced == 1);

					if(message_packet->coalesced == 1)
					{
						return;
					}
					else
					{
						fatal("cgm_mesi_l3_getx(): write failed to coalesce when all ways are transient...\n");
					}
				}

				/*stats*/
				cache->TotalMisses--;
				cache->TotalWriteMisses--;
				cache->TotalGetx--;

				//success now move block from wb to cache
				/*we are writing the block in so evict the victim and flush the copies in the core.*/
				assert(write_back_packet->l3_victim_way >= 0 && write_back_packet->l3_victim_way < cache->assoc);

				//first evict the old block if it isn't invalid already
				if(cgm_cache_get_block_state(cache, write_back_packet->set, write_back_packet->l3_victim_way) != cgm_cache_block_invalid)
					cgm_L3_cache_evict_block(cache, write_back_packet->set, write_back_packet->l3_victim_way,
							cgm_cache_get_num_shares(cache, message_packet->set, message_packet->l3_victim_way), NULL);

				cgm_cache_set_block(cache, write_back_packet->set, write_back_packet->l3_victim_way, write_back_packet->tag, write_back_packet->cache_block_state);
				//clear the old directory entry
				cgm_cache_clear_dir(cache,  write_back_packet->set, write_back_packet->l3_victim_way);
				//set the new directory entry
				cgm_cache_set_dir(cache, write_back_packet->set, write_back_packet->l3_victim_way, message_packet->l2_cache_id);

				//check for retries on successful cache read...
				if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
				{
					//enter retry state.
					cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
				}

				//set message size
				message_packet->size = l2_caches[message_packet->l2_cache_id].block_size;
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
					fatal("cgm_mesi_l3_getx(): invalid write back block state\n");
				}

				//free the write back
				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				free(write_back_packet);

				/*stats*/
				cache->TotalReads++;

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s store wb hit (getx) id %llu state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, *cache_block_state_ptr, P_TIME);
					}
				}

				//update routing headers
				message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
				message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);

				/*stats*/
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L3_hit;

				cache_put_io_up_queue(cache, message_packet);
			}
			else
			{

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s store miss ID %llu type %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, P_TIME);
					}
				}

				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
				{
					if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
					{
						if(LEVEL == 2 || LEVEL == 3)
						{
							printf("block 0x%08x %s store miss coalesce ID %llu type %d state %d cycle %llu\n",
									(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
						}
					}
					return;
				}

				//find victim because LRU has been updated on hits.
				/*message_packet->l3_victim_way = cgm_cache_replace_block(cache, message_packet->set);*/
				message_packet->l3_victim_way = cgm_cache_get_victim(cache, message_packet->set);
				assert(message_packet->l3_victim_way >= 0 && message_packet->l3_victim_way < cache->assoc);

				//evict the victim
				if(cgm_cache_get_block_state(cache, message_packet->set, message_packet->l3_victim_way) != cgm_cache_block_invalid)
					cgm_L3_cache_evict_block(cache, message_packet->set, message_packet->l3_victim_way,
							cgm_cache_get_num_shares(cache, message_packet->set, message_packet->l3_victim_way), NULL);

				//clear the directory entry
				cgm_cache_clear_dir(cache, message_packet->set, message_packet->l3_victim_way);

				//add some routing/status data to the packet
				message_packet->access_type = cgm_access_mc_load;

				//set the returned block state
				message_packet->cache_block_state = cgm_cache_block_modified;

				//set dest and src
				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);
				message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
				message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

				/*stats*/
				if(!message_packet->protocol_case)
					message_packet->protocol_case = memory;

				//transmit to SA
				cache_put_io_down_queue(cache, message_packet);
			}

			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			//stats;
			//cache->hits++;

			/*on the first GET the block should have been brought in as exclusive.
			Then it will be a hit on retry with no presence bits set (exclusive).
			On a subsequent access (by either the requesting core or a different core) the block will be here as exclusive,
			if the request comes from the original core the block can be sent as exclusive again to be modified.
			if the request comes from a different core the block will need to be invalidated and forwarded to the requesting core.
			the block should only ever be in one core if not downgraded to shared*/

			assert(sharers >= 0 && sharers <= num_cores);
			assert(owning_core >= 0 && owning_core <= 1);

			//check if the packet has coalesced accesses.
			if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			//if it is a new access (L3 retry) or a repeat access from an already owning core.
			if(sharers == 0 || owning_core == 1)
			{
				//if the block is in the E state set M before sending up
				if(*cache_block_state_ptr == cgm_cache_block_exclusive)
				{
					cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);
				}

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s store hit single shared ID %llu type %d state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
					}
				}

				//update message status
				message_packet->access_type = cgm_access_putx;

				//set cache block state modified
				message_packet->cache_block_state = cgm_cache_block_modified;

				//update directory
				cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

				cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

				// update message packet size
				message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

				//update routing headers
				message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
				message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);
				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);

				//printf("Sending %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));


				/*stats*/
				if(!message_packet->protocol_case)
					message_packet->protocol_case = L3_hit;

				cache_put_io_up_queue(cache, message_packet);

			}
			else if(sharers >= 1)
			{
				//in the exclusive state there should only be one core with the cache block
				//there better be only one owning core at this stage.
				assert(sharers == 1);

				/*printf("L3 id %d sending Getx_fwd access id %llu cycle %llu\n", l3_caches[my_pid].id, message_packet->access_id, P_TIME);
				temp_id = message_packet->access_id;
				STOP;*/

				/*forward the GETX to the owning core*/

				//change the access type
				message_packet->access_type = cgm_access_getx_fwd;

				//don't set the block state (yet)

				//don't set the presence bit in the directory for the requesting core (yet).

				//don't change the message package size (yet).

				//set the directory pending bit.
				cgm_cache_set_dir_pending_bit(cache, message_packet->set, message_packet->way);

				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s store hit multi share (getx_fwd) ID %llu type %d state %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
					}
				}

				/*update the routing headers.
				set src as requesting cache and dest as owning cache.
				We can derive the home (directory) later from the original access address.*/

				//get the id of the owning core L2
				xowning_core = cgm_cache_get_xown_core(cache, message_packet->set, message_packet->way);

				//owning node
				message_packet->dest_name = str_map_value(&l2_strn_map, xowning_core);
				message_packet->dest_id = str_map_string(&node_strn_map, message_packet->dest_name);

				//requesting node L2
				message_packet->src_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
				message_packet->src_name = str_map_value(&node_strn_map, message_packet->src_id);

				/*stats*/
				if(!message_packet->protocol_case)
					message_packet->protocol_case = getx_fwd;

				cache_put_io_up_queue(cache, message_packet);
			}

			break;

		case cgm_cache_block_shared:

			//stats
			cache->upgrade_misses++;

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s store hit shared (putx n) ID %llu type %d state %d num_shares %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, sharers, P_TIME);
				}
			}

			/*access was a miss at the L1 and L2 but hit as shared in L3
			we need to process an upgrade as a putx with n number of invals*/

			/*there should always be at least one sharer
			but no more than the number of cores.*/
			assert(sharers >= 1 && sharers <= num_cores);

			//build the upgrade_ack packet
			//set access type
			message_packet->access_type = cgm_access_upgrade_putx_n;
			message_packet->cache_block_state = cgm_cache_block_modified;

			//set number of sharers
			//if there is 1 sharer and its the owning core set 0
			if(sharers == 1 && owning_core == 1)
			{
				message_packet->upgrade_ack = 0;
			}
			else
			{
				/*set the number of inval_acks expected to receive*/
				message_packet->upgrade_ack = sharers;

				/*subtract 1 if the core is already marked as owning*/
				if(owning_core == 1)
				{
					message_packet->upgrade_ack--;
				}
			}

			//initialize the ack counter
			message_packet->upgrade_inval_ack_count = 0;

			//set destination
			message_packet->dest_id = message_packet->src_id;
			message_packet->dest_name = message_packet->src_name;

			l2_src_id = message_packet->src_id;
			l2_name = strdup(message_packet->src_name);

			//set the source of the packet as L3
			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);

			cache_put_io_up_queue(cache, message_packet);

			//invalidate the other sharers
			for(i = 0; i < num_cores; i++)
			{
				//find the other cores
				if(cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, i) && i != message_packet->l2_cache_id)
				{
					//create and init the upgrade_inval packet
					upgrade_putx_n_inval_request_packet = packet_create();
					init_upgrade_putx_n_inval_request_packet(upgrade_putx_n_inval_request_packet, message_packet->address);

					//testing
					upgrade_putx_n_inval_request_packet->access_id = message_packet->access_id;
					//testing

					upgrade_putx_n_inval_request_packet->dest_name = str_map_value(&l2_strn_map, i);
					upgrade_putx_n_inval_request_packet->dest_id = str_map_string(&node_strn_map, upgrade_putx_n_inval_request_packet->dest_name);

					//requesting node L2
					upgrade_putx_n_inval_request_packet->src_id = str_map_string(&node_strn_map, l2_name);
					upgrade_putx_n_inval_request_packet->src_name = str_map_value(&node_strn_map, l2_src_id);

					list_enqueue(cache->Tx_queue_top, upgrade_putx_n_inval_request_packet);
					advance(cache->cache_io_up_ec);
				}
			}

			//free the temp string
			free(l2_name);

			//set local cache block and directory to modified.
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);

			//clear the directory
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

			//set the directory pending bit.
			//cgm_cache_set_dir_pending_bit(cache, message_packet->set, message_packet->way);

			//set the sharer bit for the upgraded node
			cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

			break;
	}

	return;
}

void cgm_mesi_l3_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	enum cgm_cache_block_state_t block_trainsient_state;
	struct cgm_packet_t *write_back_packet = NULL;

	int pending_bit, sharers;

	//downgrade the line to shared and add sharers
	//fatal("here\n");

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);
	assert(pending_bit == 1);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	assert(sharers == 1);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);
	assert(!write_back_packet);

	//check transient state
	block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	/*assert(block_trainsient_state == cgm_cache_block_transient);*/

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s downgrade ack ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
		case cgm_cache_block_invalid:
		fatal("cgm_mesi_l3_downgrade_ack(): L3 id %d invalid block state on down_grade_ack as %s access id %llu address %u tag %d set %d way %d\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->access_id, message_packet->address, message_packet->tag, message_packet->set, message_packet->way);
			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			//processes sharing WB if there is one
			//move the block to the WB buffer
			if(message_packet->cache_block_state == cgm_cache_block_modified)
			{
				struct cgm_packet_t *write_back_packet = packet_create();
				assert(write_back_packet);

				init_write_back_packet(cache, write_back_packet, message_packet->set, message_packet->way, 0, cgm_cache_block_modified);

				//add routing/status data to the packet
				write_back_packet->access_type = cgm_access_mc_store;
				write_back_packet->size = cache->block_size;

				write_back_packet->src_name = cache->name;
				write_back_packet->src_id = str_map_string(&node_strn_map, cache->name);
				write_back_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
				write_back_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

				//transmit to SA/MC
				list_enqueue(cache->Tx_queue_bottom, write_back_packet);
				advance(cache->cache_io_down_ec);
			}

			//the modified block is written to main memory we can set the block as shared now.

			//downgrade the local block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_shared);

			//set the new sharer bit in the directory
			cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

			cgm_cache_clear_dir_pending_bit(cache, message_packet->set, message_packet->way);
			assert(cache->sets[message_packet->set].blocks[message_packet->way].directory_entry.entry_bits.pending == 0);

			//go ahead and destroy the downgrade message because we don't need it anymore.
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);
			break;
	}

	/*its a down grade which leaves the owning core and requesting core with the block in the shared state
	so we better have two cores with the block.*/
	assert(cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way) == 2);

	return;
}

void cgm_mesi_l3_downgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int sharers, pending_bit;
	int error = 0;

	enum cgm_cache_block_state_t block_trainsient_state;

	struct cgm_packet_t *write_back_packet = NULL;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);
	if(pending_bit != 1)
	{
		ort_dump(cache);
		cgm_cache_dump_set(cache, message_packet->set);

		fatal("cgm_mesi_l3_downgrade_nack(): %s block not in transient state access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d cycle %llu\n",
			cache->name, message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
			message_packet->set, message_packet->tag, message_packet->way, P_TIME);
	}

	assert(pending_bit == 1);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	assert(sharers == 0 || sharers == 1);
	if(sharers == 0)
	{
		error = cache_validate_block_flushed_from_core(message_packet->l2_cache_id, message_packet->address);
		assert(error == 0);

		/*cgm_cache_dump_set(cache, message_packet->set);

		//printf("pr set %d tag %d pr way %d mp set %d mp tag %d mp way %d\n",
			//write_back_packet->set, write_back_packet->tag, write_back_packet->way, message_packet->set, message_packet->tag, message_packet->way);

		fatal("cgm_mesi_l2_get_fwd(): %s access id %llu blk_addr 0x%08x type %d start_cycle %llu end_cycle %llu\n",
				cache->name, message_packet->access_id, message_packet->address & cache->block_address_mask, message_packet->access_type,
				message_packet->start_cycle, message_packet->end_cycle);*/
	}

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);
	assert(!write_back_packet);

	//check transient state
	block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	/*assert(block_trainsient_state == cgm_cache_block_transient);*/

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s downgrade nack ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	//failed to downgrade block in sharing core, the block may not be present
	//if hit clear directory state and set retry
	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
		case cgm_cache_block_invalid:
			fatal("cgm_mesi_l3_downgrade_nack(): L3 id %d invalid block state on down_grade_nack as %s access id %llu address %u tag %d set %d way %d\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->access_id, message_packet->address, message_packet->tag, message_packet->set, message_packet->way);
			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:


			/*if(sharers != 1)
			{
				ort_dump(cache);
				cgm_cache_dump_set(cache, message_packet->set);

				fatal("cgm_mesi_l3_downgrade_nack(): %s sharer == 1 access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d cycle %llu\n",
						cache->name, message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
						message_packet->set, message_packet->tag, message_packet->way, P_TIME);
			}

			assert(sharers == 1);*/


			/*nack from L2 clear the directory pending bit retry will come from requesting L2*/
			/*cgm_cache_clear_dir_pending_bit(cache, message_packet->set, message_packet->way);*/
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);
			assert(cache->sets[message_packet->set].blocks[message_packet->way].directory_entry.entry_bits.pending == 0);

			//destroy the nack and wait for the retry
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			/*OLD CODE*/
			/*nack from L2 clear the directory and retry the access as a new access*/
			/*cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);
			assert(cache->sets[message_packet->set].blocks[message_packet->way].directory_entry.entry_bits.pending == 0);

			//set the block state
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, *cache_block_state_ptr);

			retry the access at the L3 level
			message_packet->access_type = cgm_access_get;*/

			break;
	}

	return;
}

void cgm_mesi_l3_getx_fwd_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int sharers, pending_bit;

	enum cgm_cache_block_state_t block_trainsient_state;

	struct cgm_packet_t *write_back_packet = NULL;

	//charge delay
	P_PAUSE(cache->latency);

	//store so provide to one core

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);
	assert(pending_bit == 1);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	assert(sharers == 1);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);
	assert(!write_back_packet);

	//check transient state
	block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	/*assert(block_trainsient_state == cgm_cache_block_transient);*/

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s getx_fwd_ack ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}


	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
		case cgm_cache_block_invalid:
		fatal("cgm_mesi_l3_getx_fwd_ack(): L3 id %d invalid block state on getx_fwd_ack as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;


			/*//check WB for line...
			if(write_back_packet)
			{
				found the packet in the write back buffer
				data should not be in the rest of the cache

				assert((write_back_packet->cache_block_state == cgm_cache_block_modified
						|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

				fatal("l3 getx fwd ack\n");
			}
			else
			{
				fatal("l3 miss on downgrade ack check this\n");
			}

			break;*/

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			//this handles the sharing write back as well.

			//set the local block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);

			//clear the directory
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);
			assert(cache->sets[message_packet->set].blocks[message_packet->way].directory_entry.entry_bits.pending == 0);

			//set the new sharer bit in the directory
			cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

			//go ahead and destroy the getx_fwd_ack message because we don't need it anymore.
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			break;
	}

	//block should only be in one core
	assert(cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way) == 1);

	return;
}

void cgm_mesi_l3_get_fwd_upgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int num_sharers, owning_core, pending_bit, xowning_core;

	enum cgm_cache_block_state_t victim_trainsient_state;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//block should be valid and not in a transient state
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	assert(victim_trainsient_state != cgm_cache_block_transient);

	//get number of sharers
	num_sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);
	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	/*there should only be one core with the block and it shouldn't be the requesting core retry the get_fwd*/
	assert(num_sharers == 1 && owning_core == 0 && pending_bit == 1 && *cache_block_hit_ptr == 1);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s get_fwd_upgrade_nack ID %llu type %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, P_TIME);
		}
	}

	//This is a nack to a GetX_FWD that found the block in in a pending state at the owning L2
	//the local block should be in the pending state and there should only be one sharer

	/*cgm_cache_dump_set(cache, message_packet->set);

	unsigned int temp = message_packet->address;
	temp = temp & cache->block_address_mask;

	printf("set %d way %d tag %d\n", message_packet->set, message_packet->way, message_packet->tag);
	getchar();*/

	//assert(cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way) == 1);
	//assert(cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way) == 0);

	//add some routing/status data to the packet
	message_packet->access_type = cgm_access_get_fwd;

	//get the id of the owning core L2
	xowning_core = cgm_cache_get_xown_core(cache, message_packet->set, message_packet->way);

	//owning node
	message_packet->dest_name = str_map_value(&l2_strn_map, xowning_core);
	message_packet->dest_id = str_map_string(&node_strn_map, message_packet->dest_name);

	//requesting node L2
	//message_packet->src_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
	//message_packet->src_name = str_map_value(&node_strn_map, message_packet->src_id);

	cache_put_io_up_queue(cache, message_packet);

	return;

}

void cgm_mesi_l3_getx_fwd_upgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int num_sharers, owning_core, pending_bit, xowning_core;

	enum cgm_cache_block_state_t victim_trainsient_state;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//block should be valid and not in a transient state
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	assert(victim_trainsient_state != cgm_cache_block_transient);

	//get number of sharers
	num_sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);
	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	/*there should only be one core with the block and it shouldn't be the requesting core retry the GetX*/
	assert(num_sharers == 1 && owning_core == 0 && pending_bit == 1 && *cache_block_hit_ptr == 1);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s getx_upgrade_nack ID %llu type %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, P_TIME);
		}
	}

	//This is a nack to a GetX_FWD that found the block in in a pending state at the owning L2
	//the local block should be in the pending state and there should only be one sharer

	/*cgm_cache_dump_set(cache, message_packet->set);

	unsigned int temp = message_packet->address;
	temp = temp & cache->block_address_mask;

	printf("set %d way %d tag %d\n", message_packet->set, message_packet->way, message_packet->tag);
	getchar();*/

	//assert(cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way) == 1);
	//assert(cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way) == 0);

	//add some routing/status data to the packet
	message_packet->access_type = cgm_access_getx_fwd;

	//get the id of the owning core L2
	xowning_core = cgm_cache_get_xown_core(cache, message_packet->set, message_packet->way);

	//owning node
	message_packet->dest_name = str_map_value(&l2_strn_map, xowning_core);
	message_packet->dest_id = str_map_string(&node_strn_map, message_packet->dest_name);

	//requesting node L2
	//message_packet->src_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
	//message_packet->src_name = str_map_value(&node_strn_map, message_packet->src_id);

	cache_put_io_up_queue(cache, message_packet);

	return;

}


void cgm_mesi_l3_getx_fwd_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int sharers, pending_bit;

	/*enum cgm_cache_block_state_t block_trainsient_state;*/

	struct cgm_packet_t *write_back_packet = NULL;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);
	if(pending_bit != 1)
	{
		fatal("cgm_mesi_l3_getx_fwd_nack(): pending_bit %d  %s access_id %llu access type %s address 0x%08x blk addr 0x%08x cycle %llu\n",
				pending_bit, cache->name, message_packet->access_id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
				message_packet->address, message_packet->address & cache->block_address_mask, P_TIME);
	}
	assert(pending_bit == 1);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	assert(sharers == 1 || sharers == 0);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);
	assert(!write_back_packet);

	//check transient state
	/*block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);*/
	/*assert(block_trainsient_state == cgm_cache_block_transient);*/

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s getx_fwd_nack ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	//failed to downgrade block in sharing core, the block may not be present
	//if hit clear directory state and set retry

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
		case cgm_cache_block_invalid:
			fatal("cgm_mesi_l3_getx_fwd_nack(): L3 id %d invalid block state on getx_fwd_nack as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;


		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			assert(sharers == 1);
			/*nack from L2 clear the directory pending bit retry will come from requesting L2*/
			/*cgm_cache_clear_dir_pending_bit(cache, message_packet->set, message_packet->way);*/
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);
			assert(cache->sets[message_packet->set].blocks[message_packet->way].directory_entry.entry_bits.pending == 0);

			//destroy the nack and wait for the retry
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			/*OLD CODE*/
			//clear the cache dir
			/*cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);
			assert(cache->sets[message_packet->set].blocks[message_packet->way].directory_entry.entry_bits.pending == 0);

			//set the block state
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);

			retry the access at the L3 level
			its possible that L3 may have evicted the block
			message_packet->access_type = cgm_access_getx;*/

			break;
	}

	return;
}


void cgm_mesi_l3_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	assert(cache->cache_type == l3_cache_t);
	assert((message_packet->access_type == cgm_access_mc_put && message_packet->cache_block_state == cgm_cache_block_modified)
			|| (message_packet->access_type == cgm_access_mc_put && message_packet->cache_block_state == cgm_cache_block_exclusive)
			|| (message_packet->access_type == cgm_access_mc_put && message_packet->cache_block_state == cgm_cache_block_shared));


	enum cgm_cache_block_state_t victim_trainsient_state;

	P_PAUSE(cache->latency);


	//find the access in the ORT table and clear it.
	ort_clear(cache, message_packet);

	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l3_victim_way);
	assert(victim_trainsient_state == cgm_cache_block_transient);
	assert(cache->sets[message_packet->set].blocks[message_packet->l3_victim_way].directory_entry.entry == 0);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s write block ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, message_packet->cache_block_state, P_TIME);
		}
	}

	//set the block data
	cgm_cache_set_block(cache, message_packet->set, message_packet->l3_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);

	/*if(message_packet->access_id == 1638361)
	{
		fatal("here\n");
	}*/

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	return;
}

void cgm_mesi_l3_flush_block_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*block is flushed out of its owning core
	we will only get an ack in the case of a single core being in the M or E state
	or if the block is not in L2 cache or writeback i.e we don't know if the line was dirty or not.*/
	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *wb_packet = NULL;
	struct cgm_packet_t *pending_request_packet = NULL;
	int l3_map = 0;
	int error = 0;

	//charge delay
	P_PAUSE(cache->latency);

	//get the address set and tag
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	/*error checking the block should no longer be "in core"*/
	error = cache_validate_block_flushed_from_core(message_packet->l2_cache_id, message_packet->address);
	assert(error == 0);

	/*block should not be in L3 cache either*/
	assert(*cache_block_state_ptr == 0);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s flush block ack ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	//state should be either invalid of modified.
	assert(message_packet->cache_block_state == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_invalid);

	//find the block in the local WB buffer
	wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if(wb_packet)
	{
		/*if incoming data from core data is dirty*/
		if(wb_packet->cache_block_state == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_modified)
		{
			/*no join bit at L3 so this sould be clear*/
			assert(wb_packet->L3_flush_join == 0);

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
			packet_destroy(wb_packet);
		}
	}

	//free the message packet
	message_packet = list_remove(cache->last_queue, message_packet);
	packet_destroy(message_packet);

	return;
}


int cgm_mesi_l3_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;
	struct cgm_packet_t *wb_packet;
	enum cgm_cache_block_state_t block_trainsient_state;
	int error = 0;
	int pending_bit = 0;

	//charge the delay
	P_PAUSE(cache->latency);

	//we should only receive modified lines from L2 cache
	assert(message_packet->cache_block_state == cgm_cache_block_modified);

	//get the state of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//check for block transient state
	block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	if(block_trainsient_state == cgm_cache_block_transient)
	{
		/*if potentially merging in cache the block better not be transient, check that the tags don't match
		if they don't match the block is missing from both the cache and wb buffer when it should not be*/

		//check that the tags don't match. This should not happen as the request should have been coalesced at L1 D.
		assert(message_packet->tag != cache->sets[message_packet->set].blocks[message_packet->way].tag);
	}

	//WB from L2 cache
	if(cache->last_queue == cache->Coherance_Rx_queue)
	{
		switch(*cache_block_state_ptr)
		{
			case cgm_cache_block_noncoherent:
			case cgm_cache_block_owned:
			case cgm_cache_block_shared:
				cgm_cache_dump_set(cache, message_packet->set);

				fatal("cgm_mesi_l3_write_back(): %s invalid block state on write back as %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
					cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
					message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
					message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);

				break;

			case cgm_cache_block_invalid:

				//check the WB buffer
				wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

				if(wb_packet)
				{
					/*if the block is in the wb buffer it shouldn't be in the cache.*/
					if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
					{
						if(LEVEL == 2 || LEVEL == 3)
						{
							printf("block 0x%08x %s write back ID %llu type %d cycle %llu\n",
									(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);
						}
					}

					//cache block found in the WB buffer merge the change here
					//set modified if the line was exclusive
					wb_packet->cache_block_state = cgm_cache_block_modified;
					assert(wb_packet->flush_pending == 1 && wb_packet->L3_flush_join == 0);

					//destroy the L2 WB packet
					message_packet = list_remove(cache->last_queue, message_packet);
					packet_destroy(message_packet);
				}
				else
				{

					fatal("cgm_mesi_l3_write_back(): miss in L3 write back should no longer happen??\n");

					if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
					{
						if(LEVEL == 2 || LEVEL == 3)
						{
							printf("block 0x%08x %s write back fwd (to MC) ID %llu type %d cycle %llu\n",
									(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);
						}
					}

					//transmit WB to SA/MC
					message_packet->access_type = cgm_access_mc_store;
					message_packet->size = cache->block_size;

					message_packet->src_name = cache->name;
					message_packet->src_id = str_map_string(&node_strn_map, cache->name);
					message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
					message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

					cache_put_io_down_queue(cache, message_packet);
				}
				break;

			case cgm_cache_block_exclusive:
			case cgm_cache_block_modified:

				//hit in cache merge WB here.
				if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
				{
					if(LEVEL == 2 || LEVEL == 3)
					{
						printf("block 0x%08x %s write back - cache merge ID %llu type %d cycle %llu\n",
								(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);
					}
				}

				//set modified if the line was exclusive
				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);

				/*error checking the block should no longer be "in core"*/
				error = cache_validate_block_flushed_from_core(message_packet->l2_cache_id, message_packet->address);
				if(error == 1)
				{
					struct cgm_packet_t *L2_wb_packet = cache_search_wb(&l2_caches[message_packet->l2_cache_id], message_packet->tag, message_packet->set);

					if(L2_wb_packet)
						fatal("wbp found %llu\n", L2_wb_packet->evict_id);

					fatal("cgm_mesi_l3_write_back(): %s error %d as %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
						cache->name, error, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
						message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
						message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);
				}

				//assert(error == 0);


				/*if the block is pending when a write back is coming in don't clear the pending bit
				because the get_fwd or getx_fwd is en-route and will get nacked by the node*/
				pending_bit = cgm_cache_get_dir_pending_bit(cache,  message_packet->set, message_packet->way);

				/*clear the directory for this block*/
				if(pending_bit == 1)
				{
					cgm_cache_clear_dir(cache,  message_packet->set, message_packet->way);
					cgm_cache_set_dir_pending_bit(cache,  message_packet->set, message_packet->way);
				}
				else
				{
					cgm_cache_clear_dir(cache,  message_packet->set, message_packet->way);
				}

				//destroy the L2 WB message. L3 will clear its WB at an opportune time.
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);
				break;
		}
	}
	else if(cache->last_queue == cache->write_back_buffer)
	{
		assert(message_packet->flush_pending == 0);
		assert(*cache_block_hit_ptr == 0);

		/*error checking the block should no longer be "in core"*/
		error = cache_validate_block_flushed_from_core(message_packet->l2_cache_id, message_packet->address);
		assert(error == 0);


		if(message_packet->cache_block_state == cgm_cache_block_exclusive)
		{
			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s write back destroy ID %llu type %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);
				}
			}

			/*drop the write back*/
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);
		}
		else if (message_packet->cache_block_state ==  cgm_cache_block_modified)
		{
			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s write back sent (to MC) %llu type %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->write_back_id, message_packet->access_type, P_TIME);
				}
			}

			//add routing/status data to the packet
			message_packet->access_type = cgm_access_mc_store;
			message_packet->size = cache->block_size;

			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);
			message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
			message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

			//transmit to SA/MC
			cache_put_io_down_queue(cache, message_packet);
		}
		else
		{
			fatal("cgm_mesi_l3_write_back(): Invalid block state in write back buffer cycle %llu\n", P_TIME);
		}

		return 0;
	}

	return 1;
}

void cgm_mesi_l1_d_upgrade_inval(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 1 || LEVEL == 3)
		{
			printf("block 0x%08x %s invalidate ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}


	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:

			fatal("cgm_mesi_l1_d_upgrade_inval(): L1 d id %d invalid block state on upgrade inval as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			//the block was silently dropped and is already invalid so do nothing

			//free the upgrade_inval
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			break;

		case cgm_cache_block_shared:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			//if the block is in the cache invalidate it

			//set local cache block and directory to modified.
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			//free the upgrade_inval
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			break;
	}

	return;
}

void cgm_mesi_l1_d_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	enum cgm_cache_block_state_t victim_trainsient_state;

	//we have permission to upgrade our set block state and retry access

	//charge the delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 1 || LEVEL == 3)
		{
			printf("block 0x%08x %s upgrade ack ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			cgm_cache_dump_set(cache, message_packet->set);

			unsigned int temp = (unsigned int) 0x000422e4;
			temp = temp & cache->block_address_mask;

			fatal("cgm_mesi_l1_d_upgrade_ack(): L1 D id %d invalid block state on upgrade ack as %s set %d way %d tag %d address 0x%08x\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->set, message_packet->way, message_packet->tag, message_packet->address);
			break;


		case cgm_cache_block_invalid:

			/*if the access misses this can be due
			to it originally going out as a GETX on a cache miss
			We need to store the block and set modified before we retry*/

			assert(message_packet->cache_block_state == cgm_cache_block_modified);
			assert(message_packet->access_type == cgm_access_upgrade_ack);
			assert(message_packet->cpu_access_type == cgm_access_store);
			assert(message_packet->l1_victim_way >=0 && message_packet->l1_victim_way < cache->assoc);
			assert(message_packet->coalesced != 1);
			victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);
			assert(victim_trainsient_state == cgm_cache_block_transient);

			//find the access in the ORT table and clear it.
			ort_clear(cache, message_packet);

			//set the block and retry the access in the cache.
			cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, cgm_cache_block_modified);

			//set retry state
			message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);
			assert(message_packet->access_type == cgm_access_store_retry);

			//retry the access
			message_packet = list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->retry_queue, message_packet);

			break;

		case cgm_cache_block_shared:

			//block should be in the shared state
			assert(message_packet->access_type == cgm_access_upgrade_ack);
			assert(message_packet->cpu_access_type == cgm_access_store);
			assert(message_packet->coalesced == 0);

			//find the access in the ORT table and clear it.
			ort_clear(cache, message_packet);

			//clear the block's transient state
			cgm_cache_set_block_transient_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			//set the state to modified and clear the transient state
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);

			//enter the retry state
			message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);
			assert(message_packet->access_type == cgm_access_store_retry);

			message_packet = list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->retry_queue, message_packet);

			break;
	}

	return;
}

int cgm_mesi_l2_upgrade(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//received upgrade request from L1

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	enum cgm_cache_block_state_t victim_trainsient_state;
	struct cgm_packet_t *upgrade_request_packet;
	int l3_map;

	//charge latency
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);

	if(*cache_block_hit_ptr == 1)
		assert(victim_trainsient_state != cgm_cache_block_transient);


	/*if(victim_trainsient_state == cgm_cache_block_transient)
	{
		cgm_cache_dump_set(cache, message_packet->set);

		unsigned int temp = message_packet->address;
		temp = temp & cache->block_address_mask;

		assert((*cache_block_hit_ptr == 1 && victim_trainsient_state != cgm_cache_block_transient) || *cache_block_hit_ptr == 0);

		fatal("cgm_mesi_l2_upgrade(): %s block in transient state access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d hit %d cycle %llu\n",
			cache->name, message_packet->access_id, message_packet->address, temp,
			message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, *cache_block_hit_ptr, P_TIME);
	}*/

	/*assert(victim_trainsient_state != cgm_cache_block_transient);*/

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s upgrade ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	/*stats*/
	if(*cache_block_hit_ptr == 0)
	{
		cache->TotalMisses++;
		cache->TotalWriteMisses++;
		cache->TotalGetx++;
	}

	if (*cache_block_state_ptr == cgm_cache_block_shared)
	{
		cache->TotalMisses++;
		cache->TotalWriteMisses++;
		cache->TotalUpgrades++;
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:
			cgm_cache_dump_set(cache, message_packet->set);

			fatal("cgm_mesi_l2_upgrade(): %s invalid block state on upgrade as %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
				cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
				message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
				message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);
			break;

		case cgm_cache_block_invalid:

			/*it is possible to find the block in the invalid state here
			if L3 has sent an eviction/upgrade_inval/getx_fwd.
			At this point process like a standard GetX*/

			//stats
			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s upgrade block invalid ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}

			message_packet->access_type = cgm_access_getx;

			return 0;
			break;

		case cgm_cache_block_shared:

			/*if(message_packet->access_id == 87630)
			{
				cgm_cache_dump_set(cache, message_packet->set);
				printf("%s id %llu upgrade miss set %d way %d tag %d cycle %llu\n",
						cache->name, message_packet->access_id, message_packet->set, message_packet->way, message_packet->tag, P_TIME);
				getchar();
			}*/

			//set block transient state, but don't evict because the block is valid and just needs to be upgraded
			cgm_cache_set_block_transient_state(cache, message_packet->set, message_packet->way, cgm_cache_block_transient);

			//insert the upgrade request into the pending request buffer
			message_packet->upgrade_pending = 1;
			message_packet->upgrade_inval_ack_count = 0;
			cgm_cache_insert_pending_request_buffer(cache, message_packet);

			if((message_packet->address & cache->block_address_mask) == 0x0004e380)
			{
				cache_dump_queue(cache->pending_request_buffer);
				getchar();
			}


			//set the upgrade_pending bit to 1 in the block
			//maybe delete this? use transient
			cgm_cache_set_block_upgrade_pending_bit(cache, message_packet->set, message_packet->way);

			//add to ORT table
			cache_check_ORT(cache, message_packet);
			assert(message_packet->coalesced == 0);

			//send upgrade request to L3 (home)
			upgrade_request_packet = packet_create();
			assert(upgrade_request_packet);
			init_upgrade_request_packet(upgrade_request_packet, message_packet->address);
			upgrade_request_packet->start_cycle = message_packet->start_cycle;

			//gather some other data as well
			upgrade_request_packet->access_id = message_packet->access_id;
			upgrade_request_packet->cpu_access_type = message_packet->cpu_access_type;

			//set routing headers
			l3_map = cgm_l3_cache_map(message_packet->set);
			upgrade_request_packet->l2_cache_id = cache->id;
			upgrade_request_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

			upgrade_request_packet->src_name = cache->name;
			upgrade_request_packet->src_id = str_map_string(&node_strn_map, cache->name);

			upgrade_request_packet->dest_name = l3_caches[l3_map].name;
			upgrade_request_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

			//send the upgrade request message to L3
			list_enqueue(cache->Tx_queue_bottom, upgrade_request_packet);
			advance(cache->cache_io_down_ec);
			break;
	}

	return 1;
}

void cgm_mesi_l2_upgrade_inval(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;
	enum cgm_cache_block_state_t victim_trainsient_state;

	struct cgm_packet_t *inval_packet;

	//received upgrade_ivnal request from L3

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s upgrade inval ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:
			fatal("cgm_mesi_l2_upgrade_inval(): L2 id %d invalid block state on upgrade inval as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			/*block has been silently dropped by L2 and is dropped in L1 by inclusion.
			fwd the ack on to the requesting core.*/

			//assert(victim_trainsient_state == cgm_cache_block_invalid);

			/*if(victim_trainsient_state == cgm_cache_block_transient)
			{
				ort_dump(cache);
				cgm_cache_dump_set(cache, message_packet->set);

				unsigned int temp = message_packet->address;
				temp = temp & cache->block_address_mask;

				fatal("cgm_mesi_l2_upgrade_inval(): %s block not in transient state access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d hit %d cycle %llu\n",
					cache->name, message_packet->access_id, message_packet->address, temp,
					message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, *cache_block_hit_ptr, P_TIME);
			}*/

			//transmit ack to requesting L2 cache
			if(message_packet->upgrade == 1)
			{
				message_packet->access_type = cgm_access_upgrade_ack;
			}
			else if(message_packet->upgrade_putx_n == 1)
			{
				message_packet->access_type = cgm_access_upgrade_putx_n;
			}
			else
			{
				fatal("cgm_mesi_l2_upgrade_inval(): invalid upgrade type set\n");
			}

			message_packet->upgrade_ack = -1;
			message_packet->upgrade_inval_ack_count = 0;

			//update routing headers swap dest and src
			//requesting node
			message_packet->dest_name = message_packet->src_name;
			message_packet->dest_id = message_packet->src_id;

			//owning node L2
			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);

			cache_put_io_down_queue(cache, message_packet);
			break;

		case cgm_cache_block_shared:

			//if the block is in the cache invalidate it
			//assert(victim_trainsient_state != cgm_cache_block_transient);

			//set local cache block and directory to invalid
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);
			//cgm_cache_set_block_transient_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			//transmit upgrade ack to requesting L2 cache
			if(message_packet->upgrade == 1)
			{
				message_packet->access_type = cgm_access_upgrade_ack;
			}
			else if(message_packet->upgrade_putx_n == 1)
			{
				message_packet->access_type = cgm_access_upgrade_putx_n;
			}
			else
			{
				fatal("cgm_mesi_l2_upgrade_inval(): invalid upgrade type set\n");
			}

			if(*cache_block_state_ptr == cgm_cache_block_modified)
			{
				message_packet->upgrade_dirty = 1;
			}
			else
			{
				message_packet->upgrade_dirty = 0;
			}

			message_packet->upgrade_ack = -1;
			message_packet->upgrade_inval_ack_count = 0;

			//update routing headers swap dest and src
			//requesting node
			message_packet->dest_name = message_packet->src_name;
			message_packet->dest_id = message_packet->src_id;

			//owning node L2
			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);

			//transmit block to requesting node
			cache_put_io_down_queue(cache, message_packet);

			/*invalidate the L1 D cache lines no need for an ack
			from L1 D cache because the block is in the shared state*/

			inval_packet = packet_create();
			init_upgrade_inval_request_packet(inval_packet, message_packet->address);

			//testing
			inval_packet->access_id = message_packet->access_id;

			//send the L1 D cache the downgrade message
			inval_packet->cpu_access_type = cgm_access_store;
			list_enqueue(cache->Tx_queue_top, inval_packet);
			advance(cache->cache_io_up_ec);
			break;
	}

	return;
}

void cgm_mesi_l2_upgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int l3_map;

	struct cgm_packet_t *pending_packet;

	enum cgm_cache_block_state_t victim_trainsient_state;

	//charge delay
	P_PAUSE(cache->latency);

	/*our upgrade request has been nacked by the L3 this means another core
	has it either in the exclusive or modified state turn this into a getx*/

	//get the status of the cache block
	/*star this is broken, it only return the block way if the block is valid*/
	/*cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);*/

	cache_get_transient_block(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);
	assert(*cache_block_hit_ptr == 1);
	//assert(*cache_block_state_ptr == cgm_cache_block_invalid);

	//block should be in the transient state
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	assert(victim_trainsient_state == cgm_cache_block_transient);

	//get the request from the pending request buffer
	pending_packet = cache_search_pending_request_buffer(cache, message_packet->address);
	assert(pending_packet);

	/*we have lost the block to an eviction "or something" clear the transient state and retry access as a std getx*/
	cgm_cache_set_block_transient_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

	if(victim_trainsient_state != cgm_cache_block_transient)
	{
		ort_dump(cache);
		cgm_cache_dump_set(cache, message_packet->set);

		unsigned int temp = message_packet->address;
		temp = temp & cache->block_address_mask;

		fatal("cgm_mesi_l2_upgrade_nack(): %s block not in transient state access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d hit %d cycle %llu\n",
			cache->name, message_packet->access_id, message_packet->address, temp,
			message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, *cache_block_hit_ptr, P_TIME);
	}

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s upgrade nack ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	//change to a getx
	message_packet->access_type = cgm_access_getx;
	message_packet->cpu_access_type = pending_packet->cpu_access_type;
	message_packet->l1_victim_way = pending_packet->l1_victim_way;
	message_packet->event_queue = pending_packet->event_queue;
	message_packet->data = pending_packet->data;
	message_packet->access_id = pending_packet->access_id;
	message_packet->name = strdup(pending_packet->name);
	message_packet->start_cycle = pending_packet->start_cycle;
	assert(pending_packet->address == message_packet->address);
	//pending_packet->l2_victim_way = message_packet->way;
	//assert(pending_packet->set == message_packet->set && pending_packet->tag == message_packet->tag);

	/*OLD CODE*/
	//update routing headers for the packet
	/*l3_map = cgm_l3_cache_map(pending_packet->set);
	pending_packet->l2_cache_id = cache->id;
	pending_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

	pending_packet->src_name = cache->name;
	pending_packet->src_id = str_map_string(&node_strn_map, cache->name);
	pending_packet->dest_name = l3_caches[l3_map].name;
	pending_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);*/

	//remove the request from the buffer
	pending_packet = list_remove(cache->pending_request_buffer, pending_packet);
	free(pending_packet);
	//list_enqueue(cache->Tx_queue_bottom, pending_packet);
	//advance(cache->cache_io_down_ec);

	//destroy the upgrade request
	//message_packet = list_remove(cache->last_queue, message_packet);
	//packet_destroy(message_packet);

}

void cgm_mesi_l2_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int l3_map;
	int i = 0;
	int ort_row = 0;

	struct cgm_packet_t *pending_packet = NULL;
	struct cgm_packet_t *pending_packet_join = NULL;
	struct cgm_packet_t *reply_packet = NULL;
	struct cgm_packet_t *coalesced_packet = NULL;

	enum cgm_cache_block_state_t victim_trainsient_state;


	//charge delay
	P_PAUSE(cache->latency);

	//we have permission to upgrade the block state and retry access

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//block should be valid and in the transient state
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);

	if(victim_trainsient_state != cgm_cache_block_transient)
	{
		cgm_cache_dump_set(cache, message_packet->set);

		assert((*cache_block_hit_ptr == 1 && victim_trainsient_state != cgm_cache_block_transient) || *cache_block_hit_ptr == 0);

		fatal("cgm_mesi_l2_upgrade_ack(): %s block not in transient state access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d hit %d cycle %llu\n",
			cache->name, message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
			message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, *cache_block_hit_ptr, P_TIME);
	}

	assert(victim_trainsient_state == cgm_cache_block_transient);
	assert(*cache_block_hit_ptr == 1);

	ort_row = ort_search(cache, message_packet->tag, message_packet->set);
	assert(ort_row < cache->mshr_size);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s upgrade ack ID %llu src %s type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
					message_packet->src_name, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}


	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_invalid:

			cgm_cache_dump_set(cache, message_packet->set);

			fatal("cgm_mesi_l2_upgrade_ack(): %s invalid block state on upgrade as %s access_id %llu src %s address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
				cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
				message_packet->access_id, message_packet->src_name, message_packet->address, message_packet->address & cache->block_address_mask,
				message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);
			break;

		case cgm_cache_block_shared:
			//it is possible that an upgrade_ack can be received from a responding L2 before the L3 cache.

			//pull the upgrade from the pending request buffer NOTE don't pull a get/getx_fwd for the same block
			LIST_FOR_EACH(cache->pending_request_buffer, i)
			{
				//get pointer to access in queue and check it's status.
				pending_packet = list_get(cache->pending_request_buffer, i);

				if(((pending_packet->address & cache->block_address_mask) == (message_packet->address & cache->block_address_mask))
						&& pending_packet->access_type == cgm_access_upgrade)
				{
					break;
				}
				else
				{
					pending_packet = NULL;
				}
			}

			/*if not found uh-oh...*/
			assert(pending_packet);
			/*the address better be the same too...*/

			if((pending_packet->address & cache->block_address_mask) != (message_packet->address & cache->block_address_mask))
			{
				printf("addresses not equal pp 0x%08x mp 0x%08x\n", pending_packet->address & cache->block_address_mask, message_packet->address & cache->block_address_mask);
				cache_dump_queue(cache->pending_request_buffer);
				//cgm_dump_system();
			}

			assert((pending_packet->address & cache->block_address_mask) == (message_packet->address & cache->block_address_mask));

			//check if we are ready to perform the join
			if(message_packet->upgrade_ack >= 0)
			{
				pending_packet->upgrade_inval_ack_count = (pending_packet->upgrade_inval_ack_count + message_packet->upgrade_ack);
			}
			else if(message_packet->upgrade_ack < 0)
			{
				pending_packet->upgrade_inval_ack_count--;
			}
			else
			{
				fatal("cgm_mesi_l2_upgrade_ack(): bad upgrade_ack counter value\n");
			}

			if(pending_packet->upgrade_inval_ack_count == 0)
			{

				///////////
				//join
				///////////

				//we have received the L3 reply and the reply(s) from the other L2(s)

				/*make sure this blk is transient*/
				assert(cgm_cache_get_block_transient_state(cache, pending_packet->set, pending_packet->way));

				//clear the block's transient state
				cgm_cache_set_block_transient_state(cache, pending_packet->set, pending_packet->way, cgm_cache_block_invalid);

				//set local cache block and directory to modified.
				cgm_cache_set_block_state(cache, pending_packet->set, pending_packet->way, cgm_cache_block_modified);

				//pull the pending request from the pending request buffer
				pending_packet = list_remove(cache->pending_request_buffer, pending_packet);

				//set access type, block state,
				pending_packet->access_type = cgm_access_upgrade_ack;
				pending_packet->cache_block_state = cgm_cache_block_modified;

				list_enqueue(cache->Tx_queue_top, pending_packet);
				advance(cache->cache_io_up_ec);

				//check the pending request buffer for an outstanding get/getx_fwd sigh.....
				//pull the upgrade from the pending request buffer NOTE don't pull a get/getx_fwd for the same block
				LIST_FOR_EACH(cache->pending_request_buffer, i)
				{
					//get pointer to access in queue and check it's status.
					pending_packet_join = list_get(cache->pending_request_buffer, i);

					if(((pending_packet_join->address & cache->block_address_mask) == (pending_packet->address & cache->block_address_mask))
							&& (pending_packet_join->access_type == cgm_access_get_fwd || pending_packet_join->access_type == cgm_access_getx_fwd))
					{

						//printf("found the get/getx_fwd (join) in upgrade_ack blk_addr 0x%08x\n", pending_packet_join->address & cache->block_address_mask);
						//getchar();
						break;
					}
					else
					{
						pending_packet_join = NULL;
					}
				}

				if(pending_packet_join)
				{
					if((pending_packet_join->address & cache->block_address_mask) == 0x0004e380)
						printf("here\n");
				}

				//validate that a join is waiting if there should be one.
				if(ort_get_pending_join_bit(cache, ort_row, pending_packet->tag, pending_packet->set) == 0)
				{
					assert(pending_packet_join);
					pending_packet_join = list_remove(cache->pending_request_buffer, pending_packet_join);
					list_enqueue(cache->retry_queue, pending_packet_join);
					advance(cache->ec_ptr);

					if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
					{
						if(LEVEL == 2 || LEVEL == 3)
						{
							printf("block 0x%08x %s get/getx_fwd (join) joined in upgrade_ack ID %llu src %s type %d state %d cycle %llu\n",
									(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id,
									message_packet->src_name, message_packet->access_type, *cache_block_state_ptr, P_TIME);
						}
					}
				}

				/*any coalesced accesses?
				LIST_FOR_EACH(cache->ort_list, i)
				{
					//get pointer to access in queue and check it's status.
					coalesced_packet = list_get(cache->ort_list, i);

					if((coalesced_packet->address & cache->block_address_mask) == (pending_packet_join->address & cache->block_address_mask))
					{
							fatal("found it blk_addr 0x%08x\n", coalesced_packet->address & cache->block_address_mask);
					}
				}*/


				//find the access in the ORT table and clear it.
				ort_clear(cache, pending_packet);

			}

			//free the other L3/L2 upgrade_ack message packets
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);



			break;
	}

	return;
}

void cgm_mesi_l2_upgrade_putx_n(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *pending_packet = NULL;
	struct cgm_packet_t *pending_packet_join = NULL;
	struct cgm_packet_t *putx_n_coutner = NULL;

	int i = 0;
	int ort_row = 0;

	/*enum cgm_access_kind_t access_type;
	long long access_id = 0;
	int num_cores = x86_cpu_num_cores;
	int dirty, num_sharers, owning_core;
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;*/

	//charge delay
	P_PAUSE(cache->latency);

	//we have permission to upgrade the block state and retry access

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	ort_row = ort_search(cache, message_packet->tag, message_packet->set);
	assert(ort_row < cache->mshr_size);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s upgrade putx_n ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:
		case cgm_cache_block_shared:

			cgm_cache_dump_set(cache, message_packet->set);

			fatal("cgm_mesi_l2_upgrade_putx_n(): %s invalid block state as %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
				cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
				message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
				message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);

			fatal("cgm_mesi_l2_upgrade_putx_n(): L2 id %d invalid block state on upgrade putx n as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			/*putx_n occurs when a miss in L2 is a hit in L3 as shared
			the line comes from L3, and invalidation acks come from the other L2 caches
			we need to process a join. messages can be received in any order.*/

			//it is possible that an upgrade_inval_ack can be received from a responding L2 before the L3 cache.

			//check if we have already stored a pending request
			LIST_FOR_EACH(cache->pending_request_buffer, i)
			{
				//get pointer to access in queue and check it's status.
				pending_packet = list_get(cache->pending_request_buffer, i);

				if(((pending_packet->address & cache->block_address_mask) == (message_packet->address & cache->block_address_mask))
						&& pending_packet->access_type == cgm_access_upgrade_putx_n)
				{
					break;
				}
				else
				{
					pending_packet = NULL;
				}
			}

			if(pending_packet)
			{
				if((pending_packet->address & cache->block_address_mask) == (message_packet->address & cache->block_address_mask))
				{
					if((pending_packet->address & cache->block_address_mask) == 0x0004e380)
						fatal("cgm_mesi_l2_upgrade_putx_n(): %s invalid block state as %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
								cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
								message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
								message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);
				}
				else
				{
					/*if((pending_packet->address & cache->block_address_mask) == 0x0004e380)
						fatal("cgm_mesi_l2_upgrade_putx_n(): %s invalid block state as %s access_id %llu address 0x%08x blk_addr 0x%08x set %d tag %d way %d state %d cycle %llu\n",
								cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
								message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
								message_packet->set, message_packet->tag, message_packet->way, *cache_block_state_ptr, P_TIME);*/
					fatal("no packet\n");
				}
			}


			/*if not found this is the first reply access*/
			if(!pending_packet)
			{
				//message from L3
				if(message_packet->upgrade_ack >= 0)
				{
					//insert pending packet into pending buffer free the L3's upgrade_ack message packet
					pending_packet = list_remove(cache->last_queue, message_packet);

					pending_packet->upgrade_inval_ack_count = message_packet->upgrade_ack;

					list_enqueue(cache->pending_request_buffer, pending_packet);

					//now wait for L2 replies
				}
				else if(message_packet->upgrade_ack < 0)
				{
					//L2 ack beat the L3 reply
					pending_packet = list_remove(cache->last_queue, message_packet);

					pending_packet->upgrade_inval_ack_count--;

					list_enqueue(cache->pending_request_buffer, pending_packet);
				}
				else
				{
					fatal("cgm_mesi_l2_upgrade_ack(): bad upgrade_ack counter value\n");
				}
			}
			/*if found we have received a reply*/
			else if (pending_packet)
			{
				//message from L3
				if(message_packet->upgrade_ack >= 0)
				{
					//L2 beat L3 swap the packet in the buffer
					pending_packet = list_remove(cache->pending_request_buffer, pending_packet);

					//adjust the counter
					message_packet->upgrade_inval_ack_count = pending_packet->upgrade_inval_ack_count + message_packet->upgrade_ack;

					//free the l2 ack message
					packet_destroy(pending_packet);

					//now wait for L2 replies
					message_packet = list_remove(cache->last_queue, message_packet);
					list_enqueue(cache->pending_request_buffer, message_packet);

					//assign the point so we can check the total below.
					putx_n_coutner = message_packet;

				}
				else if(message_packet->upgrade_ack < 0)
				{
					//L3 ack beat the L2 reply
					pending_packet->upgrade_inval_ack_count--;

					//free the L2 ack
					message_packet = list_remove(cache->last_queue, message_packet);
					packet_destroy(message_packet);

					//assign the pointer so we can check the total below.
					putx_n_coutner = pending_packet;
				}
				else
				{
					fatal("cgm_mesi_l2_upgrade_ack(): bad upgrade_ack counter value\n");
				}


				if(putx_n_coutner->upgrade_inval_ack_count == 0)
				{
					//we have received the L3 reply and the reply(s) from the other L2(s)

					/*make sure this blk is transient*/
					assert(cgm_cache_get_block_transient_state(cache, putx_n_coutner->set, putx_n_coutner->l2_victim_way));

					//set local cache block to modified.
					cgm_cache_set_block(cache, putx_n_coutner->set, putx_n_coutner->l2_victim_way, putx_n_coutner->tag, cgm_cache_block_modified);

					//clear the upgrade_pending bit in the block
					cgm_cache_clear_block_upgrade_pending_bit(cache, putx_n_coutner->set, putx_n_coutner->way);

					//pull the pending request from the pending request buffer
					putx_n_coutner = list_remove(cache->pending_request_buffer, putx_n_coutner);

					//set access type, block state,
					putx_n_coutner->access_type = cgm_access_upgrade_ack;
					putx_n_coutner->cache_block_state = cgm_cache_block_modified;

					list_enqueue(cache->Tx_queue_top, putx_n_coutner);
					advance(cache->cache_io_up_ec);

					LIST_FOR_EACH(cache->pending_request_buffer, i)
					{
						//get pointer to access in queue and check it's status.
						pending_packet_join = list_get(cache->pending_request_buffer, i);

						if(((pending_packet_join->address & cache->block_address_mask) == (putx_n_coutner->address & cache->block_address_mask))
								&& (pending_packet_join->access_type == cgm_access_get_fwd || pending_packet_join->access_type == cgm_access_getx_fwd))
						{
							//printf("found the get/getx_fwd (join) in upgrade_putx_n blk_addr 0x%08x\n", pending_packet_join->address & cache->block_address_mask);
							//getchar();
							break;
						}
						else
						{
							pending_packet_join = NULL;
						}
					}

					if(pending_packet_join)
					{
						if((pending_packet_join->address & cache->block_address_mask) == 0x0004e380)
							printf("here_2\n");
					}

					//validate that a join is waiting if there should be one.
					if(ort_get_pending_join_bit(cache, ort_row, putx_n_coutner->tag, putx_n_coutner->set) == 0)
					{
						assert(pending_packet_join);
						pending_packet_join = list_remove(cache->pending_request_buffer, pending_packet_join);
						list_enqueue(cache->retry_queue, pending_packet_join);
						advance(cache->ec_ptr);

						if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
						{
							if(LEVEL == 2 || LEVEL == 3)
							{
								printf("block 0x%08x %s get/getx_fwd (join) joined in upgrade_putx_n ID %llu type %d state %d cycle %llu\n",
										(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
							}
						}
					}

					//last thing is to clear this access in the ORT table.
					ort_clear(cache, putx_n_coutner);

				}
			}
			else
			{
				fatal("cgm_mesi_l2_upgrade_putx_n(): putx n packet trouble\n");
			}


			break;
	}

	return;
}

/*void cgm_mesi_l3_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	enum cgm_cache_block_state_t block_trainsient_state;
	struct cgm_packet_t *write_back_packet = NULL;

	int pending_bit, sharers;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);
	assert(pending_bit == 1);

	//get number of sharers
	sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	assert(sharers == 1);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);
	assert(!write_back_packet);

	//check transient state
	//block_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	assert(block_trainsient_state == cgm_cache_block_transient);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s upgrade ack ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_invalid:
		case cgm_cache_block_shared:
		case cgm_cache_block_exclusive:

			fatal("cgm_mesi_l3_downgrade_ack(): L3 id %d invalid block state on upgrade_ack as %s (should be M state) access id %llu address %u tag %d set %d way %d\n",
					cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->access_id, message_packet->address, message_packet->tag, message_packet->set, message_packet->way);
			break;

		case cgm_cache_block_modified:

			//set the new sharer bit in the directory
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

			cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

			cgm_cache_clear_dir_pending_bit(cache, message_packet->set, message_packet->way);
			assert(cache->sets[message_packet->set].blocks[message_packet->way].directory_entry.entry_bits.pending == 0);

			//go ahead and destroy the upgrade ack message because we don't need it anymore.
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);
			break;
	}

	return;
}*/


int cgm_mesi_l3_upgrade(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *upgrade_inval_request_packet;

	enum cgm_cache_block_state_t victim_trainsient_state;

	int num_cores = x86_cpu_num_cores;
	int num_sharers, owning_core, pending_bit, xowning_core, i;

	int l2_src_id;
	char *l2_name;

	//charge the delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//block should be valid and not in a transient state
	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);
	assert(victim_trainsient_state != cgm_cache_block_transient);

	//get the directory state

	//get number of sharers
	num_sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);
	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s upgrade request ID %llu type %d state %d cycle %llu\n",
					(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
		}
	}


	if(pending_bit == 1)
	{
		/*if pending another core is trying to get the block
		and has beaten us to it. A fwd of some kind has already
		gone out to the L2 requesting so a nack is required.*/

		/*there should only be one core with the block and it shouldn't be the requesting core process a nack for this*/
		assert(num_sharers == 1 && owning_core == 0 && pending_bit == 1 && *cache_block_hit_ptr == 1);

		/*block should be in the exclusive or modified state*/
		assert(*cache_block_state_ptr == cgm_cache_block_exclusive || *cache_block_state_ptr == cgm_cache_block_modified);

		/*cgm_cache_dump_set(cache, message_packet->set);
		unsigned int temp = message_packet->address;
		temp = temp & cache->block_address_mask;
		fatal("cgm_mesi_l3_upgrade(): pending_bit set access id %llu type %d block state %d set %d tag %d pending_bit %d block_addr 0x%08x cycle %llu\n",
				message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, message_packet->set, message_packet->tag,
				cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way), temp, P_TIME);*/

		//send the reply up as a NACK!
		message_packet->access_type = cgm_access_upgrade_nack;

		//set message package size
		message_packet->size = 1;

		//update routing headers
		message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
		message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
		message_packet->src_name = cache->name;
		message_packet->src_id = str_map_string(&node_strn_map, cache->name);

		//send the reply
		cache_put_io_up_queue(cache, message_packet);

		return 1;
	}



	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
			cgm_cache_dump_set(cache, message_packet->set);

			fatal("cgm_mesi_l3_upgrade(): %s invalid block state on upgrade as %s access_id %llu address 0x%08x blk_addr 0x%08x set %d way %d tag %d state %d hit %d cycle %llu\n",
				cache->name, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr),
				message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
				message_packet->set, message_packet->way, message_packet->tag, *cache_block_state_ptr, *cache_block_hit_ptr, P_TIME);

			break;

		case cgm_cache_block_invalid:

			/*cgm_cache_dump_set(cache, message_packet->set);

			temp = message_packet->address;
			temp = temp & cache->block_address_mask;*/

			/*L3 just evicted the block and the block is no longer found here or up in any core.
			need to convert to a getx/putx. The block should still be transient up in the L2
			and have an entry in the pending request queue so look for the pending request and join them
			on L2 write block*/

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s upgrade invalid id %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}

			message_packet->access_type = cgm_access_getx;

			return 0;

			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			/*if the block is exclusive or modified this means an upgrade
			request just beat this one to the L3 cache*/

			/*there should only be one core with the block and it shouldn't
			be the requesting core process as a GetX*/
			if(num_sharers != 1 || owning_core != 0 || pending_bit != 0 || *cache_block_hit_ptr != 1)
			{

				printf("bits %d %d %d %d\n", num_sharers, owning_core, pending_bit, *cache_block_hit_ptr);

				printf("write block access id %llu\n", message_packet->access_id);

				ort_dump(cache);

				cgm_cache_dump_set(cache, message_packet->set);

				fatal("cgm_mesi_l3_upgrade(): %s access_id %llu address 0x%08x blk_addr 0x%08x mp set %d mp tag %d mp way %d cycle %llu\n",
					cache->name, message_packet->access_id, message_packet->address, message_packet->address & cache->block_address_mask,
					message_packet->set, message_packet->tag, message_packet->way, P_TIME);
			}

			assert(num_sharers == 1 && owning_core == 0 && pending_bit == 0 && *cache_block_hit_ptr == 1);
			/*note that upgrades don't set the pending state in L3 so the
			request won't get nacked*/

			//change the access type
			message_packet->access_type = cgm_access_getx_fwd;

			//set the directory pending bit.
			cgm_cache_set_dir_pending_bit(cache, message_packet->set, message_packet->way);

			if((((message_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(LEVEL == 2 || LEVEL == 3)
				{
					printf("block 0x%08x %s upgrade getx_fwd ID %llu type %d state %d cycle %llu\n",
							(message_packet->address & cache->block_address_mask), cache->name, message_packet->access_id, message_packet->access_type, *cache_block_state_ptr, P_TIME);
				}
			}

			/*update the routing headers.
			set src as requesting cache and dest as owning cache.
			We can derive the home (directory) later from the original access address.*/

			//get the id of the owning core L2
			xowning_core = cgm_cache_get_xown_core(cache, message_packet->set, message_packet->way);

			//owning node
			message_packet->dest_name = str_map_value(&l2_strn_map, xowning_core);
			message_packet->dest_id = str_map_string(&node_strn_map, message_packet->dest_name);

			/*printf("name %s id %d\n", message_packet->l2_cache_name, message_packet->src_id);
			getchar();*/

			//requesting node L2
			message_packet->src_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
			message_packet->src_name = str_map_value(&node_strn_map, message_packet->src_id);

			cache_put_io_up_queue(cache, message_packet);

			break;


		case cgm_cache_block_shared:


			/*there should always be at least one sharer
			but no more than the number of cores.*/
			assert(num_sharers >= 1 && num_sharers <= num_cores);

			//build the upgrade_ack packet
			//set access type
			message_packet->access_type = cgm_access_upgrade_ack;
			//set number of sharers
			//if there is 1 sharer and its the owning core set 0
			if(num_sharers == 1 && owning_core == 1)
			{
				message_packet->upgrade_ack = 0;
			}
			else
			{
				if(*cache_block_state_ptr == cgm_cache_block_shared && owning_core == 1)
				{
					/*trying to unconfuse myself...*/
					/*the block is shared over n cores, but one of the cores is the requesting core so minus 1*/
					message_packet->upgrade_ack = (num_sharers - 1);
				}
				else if (*cache_block_state_ptr == cgm_cache_block_shared && owning_core == 0)
				{
					/*the block is shared over n cores but is NOT in the requesting core*/
					message_packet->upgrade_ack = num_sharers;
				}
				else if(*cache_block_state_ptr == cgm_cache_block_exclusive)
				{
					/*another core has full control over the block. */
					message_packet->upgrade_ack = num_sharers;

					/*should only be one other core*/
					assert(message_packet->upgrade_ack == 1);
				}
				else
				{
					fatal("L3 upgrade check this\n");
				}
			}

			/*if(message_packet->access_id == 4976121)
				fatal("\t requesting core %d num shares %d num joins = %d", owning_core, num_sharers, message_packet->upgrade_ack);*/

			//set block state
			message_packet->cache_block_state = cgm_cache_block_modified;

			//set destination
			message_packet->dest_id = message_packet->src_id;
			message_packet->dest_name = message_packet->src_name;

			l2_src_id = message_packet->src_id;
			l2_name = strdup(message_packet->src_name);

			//set the source of the packet as L3
			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);

			cache_put_io_up_queue(cache, message_packet);

			//invalidate the other sharers
			for(i = 0; i < num_cores; i++)
			{
				//find the other cores
				if(cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, i) && i != message_packet->l2_cache_id)
				{
					//create and init the upgrade_inval packet
					upgrade_inval_request_packet = packet_create();
					init_upgrade_inval_request_packet(upgrade_inval_request_packet, message_packet->address);

					//testing
					upgrade_inval_request_packet->access_id = message_packet->access_id;
					//testing

					upgrade_inval_request_packet->dest_name = str_map_value(&l2_strn_map, i);
					upgrade_inval_request_packet->dest_id = str_map_string(&node_strn_map, upgrade_inval_request_packet->dest_name);

					//requesting node L2
					upgrade_inval_request_packet->src_id = str_map_string(&node_strn_map, l2_name);
					upgrade_inval_request_packet->src_name = str_map_value(&node_strn_map, l2_src_id);

					list_enqueue(cache->Tx_queue_top, upgrade_inval_request_packet);
					advance(cache->cache_io_up_ec);

				}
			}

			//free the temp string
			free(l2_name);

			//set local cache block and directory to modified.
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);

			//clear the directory
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

			//set the directory pending bit.
			//cgm_cache_set_dir_pending_bit(cache, message_packet->set, message_packet->way);

			//set the sharer bit for the upgraded node
			cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);
			break;
	}

	return 1;
}
