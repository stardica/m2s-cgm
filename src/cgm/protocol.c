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
	//dont' need to free these because we never malloc anything.
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
	/*printf("address 0x%08x\n", write_back_packet->address);
	STOP;*/

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
	inval_packet->address = cgm_cache_build_address(cache, cache->sets[set].blocks[way].set, cache->sets[set].blocks[way].tag);
	return;
}


void cgm_mesi_fetch(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("l1 i %d fetching\n", cache->id);*/
	/*STOP;*/

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

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

			//stats
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//add some routing/status data to the packet
			message_packet->access_type = cgm_access_gets;
			message_packet->l1_access_type = cgm_access_gets;

			//find victim and evict on return l1_i_cache just drops the block on return
			message_packet->l1_victim_way = cgm_cache_replace_block(cache, message_packet->set);

			//transmit to L2
			cache_put_io_down_queue(cache, message_packet);
			break;

		case cgm_cache_block_shared:

			//stats
			cache->hits++;

			//set retry state and delay
			if(message_packet->access_type == cgm_access_fetch_retry || message_packet->coalesced == 1)
			{
				//charge a delay only in the event of a retry.
				P_PAUSE(cache->latency);

				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			message_packet->end_cycle = P_TIME;
			cache_l1_i_return(cache,message_packet);
			break;
	}

	return;
}

void cgm_mesi_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("l1 d %d loading\n", cache->id);*/
	/*STOP;*/

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	//enum cgm_cache_block_state_t transient_state = cgm_cache_block_invalid;

	int upgrade = 0;

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	/*if(message_packet->set == 11 && message_packet->tag == 66)
	{
		printf("access_id %llu access_type (%s) tag %d set %d cycle %llu\n",
				message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->tag, message_packet->set, P_TIME);

	}*/


	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if(write_back_packet)
	{
		/*found the packet in the write back buffer
		data should not be in the rest of the cache*/
		//assert(*cache_block_state_ptr == cgm_cache_block_invalid);

		assert((write_back_packet->cache_block_state == cgm_cache_block_modified
				|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

		//printf("l1 WB found WB state %d cache state %d\n", write_back_packet->cache_block_state, *cache_block_state_ptr);

		message_packet->end_cycle = P_TIME;
		cache_l1_d_return(cache, message_packet);
		return;
	}

	//check for transient state
	//transient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->way);

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

			//stats
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//add some routing/status data to the packet
			message_packet->access_type = cgm_access_get;
			message_packet->l1_access_type = cgm_access_get;

			//find victim
			message_packet->l1_victim_way = cgm_cache_replace_block(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			cgm_L1_cache_evict_block(cache, message_packet->set, message_packet->l1_victim_way);

			//transmit to L2
			cache_put_io_down_queue(cache, message_packet);

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu miss changed (%s) cycle %llu\n",
					cache->name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);

			break;

		//hit states
		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_shared:

			//stats
			cache->hits++;

			//check for pending upgrade before finishing
			upgrade = ort_search(cache, message_packet->tag, message_packet->set);

			/*star todo start separating out these kinds of things,
			this should be done in parallel with the cache access.*/
			if(upgrade < cache->mshr_size)
			{
				/*there is a pending upgrade this means we have a valid block
				in the shared state, but an earlier store is waiting on an upgrade to modified.
				We must coalesce this access and wait for the earlier store to finish.*/
				assert(*cache_block_state_ptr == cgm_cache_block_shared);

				//charge delay
				P_PAUSE(cache->latency);

				//stats
				//cache->transient_misses++;

				//check ORT for coalesce
				cache_check_ORT(cache, message_packet);

				if(message_packet->coalesced == 1)
					return;

				//should always coalesce because we are waiting on an upgrade miss.
				fatal("cgm_mesi_load(): transient state with no load coalesce\n");
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

			message_packet->end_cycle = P_TIME;
			cache_l1_d_return(cache,message_packet);

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu hit cycle %llu\n", cache->name, message_packet->access_id, P_TIME);
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

	/*enum cgm_access_kind_t access_type;
	long long access_id = 0;
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;*/

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	/*if(message_packet->set == 11 && message_packet->tag == 66)
	{
		printf("access_id %llu access_type (%s) tag %d set %d cycle %llu\n",
				message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->tag, message_packet->set, P_TIME);

	}*/
	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if(write_back_packet)
	{
		/*found the packet in the write back buffer
		data should not be in the rest of the cache*/
		/*printf("store done\n");*/
		//assert(*cache_block_state_ptr == cgm_cache_block_invalid);

		assert((write_back_packet->cache_block_state == cgm_cache_block_modified
				|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

		//printf("l1 WB found WB state %d cache state %d\n", write_back_packet->cache_block_state, *cache_block_state_ptr);

		write_back_packet->cache_block_state = cgm_cache_block_modified;

		message_packet->end_cycle = P_TIME;
		cache_l1_d_return(cache,message_packet);
		return;
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

			//stats
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//add some routing/status data to the packet
			message_packet->access_type = cgm_access_getx;
			message_packet->l1_access_type = cgm_access_getx;

			//find victim
			message_packet->l1_victim_way = cgm_cache_replace_block(cache, message_packet->set);
			assert(message_packet->l1_victim_way >= 0 && message_packet->l1_victim_way < cache->assoc);

			cgm_L1_cache_evict_block(cache, message_packet->set, message_packet->l1_victim_way);

			//transmit to L2
			cache_put_io_down_queue(cache, message_packet);

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu miss changed (%s) cycle %llu\n",
					cache->name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			break;

		case cgm_cache_block_shared:

			//this is an upgrade_miss
			fatal("L1 D upgrade delete me\n");

			//charge delay
			P_PAUSE(cache->latency);

			//stats
			cache->upgrade_misses++;

			//put back on the core event queue to end memory system access.
			/*cache_l1_d_return(cache, message_packet);
			return;*/


			/*if(message_packet->address == (unsigned int) 0x00004810)
			{
				cgm_cache_dump_set(cache, message_packet->set);
				printf("L1 D %d %s access_id %llu address 0x%08x set %d tag %d cycle %llu\n",
						cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->access_id, message_packet->address, message_packet->set, message_packet->tag, P_TIME);
			}*/

			/*if(message_packet->access_type == cgm_access_store_retry)
			{
				printf("access_id %llu addr 0x%08x cycle %llu set %d tag %d way %d\n",
						message_packet->access_id, message_packet->address, P_TIME, message_packet->set, message_packet->tag, message_packet->way);

				ort_dump(cache);

				assert(message_packet->access_type != cgm_access_store_retry);
			}*/

			/*printf("L1 D %d upgrade miss access id %llu addr 0x%08x cycle %llu\n", cache->id, message_packet->access_id, message_packet->address, P_TIME);*/
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
				return;

			//set block transient state
			//cgm_cache_set_block_transient_state(cache, message_packet->set, message_packet->way, message_packet->access_id, cgm_cache_block_transient);

			message_packet->access_type = cgm_access_upgrade;

			//transmit upgrade request to L2
			//printf("access_id %llu forwarded set %d tag %d cycle %llu\n", message_packet->access_id, message_packet->set, message_packet->tag, P_TIME);
			cache_put_io_down_queue(cache, message_packet);

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu upgrade miss changed (%s) cycle %llu\n",
					cache->name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			cache->hits++;

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

			message_packet->end_cycle = P_TIME;
			cache_l1_d_return(cache,message_packet);

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu hit cycle %llu\n", cache->name, message_packet->access_id, P_TIME);
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

	cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	return;
}

void cgm_mesi_l1_d_downgrade(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	/*enum cgm_access_kind_t access_type;
	long long access_id = 0;
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;*/

	//Received downgrade from L2; a block needs to be shared...

	//charge the delay
	P_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		fatal("cgm_mesi_l1_d_downgrade(): L1 id %d invalid block state on downgrade as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
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

				/*fatal("L1 downgrade found in WB *cache_block_state_ptr = %d\n", *cache_block_state_ptr);*/

			}
			else
			{
				//if invalid and not in write back it was silently dropped
				message_packet->size = 1;
				message_packet->cache_block_state = cgm_cache_block_invalid;
			}

			break;

		case cgm_cache_block_exclusive:
			//if E it is not dirty
			message_packet->size = 1;
			message_packet->cache_block_state = cgm_cache_block_exclusive;
			break;

		case cgm_cache_block_shared:
			//if S it is not dirty
			message_packet->size = 1;
			message_packet->cache_block_state = cgm_cache_block_shared;
			break;

		case cgm_cache_block_modified:
			//hit and its dirty send the ack and block down (sharing write back) to the L2 cache.
			message_packet->size = cache->block_size;
			message_packet->cache_block_state = cgm_cache_block_modified;
			break;
	}

	message_packet->l1_cache_id = cache->id;

	//set the access type
	message_packet->access_type = cgm_access_downgrade_ack;

	//uncomment later
	//downgrade the local block
	//cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_shared);
	//uncomment later

	//delete later removing s state
	cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);
	//delete later removing s state

	//reply to the L2 cache
	cache_put_io_down_queue(cache, message_packet);

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
			}
			else
			{
				//star todo Second check (snoop) the WB buffer

				//if invalid it was silently dropped
				message_packet->size = 1;
				message_packet->cache_block_state = cgm_cache_block_invalid;
			}
			break;

		case cgm_cache_block_exclusive:
			//if E or S it is not dirty
			message_packet->size = 1;
			message_packet->cache_block_state = cgm_cache_block_exclusive;
			break;

		case cgm_cache_block_modified:
			//hit and its dirty send the ack and block down (write back) to the L2 cache.
			message_packet->size = cache->block_size;
			message_packet->cache_block_state = cgm_cache_block_modified;
			break;
	}

		message_packet->l1_cache_id = cache->id;

		//invalidate the local block
		cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

		//set access type
		message_packet->access_type = cgm_access_getx_fwd_inval_ack;

		//reply to the L2 cache
		cache_put_io_down_queue(cache, message_packet);

	return;
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
			free(message_packet);

			break;

		case cgm_cache_block_shared:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			//if the block is in the cache invalidate it

			//set local cache block and directory to modified.
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

			//free the upgrade_inval
			message_packet = list_remove(cache->last_queue, message_packet);
			free(message_packet);

			break;
	}

	return;
}

