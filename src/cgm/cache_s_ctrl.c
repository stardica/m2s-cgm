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


	CGM_DEBUG(cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//////testing
	cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_shared);
	//////testing

	//get the block and the state of the block and charge cycles
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(2);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{
		CGM_DEBUG(cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

		cache->hits++;

		//remove packet from cache queue, global queue, and simulator memory
		(*message_packet->witness_ptr)++;
		list_remove(cache->last_queue, message_packet);
		//free(message_packet);
	}
	//Cache Miss!
	else if(cache_status == 0 || *state_ptr == 0)
	{
		cache->misses++;

		CGM_DEBUG(cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		miss_status_packet = miss_status_packet_copy(message_packet, *set_ptr, *tag_ptr, *offset_ptr, str_map_string(&l1_strn_map, cache->name));
		mshr_status = mshr_set((void *)cache, miss_status_packet);

		CGM_DEBUG(cache_debug_file, "%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, mshr_status);

		if(mshr_status == 2)
		{
			//access was coalesced
			//remove the message packet on coalesce, but don't send to next cache
			list_remove(cache->last_queue, message_packet);

			CGM_DEBUG(cache_debug_file, "%s access_id %llu cycle %llu coalesced packet removed removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
		}
		else if(mshr_status == 1)
		{
			//access is unique in the MSHR so send forward
			//while the next level of cache's in queue is full stall
			while(!cache_can_access_top(&l2_caches[cache->id]))
			{
				P_PAUSE(1);
			}

			CGM_DEBUG(cache_debug_file, "%s access_id %llu cycle %llu l2 queue free size %d\n",
					cache->name, access_id, P_TIME, list_count(l2_caches[cache->id].Rx_queue_top));

			/*change the access type for the coherence protocol and drop into the L2's queue
			remove the access from the l1 cache queue and place it in the l2 cache ctrl queue*/

			message_packet->access_type = cgm_access_gets_i;
			list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			list_enqueue(l2_caches[cache->id].Rx_queue_top, message_packet);

			CGM_DEBUG(cache_debug_file, "%s access_id %llu cycle %llu l2_cache[%d] as %s\n",
					cache->name, access_id, P_TIME, cache->id, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));


			CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu %s Miss SEND %s %s\n",
					access_id, P_TIME, cache->name, l2_caches[cache->id].name, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), cache->id);

			//advance the L2 cache adding some wire delay time.
			future_advance(&l2_cache[cache->id], WIRE_DELAY(l2_caches[cache->id].wire_latency));
		}
		else //mshr == 0
		{
			printf("breaking MSHR full\n");
			mshr_dump(cache);
			STOP;


			//mshr is full so we can't progress, retry.
			fatal("l1_i_cache_access_load(): MSHR full\n");
		}
	}

	return;
}

void gpu_s_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet){


	return;
}


void gpu_s_cache_access_puts(struct cache_t *cache, struct cgm_packet_t *message_packet){


	return;
}




void gpu_s_cache_ctrl(void){

	int my_pid = gpu_s_pid++;
	long long step = 1;
	int num_cus = si_gpu_num_compute_units;

	struct cgm_packet_t *message_packet;
	struct cgm_packet_status_t *mshr_packet;

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

		//printf("retry type %s access id %llu at %llu\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), access_id, P_TIME);

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
