/*
 * cache_v_ctrl.c
 *
 *  Created on: Apr 28, 2015
 *      Author: stardica
 */



#include <stdio.h>

#include <arch/si/timing/gpu.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>

#include <cgm/cache.h>
#include <cgm/cgm.h>
#include <cgm/switch.h>
#include <cgm/protocol.h>


void gpu_v_cache_access_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	struct cgm_packet_t *miss_status_packet;
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
	int mshr_status = 0;

	//stats
	cache->loads++;

	//access information
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	//store the decode
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;


	CGM_DEBUG(GPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//////testing
	//cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_noncoherent);
	//////testing


	/*printf("gpu_v_cache addr 0x%08u\n",  addr);
	getchar();*/

	//get the block and the state of the block and charge cycles
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(cache->latency);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{

		assert(*state_ptr != cache_block_invalid);

		if(*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
		{
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

			cache->hits++;

			(*message_packet->witness_ptr)++;
			list_remove(cache->last_queue, message_packet);
		}
		else
		{
			fatal("gpu_v_cache_access_load(): incorrect block state set");
		}

	}

	//Cache Miss!
	else if(cache_status == 0 || *state_ptr == 0)
	{
		//on both a miss and invalid hit the state_ptr should be zero
		cache->misses++;

		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		miss_status_packet = miss_status_packet_copy(message_packet, *set_ptr, *tag_ptr, *offset_ptr, str_map_string(&gpu_l1_strn_map, cache->name));
		mshr_status = mshr_set(cache, miss_status_packet);

		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, mshr_status);

		if(mshr_status == 2)
		{
			//access was coalesced
			//remove the message packet on coalesce, but don't send to next cache
			list_remove(cache->last_queue, message_packet);

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu coalesced packet removed removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
		}
		else if(mshr_status == 1)
		{
			//access is unique in the MSHR so send forward
			//while the next level of cache's in queue is full stall
			while(!cache_can_access_top(&gpu_l2_caches[cgm_gpu_cache_map(cache->id)]))
			{
				printf("%s cache_can_access_top(&gpu_l2_caches[cgm_gpu_cache_map(cache->id)]))\n", cache->name);
				P_PAUSE(1);
			}

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu l2 queue free size %d\n",
					cache->name, access_id, P_TIME, list_count(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].Rx_queue_top));

			/*change the access type for the coherence protocol and drop into the L2's queue
			remove the access from the l1 cache queue and place it in the l2 cache ctrl queue*/

			message_packet->access_type = cgm_access_gets_v;
			message_packet->gpu_cache_id = cache->id;
			list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			list_enqueue(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].Rx_queue_top, message_packet);

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu %s as %s\n",
					cache->name, access_id, P_TIME, gpu_l2_caches[cgm_gpu_cache_map(cache->id)].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));


			CGM_DEBUG(protocol_debug_file, "%s Access_id %llu cycle %llu %s miss SEND %s %s\n",
					cache->name, access_id, P_TIME, cache->name, gpu_l2_caches[cgm_gpu_cache_map(cache->id)].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			//advance the L2 cache adding some wire delay time.
			future_advance(&gpu_l2_cache[cgm_gpu_cache_map(cache->id)], WIRE_DELAY(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].wire_latency));
		}
		else //mshr == 0
		{
			printf("breaking\n");
			mshr_dump(cache);
			STOP;

			//mshr is full so we can't progress, retry.
			fatal("gpu_v_cache_access_load(): MSHR full\n");
		}
	}

	return;
}