void cgm_mesi_l1_d_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//we have permission to upgrade our set block state and retry access

	//charge the delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:
			fatal("cgm_mesi_l1_d_upgrade_ack(): L1 D id %d invalid block state on upgrade ack as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			/*if the access misses this can be due
			to it originally going out as a GETX on a cache miss
			We need to store the block and set modified before we retry*/

			assert(message_packet->cache_block_state == cgm_cache_block_modified);
			assert(message_packet->access_type == cgm_access_upgrade_ack);
			assert(message_packet->cpu_access_type == cgm_access_store);
			assert(message_packet->coalesced != 1);

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
			assert(message_packet->coalesced != 1);

			//find the access in the ORT table and clear it.
			ort_clear(cache, message_packet);

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

void cgm_mesi_l1_d_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//charge the latency
	P_PAUSE(cache->latency);

	//if the line is still in the exclusive state at this point drop it.
	if(message_packet->cache_block_state == cgm_cache_block_exclusive)
	{
		message_packet = list_remove(cache->last_queue, message_packet);
		free(message_packet);
	}
	else if (message_packet->cache_block_state == cgm_cache_block_modified)
	{
		//block is dirty send the write back down to the L2 cache.
		cache_put_io_down_queue(cache, message_packet);
	}
	else
	{
		fatal("cgm_mesi_l1_d_write_back(): Invalid block state in write back buffer cycle %llu\n", P_TIME);
	}

	return;
}

void cgm_mesi_l1_d_inval(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//Invalidation/eviction request from L2 cache

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *wb_packet;

	//charge the delay
	P_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

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

	return;
}


int cgm_mesi_l1_d_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("access type %d block state %d\n", message_packet->access_type, message_packet->cache_block_state);*/

	assert(cache->cache_type == l1_d_cache_t);
	assert((message_packet->access_type == cgm_access_puts && message_packet->cache_block_state == cgm_cache_block_shared)
			|| (message_packet->access_type == cgm_access_put_clnx && message_packet->cache_block_state == cgm_cache_block_exclusive)
			|| (message_packet->access_type == cgm_access_putx && message_packet->cache_block_state == cgm_cache_block_modified));

	/*enum cgm_access_kind_t access_type;*/
	/*long long access_id = 0;*/
	/*int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;*/

	/*access_type = message_packet->access_type;*/
	/*access_id = message_packet->access_id;*/


	/*enum cgm_cache_block_state_t victim_trainsient_state;
	long long t_state_id;*/

	//check the transient state of the victim
	//if the state is set, an earlier access is bringing the block
	//if it is not set the victim is clear to evict
	/*cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l1_victim_way);*/

	//if the block is in the transient state there are stores waiting to write to the block.
	/*if(victim_trainsient_state == cgm_cache_block_transient)
	{

		//the victim is locked, either wait, choose another victim, or schedule something else.
		t_state_id = cgm_cache_get_block_transient_state_id(cache, message_packet->set, message_packet->l1_victim_way);

		//check ordering
		assert(message_packet->access_id >= t_state_id);

		fatal("l1 D cache stalled on transient\n");
		printf("***L1 id %d 373844 write block stalled on transient\n", cache->id);

		//try again we will pull the coherence message eventually.
		P_PAUSE(1);
		return 0;
	}
	else
	{*/
		//find the access in the ORT table and clear it.
	ort_clear(cache, message_packet);

		//set the block and retry the access in the cache.
	/*cache_put_block(cache, message_packet);*/

	//write the block
	cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

	//set retry state
	message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);
	/*advance(cache->ec_ptr);*/

	/*}*/

	return 1;
}

void cgm_mesi_l2_gets(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int l3_map;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

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

			//stats
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//add some routing/status data to the packet
			message_packet->access_type = cgm_access_gets;

			l3_map = cgm_l3_cache_map(message_packet->set);
			message_packet->l2_cache_id = cache->id;
			message_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);
			message_packet->dest_name = l3_caches[l3_map].name;
			message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

			//find victim, on return OK to just drop the block this is I$ traffic
			message_packet->l2_victim_way = cgm_cache_replace_block(cache, message_packet->set);
			assert(message_packet->l2_victim_way >= 0 && message_packet->l2_victim_way < cache->assoc);

			//transmit to L3
			cache_put_io_down_queue(cache, message_packet);

			break;

		case cgm_cache_block_shared:

			//stats
			cache->hits++;

			//check if the packet has coalesced accesses.
			if(message_packet->access_type == cgm_access_fetch_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			/*star todo this is broken, try to fix this.
			the I$ is accessing memory in the D$'s swim lane*/
			/*if(*cache_block_state_ptr == cgm_cache_block_exclusive)
			{
				message_packet->cache_block_state = cgm_cache_block_shared;
			}
			else
			{
				message_packet->cache_block_state = *cache_block_state_ptr;
			}*/

			//set block state
			message_packet->cache_block_state = *cache_block_state_ptr;

			//set message size
			message_packet->size = l1_i_caches[cache->id].block_size; //this can be either L1 I or L1 D cache block size.

			//update message status
			/*message_packet->cache_block_state = *cache_block_state_ptr;*/

			message_packet->access_type = cgm_access_puts;
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

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);


	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if(write_back_packet)
	{
		/*found the packet in the write back buffer
		data should not be in the rest of the cache*/

		/*fatal("l2 wb load delete me\n");*/

		//printf("l2 WB found WB state %d cache state %d cycle %llu\n", write_back_packet->cache_block_state, *cache_block_state_ptr, P_TIME);

		assert((write_back_packet->cache_block_state == cgm_cache_block_modified
				|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

		//assert(*cache_block_state_ptr == cgm_cache_block_invalid);

		//set message size
		message_packet->size = l1_d_caches[cache->id].block_size; //this can be either L1 I or L1 D cache block size.

		//update message status
		if(write_back_packet->cache_block_state == cgm_cache_block_modified)
		{
			message_packet->access_type = cgm_access_putx;
			message_packet->cache_block_state = cgm_cache_block_modified;
		}
		else if(write_back_packet->cache_block_state == cgm_cache_block_exclusive)
		{
			message_packet->access_type = cgm_access_put_clnx;
			message_packet->cache_block_state = cgm_cache_block_exclusive;
		}

		cache_put_io_up_queue(cache, message_packet);
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

			//stats
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//error checking check this in L3 cache
			//message_packet->cache_block_state = *cache_block_hit_ptr;

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
			message_packet->l2_victim_way = cgm_cache_replace_block(cache, message_packet->set);
			cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->l2_victim_way);

			//transmit to L3
			cache_put_io_down_queue(cache, message_packet);

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu miss changed (%s) cycle %llu\n",
					cache->name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_shared:

			if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
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

			/*this will send the block and block state up to the high level cache.*/
			message_packet->cache_block_state = *cache_block_state_ptr;

			cache_put_io_up_queue(cache, message_packet);

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu hit cycle %llu\n", cache->name, message_packet->access_id, P_TIME);
			break;
	}
}

