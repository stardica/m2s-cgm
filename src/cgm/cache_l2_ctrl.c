/*
 * cache_l2_ctrl.c
 *
 *  Created on: Apr 22, 2015
 *      Author: stardica
 */

#include <stdio.h>
#include <assert.h>

#include <arch/x86/timing/cpu.h>
#include <lib/util/debug.h>

#include <cgm/cache.h>
#include <cgm/cgm.h>
#include <cgm/switch.h>
//#include <cgm/protocol.h>
#include <cgm/tasking.h>



void l2_cache_access_gets_i(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//int num_cores = x86_cpu_num_cores;
	//struct cgm_packet_t *message_packet;
	struct cgm_packet_status_t *miss_status_packet;

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

	int mshr_status = 0;
	int retry = 0;
	int *retry_ptr = &retry;

	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;

	//stats
	/*if(access_type == cgm_access_gets_i)*/
	cache->loads++;

	/*if(access_type == cgm_access_retry)
		cache->retries++;*/


	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(cache_debug_file,"l2_cache[%d] access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->id, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//////testing
	//cgm_cache_set_block(cache, *set_ptr, *way_ptr, tag, cache_block_shared);
	//////testing

	//look up, and charge a cycle.
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(2);


	// L2 Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{

		CGM_DEBUG(cache_debug_file, "l2_cache[%d] access_id %llu cycle %llu hit\n", cache->id, access_id, P_TIME);

		/*if(access_type == cgm_access_retry_i)
			cache->retries++;*/

		/*if(access_type == cgm_access_gets_i)*/
		cache->hits++;

		//while the next level of cache's in queue is full stall
		while(!cache_can_access_bottom(&l1_i_caches[cache->id]))
		{
			P_PAUSE(1);
		}

		//success, remove packet from l2 cache in queue
		list_remove(cache->last_queue, message_packet);

		//change access type, i cache only ever reads so puts is ok.
		message_packet->access_type = cgm_access_puts;
		list_enqueue(l1_i_caches[cache->id].Rx_queue_bottom, message_packet);
		future_advance(&l1_i_cache[cache->id], WIRE_DELAY(l1_i_caches[cache->id].wire_latency));

		CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu l2_cache[%d] Hit SEND %s to l1_i_cache[%d]\n",
			access_id, P_TIME, cache->id, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), cache->id);

	}
	// L2 Cache Miss!
	else if(cache_status == 0 || *state_ptr == 0)
	{

		assert(message_packet->access_type != cgm_access_retry);

		/*if(access_type == cgm_access_gets_i)*/
		cache->misses++;

		CGM_DEBUG(cache_debug_file, "l2_cache[%d] access_id %llu cycle %llu miss\n", cache->id, access_id, P_TIME);

		miss_status_packet = miss_status_packet_create(message_packet->access_id, message_packet->access_type, set, tag, offset, str_map_string(&node_strn_map, cache->name));
		mshr_status = mshr_set(cache, miss_status_packet, message_packet);

		CGM_DEBUG(cache_debug_file, "l2_cache[%d] access_id %llu cycle %llu miss mshr status %d\n", cache->id, access_id, P_TIME, mshr_status);

		if(mshr_status == 1)
		{
			//access is unique in the MSHR
			//while the next level's queue is full stall
			while(!switch_can_access(switches[cache->id].north_queue))
			{
				P_PAUSE(1);
			}

			CGM_DEBUG(cache_debug_file, "l2_cache[%d] access_id %llu cycle %llu miss switch north queue free size %d\n", cache->id, access_id, P_TIME, list_count(switches[cache->id].north_queue));

			//send to L3 cache over switching network add source and dest here
			//star todo send to correct l3 dest
			message_packet->access_type = cgm_access_gets_i;
			message_packet->src_name = cache->name;
			message_packet->source_id = str_map_string(&node_strn_map, cache->name);
			message_packet->dest_name = l3_caches[cache->id].name;
			message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[cache->id].name);

			//success

			list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(cache_debug_file, "l2_cache[%d] access_id %llu cycle %llu removed from %s size %d\n",
					cache->id, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
			list_enqueue(switches[cache->id].north_queue, message_packet);

			future_advance(&switches_ec[cache->id], WIRE_DELAY(switches[cache->id].wire_latency));

			CGM_DEBUG(cache_debug_file, "l2_cache[%d] access_id %llu cycle %llu l2_cache[%d] as %s\n",
				cache->id, access_id, P_TIME, cache->id, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu l1_i_cache[%d] Miss\tSEND l2_cache[%d] -> %s\n",
				access_id, P_TIME, cache->id, cache->id, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

		}
		else if(mshr_status == 0)
		{
			//mshr is full so we can't progress, retry.
			fatal("l2_cache_access_load(): MSHR full\n");

			message_packet->access_type = cgm_access_gets_i;
			list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->retry_queue, message_packet);
			future_advance(&l2_cache[cache->id], (etime.count + 4));

		}
		else
		{
			//access was coalesced. For now do nothing until later.
		}
		//done

	}
	return;
}