void gpu_v_cache_access_store(struct cache_t *cache, struct cgm_packet_t *message_packet){


	struct cgm_packet_t *miss_status_packet;
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
	int mshr_status = 0;

	//stats
	cache->stores++;

	//access information
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);
	//store the decode for later
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	CGM_DEBUG(GPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);


	/////////testing
	//cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_noncoherent);
	/////////testing


	//get the block and the state of the block and charge cycles
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(cache->latency);


	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)// && *state_ptr != cache_block_shared)
	{
		assert(*state_ptr != cache_block_invalid);

		//star todo this is wrong
		if(*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
		{
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu hit state %s\n", cache->name, access_id, P_TIME, str_map_value(&cache_block_state_map, *state_ptr));

			cache->hits++;

			(*message_packet->witness_ptr)++;
			list_remove(cache->last_queue, message_packet);

		}
		else
		{
			fatal("gpu_v_cache_access_load(): incorrect block state set");
		}

	}

	//Cache Miss!
	if(cache_status == 0 || *state_ptr == 0)
	{
		//on both a miss and invalid hit the state_ptr should be zero
		cache->misses;

		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		miss_status_packet = miss_status_packet_copy(message_packet, *set_ptr, *tag_ptr, *offset_ptr, str_map_string(&gpu_l1_strn_map, cache->name));
		mshr_status = mshr_set(cache, miss_status_packet);

		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, mshr_status);

		if(mshr_status == 2)
		{
			//access was coalesced
			//remove the message packet on coalesce, but don't send to next cache
			list_remove(cache->last_queue, message_packet);

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu coalesced packet removed removed from %s size %d\n",
				cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
		}
		else if(mshr_status == 1)
		{
			//access is unique in the MSHR so send forward
			//while the next level of cache's in queue is full stall
			while(!cache_can_access_top(&gpu_l2_caches[cgm_gpu_cache_map(cache->id)]))
			{
				P_PAUSE(1);
			}

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu l2 queue free size %d\n",
				cache->name, access_id, P_TIME, list_count(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].Rx_queue_top));

			/*change the access type for the coherence protocol and drop into the L2's queue
			remove the access from the l1 cache queue and place it in the l2 cache ctrl queue*/

			message_packet->gpu_access_type = cgm_access_store;
			message_packet->gpu_cache_id = cache->id;
			message_packet->access_type = cgm_access_gets_v;
			list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
				cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			list_enqueue(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].Rx_queue_top, message_packet);

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu %s as %s\n",
				cache->name, access_id, P_TIME, gpu_l2_caches[cgm_gpu_cache_map(cache->id)].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));


			CGM_DEBUG(protocol_debug_file, "%s Access_id %llu cycle %llu %s miss SEND %s %s\n",
				cache->name, access_id, P_TIME, cache->name, gpu_l2_caches[cgm_gpu_cache_map(cache->id)].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			//advance the L2 cache adding some wire delay time.
			future_advance(&gpu_l2_cache[cgm_gpu_cache_map(cache->id)], WIRE_DELAY(gpu_l2_caches[cgm_gpu_cache_map(cache->id)].wire_latency));
		}
		else //mshr == 0
		{
			printf("breaking\n");
			mshr_dump(cache);
			STOP;
			//mshr is full so we can't progress, retry.
			fatal("l1_d_cache_access_load(): MSHR full\n");
		}
	}

	printf("gpu c cache store\n");
	STOP;

	return;
}

void gpu_v_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet){

	struct cgm_packet_status_t *miss_status_packet;
	enum cgm_access_kind_t access_type;
	unsigned int addr = 0;
	long long access_id = 0;
	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int state = 0;
	int cache_status = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;

	int mshr_status = 0;
	int retry = 0;
	int *retry_ptr = &retry;

	//access information
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;

	//stats
	cache->retries++;


	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(GPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
		cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//get the block and the state of the block and charge a cycle
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	//P_PAUSE(2);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{

		/*if(message_packet->gpu_access_type == cgm_access_nc_store || message_packet->gpu_access_type == cgm_access_store || message_packet->gpu_access_type == cgm_access_load)
		{*/
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

			(*message_packet->witness_ptr)++;
			list_remove(cache->retry_queue, message_packet);

		/*}
		else
		{
			fatal("gpu_v_cache_access_retry(): %s miss on retry cycle %llu access_id %llu %s\n", cache->name, P_TIME, access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->gpu_access_type));
		}*/
	}
	else
	{
		printf("breaking\n");
		STOP;
		fatal("gpu_v_cache_access_retry(): miss on retry cycle %llu access_id %llu\n", P_TIME, access_id);
	}

	return;
}