void cgm_mesi_l2_get_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int ort_row = 0;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int l3_map = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;

	//charge delay
	P_PAUSE(cache->latency);

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

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

	/*enum cgm_access_kind_t access_type;
	long long access_id = 0;
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;*/

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);


	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if(write_back_packet)
	{
		/*found the packet in the write back buffer
		data should not be in the rest of the cache*/
		//assert(*cache_block_state_ptr == cgm_cache_block_invalid);

		//printf("l2 WB found WB state %d cache state %d\n", write_back_packet->cache_block_state, *cache_block_state_ptr);

		assert((write_back_packet->cache_block_state == cgm_cache_block_modified
				|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

		//set message size
		message_packet->size = l1_d_caches[cache->id].block_size; //this can be either L1 I or L1 D cache block size.

		//update message status
		if(write_back_packet->cache_block_state == cgm_cache_block_modified)
		{
			message_packet->access_type = cgm_access_putx;
			message_packet->cache_block_state = cgm_cache_block_modified;
		}
		else if(write_back_packet->cache_block_state == cgm_cache_block_exclusive)
		{
			message_packet->access_type = cgm_access_put_clnx;
			message_packet->cache_block_state = cgm_cache_block_exclusive;
		}

		cache_put_io_up_queue(cache, message_packet);
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
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				break;

			//find victim
			message_packet->l2_victim_way = cgm_cache_replace_block(cache, message_packet->set);

			//evict the victim
			cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->l2_victim_way);

			//set the data type bit in the block
			/*int type;
			type = message_packet->cpu_access_type == cgm_access_fetch ? 1 : 0;
			cgm_cache_set_block_type(&(l2_caches[my_pid]), type, message_packet->set, message_packet->l2_victim_way);*/

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

			//transmit to L3
			cache_put_io_down_queue(cache, message_packet);

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu miss changed (%s) cycle %llu\n",
					cache->name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);

			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			//stats;
			cache->hits++;

			//set retry state
			if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			//set message status and size
			message_packet->size = l1_d_caches[cache->id].block_size; //this can be either L1 I or L1 D cache block size.

			//message must be in exclusive or modified state for a hit on GetX

			//set the local block state
			message_packet->access_type = cgm_access_putx;
			message_packet->cache_block_state = cgm_cache_block_modified;

			if(*cache_block_state_ptr == cgm_cache_block_exclusive)
			{
				/*if the block is in the E state set M because the message is a store
				a flush will bring the modified line down later
				the block remains in the E state at L3*/
				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);
			}

			//send up to L1 D cache
			cache_put_io_up_queue(cache, message_packet);

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu hit cycle %llu\n", cache->name, message_packet->access_id, P_TIME);

			break;

		case cgm_cache_block_shared:

			//stats
			cache->upgrade_misses++;

			//access was a miss in L1 D but a hit at the L2 level, set upgrade and run again.
			message_packet->access_type = cgm_access_upgrade;

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu miss changed (%s) cycle %llu\n",
					cache->name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);

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
	int way = 0;
	int l3_map = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;

	//charge delay
	P_PAUSE(cache->latency);

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	//store the decoded address
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	//do not set retry because this contains the coalesce set and tag.
	//check that there is an ORT entry for this address
	ort_row = ort_search(cache, message_packet->tag, message_packet->set);
	assert(ort_row < cache->mshr_size);

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

	/*enum cgm_access_kind_t access_type;
	long long access_id = 0;
	struct cgm_packet_t *wb_packet;
	struct cgm_packet_t *downgrade_packet;
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;*/

	int l3_map;
	/*int temp_id;*/

	//charge delay
	P_PAUSE(cache->latency);

	//L1 D cache flushed

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

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

				assert((write_back_packet->cache_block_state == cgm_cache_block_modified
						|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

				//fatal("L2 down grade ack in wb *cache_block_state_ptr = %d\n", *cache_block_state_ptr);

				/////////
				//GET_FWD
				/////////

				//pull the GET_FWD from the pending request buffer
				pending_request = cache_search_pending_request_buffer(cache, message_packet->address);

				/*if not found uh-oh...*/
				assert(pending_request);
				/*the address better be the same too...*/
				assert(pending_request->address == message_packet->address);
				/*printf("L2 id %d downgrade pending_packet pulled from buffer access_id %llu\n", cache->id, pending_request->access_id);*/


				//uncomment here
				//downgrade the local block
				//cgm_cache_set_block_state(cache, pending_request->set, pending_request->way, cgm_cache_block_shared);

				//prepare to forward the block
				//set access type
				//pending_request->access_type = cgm_access_puts;

				//set the block state
				//pending_request->cache_block_state = cgm_cache_block_shared;
				//end uncomment here

				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				free(write_back_packet);

				//prepare to forward the block
				//set access type
				pending_request->access_type = cgm_access_put_clnx;

				//set the block state
				pending_request->cache_block_state = cgm_cache_block_exclusive;

				//******delete removing s state

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

				/*printf("requester name %s and id %d\n", pending_request->src_name, pending_request->l2_cache_id);*/
				/*temp_id = pending_request->access_id;*/

				//transmit block to requesting node
				pending_request = list_remove(cache->pending_request_buffer, pending_request);
				list_enqueue(cache->Tx_queue_bottom, pending_request);
				advance(cache->cache_io_down_ec);
				/*printf("L2 id %d shared block forwarded to L2 cache id %d\n", l1_d_caches[my_pid].id, pending_request->l2_cache_id);*/

				///////////////
				//downgrade_ack
				///////////////

				//send the downgrade ack to L3 cache.

				//create downgrade_ack
				reply_packet = packet_create();
				assert(reply_packet);

				init_downgrade_ack_packet(reply_packet, message_packet->address);

				//delete removing s state
				reply_packet->size = 1;
				reply_packet->cache_block_state = cgm_cache_block_exclusive;
				//delete removing s state

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

				//destroy the downgrade message because we don't need it anymore.
				message_packet = list_remove(cache->last_queue, message_packet);
				free(message_packet);



			}
			else
			{
				fatal("cgm_mesi_l2_downgrade_ack(): line missing in L2 after downgrade\n");
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
			/*printf("L2 id %d downgrade pending_packet pulled from buffer access_id %llu\n", cache->id, pending_request->access_id);*/


			//uncomment here
			//downgrade the local block
			//cgm_cache_set_block_state(cache, pending_request->set, pending_request->way, cgm_cache_block_shared);

			//prepare to forward the block
			//set access type
			//pending_request->access_type = cgm_access_puts;

			//set the block state
			//pending_request->cache_block_state = cgm_cache_block_shared;
			//end uncomment here


			//******delete removing s state
			cgm_cache_set_block_state(cache, pending_request->set, pending_request->way, cgm_cache_block_invalid);

			//prepare to forward the block
			//set access type
			pending_request->access_type = cgm_access_put_clnx;

			//set the block state
			pending_request->cache_block_state = cgm_cache_block_exclusive;

			//******delete removing s state

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

			/*printf("requester name %s and id %d\n", pending_request->src_name, pending_request->l2_cache_id);*/
			/*temp_id = pending_request->access_id;*/

			//transmit block to requesting node
			pending_request = list_remove(cache->pending_request_buffer, pending_request);
			list_enqueue(cache->Tx_queue_bottom, pending_request);
			advance(cache->cache_io_down_ec);
			/*printf("L2 id %d shared block forwarded to L2 cache id %d\n", l1_d_caches[my_pid].id, pending_request->l2_cache_id);*/

			///////////////
			//downgrade_ack
			///////////////

			//send the downgrade ack to L3 cache.

			//create downgrade_ack
			reply_packet = packet_create();
			assert(reply_packet);

			init_downgrade_ack_packet(reply_packet, message_packet->address);

			//delete removing s state
			reply_packet->size = 1;
			reply_packet->cache_block_state = cgm_cache_block_exclusive;
			//delete removing s state

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

			//destroy the downgrade message because we don't need it anymore.
			message_packet = list_remove(cache->last_queue, message_packet);
			free(message_packet);

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

	/*enum cgm_access_kind_t access_type;
	long long access_id = 0;
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;*/

	//charge delay
	P_PAUSE(cache->latency);

	//L1 D cache has been flushed
	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

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

				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/

				assert((write_back_packet->cache_block_state == cgm_cache_block_modified
						|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

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

				//invalidate the local block
				//cgm_cache_set_block_state(cache, pending_getx_fwd_request->set, pending_getx_fwd_request->way, cgm_cache_block_invalid);

				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				free(write_back_packet);

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
				free(message_packet);

			}
			else
			{
				fatal("cgm_mesi_l2_getx_fwd_inval_ack(): line missing in L2 after downgrade\n");
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

			//invalidate the local block
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
			free(message_packet);

			break;
	}

	return;
}


void cgm_mesi_l2_upgrade(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//received upgrade request from L1

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *upgrade_request_packet;

	/*enum cgm_access_kind_t access_type;
	long long access_id = 0;
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;*/

	int l3_map;

	//charge latency
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			cgm_cache_dump_set(cache, message_packet->set);

			fatal("cgm_mesi_l2_upgrade(): L2 id %d invalid block state on upgrade as %s access_id %llu address 0x%08x set %d tag %d cycle %llu\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->access_id, message_packet->address, message_packet->set, message_packet->tag, P_TIME);
			break;

		case cgm_cache_block_invalid:
			/*star todo block evicted by L2 and inval is on its way to L1
			change upgrade request to GetX and send on to L3 cache.*/
			break;

		case cgm_cache_block_shared:

			//insert the upgrade request into the pending request buffer
			message_packet->upgrade_pending = 1;
			message_packet->upgrade_inval_ack_count = 0;
			cgm_cache_insert_pending_request_buffer(cache, message_packet);
			/*printf("L2 id %d upgrade request pending_packet inserted into buffer access_id %llu\n", cache->id, message_packet->access_id);*/

			//set the upgrade_pending bit to 1 in the block
			cgm_cache_set_block_upgrade_pending_bit(cache, message_packet->set, message_packet->way);

			//send upgrade request to L3 (home)
			upgrade_request_packet = packet_create();
			init_upgrade_request_packet(upgrade_request_packet, message_packet->address);

			//delete later
			upgrade_request_packet->access_id = message_packet->access_id;
			//delete later

			//set routing headers
			l3_map = cgm_l3_cache_map(message_packet->set);
			upgrade_request_packet->l2_cache_id = cache->id;
			upgrade_request_packet->l2_cache_name = str_map_value(&l2_strn_map, cache->id);

			/*printf("L2 packet l2 name/id %s and %d\n", upgrade_request_packet->l2_cache_name, upgrade_request_packet->l2_cache_id);*/

			upgrade_request_packet->src_name = cache->name;
			upgrade_request_packet->src_id = str_map_string(&node_strn_map, cache->name);

			/*printf("L2 packet src name/id %s and %d\n", upgrade_request_packet->src_name, upgrade_request_packet->src_id);*/

			upgrade_request_packet->dest_name = l3_caches[l3_map].name;
			upgrade_request_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

			/*printf("L2 packet dest name/id %s and %d\n", upgrade_request_packet->dest_name, upgrade_request_packet->dest_id);
			STOP;*/

			//send the upgrade request message to L3
			list_enqueue(cache->Tx_queue_bottom, upgrade_request_packet);
			advance(cache->cache_io_down_ec);

			break;
	}
	return;
}

void cgm_mesi_l2_upgrade_inval(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *inval_packet;

	//received upgrade_ivnal request from L3

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_exclusive:
			fatal("cgm_mesi_l2_upgrade_inval(): L2 id %d invalid block state on upgrade inval as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			/*block has been silently dropped by L2 and is dropped in L1 by inclusion.
			fwd the ack on to the requesting core.*/

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
		case cgm_cache_block_modified:

			//if the block is in the cache invalidate it

			//set local cache block and directory to invalid
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_invalid);

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

void cgm_mesi_l2_inval(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//Invalidation/eviction request from lower cache

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//charge delay
	P_PAUSE(cache->latency);

	//get the block status
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
			fatal("cgm_mesi_l2_inval(): L2 id %d invalid block state on inval as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			/*find and invalidate the block
			if the block is missing at L2, the block has previously
			been removed from the L1 D cache as well, so we can ignore*/

			//free the message packet
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);

			break;

		case cgm_cache_block_shared:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			/*if the block is found in the L2 it may or may not be in the L1 cache
			we must invalidate here and send an invalidation to the L1 D cache*/

			//get the way of the block
			/*message_packet->l2_victim_way = cgm_cache_replace_block(cache, message_packet->set);*/
			cgm_L2_cache_evict_block(cache, message_packet->set, message_packet->way);

			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);
			break;
	}

	return;
}

void cgm_mesi_l2_inval_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *wb_packet;

	//charge delay
	P_PAUSE(cache->latency);

	//inval ack from L1 D cache...

	//get the address set and tag
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//state should be either invalid of modified.
	assert(message_packet->cache_block_state == cgm_cache_block_modified || message_packet->cache_block_state == cgm_cache_block_invalid);

	//find the block in the local WB buffer
	wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	//merge data if dirty
	if(wb_packet)
	{
		//incoming data from L1 is dirty
		if(message_packet->cache_block_state == cgm_cache_block_modified || wb_packet->cache_block_state == cgm_cache_block_modified)
		{
			//merge the block.
			wb_packet->cache_block_state = cgm_cache_block_modified;

			//clear the pending bit and leave the wb in the buffer
			wb_packet->flush_pending = 0;
		}
		else
		{
			//Neither the l1 line or L2 line are dirty clear the wb from the buffer
			wb_packet = list_remove(cache->write_back_buffer, wb_packet);
			free(wb_packet);
		}
	}

	//free the message packet (inval_ack)
	message_packet = list_remove(cache->last_queue, message_packet);
	free(message_packet);

	return;
}


