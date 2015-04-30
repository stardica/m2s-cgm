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



void l2_cache_access_gets(struct cache_t *cache, struct cgm_packet_t *message_packet){

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

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(CPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//////testing
	//cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_shared);
	//////testing

	//look up, and charge a cycle.
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(2);

	// L2 Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

		cache->hits++;

		assert(*state_ptr != cache_block_invalid);


		if(*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
		{

			//success, remove packet from l2 cache in queue
			list_remove(cache->last_queue, message_packet);

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));


			//send to correct l1 cache and change access type
			if (message_packet->access_type == cgm_access_gets_i)
			{
				//while the next level of cache's in queue is full stall
				while(!cache_can_access_bottom(&l1_i_caches[cache->id]))
				{
					P_PAUSE(1);
				}

				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu L1 bottom queue free size %d\n",
						cache->name, access_id, P_TIME, list_count(l1_i_caches[cache->id].Rx_queue_bottom));

				message_packet->access_type = cgm_access_puts;
				list_enqueue(l1_i_caches[cache->id].Rx_queue_bottom, message_packet);
				future_advance(&l1_i_cache[cache->id], WIRE_DELAY(l1_i_caches[cache->id].wire_latency));

			}
			else if (message_packet->access_type == cgm_access_gets_d)
			{
				//while the next level of cache's in queue is full stall
				while(!cache_can_access_bottom(&l1_d_caches[cache->id]))
				{
					P_PAUSE(1);
				}

				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu %s free size %d\n",
						cache->name, access_id, P_TIME, l1_d_caches[cache->id].Rx_queue_bottom->name, list_count(l1_d_caches[cache->id].Rx_queue_bottom));

				message_packet->access_type = cgm_access_puts;
				list_enqueue(l1_d_caches[cache->id].Rx_queue_bottom, message_packet);
				future_advance(&l1_d_cache[cache->id], WIRE_DELAY(l1_d_caches[cache->id].wire_latency));
			}
			else
			{
				fatal("l2_cache_access_gets(): %s access_id %llu cycle %llu incorrect access type\n", cache->name, access_id, P_TIME);
			}
		}
		else
		{
			fatal("l1_d_cache_access_load(): incorrect block state set");
		}

		CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu %s Hit SEND %s to %s\n",
			access_id, P_TIME, cache->name, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), l1_i_caches[cache->id].name);

	}
	// L2 Cache Miss!
	else if(cache_status == 0 || *state_ptr == 0)
	{

		cache->misses++;

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		miss_status_packet = miss_status_packet_copy(message_packet, set, tag, offset, str_map_string(&node_strn_map, cache->name));
		mshr_status = mshr_set(cache, miss_status_packet);


		/*printf("%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, mshr_status);
		mshr_dump(cache);
		getchar();*/

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, mshr_status);

		if(mshr_status == 2)
		{
			//access was coalesced
			//remove the message packet on coalesce, but dont send to L2
			list_remove(cache->last_queue, message_packet);


			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu coalesced packet removed removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
		}
		else if(mshr_status == 1)
		{
			//access is unique in the MSHR so send forward
			//while the next level of cache's in queue is full stall
			while(!switch_can_access(switches[cache->id].north_queue))
			{
				P_PAUSE(1);
			}

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss switch north queue free size %d\n",
					cache->name, access_id, P_TIME, list_count(switches[cache->id].north_queue));

			//printf(" l2 miss type %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			message_packet->l1_access_type = message_packet->access_type;
			message_packet->access_type = cgm_access_gets;
			message_packet->l2_cache_id = cache->id;
			message_packet->src_name = cache->name;
			message_packet->src_id = str_map_string(&node_strn_map, cache->name);
			message_packet->dest_name = l3_caches[cache->id].name;
			message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[cache->id].name);

			list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			list_enqueue(switches[cache->id].north_queue, message_packet);

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu l3_cache[%d] send %s\n",
					cache->name, access_id, P_TIME, cache->id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu %s Miss SEND l2_cache[%d] %s\n",
					access_id, P_TIME, cache->name, l3_caches[cache->id].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			//advance the L2 cache adding some wire delay time.
			future_advance(&switches_ec[cache->id], WIRE_DELAY(switches[cache->id].wire_latency));
		}
		else //mshr == 0 || -1
		{
			//mshr is full so we can't progress, retry.

			printf("breaking MSHR full\n");
			mshr_dump(cache);
			STOP;

			fatal("l2_cache_access_load(): MSHR full\n");
		}

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

	CGM_DEBUG(CPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
		cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//look up, and charge a cycle.
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(2);


	// L2 Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu retry hit\n", cache->name, access_id, P_TIME);



		if(*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared)
		{

			list_remove(cache->retry_queue, message_packet);
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
				cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			//send to correct l1 cache and change access type
			if (message_packet->l1_access_type == cgm_access_gets_i)
			{
				//while the next level of cache's in queue is full stall
				while(!cache_can_access_bottom(&l1_i_caches[cache->id]))
				{
					P_PAUSE(1);
				}

					CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu L1 bottom queue free size %d\n",
							cache->name, access_id, P_TIME, list_count(l1_i_caches[cache->id].Rx_queue_bottom));

					message_packet->access_type = cgm_access_puts;
					list_enqueue(l1_i_caches[cache->id].Rx_queue_bottom, message_packet);
					future_advance(&l1_i_cache[cache->id], WIRE_DELAY(l1_i_caches[cache->id].wire_latency));

			}
			else if (message_packet->l1_access_type == cgm_access_gets_d)
			{
				//while the next level of cache's in queue is full stall
				while(!cache_can_access_bottom(&l1_d_caches[cache->id]))
				{
					P_PAUSE(1);
				}

				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu %s free size %d\n",
					cache->name, access_id, P_TIME, l1_d_caches[cache->id].Rx_queue_bottom->name, list_count(l1_d_caches[cache->id].Rx_queue_bottom));

				message_packet->access_type = cgm_access_puts;
				list_enqueue(l1_d_caches[cache->id].Rx_queue_bottom, message_packet);
				future_advance(&l1_d_cache[cache->id], WIRE_DELAY(l1_d_caches[cache->id].wire_latency));
			}
			else
			{
				fatal("l2_cache_access_gets(): %s access_id %llu cycle %llu incorrect access type\n", cache->name, access_id, P_TIME);
			}
		}
		else
		{
			fatal("l1_d_cache_access_load(): incorrect block state set");
		}
	}


		/*old code
		//while the next level of cache's in queue is full stall
		while(!cache_can_access_bottom(&l1_i_caches[cache->id]))
		{
			P_PAUSE(1);
		}

		//change access type, i cache only ever reads so puts is ok.
		message_packet->access_type = cgm_access_puts;

		//success, remove packet from l2 cache in queue
		list_remove(cache->last_queue, message_packet);

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
				cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

		list_enqueue(l1_i_caches[cache->id].Rx_queue_bottom, message_packet);
		future_advance(&l1_i_cache[cache->id], WIRE_DELAY(l1_i_caches[cache->id].wire_latency));
	}
	else
	{
		fatal("l2_cache_access_retry(): miss on retry\n");
	}*/


	return;
}