void l2_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//int num_cores = x86_cpu_num_cores;
	//struct cgm_packet_t *message_packet;
	//struct cgm_packet_status_t *mshr_packet;

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

	//int mshr_status = 0;
	//int retry = 0;
	//int *retry_ptr = &retry;

	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;

	//stats
	cache->retries++;

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(cache_debug_file,"l2_cache[%d] access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
		cache->id, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//look up, and charge a cycle.
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(2);


	// L2 Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{

		CGM_DEBUG(cache_debug_file, "l2_cache[%d] access_id %llu cycle %llu retry hit\n", cache->id, access_id, P_TIME);

		//while the next level of cache's in queue is full stall
		while(!cache_can_access_bottom(&l1_i_caches[cache->id]))
		{
			P_PAUSE(1);
		}

		//change access type, i cache only ever reads so puts is ok.
		message_packet->access_type = cgm_access_puts;
		//success, remove packet from l2 cache in queue
		list_remove(cache->last_queue, message_packet);
		CGM_DEBUG(cache_debug_file, "l2_cache[%d] access_id %llu cycle %llu removed from %s size %d\n",
				cache->id, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
		list_enqueue(l1_i_caches[cache->id].Rx_queue_bottom, message_packet);
		future_advance(&l1_i_cache[cache->id], WIRE_DELAY(l1_i_caches[cache->id].wire_latency));
	}
	else
	{
		fatal("l2_cache_access_retry(): miss on retry\n");
	}
	return;
}

void l2_cache_access_puts(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//int num_cores = x86_cpu_num_cores;
	//struct cgm_packet_t *message_packet;
	struct cgm_packet_status_t *miss_status_packet;

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

	int mshr_status = 0;
	int retry = 0;
	int *retry_ptr = &retry;

	int i = 0;

	//the packet is from the L2 cache
	access_type = message_packet->access_type;
	addr = message_packet->address;
	access_id = message_packet->access_id;

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(cache_debug_file, "l2_cache[%d] access_id %llu cycle %llu puts\n", cache->id, access_id, P_TIME);

	//charge the delay for writing cache block
	cgm_cache_set_block(cache, *set_ptr, *way_ptr, tag, cache_block_shared);
	P_PAUSE(1);

	//get the mshr status
	mshr_status = mshr_get(cache, set_ptr, tag_ptr);
	if(mshr_status == -1)
	{
		//STOP;
		fatal("L2 mshr_status == -1\n");
	}

	if(mshr_status >= 0)
	{
		/*we have outstanding mshr requests so set the retry state bit*/
		*retry_ptr = cache->mshrs[mshr_status].num_entries;
		assert(*retry_ptr > 0);
	}

	//move the access and any coalesced accesses to the retry queue.
	for(i = 0; i < *retry_ptr; i++)
	{
		if( i == 0)
		{
			//move current message_packet to retry queue
			message_packet->access_type = cgm_access_retry;
			list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->retry_queue, message_packet);
			advance(&l2_cache[cache->id]);
		}
		else if( i > 0)
		{
			miss_status_packet = list_remove_at(cache->mshrs[mshr_status].entires, i);
			miss_status_packet->coalesced_packet->access_type = cgm_access_retry;
			list_enqueue(cache->retry_queue, miss_status_packet->coalesced_packet);
			free(miss_status_packet);
			advance(&l2_cache[cache->id]);
		}
	}

	mshr_clear(&(cache->mshrs[mshr_status]));

	return;
}