void cgm_mesi_l2_upgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *pending_packet;

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


	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_invalid:
			fatal("cgm_mesi_l2_upgrade_ack(): L2 id %d invalid block state on upgrade ack as %s access_id %llu address 0x%08x cycle %llu\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->access_id, message_packet->address, P_TIME);
			break;

		case cgm_cache_block_shared:
			//it is possible that an upgrade_ack can be received from a responding L2 before the L3 cache.

			//pull the GET_FWD from the pending request buffer
			pending_packet = cache_search_pending_request_buffer(cache, message_packet->address);
			/*if not found uh-oh...*/
			/*if(!pending_packet)
			{
				printf("l2 upgrade ack crashing\n");

			}*/

			assert(pending_packet);

			/*the address better be the same too...*/
			assert(pending_packet->address == message_packet->address);

			//check if we are ready to perform the join
			if(message_packet->upgrade_ack >= 0)
			{
				pending_packet->upgrade_inval_ack_count = (pending_packet->upgrade_inval_ack_count + message_packet->upgrade_ack);

				//free the L3's upgrade_ack message packet
				message_packet = list_remove(cache->last_queue, message_packet);
				free(message_packet);
			}
			else if(message_packet->upgrade_ack < 0)
			{
				pending_packet->upgrade_inval_ack_count--;

				//free the other L2's upgrade_ack message packet
				message_packet = list_remove(cache->last_queue, message_packet);
				free(message_packet);
			}
			else
			{
				fatal("cgm_mesi_l2_upgrade_ack(): bad upgrade_ack counter value\n");
			}


			if(pending_packet->upgrade_inval_ack_count == 0)
			{
				//we have received the L3 reply and the reply(s) from the other L2(s)

				//set local cache block and directory to modified.
				cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);

				//clear the upgrade_pending bit in the block
				cgm_cache_clear_block_upgrade_pending_bit(cache, message_packet->set, message_packet->way);

				//pull the pending request from the pending request buffer
				pending_packet = list_remove(cache->pending_request_buffer, pending_packet);

				//set access type, block state,
				pending_packet->access_type = cgm_access_upgrade_ack;
				pending_packet->cache_block_state = cgm_cache_block_modified;

				list_enqueue(cache->Tx_queue_top, pending_packet);
				advance(cache->cache_io_up_ec);
			}
			break;
	}

	return;
}