void gpu_v_cache_access_puts(struct cache_t *cache, struct cgm_packet_t *message_packet){

	struct cgm_packet_t *miss_status_packet;
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

	int mshr_row = -1;
	int i = 0;


	//the packet is from the L2 cache
	access_type = message_packet->access_type;
	addr = message_packet->address;
	access_id = message_packet->access_id;

	//probe the address for set, tag, and offset.
	//assert(addr != 0);
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu puts\n", cache->name, access_id, P_TIME);

	//charge the delay for writing cache block

	//star todo insert a function to set the correct block state here
	cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_noncoherent);
	P_PAUSE(cache->latency);

	//get the mshr status
	mshr_row = mshr_get_status(cache, set_ptr, tag_ptr, access_id);
	assert(mshr_row != -1);


	//check the number of entries in the mshr row
	assert(list_count(cache->mshrs[mshr_row].entires) == cache->mshrs[mshr_row].num_entries);
	assert(cache->mshrs[mshr_row].num_entries > 0);

	CGM_DEBUG(mshr_debug_file, "%s access_id %llu cycle %llu mshr_row %d num_entries %d\n",
			cache->name, access_id, P_TIME, mshr_row, cache->mshrs[mshr_row].num_entries);

	//move them to the retry queueS
	for(i = 0; i < cache->mshrs[mshr_row].num_entries; i++)
	{
		//mshr_dump(cache);
		miss_status_packet = list_dequeue(cache->mshrs[mshr_row].entires);

		//printf(" l1 D puts name %s access_id %llu\n", miss_status_packet->name, access_id);


		CGM_DEBUG(mshr_debug_file, "%s access_id %llu coalesced %d tag %d set %d\n",
				cache->name, miss_status_packet->access_id, miss_status_packet->coalesced, miss_status_packet->tag, miss_status_packet->set);

		assert(miss_status_packet != NULL);
		//assert(miss_status_packet->address != 0);


		if (miss_status_packet->access_id == access_id)
		{
			//this is the first entry and was not coalesced
			assert(miss_status_packet->access_id == access_id);
			assert(miss_status_packet->coalesced == 0);

			//we can put either the message_packet or miss_status_packet in the retry queue.
			message_packet->access_type = cgm_access_retry;
			list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->retry_queue, message_packet);

			/*printf("miss_status_packet->access_id %llu access_id %llu\n", miss_status_packet->access_id, access_id );
			printf("miss_status_packet %s\n", message_packet->name);//miss_status_packet->coalesced_packet->name);
			printf("test\n");*/
		}
		else
		{
			//this is a coalesced packet
			if(miss_status_packet->coalesced != 1)
			{
				printf("%s Crashing access_id %llu cycle %llu\n", cache->name, access_id, P_TIME);
				printf("i %d miss sp %llu, coalesced %d\n", i, miss_status_packet->access_id, miss_status_packet->coalesced);
				mshr_dump(cache);
				STOP;
			}

			assert(miss_status_packet->coalesced == 1);

			//drop it into the retry queue
			list_enqueue(cache->retry_queue, miss_status_packet);
		}
	}

	long long time /* :-P */ = etime.count;

	//advance the cache by the number of packets
	for(i = 0; i < cache->mshrs[mshr_row].num_entries; i ++)
	{
		/*printf("entries %d\n", cache->mshrs[mshr_row].num_entries);
		printf("advances\n");*/

		time += 2;
		future_advance(&gpu_v_cache[cache->id], time);
	}

	//clear the mshr row for future use
	mshr_clear(&(cache->mshrs[mshr_row]));

	/*printf("gpu v cache puts\n");
	STOP;*/

	return;
}