void l2_cache_access_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*else if (access_type == cgm_access_load)
			{
				//stats
				l2_caches[my_pid].loads++;

				cache_status = cgm_cache_find_block(&(l2_caches[my_pid]), tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);

				// L2 Cache Hit!
				if(cache_status == 1)
				{
					//stats
					l2_caches[my_pid].hits++;

					//This is a hit in the L2 cache need to send up to L1 cache
					//remove packet from l2 cache in queue
					message_packet->access_type = cgm_access_l2_load_reply;

					list_remove(l2_caches[my_pid].Rx_queue_top, message_packet);
					list_enqueue(l1_d_caches[my_pid].Rx_queue_top, message_packet);
					//cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);

					future_advance(l1_d_cache, (etime.count + l1_d_caches[my_pid].wire_latency));
				}
				// L2 Cache Miss!
				else if(cache_status == 0)
				{
					//stats
					l2_caches[my_pid].misses++;
					//for now pretend that it is the last level of cache and memory ctrl.
					P_PAUSE(mem_miss);

					message_packet->access_type = cgm_access_l2_load_reply;

					cgm_cache_set_block(&(l2_caches[my_pid]), *set_ptr, *way_ptr, tag, 4);

					list_remove(l2_caches[my_pid].Rx_queue_top, message_packet);
					list_enqueue(l1_d_caches[my_pid].Rx_queue_top, message_packet);

					future_advance(l1_d_cache, (etime.count + l1_d_caches[my_pid].wire_latency));
				}

			}*/
	return;
}

void l2_cache_access_store(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*else if (access_type == cgm_access_store)
	{
		//stats
		l2_caches[my_pid].stores++;
		cache_status = cgm_cache_find_block(&(l2_caches[my_pid]), tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);

		// L2 Cache Hit!
		if(cache_status == 1)
		{
			//stats
			l2_caches[my_pid].hits++;

			cgm_cache_set_block(&(l2_caches[my_pid]), *set_ptr, *way_ptr, tag, 4);
					//This is a hit in the L2 cache need to send up to L1 cache
					//remove packet from l2 cache in queue
					message_packet->access_type = cgm_access_l2_store_reply;
					list_remove(l2_caches[my_pid].Rx_queue_top, message_packet);
					list_enqueue(l1_d_caches[my_pid].Rx_queue_top, message_packet);
					//cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);

					future_advance(l1_d_cache, (etime.count + l1_d_caches[my_pid].wire_latency));
				}
				// L2 Cache Miss!
				else if(cache_status == 0)
				{
					//stats
					l2_caches[my_pid].misses++;

					//for now pretend that it is the last level of cache and memory ctrl.
					P_PAUSE(mem_miss);

					message_packet->access_type = cgm_access_l2_store_reply;

					cgm_cache_set_block(&(l2_caches[my_pid]), *set_ptr, *way_ptr, tag, 4);

					list_remove(l2_caches[my_pid].Rx_queue_top, message_packet);
					list_enqueue(l1_d_caches[my_pid].Rx_queue_top, message_packet);

					future_advance(l1_d_cache, (etime.count + l1_d_caches[my_pid].wire_latency));
				}
			}*/
		//}


	return;
}


void l2_cache_ctrl(void){

	int my_pid = l2_pid++;
	long long step = 1;

	int num_cores = x86_cpu_num_cores;
	struct cgm_packet_t *message_packet;
	//struct cgm_packet_status_t *mshr_packet;

	enum cgm_access_kind_t access_type;
	//unsigned int addr = 0;
	long long access_id = 0;
	//int set = 0;
	//int tag = 0;
	//unsigned int offset = 0;
	//int way = 0;
	//int state = 0;
	//int cache_status;

	//int *set_ptr = &set;
	//int *tag_ptr = &tag;
	//unsigned int *offset_ptr = &offset;
	//int *way_ptr = &way;
	//int *state_ptr = &state;

	//int mshr_status = 0;
	int retry = 0;
	int *retry_ptr = &retry;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{

		/*wait here until there is a job to do.*/
		await(&l2_cache[my_pid], step);
		step++;

		//check the top or bottom rx queues for messages.
		message_packet = cache_get_message(&(l2_caches[my_pid]));

		access_type = message_packet->access_type;

		if(access_type == cgm_access_gets_i)
		{
			l2_cache_access_gets_i(&l2_caches[my_pid], message_packet);
		}
		else if (access_type == cgm_access_retry)
		{
			l2_cache_access_retry(&l2_caches[my_pid], message_packet);
		}
		else if(access_type == cgm_access_puts)
		{
			l2_cache_access_puts(&l2_caches[my_pid], message_packet);
		}
		else if(access_type == cgm_access_load)
		{
			l2_cache_access_load(&l2_caches[my_pid], message_packet);
		}
		else if (access_type == cgm_access_store)
		{
			l2_cache_access_store(&l2_caches[my_pid], message_packet);
		}
		else
		{
			fatal("l2_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
				access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
		}
	}

	/* should never get here*/
	fatal("l2_cache_ctrl task is broken\n");
	return;
}