void cgm_mesi_l2_upgrade_putx_n(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *pending_packet;
	struct cgm_packet_t *putx_n_coutner;

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


	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:
		case cgm_cache_block_shared:
			fatal("cgm_mesi_l2_upgrade_putx_n(): L2 id %d invalid block state on upgrade putx n as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			/*putx_n occurs when a miss in L2 is a hit in L3 as shared
			the line come from L3, and invalidation acks come from the other L2 caches
			we need to process a join. messages can be received in any order.*/

			//it is possible that an upgrade_ack can be received from a responding L2 before the L3 cache.

			//check if we have already stored a pending request
			pending_packet = cache_search_pending_request_buffer(cache, message_packet->address);

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

					pending_packet->upgrade_inval_ack_count = -1;

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
					free(pending_packet);

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
					free(message_packet);

					//assign the point so we can check the total below.
					putx_n_coutner = pending_packet;
				}
				else
				{
					fatal("cgm_mesi_l2_upgrade_ack(): bad upgrade_ack counter value\n");
				}


				if(putx_n_coutner->upgrade_inval_ack_count == 0)
				{
					//we have received the L3 reply and the reply(s) from the other L2(s)

					//find the access in the ORT table and clear it.
					ort_clear(cache, putx_n_coutner);

					//set local cache block to modified.
					cgm_cache_set_block(cache, message_packet->set, message_packet->l2_victim_way, message_packet->tag, cgm_cache_block_modified);

					//clear the upgrade_pending bit in the block
					cgm_cache_clear_block_upgrade_pending_bit(cache, message_packet->set, message_packet->way);

					//pull the pending request from the pending request buffer
					putx_n_coutner = list_remove(cache->pending_request_buffer, putx_n_coutner);

					//set access type, block state,
					putx_n_coutner->access_type = cgm_access_upgrade_ack;
					putx_n_coutner->cache_block_state = cgm_cache_block_modified;

					list_enqueue(cache->Tx_queue_top, putx_n_coutner);
					advance(cache->cache_io_up_ec);
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


void cgm_mesi_l2_get_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*printf("l2 get fwd\n");*/

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *downgrade_packet;

	struct cgm_packet_t *write_back_packet = NULL;

	int l3_map;

	//charge delay
	P_PAUSE(cache->latency);

	/*struct cgm_packet_t *wb_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;
	struct cgm_packet_t *reply_packet;
	struct cgm_packet_t *pending_request;
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	int temp_id;*/

	/*we have received a get_fwd from the home.
	this is for a block that we have in our core
	and can forward to the requesting core.

	The block should be here in the exclusive or modified state
	however it is possible that the block may be in the wb buffer or
	have been dropped or written back earlier (dirty).

	3 way hop implementation

	if the block is present in cache or wb buffer and exclusive in L2/L1
		(1) downgrade L1 to shared (upper level cache probe).
			(a) move get_fwd to pending request buffer in L2
			(b) send downgrade message to L1 from L2
			(c) check L1 cache and wb buffer for block status
			(d) respond with downgrade ack to L2 from L1
		(2) downgrade L2 to shared.
			(a) receive the downgrade_ack from L1
			(b) pull pending request from buffer
			(c) check L1's inputs from downgrade_ack
			(d) downgrade block to shared
		(3) fwd block to requesting core.
		(4) send downgrade_ack to L3 (home node).
		(5) Done

	if the block is not present in L2/L1
		(1) send downgrade_nack (original GET) to L3
		(2) reply to GET from L3

	if the block is present and modified (stored) in either L1 or L2
		(1) downgrade L1 to shared and write back (if modified)
		(2) merge and downgrade L2 to shared
		(3) fwd block to requesting core (shared).
		(4) issue sharing WB to L3

	it is possible for the GET_FWD to miss,
	this means the block was silently dropped by the owning node
		(1) send nack to L3 (home)
		(2) L3 sends reply to requester for GET*/

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
		fatal("cgm_mesi_l2_get_fwd(): L2 id %d invalid block state on get_fwd as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			//check the WB buffer
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/

				//printf("l2 WB found WB state %d cache state %d cycle %llu\n", write_back_packet->cache_block_state, *cache_block_state_ptr, P_TIME);

				assert((write_back_packet->cache_block_state == cgm_cache_block_modified
						|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

				//set the flush pending bit, to keep this line in WB until the L1 can reply.
				write_back_packet->flush_pending = 1;

				//a GET_FWD means the block is exclusive in this core, but could also be modified

				//store the get_fwd in the pending request buffer
				message_packet->downgrade_pending = 1;
				cgm_cache_insert_pending_request_buffer(cache, message_packet);

				//flush the L1 cache because the line may be dirty in L1
				downgrade_packet = packet_create();
				init_downgrade_packet(downgrade_packet, message_packet->address);

				//send the L1 D cache the downgrade message
				downgrade_packet->cpu_access_type = cgm_access_load;
				list_enqueue(cache->Tx_queue_top, downgrade_packet);
				advance(cache->cache_io_up_ec);

			}
			else
			{

				/* block was evicted silently*/

				//set downgrade_nack
				message_packet->access_type = cgm_access_downgrade_nack;

				//fwd reply (downgrade_nack) to L3
				l3_map = cgm_l3_cache_map(message_packet->set);

				message_packet->dest_name = l3_caches[l3_map].name;
				message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

				//transmit block to L3
				cache_put_io_down_queue(cache, message_packet);
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
			init_downgrade_packet(downgrade_packet, message_packet->address);

			//send the L1 D cache the downgrade message
			downgrade_packet->cpu_access_type = cgm_access_load;
			list_enqueue(cache->Tx_queue_top, downgrade_packet);
			advance(cache->cache_io_up_ec);
			break;
	}

	return;
}

void cgm_mesi_l2_getx_fwd(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *inval_packet;

	struct cgm_packet_t *write_back_packet = NULL;

	int l3_map;

	//charge delay
	P_PAUSE(cache->latency);

	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
		fatal("cgm_mesi_l2_getx_fwd(): L2 id %d invalid block state on getx_fwd as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			//check the WB buffer
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/

				//fatal("here ytooooo\n");

				//printf("l2 WB found WB state %d cache state %d cycle %llu\n", write_back_packet->cache_block_state, *cache_block_state_ptr, P_TIME);

				assert((write_back_packet->cache_block_state == cgm_cache_block_modified
						|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

				//set the flush pending bit, to keep this line in WB until the L1 can reply.
				write_back_packet->flush_pending = 1;

				//a GET_FWD means the block is exclusive in this core, but could also be modified

				//store the get_fwd in the pending request buffer
				message_packet->inval_pending = 1;
				cgm_cache_insert_pending_request_buffer(cache, message_packet);

				//flush the L1 cache because the line may be dirty in L1
				inval_packet = packet_create();
				init_getx_fwd_inval_packet(inval_packet, message_packet->address);

				//send the L1 D cache the downgrade message
				inval_packet->cpu_access_type = cgm_access_load;
				list_enqueue(cache->Tx_queue_top, inval_packet);
				advance(cache->cache_io_up_ec);

			}
			else
			{
				//block was locally dropped

				//set cgm_access_getx_fwd_nack
				message_packet->access_type = cgm_access_getx_fwd_nack;

				//fwd reply (downgrade_nack) to L3
				l3_map = cgm_l3_cache_map(message_packet->set);

				/*here send the nack down to the L3
				don't change any of the source information

				message_packet->l2_cache_id = l2_caches[my_pid].id;
				message_packet->l2_cache_name = str_map_value(&l2_strn_map, l2_caches[my_pid].id);
				reply_packet->src_name = l2_caches[my_pid].name;
				reply_packet->src_id = str_map_string(&node_strn_map, l2_caches[my_pid].name);*/

				message_packet->dest_name = l3_caches[l3_map].name;
				message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

				//transmit block to L3
				cache_put_io_down_queue(cache, message_packet);
			}
			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			//a GETX_FWD means the block is E/M in this core. The block will be E/M in the L1

			//store the getx_fwd in the pending request buffer
			message_packet->inval_pending = 1;
			cgm_cache_insert_pending_request_buffer(cache, message_packet);

			//set the flush_pending bit to 1 in the block
			cgm_cache_set_block_flush_pending_bit(cache, message_packet->set, message_packet->way);

			//flush the L1 cache because the line may be dirty in L1
			inval_packet = packet_create();
			init_getx_fwd_inval_packet(inval_packet, message_packet->address);

			//send the L1 D cache the inval message
			inval_packet->cpu_access_type = cgm_access_store;
			list_enqueue(cache->Tx_queue_top, inval_packet);
			advance(cache->cache_io_up_ec);
			break;
	}
	return;
}

int cgm_mesi_l2_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	assert(cache->cache_type == l2_cache_t);
	assert((message_packet->access_type == cgm_access_puts && message_packet->cache_block_state == cgm_cache_block_shared)
			|| (message_packet->access_type == cgm_access_put_clnx && message_packet->cache_block_state == cgm_cache_block_exclusive)
			|| (message_packet->access_type == cgm_access_putx && message_packet->cache_block_state == cgm_cache_block_modified));

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	/*enum cgm_access_kind_t access_type;
	long long access_id = 0;
	struct cgm_packet_t *reply_packet;
	struct cgm_packet_t *wb_packet;
	struct cgm_packet_t *downgrade_packet;
	struct cgm_packet_t *pending_request;
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	int l3_map;
	int temp_id;*/

	enum cgm_cache_block_state_t victim_trainsient_state;
	long long t_state_id;

	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	victim_trainsient_state = cgm_cache_get_block_transient_state(cache, message_packet->set, message_packet->l2_victim_way);

	if(victim_trainsient_state == cgm_cache_block_transient)
	{
		//the victim is locked, either wait or choose another victim.
		t_state_id = cgm_cache_get_block_transient_state_id(cache, message_packet->set, message_packet->l2_victim_way);

		//check for write before read condition.
		if(message_packet->access_id >= t_state_id)
		{
			//star todo i don't know if this is an actually problem or not
			/*printf("t_state_id %llu message_packet id %llu as %s\n", t_state_id, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));*/
			assert(message_packet->access_id >= t_state_id);
		}

		//try again we will pull the coherence message eventually.
		fatal("cgm_mesi_l2_put(): l2 looping cache block %d\n", victim_trainsient_state);
		//printf("access_id %llu as %s cycle %llu\n", message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);

		P_PAUSE(1);

		return 0;
	}
	else
	{
		//find the access in the ORT table and clear it.
		ort_clear(cache, message_packet);

		//write the block
		cgm_cache_set_block(cache, message_packet->set, message_packet->l2_victim_way, message_packet->tag, message_packet->cache_block_state);

		//testing
		//set block address
		cgm_cache_set_block_address(cache, message_packet->set, message_packet->l2_victim_way, message_packet->address);
		//testing

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

	int set;
	int tag;
	int way;
	int i = 0;

	//charge the delay
	P_PAUSE(cache->latency);

	/*TOP:*/

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	set = message_packet->set;
	tag = message_packet->tag;
	way = message_packet->way;

	/*if(message_packet->set == 390 && message_packet->tag == 12)
	{
		printf("gets hit_ptr %d access %d tag %d way %d cycle %llu\n", *cache_block_hit_ptr, message_packet->set, message_packet->tag, message_packet->way, P_TIME);
		printf("address 0x%08x\n", message_packet->address);
		printf("address (unsigned int) %u\n", message_packet->address);

		for(i = 0; i < cache->assoc; i++)
		{
			printf("cache set %d way %d tag %d state %d\n", cache->sets[set].id, i, cache->sets[set].blocks[i].tag, cache->sets[set].blocks[i].state);

			if(cgm_cache_get_block_state(cache, message_packet->set, i) == 4)
			{
				printf("error detected access_id %llu access type %d cycle %llu\n", message_packet->access_id, message_packet->access_type, P_TIME);
			}
		}

		printf("\n\n");
	}*/

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:
			/*cgm_cache_set_block(cache, set, way, tag, cgm_cache_block_shared);*/

			/*printf("Crashing: access_id %llu address 0x%08x set %d tag %d way %d cpu_access_type %d cycle %llu\n",
					message_packet->access_id, message_packet->address, set, tag, way, message_packet->cpu_access_type, P_TIME);
			printf("*** set %d way %d tag %d state %d\n", cache->sets[set].id, way, cache->sets[set].blocks[way].tag, cache->sets[set].blocks[way].state);

			goto TOP;

			for(i = 0; i < cache->assoc; i++)
			{
				printf("cache set %d way %d tag %d state %d\n", cache->sets[set].id, i, cache->sets[set].blocks[i].tag, cache->sets[set].blocks[i].state);

				if(cgm_cache_get_block_state(cache, message_packet->set, i) == 4)
				{
					printf("error detected access_id %llu access type %d cycle %llu\n", message_packet->access_id, message_packet->access_type, P_TIME);
				}
			}

			printf("\n\n");*/

			fatal("l3_cache_ctrl(): L3 id %d GetS invalid block state as %s cycle %llu\n", cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), P_TIME);
			break;

		case cgm_cache_block_invalid:

			//stats;
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//find the victim.
			message_packet->l3_victim_way = cgm_cache_replace_block(cache, message_packet->set);

			//set the block state to invalid
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->l3_victim_way, cgm_cache_block_invalid);

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

			//transmit to SA/MC
			cache_put_io_down_queue(cache, message_packet);

			break;

		case cgm_cache_block_shared:

			//stats;
			cache->hits++;

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

			/*star todo this is broken, try to fix this.
			the I$ is accessing memory in the D$'s swim lane*/
			/*if(*cache_block_state_ptr == cgm_cache_block_exclusive ||*cache_block_state_ptr == cgm_cache_block_modified)
			{
				message_packet->cache_block_state = cgm_cache_block_shared;
			}
			else
			{
				message_packet->cache_block_state = *cache_block_state_ptr;
			}*/

			message_packet->cache_block_state = *cache_block_state_ptr;
			message_packet->access_type = cgm_access_puts;

			message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
			message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);

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

	int l3_map;

	/*int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int state = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;*/

	/*on a write back with inclusive caches L2 Merges the line
	if the write back is a surprise the block will be exclusive in the L2 cache, but the data is old.*/

	//charge the delay
	P_PAUSE(cache->latency);

	//WB from L1 D cache
	if(cache->last_queue == cache->Rx_queue_top)
	{
		//printf("WB from L1\n");

		//we should only receive modified lines from L1 D cache
		assert(message_packet->cache_block_state != cgm_cache_block_exclusive
				&& message_packet->cache_block_state != cgm_cache_block_shared
				&& message_packet->cache_block_state != cgm_cache_block_invalid);

		//get the state of the local cache block
		cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

		switch(*cache_block_state_ptr)
		{
			case cgm_cache_block_noncoherent:
			case cgm_cache_block_owned:
			case cgm_cache_block_shared:
				/*if(*cache_block_state_ptr == cgm_cache_block_shared)
				{
					printf("L2 id %d WB received with block in shared state\n", cache->id);
				}*/
			/*printf("l2 %d write_back_id %llu access_type (%s) addr 0x%08x set %d tag %d cycle %llu\n",
						cache->id, message_packet->write_back_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->address, message_packet->set, message_packet->tag, P_TIME);*/

			fatal("cgm_mesi_l2_write_back(): L2 id %d invalid block state on write back as %s address 0x%08x\n",
					cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
				break;

			case cgm_cache_block_invalid:

				/*Star it is possible for the WB from L1 D to
				miss at the L2. This means there was a recent L2 eviction
				and the eviction is on its way up to the L1 D cache.*/

				/*When this happens check the local WB buffer for the line which should be in the E or M state.*/

				//check the WB buffer
				wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

				if(wb_packet)
				{
					//cache block found in the WB buffer merge the change here
					//set modified if the line was exclusive
					if(wb_packet->cache_block_state == cgm_cache_block_exclusive)
					{
						wb_packet->cache_block_state = cgm_cache_block_modified;
					}
					else if(wb_packet->cache_block_state == cgm_cache_block_modified)
					{
						/*technically, the line in L2 maybe in the modified state,
						but the line form L1 D maybe modified and is newer then the L2 line*/
						wb_packet->cache_block_state = cgm_cache_block_modified;
					}

					//destroy the L1 D WB message. L2 will clear its WB at an opportune time.
					message_packet = list_remove(cache->last_queue, message_packet);
					packet_destroy(message_packet);
				}
				else
				{
					//block not found in either cache or WB buffer, fwd WB down to L3
					l3_map = cgm_l3_cache_map(message_packet->set);
					message_packet->l2_cache_id = cache->id;
					message_packet->l2_cache_name = cache->name;

					message_packet->src_name = cache->name;
					message_packet->src_id = str_map_string(&node_strn_map, cache->name);
					message_packet->dest_name = l3_caches[l3_map].name;
					message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

					//send the write back to the L3 cache.
					cache_put_io_down_queue(cache, message_packet);
				}
				break;

			case cgm_cache_block_exclusive:
			case cgm_cache_block_modified:

				//hit in cache merge write back here.

				/*if(message_packet->address == (unsigned int) 0x00029804)
				{
					printf("l2 %d write_back_id %llu access_type (%s) addr 0x%08x cycle %llu\n",
							cache->id, message_packet->write_back_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->address, P_TIME);
				}*/

				//set modified if the line is exclusive
				if(*cache_block_state_ptr == cgm_cache_block_exclusive)
				{
					cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);
				}
				else if(*cache_block_state_ptr == cgm_cache_block_modified)
				{
					/*technically, the line in L2 maybe in the modified state,
					but the line form L1 D maybe modified and is newer then the L2 line*/
					cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);
				}

				//destroy the L1 D WB message. L2 will clear its WB at an opportune time.
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);
				break;
		}
	}
	//if here the L2 generated it's own write back.
	else if(cache->last_queue == cache->write_back_buffer)
	{

		/*star todo figure out a better way to deal with the pending flush state
		maybe look into the scheduler*/

		//if zero the flush has finished.
		if(message_packet->flush_pending == 0)
		{

			//if the line is still in the exclusive state at this point drop it.
			if(message_packet->cache_block_state == cgm_cache_block_exclusive)
			{
				message_packet = list_remove(cache->last_queue, message_packet);
				free(message_packet);
			}
			else if (message_packet->cache_block_state == cgm_cache_block_modified)
			{
				//delete this
				//printf("WB from L2\n");

				//block is dirty send the write back down to the L3 cache.
				l3_map = cgm_l3_cache_map(message_packet->set);
				message_packet->l2_cache_id = cache->id;
				message_packet->l2_cache_name = cache->name; /*str_map_value(&l2_strn_map, l2_caches[my_pid].id);*/

				message_packet->src_name = cache->name; /*l2_caches[my_pid].name;*/
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);
				message_packet->dest_name = l3_caches[l3_map].name;
				message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

				//send the write back to the L3 cache.
				cache_put_io_down_queue(cache, message_packet);
			}
			else
			{
				fatal("cgm_mesi_l2_write_back(): Invalid block state in write back buffer cycle %llu\n", P_TIME);
			}

		}
		//still waiting so run again or find something else to do.
		else if (message_packet->flush_pending == 1 || message_packet->downgrade_pending == 1)
		{
			//NOTE: flush may have been sent prior to receiving a get_fwd and a subsequent downgrade
			//wait on either case.

			//do nothing for now.
			fatal("l2 flush pending bit set on wb packet!\n");
		}
		else
		{
			fatal("l2 cache invalid flush_pending bit on write back packet\n");
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

	/*int set;
	int tag;
	int way;
	int i = 0;*/

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if(write_back_packet)
	{
		/*found the packet in the write back buffer
		data should not be in the rest of the cache*/

		//printf("l3 WB found WB state %d cache state %d\n", write_back_packet->cache_block_state, *cache_block_state_ptr);

		assert((write_back_packet->cache_block_state == cgm_cache_block_modified
				|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

		//update message status
		if(write_back_packet->cache_block_state == cgm_cache_block_exclusive)
		{
			message_packet->access_type = cgm_access_put_clnx;
			message_packet->cache_block_state = cgm_cache_block_exclusive;
		}
		else if(write_back_packet->cache_block_state == cgm_cache_block_modified)
		{
			message_packet->access_type = cgm_access_putx;
			message_packet->cache_block_state = cgm_cache_block_modified;
		}

		//get the cache block state
		//message_packet->cache_block_state = *cache_block_state_ptr;

		//set the presence bit in the directory for the requesting core.
		//cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);
		//cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

		//set message package size
		message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

		//update routing headers
		message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
		message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
		message_packet->src_name = cache->name;
		message_packet->src_id = str_map_string(&node_strn_map, cache->name);

		//send the cache block out
		cache_put_io_up_queue(cache, message_packet);
		return;
	}
	/*else
	{
		//update message status
		if(*cache_block_state_ptr == cgm_cache_block_exclusive || *cache_block_state_ptr == cgm_cache_block_invalid)
		{
			message_packet->access_type = cgm_access_put_clnx;
			message_packet->cache_block_state = cgm_cache_block_exclusive;
		}
		else
		{
			fatal("bad cache block state\n");
		}

		//get the cache block state
		//message_packet->cache_block_state = *cache_block_state_ptr;

		//set the presence bit in the directory for the requesting core.
		//cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);
		//cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

		//set message package size
		message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

		//update routing headers
		message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
		message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
		message_packet->src_name = cache->name;
		message_packet->src_id = str_map_string(&node_strn_map, cache->name);

		//send the cache block out
		cache_put_io_up_queue(cache, message_packet);
		return;
	}*/

	/*set = message_packet->set;
	tag = message_packet->tag;
	way = message_packet->way;*/

	/*if(message_packet->set == 390 && message_packet->tag == 12)
	{
		printf("get hit_ptr %d access %d tag %d way %d\n", *cache_block_hit_ptr, set, tag, way);
		printf("address 0x%08x\n", message_packet->address);
		printf("address (unsigned int) %u\n", message_packet->address);

		for(i = 0; i < cache->assoc; i++)
		{
			printf("cache set %d way %d tag %d state %d\n", cache->sets[set].id, i, cache->sets[set].blocks[i].tag, cache->sets[set].blocks[i].state);

			if(cgm_cache_get_block_state(cache, message_packet->set, i) == 4)
			{
				printf("error detected access_id %llu access type %d cycle %llu\n", message_packet->access_id, message_packet->access_type, P_TIME);
			}
		}
		printf("\n\n");
	}*/

	//get the directory state
	//check the directory dirty bit status
	//dirty = cgm_cache_get_dir_dirty_bit(cache, message_packet->set, message_packet->way);
	//get number of sharers
	sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);
	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	//if access to the block is pending send nack back to requesting core.
	if(pending_bit == 1)
	{
		/*there should be at least 1 or more sharers
		and the requester should not be the owning core
		because the access should be coalesced.*/
		//assert(sharers >= 1 &&  owning_core == 0);

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
			cache->misses++;
			assert(message_packet->cpu_access_type == cgm_access_load);

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//find victim again because LRU has been updated on hits.
			message_packet->l3_victim_way = cgm_cache_replace_block(cache, message_packet->set);

			//evict the block
			cgm_L3_cache_evict_block(cache, message_packet->set, message_packet->l3_victim_way, sharers);

			//clear the directory entry
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->l3_victim_way);

			//add some routing/status data to the packet
			message_packet->access_type = cgm_access_mc_load;

			//star todo this should be exclusive when Get is fully working
			/*message_packet->cache_block_state = cgm_cache_block_modified;*/
			message_packet->cache_block_state = cgm_cache_block_exclusive;
			/*message_packet->cache_block_state = cgm_cache_block_shared;*/

			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);
			message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
			message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

			//transmit to SA/MC
			cache_put_io_down_queue(cache, message_packet);

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu miss changed (%s) cycle %llu\n",
					cache->name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);

			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			//stats;
			cache->hits++;

			//star todo update this message when working in GETX
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

				//send the cache block out
				cache_put_io_up_queue(cache, message_packet);


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

				cache_put_io_up_queue(cache, message_packet);
			}
			else
			{
				fatal("cgm_mesi_l3_get(): invalid sharer/owning_core state\n");
			}

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu hit changed (%s) cycle %llu\n",
					cache->name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);

			break;

		case cgm_cache_block_shared:

			//delete me
			fatal("L3 get shared\n");

			//stats;
			cache->hits++;

			//check if the packet has coalesced accesses.
			if(message_packet->access_type == cgm_access_load_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
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

			cache_put_io_up_queue(cache, message_packet);

			//debug
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu hit changed (%s) cycle %llu\n",
					cache->name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);

			break;
	}

	return;
}

