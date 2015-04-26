/*
 * i_cache_ctr.c
 *
 *  Created on: Apr 22, 2015
 *      Author: stardica
 */

#include <stdio.h>

#include <arch/x86/timing/cpu.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>

#include <cgm/cache.h>
#include <cgm/cgm.h>
#include <cgm/switch.h>

void l1_i_cache_access_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

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
	cache->loads++;

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(cache_debug_file,"l1_i_cache[%d] access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
		cache->id, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//get the block and the state of the block and charge cycles
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(2);

	//L1 I Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{
		CGM_DEBUG(cache_debug_file, "l1_i_cache[%d] access_id %llu cycle %llu hit\n", cache->id, access_id, P_TIME);

		/*if(access_type == cgm_access_retry)
			retry_ptr--;*/

		/*if(access_type == cgm_access_fetch)*/
			cache->hits++;

		//remove packet from cache queue, global queue, and simulator memory
		//note cycle already charged

		if(access_id == 1)
		{
			printf("access_id 1 end\n");
		}

		list_remove(cache->last_queue, message_packet);
		remove_from_global(access_id);
		free(message_packet);

	}
	//L1 I Cache Miss!
	else if(cache_status == 0 || *state_ptr == 0)
	{
		//all mshr based retires should be hits
		//star todo there is a bug here 1 access fails retry in our MM.
		assert(message_packet->access_type != cgm_access_retry);

		/*if(access_type == cgm_access_fetch)*/
		cache->misses++;

		CGM_DEBUG(cache_debug_file, "l1_i_cache[%d] access_id %llu cycle %llu miss\n", cache->id, access_id, P_TIME);


		miss_status_packet = miss_status_packet_create(message_packet->access_id, message_packet->access_type, set, tag, offset, str_map_string(&node_strn_map, cache->name));
		mshr_status = mshr_set(cache, miss_status_packet, message_packet);

		//printf("here\n");
		//printf("miss_status_packet->coalesced_packet->name %llu\n", miss_status_packet->coalesced_packet->access_id);
		//printf("here\n");

		CGM_DEBUG(cache_debug_file, "l1_i_cache[%d] access_id %llu cycle %llu miss mshr status %d\n", cache->id, access_id, P_TIME, mshr_status);

		if(mshr_status == 1)
		{
			//access is unique in the MSHR
			//while the next level of cache's in queue is full stall
			while(!cache_can_access_top(&l2_caches[cache->id]))
			{
				P_PAUSE(1);
			}

			CGM_DEBUG(cache_debug_file, "l1_i_cache[%d] access_id %llu cycle %llu miss l2 queue free size %d\n",
					cache->id, access_id, P_TIME, list_count(l2_caches[cache->id].Rx_queue_top));

			/*change the access type for the coherence protocol and drop into the L2's queue
			remove the access from the l1 cache queue and place it in the l2 cache ctrl queue*/



			message_packet->access_type = cgm_access_gets_i;
			list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(cache_debug_file, "l1_i_cache[%d] access_id %llu cycle %llu removed from %s size %d\n",
					cache->id, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
			list_enqueue(l2_caches[cache->id].Rx_queue_top, message_packet);

			CGM_DEBUG(cache_debug_file, "l1_i_cache[%d] access_id %llu cycle %llu l2_cache[%d] as %s\n",
				cache->id, access_id, P_TIME, cache->id, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu l1_i_cache[%d] Miss SEND %s to l2_cache[%d]\n",
					access_id, P_TIME, cache->id, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), cache->id);

			//advance the L2 cache adding some wire delay time.
			future_advance(&l2_cache[cache->id], WIRE_DELAY(l2_caches[cache->id].wire_latency));
		}
		else if(mshr_status == 0)
		{
			//mshr is full so we can't progress, retry.
			fatal("l1_i_cache_access_load(): MSHR full\n");

			message_packet->access_type = cgm_access_load;
			list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->retry_queue, message_packet);
			future_advance(&l1_i_cache[cache->id], (etime.count + 4));

		}
		else
		{
			//access was coalesced. For now do nothing until later.
		}

		//done
	}
	return;
}

