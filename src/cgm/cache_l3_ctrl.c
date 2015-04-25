/*
 * cache_l3_ctrl.c
 *
 *  Created on: Apr 22, 2015
 *      Author: stardica
 */

#include <cgm/cache.h>
#include <cgm/switch.h>
#include <arch/x86/timing/cpu.h>


void l3_cache_access_gets_i(struct cache_t *cache, struct cgm_packet_t *message_packet){

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
	cache->loads++;

	/*if (access_type == cgm_access_retry_i)
		l3_caches[my_pid].retries++;*/


	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);



	CGM_DEBUG(cache_debug_file,"l3_cache[%d] access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->id, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//printf("made it here with id %d\n", cache->id);
	//STOP;

	//charge the cycle for the look up.
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(1);

	/////////testing
	cgm_cache_set_block(cache, *set_ptr, *way_ptr, tag, cache_block_shared);
	/////////testing

	//L3 Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{



		CGM_DEBUG(cache_debug_file, "\tl3_cache[%d] access_id %llu cycle %llu hit\n", cache->id, access_id, P_TIME);

		cache->hits++;

		//This is a hit in the L3 cache, send up to L2 cache

		//while the next level of cache's in queue is full stall
		//star todo possible deadlock situation if both the l2 and core are trying to fill a full queue
		while(!switch_can_access(switches[cache->id].south_queue))
		{
			P_PAUSE(1);
		}

		CGM_DEBUG(cache_debug_file, "\tl3_cache[%d] access_id %llu cycle %llu miss switch south queue free\n", cache->id, access_id, P_TIME);

		//success
		//remove packet from l3 cache in queue
		message_packet->access_type = cgm_access_puts;
		message_packet->dest_name = message_packet->src_name;
		message_packet->dest_id = str_map_string(&node_strn_map, message_packet->src_name);
		message_packet->src_name = cache->name;
		message_packet->source_id = str_map_string(&node_strn_map, cache->name);

		list_remove(cache->last_queue, message_packet);
		list_enqueue(switches[cache->id].south_queue, message_packet);
		future_advance(&switches_ec[cache->id], WIRE_DELAY(switches[cache->id].wire_latency));
		//done

	}
	//L3 Cache Miss!
	else if(cache_status == 0 || *state_ptr == 0)
	{

		assert(message_packet->access_type != cgm_access_retry);

		cache->misses++;

		CGM_DEBUG(cache_debug_file, "\tl3_cache[%d] access_id %llu cycle %llu miss\n", cache->id, access_id, P_TIME);

		miss_status_packet = miss_status_packet_create(message_packet->access_id, message_packet->access_type, set, tag, offset, str_map_string(&node_strn_map, cache->name));
		mshr_status = mshr_set(cache, miss_status_packet, message_packet);

		CGM_DEBUG(cache_debug_file, "\tl3_cache[%d] access_id %llu cycle %llu miss mshr status %d\n", cache->id, access_id, P_TIME, mshr_status);

		if(mshr_status == 1)
		{
			//access is unique in the MSHR
			//while the next level's queue is full stall
			while(!switch_can_access(switches[cache->id].south_queue))
			{
				P_PAUSE(1);
			}

			CGM_DEBUG(cache_debug_file, "\tl3_cache[%d] access_id %llu cycle %llu miss switch south queue free\n", cache->id, access_id, P_TIME);

			//star todo send to correct l3 dest
			message_packet->access_type = cgm_access_gets_i;
			message_packet->src_name = cache->name;
			message_packet->source_id = str_map_string(&node_strn_map, cache->name);
			message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
			message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

			//success
			list_remove(cache->last_queue, message_packet);
			list_enqueue(switches[cache->id].south_queue, message_packet);

			future_advance(&switches_ec[cache->id], WIRE_DELAY(switches[cache->id].wire_latency));

			CGM_DEBUG(cache_debug_file, "\tl3_cache[%d] access_id %llu cycle %llu l3_cache[%d] -> %s\n",
				cache->id, access_id, P_TIME, cache->id, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu l1_i_cache[%d] Miss\tSEND l3_cache[%d] -> %s\n",
				access_id, P_TIME, cache->id, cache->id, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			//advance the L2 cache adding some wire delay time.
			future_advance(&l2_cache[cache->id], WIRE_DELAY(l2_caches[cache->id].wire_latency));
		}
		else if(mshr_status == 0)
		{
			//mshr is full so we can't progress, retry.
			fatal("l3_cache_access_load(): MSHR full\n");

			message_packet->access_type = cgm_access_gets_i;
			list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->retry_queue, message_packet);
			future_advance(&l3_cache[cache->id], (etime.count + 4));
		}
		else
		{
			//access was coalesced. For now do nothing until later.
		}
		//done
	}

	return;
}