void cgm_mesi_l3_getx(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int num_cores = x86_cpu_num_cores;
	int num_sharers, owning_core, xowning_core, pending_bit;

	struct cgm_packet_t *upgrade_putx_n_inval_request_packet;

	struct cgm_packet_t *write_back_packet = NULL;

	int i = 0;
	int l2_src_id;
	char *l2_name;

	/*enum cgm_access_kind_t access_type;
	long long access_id = 0;
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;*/



	//charge latency
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);


	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	if(write_back_packet)
	{
		/*found the packet in the write back buffer
		data should not be in the rest of the cache*/

		//printf("l3 WB found WB state %d cache state %d\n", write_back_packet->cache_block_state, *cache_block_state_ptr);

		//update message status
		if(write_back_packet->cache_block_state == cgm_cache_block_exclusive)
		{
			message_packet->access_type = cgm_access_put_clnx;
			message_packet->cache_block_state = cgm_cache_block_exclusive;
		}
		else if(write_back_packet->cache_block_state == cgm_cache_block_modified)
		{
			message_packet->access_type = cgm_access_putx;
			message_packet->cache_block_state = cgm_cache_block_modified;
		}

		//get the cache block state
		//message_packet->cache_block_state = *cache_block_state_ptr;

		//set the presence bit in the directory for the requesting core.
		//cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);
		//cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

		//set message package size
		message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

		//update routing headers
		message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
		message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
		message_packet->src_name = cache->name;
		message_packet->src_id = str_map_string(&node_strn_map, cache->name);

		//send the cache block out
		cache_put_io_up_queue(cache, message_packet);
		return;
	}
	/*else
	{
		printf("here\n");
		STOP;

		//update message status
		if(*cache_block_state_ptr == cgm_cache_block_modified || *cache_block_state_ptr == cgm_cache_block_invalid)
		{
			message_packet->access_type = cgm_access_putx;
			message_packet->cache_block_state = cgm_cache_block_modified;
		}
		else
		{
			fatal("bad cache block state\n");
		}

		//get the cache block state
		//message_packet->cache_block_state = *cache_block_state_ptr;

		//set the presence bit in the directory for the requesting core.
		//cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);
		//cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

		//set message package size
		message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

		//update routing headers
		message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
		message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
		message_packet->src_name = cache->name;
		message_packet->src_id = str_map_string(&node_strn_map, cache->name);

		//send the cache block out
		cache_put_io_up_queue(cache, message_packet);
		return;
	}*/

	//get the directory state
	//check the directory dirty bit status
	//dirty = cgm_cache_get_dir_dirty_bit(cache, message_packet->set, message_packet->way);
	//get number of sharers
	num_sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);
	//check pending state
	pending_bit = cgm_cache_get_dir_pending_bit(cache, message_packet->set, message_packet->way);

	if(pending_bit == 1)
	{
		/*there should be at least 1 or more sharers
		and the requester should not be the owning core
		because the access should be coalesced.*/
		//assert(num_sharers >= 1 &&  owning_core == 0);

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
			cache->misses++;

			//check ORT for coalesce
			cache_check_ORT(cache, message_packet);

			if(message_packet->coalesced == 1)
				return;

			//find victim because LRU has been updated on hits.
			message_packet->l3_victim_way = cgm_cache_replace_block(cache, message_packet->set);

			//evict the victim
			cgm_L3_cache_evict_block(cache, message_packet->set, message_packet->l3_victim_way, num_sharers);

			//clear the directory entry
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->l3_victim_way);

			//add some routing/status data to the packet
			message_packet->access_type = cgm_access_mc_load;

			//set the returned block state
			message_packet->cache_block_state = cgm_cache_block_modified;
			/*message_packet->cache_block_state = cgm_cache_block_exclusive;*/
			/*message_packet->cache_block_state = cgm_cache_block_shared;*/

			//set dest and src
			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);
			message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
			message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

			//transmit to SA
			cache_put_io_down_queue(cache, message_packet);
			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			//stats;
			cache->hits++;

			/*on the first GET the block should have been brought in as exclusive.
			Then it will be a hit on retry with no presence bits set (exclusive).
			On a subsequent access (by either the requesting core or a different core) the block will be here as exclusive,
			if the request comes from the original core the block can be sent as exclusive again to be modified.
			if the request comes from a different core the block will need to be invalidated and forwarded to the requesting core.
			the block should only ever be in one core if not downgraded to shared*/

			assert(num_sharers >= 0 && num_sharers <= num_cores);
			assert(owning_core >= 0 && owning_core <= 1);

			//check if the packet has coalesced accesses.
			if(message_packet->access_type == cgm_access_store_retry || message_packet->coalesced == 1)
			{
				//enter retry state.
				cache_coalesed_retry(cache, message_packet->tag, message_packet->set);
			}

			//if it is a new access (L3 retry) or a repeat access from an already owning core.
			if(num_sharers == 0 || owning_core == 1)
			{
				//if the block is in the E state set M before sending up
				if(*cache_block_state_ptr == cgm_cache_block_exclusive)
				{
					cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);
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
				message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);

				//printf("Sending %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

				cache_put_io_up_queue(cache, message_packet);

			}
			else if(num_sharers >= 1)
			{
				//in the exclusive state there should only be one core with the cache block
				//there better be only one owning core at this stage.
				assert(num_sharers == 1);

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

				cache_put_io_up_queue(cache, message_packet);
			}

			break;

		case cgm_cache_block_shared:

			//delete me
			fatal("L3 getx shared\n");

			//stats
			cache->upgrade_misses++;

			/*access was a miss at the L1 and L2 but hit as shared in L3
			we need to process an upgrade as a putx with n number of invals*/

			/*there should always be at least one sharer
			but no more than the number of cores.*/
			assert(num_sharers >= 1 && num_sharers <= num_cores);

			//build the upgrade_ack packet
			//set access type
			message_packet->access_type = cgm_access_upgrade_putx_n;
			message_packet->cache_block_state = cgm_cache_block_modified;

			//set number of sharers
			//if there is 1 sharer and its the owning core set 0
			if(num_sharers == 1 && owning_core == 1)
			{
				message_packet->upgrade_ack = 0;
			}
			else
			{
				/*set the number of inval_acks expected to receive
				number of sharers minus your self*/
				message_packet->upgrade_ack = (num_sharers - 1);
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

			//set the sharer bit for the upgraded node
			cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);
	}

	return;
}