void l1_i_cache_access_puts(struct cache_t *cache, struct cgm_packet_t *message_packet){

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

	long long advance_time = 0;
	int i = 0;

	//the packet is from the L2 cache
	access_type = message_packet->access_type;
	addr = message_packet->address;
	access_id = message_packet->access_id;

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(cache_debug_file, "l1_i_cache[%d] access_id %llu cycle %llu puts\n", cache->id, access_id, P_TIME);

	//charge the delay for writing cache block
	cgm_cache_set_block(cache, *set_ptr, *way_ptr, tag, cache_block_shared);
	P_PAUSE(1);

	//get the mshr status
	mshr_status = mshr_get(cache, set_ptr, tag_ptr);
	assert(mshr_status != -1);

	//get the number of entries in the mshr row
	int entry_size = list_count(cache->mshrs[mshr_status].entires);
	assert(entry_size == cache->mshrs[mshr_status].num_entries);
	assert(entry_size > 0);

	//printf("entry size %d\n", entry_size);
	//move them to the retry queue
	for(i = 0; i < entry_size; i ++)
	{
		//printf("i %d entry size %d\n", i, entry_size);

		/*assert( i < list_count(cache->mshrs[mshr_status].entires));*/

		miss_status_packet = list_dequeue(cache->mshrs[mshr_status].entires);

		assert(miss_status_packet != NULL);

		if (miss_status_packet->access_id == access_id)
		{
			//this is the first entry
			//move current message_packet to retry queue
			message_packet->access_type = cgm_access_retry;
			list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->retry_queue, message_packet);

			printf("miss_status_packet->access_id %llu access_id %llu\n", miss_status_packet->access_id, access_id );
			printf("miss_status_packet %s\n", message_packet->name);//miss_status_packet->coalesced_packet->name);
			printf("test\n");

		}
		else
		{
			printf("miss_status_packet->access_id %llu access_id %llu\n", miss_status_packet->access_id, access_id );


			//printf("miss_status_packet->coalesced_packet->name %s\n", miss_status_packet->coalesced_packet->name);
		}
	}

	getchar();

		//move the access and any coalesced accesses to the retry queue.
	//for(i = 0; i < *retry_ptr; i++)

	//printf("number of entries %d\n", list_count(cache->mshrs[mshr_status].entires));

	//advance(&l1_i_cache[cache->id]);

	/*for(i = 0; i < rt; i++)
	{
		if( i == 0)
		{

		}

		else if( i > 0 && i < rt)
		{
			miss_status_packet = list_remove_at(cache->mshrs[mshr_status].entires, i);


			list_enqueue(cache->retry_queue, miss_status_packet->coalesced_packet);
			free(miss_status_packet);
			advance(&l1_i_cache[cache->id]);
		}
	}*/

	/*printf("mshr_status %d retry_ptr %d\n", mshr_status, *retry_ptr);
	printf("list queue size %d\n", list_count(cache->retry_queue));
	STOP;*/

	//clear the mshr row for future use
	mshr_clear(&(cache->mshrs[mshr_status]));

	//done.
	return;
}

void l1_i_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet){

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
	/*if(access_type == cgm_access_retry)*/
	cache->retries++;

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(cache_debug_file,"l1_i_cache[%d] access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
		cache->id, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//get the block and the state of the block and charge a cycle
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(2);

	//L1 I Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{
		CGM_DEBUG(cache_debug_file, "l1_i_cache[%d] access_id %llu cycle %llu hit\n", cache->id, access_id, P_TIME);

		//remove packet from cache queue, global queue, and simulator memory

		list_remove(cache->retry_queue, message_packet);
		remove_from_global(access_id);
		free(message_packet);
	}
	else
	{
		fatal("cache_access_retry(): miss on retry\n");
	}

	return;
}

void l1_i_cache_ctrl(void){

	//my_pid increments for the number of CPU cores. i.e. 0 - 4 for a quad core
	int my_pid = l1_i_pid++;
	//l1_i_caches[my_pid].id = my_pid;

	int num_cores = x86_cpu_num_cores;
	long long step = 1;

	struct cgm_packet_t *message_packet;
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

	/*int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;*/

	int mshr_status = 0;
	int retry = 0;
	int *retry_ptr = &retry;

	/*int i = 0;
	long long advance_time = 0;*/

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{

		//wait here until there is a job to do
		await(&l1_i_cache[my_pid], step);
		step++;

		//get a message from the top or bottom queues.
		message_packet = cache_get_message(&(l1_i_caches[my_pid]));

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;

		//printf("retry type %s access id %llu at %llu\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), access_id, P_TIME);


		if (access_type == cgm_access_fetch)
		{
			l1_i_cache_access_load(&(l1_i_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_retry)
		{
			l1_i_cache_access_retry(&(l1_i_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_puts)
		{
			l1_i_cache_access_puts(&(l1_i_caches[my_pid]), message_packet);
		}
		else
		{
			fatal("l1_i_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
				access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
		}

	}
	/* should never get here*/
	fatal("l1_i_cache_ctrl task is broken\n");
	return;
}