void l2_cache_access_puts(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//int num_cores = x86_cpu_num_cores;
	//struct cgm_packet_t *message_packet;
	struct cgm_packet_t *miss_status_packet;

	enum cgm_access_kind_t access_type;
	unsigned int addr = 0;
	long long access_id = 0;
	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int state = 0;
	//int cache_status;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;

	int mshr_row = -1;
	/*int retry = 0;
	int *retry_ptr = &retry;*/

	int i = 0;

	//the packet is from the L2 cache
	access_type = message_packet->access_type;
	addr = message_packet->address;
	access_id = message_packet->access_id;

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu puts\n", cache->name, access_id, P_TIME);

	//charge the delay for writing cache block
	cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_shared);
	P_PAUSE(1);

	//get the mshr status
	mshr_row = mshr_get(cache, set_ptr, tag_ptr, access_id);
	assert(mshr_row != -1);

	//printf("mshr_row %d\n", mshr_row);

	//check the number of entries in the mshr row
	assert(list_count(cache->mshrs[mshr_row].entires) == cache->mshrs[mshr_row].num_entries);
	assert(cache->mshrs[mshr_row].num_entries > 0);

	for(i = 0; i < cache->mshrs[mshr_row].num_entries; i++)
	{
		//printf("i %d entry size %d\n", i, entry_size)
		miss_status_packet = list_dequeue(cache->mshrs[mshr_row].entires);

		assert(miss_status_packet != NULL);

		if (miss_status_packet->access_id == access_id)
		{
			//this is the first entry and was not coalesced
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
			assert(miss_status_packet->coalesced == 1);

			//drop it into the retry queue
			list_enqueue(cache->retry_queue, miss_status_packet);

			/*printf("miss_status_packet->access_id %llu access_id %llu\n", miss_status_packet->access_id, access_id );*/
			//printf("miss_status_packet->coalesced_packet->name %s\n", miss_status_packet->coalesced_packet->name);
		}
	}

	long long time = etime.count; //:-P

	//advance the cache by the number of packets
	for(i = 0; i < cache->mshrs[mshr_row].num_entries; i ++)
	{
		time += 2;
		future_advance(&l2_cache[cache->id], time);
	}

	//clear the mshr row for future use
	mshr_clear(&(cache->mshrs[mshr_row]));


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

		if(access_type == cgm_access_gets_i || access_type == cgm_access_gets_d)
		{
			l2_cache_access_gets(&l2_caches[my_pid], message_packet);
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