void cgm_mesi_l3_downgrade_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	//downgrade the line to shared and add sharers

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);



	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
		fatal("cgm_mesi_l3_downgrade_ack(): L3 id %d invalid block state on down_grade_ack as %s access id %llu address %u tag %d set %d way %d\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->access_id, message_packet->address, message_packet->tag, message_packet->set, message_packet->way);
			break;

		case cgm_cache_block_invalid:

			//block should be in wb buffer waiting on flush

			//check WB for line...
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/

				assert((write_back_packet->cache_block_state == cgm_cache_block_modified
						|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

				fatal("l3 down grade ack\n");
			}
			else
			{
				fatal("l3 miss on downgrade ack check this\n");
			}

			break;

		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			//uncomment later
			//downgrade the local block
			//cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_shared);
			//uncomment later

			//delete removing s state
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, *cache_block_state_ptr);
			//delete removing s state

			//clear the directory entry
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

			//set the new sharer bit in the directory
			cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

			//go ahead and destroy the downgrade message because we don't need it anymore.
			message_packet = list_remove(cache->last_queue, message_packet);
			free(message_packet);
			break;
	}
	return;
}

void cgm_mesi_l3_downgrade_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	//star todo when adding in GETX we need to deal with a incoming WB that needs to be processed before this nack
	//failed to downgrade block in sharing core, the block may not be present
	//if hit clear directory state and set retry
	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
		fatal("cgm_mesi_l3_downgrade_nack(): L3 id %d invalid block state on down_grade_nack as %s access id %llu address %u tag %d set %d way %d\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->access_id, message_packet->address, message_packet->tag, message_packet->set, message_packet->way);
			break;

		case cgm_cache_block_invalid:

			//check WB for line...
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/

				assert((write_back_packet->cache_block_state == cgm_cache_block_modified
						|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);


				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				fatal("down grade nack check this\n");
			}
			else
			{
				/*its possible that L3 may have evicted the block handle it, if this case comes up.
				Should be a memory request to the memory controller*/
				fatal("l3 miss on downgrade nack check this\n");
			}
			break;


		case cgm_cache_block_exclusive:
		case cgm_cache_block_modified:

			/*note on shared state, this is possible because L3 evicts lines the line was evicted from the owning core*/

			/*nack from L2 clear the directory and retry the access as a new access*/
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

			//set the block state
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, *cache_block_state_ptr);

			/*retry the access at the L3 level*/
			message_packet->access_type = cgm_access_get;

			break;
	}

	return;
}

void cgm_mesi_l3_getx_fwd_ack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	//charge delay
	P_PAUSE(cache->latency);

	//downgrade the line to shared and add sharers
	/*printf("L3 id %d access getx_fwd_ack received\n", cache->id);*/

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

	/*printf("L3 id %d block hit %d as %s\n", cache->id, *cache_block_hit_ptr, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));*/

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
		fatal("cgm_mesi_l3_getx_fwd_ack(): L3 id %d invalid block state on getx_fwd_ack as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

		case cgm_cache_block_invalid:

			//check WB for line...
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/

				assert((write_back_packet->cache_block_state == cgm_cache_block_modified
						|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);

				fatal("l3 getx fwd ack\n");
			}
			else
			{
				fatal("l3 miss on downgrade ack check this\n");
			}

			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			//this handles the sharing write back as well.

			//set the local block
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);

			//clear the directory
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

			//set the new sharer bit in the directory
			cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

			//go ahead and destroy the getx_fwd_ack message because we don't need it anymore.
			message_packet = list_remove(cache->last_queue, message_packet);
			free(message_packet);

			break;
	}

	return;
}