void l3_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet){

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

	CGM_DEBUG(cache_debug_file,"l3_cache[%d] access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
		cache->id, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//look up, and charge a cycle.
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(2);


	// L3 Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{
		CGM_DEBUG(cache_debug_file, "\tl3_cache[%d] access_id %llu cycle %llu hit\n", cache->id, access_id, P_TIME);

		cache->hits++;

		//This is a hit in the L3 cache, send up to L2 cache

		//while the next level of cache's in queue is full stall
		//star todo possible deadlock situation if both the l2 and core are trying to fill a full queue
		while(!switch_can_access(switches[cache->id].south_queue))
		{
			P_PAUSE(1);
		}

		CGM_DEBUG(cache_debug_file, "\tl3_cache[%d] access_id %llu cycle %llu miss switch south queue free\n", cache->id, access_id, P_TIME);

		//success
		//remove packet from l3 cache in queue
		message_packet->access_type = cgm_access_puts;
		message_packet->dest_name = message_packet->src_name;
		message_packet->dest_id = str_map_string(&node_strn_map, message_packet->src_name);
		message_packet->src_name = cache->name;
		message_packet->source_id = str_map_string(&node_strn_map, cache->name);

		list_remove(cache->last_queue, message_packet);
		list_enqueue(switches[cache->id].south_queue, message_packet);
		future_advance(&switches_ec[cache->id], WIRE_DELAY(switches[cache->id].wire_latency));
		//done

	}
	else
	{
		fatal("l2_cache_access_retry(): miss on retry\n");
	}

return;
}


void l3_cache_access_puts(struct cache_t *cache, struct cgm_packet_t *message_packet){

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

	CGM_DEBUG(cache_debug_file, "l3_cache[%d] access_id %llu cycle %llu puts\n", cache->id, access_id, P_TIME);

	//charge the delay for writing cache block
	cgm_cache_set_block(cache, *set_ptr, *way_ptr, tag, cache_block_shared);
	P_PAUSE(1);

	//get the mshr status
	mshr_status = mshr_get(cache, set_ptr, tag_ptr);
	assert(mshr_status != -1);

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
			advance(&l3_cache[cache->id]);
		}
		else if( i > 0)
		{
			miss_status_packet = list_remove_at(cache->mshrs[mshr_status].entires, i);
			list_enqueue(cache->retry_queue, miss_status_packet->coalesced_packet);
			free(miss_status_packet);
			advance(&l3_cache[cache->id]);
		}
	}

	mshr_clear(&(cache->mshrs[mshr_status]));

	return;
}



void l3_cache_ctrl(void){

	int my_pid = l3_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;
	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

	/*
	struct cgm_packet_status_t *mshr_packet;

	unsigned int addr = 0;

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

	int retry = 0;
	int *retry_ptr = &retry;*/

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);


	while(1)
	{
		/*wait here until there is a job to do.*/
		await(&l3_cache[my_pid], step);
		step++;


		//printf("l3_cache[%d] %d\n", my_pid);
		//printf("size l3_caches[%d].retry_queue %d\n",  my_pid, list_count(l3_caches[my_pid].retry_queue));
		//printf("size l3_caches[%d]. top %d\n",  my_pid, list_count(l3_caches[my_pid].Rx_queue_top));
		//printf("size l3_caches[%d]. bottom %d\n", my_pid, list_count(l3_caches[my_pid].Rx_queue_bottom));

		//get the message out of the queue
		message_packet = get_message(&(l3_caches[my_pid]));

		//printf("made it here\n");

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;

		if (access_type == cgm_access_gets_i)
		{
			l3_cache_access_gets_i(&l3_caches[my_pid], message_packet);
		}
		else if (access_type == cgm_access_retry)
		{
			l3_cache_access_retry(&l3_caches[my_pid], message_packet);
		}
		else if (access_type == cgm_access_puts)
		{
			l3_cache_access_puts(&l3_caches[my_pid], message_packet);
		}
		else
		{
			fatal("l3_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
				access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
		}

	}
	/* should never get here*/
	fatal("l3_cache_ctrl task is broken\n");
	return;
}
