/*
 * cache_s_ctrl.c
 *
 *  Created on: Apr 28, 2015
 *      Author: stardica
 */


#include <stdio.h>

#include <arch/si/timing/gpu.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/linked-list.h>

#include <cgm/cache.h>
#include <cgm/cgm.h>
#include <cgm/switch.h>
#include <cgm/protocol.h>


void gpu_s_cache_access_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

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


	/*printf("gpu_s_cache addr 0x%08u\n",  addr);
	getchar();*/

	//get the block and the state of the block and charge cycles
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(2);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{
		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

		cache->hits++;

		//remove packet from cache queue, global queue, and simulator memory
		(*message_packet->witness_ptr)++;
		list_remove(cache->last_queue, message_packet);
	}
	//Cache Miss!
	else if(cache_status == 0 || *state_ptr == 0)
	{

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
			while(!cache_can_access_top(&gpu_l2_caches[cgm_cache_map(cache->id)]))
			{
				printf("cache_can_access_top(&gpu_l2_caches[cgm_cache_map(cache->id)]))\n");

				P_PAUSE(1);
			}



			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu l2 queue free size %d\n",
					cache->name, access_id, P_TIME, list_count(gpu_l2_caches[cgm_cache_map(cache->id)].Rx_queue_top));

			/*change the access type for the coherence protocol and drop into the L2's queue
			remove the access from the l1 cache queue and place it in the l2 cache ctrl queue*/

			message_packet->access_type = cgm_access_gets_s;
			message_packet->gpu_cache_id = cache->id;
			list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			list_enqueue(gpu_l2_caches[cgm_cache_map(cache->id)].Rx_queue_top, message_packet);

			CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu %s as %s\n",
					cache->name, access_id, P_TIME, gpu_l2_caches[cgm_cache_map(cache->id)].name, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));


			CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu %s Miss SEND %s %s\n",
					access_id, P_TIME, cache->name, gpu_l2_caches[cgm_cache_map(cache->id)].name, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			//advance the L2 cache adding some wire delay time.
			future_advance(&gpu_l2_cache[cgm_cache_map(cache->id)], WIRE_DELAY(gpu_l2_caches[cgm_cache_map(cache->id)].wire_latency));

		}
		else //mshr == 0
		{
			printf("breaking MSHR full\n");
			mshr_dump(cache);

			//mshr is full so we can't progress, retry.
			fatal("l1_i_cache_access_load(): MSHR full\n");
		}
	}

	return;
}

void gpu_s_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet){

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
	P_PAUSE(2);

	//L1 I Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{
		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

		//remove packet from cache queue, global queue, and simulator memory

		//remove packet from cache queue, global queue, and simulator memory
		(*message_packet->witness_ptr)++;
		list_remove(cache->retry_queue, message_packet);
	}
	else
	{
		printf("breaking\n");
		STOP;
		fatal("cache_access_retry(): miss on retry cycle %llu access_id %llu\n", P_TIME, access_id);
	}


	return;
}


void gpu_s_cache_access_puts(struct cache_t *cache, struct cgm_packet_t *message_packet){

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
	//assert(addr != NULL);
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);


	CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu puts\n", cache->name, access_id, P_TIME);

	//charge the delay for writing cache block
	cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_noncoherent);
	P_PAUSE(1);

	//get the mshr status
	mshr_row = mshr_get(cache, set_ptr, tag_ptr, access_id);
	assert(mshr_row != -1);

	//check the number of entries in the mshr row
	assert(list_count(cache->mshrs[mshr_row].entires) == cache->mshrs[mshr_row].num_entries);
	assert(cache->mshrs[mshr_row].num_entries > 0);

	CGM_DEBUG(mshr_debug_file, "%s access_id %llu cycle %llu mshr_row %d num_entries %d\n", cache->name, access_id, P_TIME, mshr_row, cache->mshrs[mshr_row].num_entries);

	//move them to the retry queueS
	for(i = 0; i < cache->mshrs[mshr_row].num_entries; i++)
	{

		miss_status_packet = list_dequeue(cache->mshrs[mshr_row].entires);

		CGM_DEBUG(mshr_debug_file, "%s access_id %llu coalesced %d tag %d set %d\n",
				cache->name, miss_status_packet->access_id, miss_status_packet->coalesced, miss_status_packet->tag, miss_status_packet->set);

		assert(miss_status_packet != NULL);
		assert(miss_status_packet->address != 0);


		if (miss_status_packet->access_id == access_id)
		{
			assert(miss_status_packet->access_id == access_id);

			//this is the first entry and was not coalesced
			assert(miss_status_packet->coalesced == 0);

			//we can put eighter the message_packet or miss_status_packet in the retry queue.
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
				printf("breaking access_id %llu cycle %llu\n", access_id, P_TIME);
				printf("i %d miss sp %llu, coalesced %d\n", i, miss_status_packet->access_id, miss_status_packet->coalesced);

				mshr_dump(cache);

				STOP;
			}

			assert(miss_status_packet->coalesced == 1);

			//drop it into the retry queue
			list_enqueue(cache->retry_queue, miss_status_packet);

		}
	}

	long long time = etime.count; //:-P

	//advance the cache by the number of packets
	for(i = 0; i < cache->mshrs[mshr_row].num_entries; i ++)
	{
		/*printf("entries %d\n", cache->mshrs[mshr_row].num_entries);
		printf("advances\n");*/

		time += 2;
		future_advance(&gpu_s_cache[cache->id], time);
	}

	//clear the mshr row for future use
	mshr_clear(&(cache->mshrs[mshr_row]));

	return;
}




void gpu_s_cache_ctrl(void){

	int my_pid = gpu_s_pid++;
	long long step = 1;
	int num_cus = si_gpu_num_compute_units;

	struct cgm_packet_t *message_packet;
	//struct cgm_packet_status_t *mshr_packet;

	enum cgm_access_kind_t access_type;
	long long access_id = 0;

/*	unsigned int addr = 0;

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
	int *state_ptr = &state;*/

	assert(my_pid <= num_cus);
	set_id((unsigned int)my_pid);

	while(1)
	{


		//wait here until there is a job to do
		await(&gpu_s_cache[my_pid], step);
		step++;


		//get a message from the top or bottom queues.
		message_packet = cache_get_message(&(gpu_s_caches[my_pid]));

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;

		/////////testing
		//(*message_packet->witness_ptr)++;
		//list_remove(gpu_s_caches[my_pid].Rx_queue_top, message_packet);
		//continue;
		/////////testing


		if (access_type == cgm_access_load)
		{
			gpu_s_cache_access_load(&(gpu_s_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_retry)
		{
			gpu_s_cache_access_retry(&(gpu_s_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_puts)
		{
			gpu_s_cache_access_puts(&(gpu_s_caches[my_pid]), message_packet);
		}
		else
		{
			fatal("gpu_s_cache_ctrl(): access_id %llu bad access type %s at cycle %llu\n",
				access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
		}
	}

	/* should never get here*/
	fatal("gpu_s_cache_ctrl task is broken\n");
	return;
}