void cgm_mesi_l3_getx_fwd_nack(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *write_back_packet = NULL;

	//charge delay
	P_PAUSE(cache->latency);

	//get the status of the cache block and try to find it in either the cache or wb buffer
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//search the WB buffer for the data
	write_back_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);


	//failed to downgrade block in sharing core, the block may not be present
	//if hit clear directory state and set retry

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_shared:
		fatal("cgm_mesi_l3_getx_fwd_nack(): L3 id %d invalid block state on getx_fwd_nack as %s address %u\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
			break;

			/*star todo block evicted by L2 and inval is on its way to L1
			change upgrade request to GetX and send on to L3 cache.*/

		case cgm_cache_block_invalid:

			//check WB for line...
			if(write_back_packet)
			{
				/*found the packet in the write back buffer
				data should not be in the rest of the cache*/

				assert((write_back_packet->cache_block_state == cgm_cache_block_modified
						|| write_back_packet->cache_block_state == cgm_cache_block_exclusive) && *cache_block_state_ptr == 0);


				write_back_packet = list_remove(cache->write_back_buffer, write_back_packet);
				packet_destroy(write_back_packet);

				fatal("getx fwd nack check this\n");

			}
			else
			{
				fatal("cgm_mesi_l3_getx_fwd_nack(): miss on getx_forward nack\n");
			}

			break;


		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:

			//clear the cache dir
			cgm_cache_clear_dir(cache, message_packet->set, message_packet->way);

			//set the block state
			cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);

			/*retry the access at the L3 level
			its possible that L3 may have evicted the block*/
			message_packet->access_type = cgm_access_getx;

			break;
	}

	return;
}

int cgm_mesi_l3_upgrade(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *upgrade_inval_request_packet;

	int num_cores = x86_cpu_num_cores;
	int num_sharers, owning_core, i;

	int l2_src_id;
	char *l2_name;

	/*enum cgm_access_kind_t access_type;
	long long access_id = 0;
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;*/

	//charge the delay
	P_PAUSE(cache->latency);

	//get the status of the cache block
	cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

	//get the directory state
	//check the directory dirty bit status
	//dirty = cgm_cache_get_dir_dirty_bit(cache, message_packet->set, message_packet->way);
	//get number of sharers
	num_sharers = cgm_cache_get_num_shares(cache, message_packet->set, message_packet->way);
	//check to see if access is from an already owning core
	owning_core = cgm_cache_is_owning_core(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);

	switch(*cache_block_state_ptr)
	{
		case cgm_cache_block_noncoherent:
		case cgm_cache_block_owned:
		case cgm_cache_block_invalid:
			fatal("cgm_mesi_l3_upgrade(): L3 id %d invalid block state on upgrade as %s access_id %llu address 0x%08x set %d and way %d sharers %d owning core %d\n",
				cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->access_id, message_packet->address, message_packet->set, message_packet->way, num_sharers, owning_core);
			break;

		case cgm_cache_block_modified:
		case cgm_cache_block_exclusive:
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
				if(*cache_block_state_ptr == cgm_cache_block_shared)
				{
					/*set the number of inval_acks expected to receive number of sharers minus your self*/
					message_packet->upgrade_ack = (num_sharers - 1);
				}
				else if(*cache_block_state_ptr == cgm_cache_block_shared || *cache_block_state_ptr == cgm_cache_block_modified)
				{
					/*another core has gained control. */
					message_packet->upgrade_ack = num_sharers;

					/*should only be one other core*/
					assert(message_packet->upgrade_ack == 1);
				}
			}

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

			//set the sharer bit for the upgraded node
			cgm_cache_set_dir(cache, message_packet->set, message_packet->way, message_packet->l2_cache_id);
			break;
	}

	return 1;
}


void cgm_mesi_l3_write_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	assert(cache->cache_type == l3_cache_t);
	assert((message_packet->access_type == cgm_access_mc_put && message_packet->cache_block_state == cgm_cache_block_modified)
			|| (message_packet->access_type == cgm_access_mc_put && message_packet->cache_block_state == cgm_cache_block_exclusive)
			|| (message_packet->access_type == cgm_access_mc_put && message_packet->cache_block_state == cgm_cache_block_shared));


	/*if(message_packet->tag == 8 && message_packet->set == 320)
	{
		printf("here %d\n", message_packet->cache_block_state);
	}*/

	//find the access in the ORT table and clear it.
	ort_clear(cache, message_packet);

	//set the block data
	cgm_cache_set_block(cache, message_packet->set, message_packet->l3_victim_way, message_packet->tag, message_packet->cache_block_state);

	//testing
	//set block address
	//cgm_cache_set_block_address(cache, message_packet->set, message_packet->l3_victim_way, message_packet->address);
	//testing

	//set retry state
	message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	return;
}

int cgm_mesi_l3_write_back(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	struct cgm_packet_t *wb_packet;

	//charge the delay
	P_PAUSE(cache->latency);

	//WB from L2 cache
	if(cache->last_queue == cache->Rx_queue_top)
	{
		//we should only receive modified lines from L2 cache
		assert(message_packet->cache_block_state != cgm_cache_block_exclusive
				&& message_packet->cache_block_state != cgm_cache_block_shared
				&& message_packet->cache_block_state != cgm_cache_block_invalid);

		//get the state of the cache block
		cache_get_block_status(cache, message_packet, cache_block_hit_ptr, cache_block_state_ptr);

		switch(*cache_block_state_ptr)
		{
			case cgm_cache_block_noncoherent:
			case cgm_cache_block_owned:
			case cgm_cache_block_shared:
			fatal("cgm_mesi_l3_write_back(): L3 id %d invalid block state on write back as %s address %u\n",
					cache->id, str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), message_packet->address);
				break;

			case cgm_cache_block_invalid:

				/*Star it is possible for the WB from L2 to
				miss at the L3. This means there was a recent L3 eviction
				and the eviction is on its way up to the L2 and L1 D cache.*/

				/*When this happens check the local WB buffer for the line which should be in the E or M state.*/

				//check the WB buffer
				wb_packet = cache_search_wb(cache, message_packet->tag, message_packet->set);

				if(wb_packet)
				{
					/*fatal("L3 found block in WB should not occur until l3 inf is off\n");*/

					//cache block found in the WB buffer merge the change here
					//set modified if the line was exclusive
					if(wb_packet->cache_block_state == cgm_cache_block_exclusive)
					{
						wb_packet->cache_block_state = cgm_cache_block_modified;
					}
					else if(wb_packet->cache_block_state == cgm_cache_block_modified)
					{
						/*technically, the line in L2 maybe in the modified state,
						but the line form L1 D maybe modified and is newer then the L2 line*/
						wb_packet->cache_block_state = cgm_cache_block_modified;
					}

					//destroy the L2 WB message. L3 will clear its WB at an opportune time.
					message_packet = list_remove(cache->last_queue, message_packet);
					packet_destroy(message_packet);
				}
				else
				{
					//block not found in either cache or WB buffer, fwd WB down to memory controller
					//add routing/status data to the packet
					message_packet->access_type = cgm_access_mc_store;

					message_packet->src_name = cache->name;
					message_packet->src_id = str_map_string(&node_strn_map, cache->name);
					message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
					message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

					//transmit to SA/MC
					cache_put_io_down_queue(cache, message_packet);
				}

				break;

			case cgm_cache_block_exclusive:
			case cgm_cache_block_modified:

				//hit in cache merge WB here.

				//set modified if the line was exclusive
				if(*cache_block_state_ptr == cgm_cache_block_exclusive)
				{
					cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);
				}
				else if(*cache_block_state_ptr == cgm_cache_block_modified)
				{
					/*technically, the line in L3 maybe in the modified state,
					but the line form L1 D maybe modified and is newer then the L3 line*/
					cgm_cache_set_block_state(cache, message_packet->set, message_packet->way, cgm_cache_block_modified);
				}

				//destroy the L2 WB message. L3 will clear its WB at an opportune time.
				message_packet = list_remove(cache->last_queue, message_packet);
				packet_destroy(message_packet);
				break;
		}
	}
	//if here the L3 generated it's own write back.
	else if(cache->last_queue == cache->write_back_buffer)
	{

		if(message_packet->cache_block_state == cgm_cache_block_exclusive)
		{
			/*drop the write back*/
			message_packet = list_remove(cache->last_queue, message_packet);
			packet_destroy(message_packet);
		}
		else if (message_packet->cache_block_state ==  cgm_cache_block_modified)
		{
			/*send the Write Back to the memory controller if the line is modified*/
			//printf("WB from L3\n");

			//add routing/status data to the packet
			message_packet->access_type = cgm_access_mc_store;

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
	if(gpu_l1_inf && (cache->cache_type == gpu_v_cache_t || cache->cache_type == gpu_s_cache_t))
	{
		if(cache->cache_type == gpu_v_cache_t || cache->cache_type == gpu_s_cache_t)
		{
		cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_noncoherent);
		}
	}
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
	if(gpu_l1_inf && cache->cache_type == gpu_v_cache_t)
	{
		if(cache->cache_type == gpu_v_cache_t)
		{
		cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_noncoherent);
		}
	}
	//////testing

	CGM_DEBUG(GPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);


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
	if(gpu_l2_inf && cache->cache_type == gpu_l2_cache_t)
	{
		if(cache->cache_type == gpu_l2_cache_t)
		{
		cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_noncoherent);
		}
	}
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

			while(!hub_iommu_can_access(hub_iommu->Rx_queue_top[cache->id]))
			{
				printf("%s stalled cycle %llu\n", cache->name, P_TIME);
				P_PAUSE(1);
			}

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu miss %s size %d\n",
					cache->name, access_id, P_TIME, hub_iommu->Rx_queue_top[cache->id]->name, list_count(hub_iommu->Rx_queue_top[cache->id]));

			P_PAUSE(cache->latency);
			//P_PAUSE(hub_iommu->wire_latency);

			message_packet->l1_access_type = message_packet->access_type;

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

	CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu puts\n", cache->name, access_id, P_TIME);

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
					free(message_packet);

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
					free(message_packet);

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
