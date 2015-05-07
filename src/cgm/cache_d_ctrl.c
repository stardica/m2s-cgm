/*
 * cache_d_ctrl.c
 *
 *  Created on: Apr 22, 2015
 *      Author: stardica
 */

#include <cgm/cache.h>
#include <cgm/switch.h>
#include <cgm/protocol.h>
#include <arch/x86/timing/cpu.h>

#include <lib/util/linked-list.h>
#include <lib/util/list.h>

void l1_d_cache_access_store(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//enum cgm_access_kind_t access_type;
	//long long access_id = 0;

	//access_type = message_packet->access_type;
	//access_id = message_packet->access_id;
	//addr = message_packet->address;


	//CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu %s\n", cache->name, access_id, P_TIME, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

	/////////testing
	//list_remove(cache->Rx_queue_top, message_packet);
	//linked_list_add(message_packet->event_queue, message_packet->data);
	/////////testing

	//return;

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


	//////testing
	//cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_shared);
	//////testing


	CGM_DEBUG(CPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);


	//get the block and the state of the block and charge cycles
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(cache->latency);


	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)// && *state_ptr != cache_block_shared)
	{
		/*cache_block_invalid = 0,
		cache_block_noncoherent,
		cache_block_modified,
		cache_block_owned,
		cache_block_exclusive,
		cache_block_shared*/

		//check state of the block
		//block is valid

		assert(*state_ptr != cache_block_invalid);

		//star todo this is wrong
		if(*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared)
		{
			cache->hits++;


			//star todo change this to work as a message sent to the directory
			//also need to send invalidataions out.
			/*if(*state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared)
			{
				cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_modified);
			}*/

			//here we would write the data into the block if we had the correct access.

			list_remove(cache->last_queue, message_packet);
			linked_list_add(message_packet->event_queue, message_packet->data);
		}
		else
		{
			fatal("l1_d_cache_access_store(): incorrect block state set");
		}
		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu hit state %s\n", cache->name, access_id, P_TIME, str_map_value(&cache_block_state_map, *state_ptr));
	}

	//Cache Miss!
	if(cache_status == 0 || *state_ptr == 0)
	{
		//on both a miss and invalid hit the state_ptr should be zero

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		message_packet->cpu_access_type = cgm_access_store;
		message_packet->access_type = cgm_access_gets_d;
		message_packet->l1_access_type = cgm_access_gets_d;
		miss_status_packet = miss_status_packet_copy(message_packet, *set_ptr, *tag_ptr, *offset_ptr, str_map_string(&l1_strn_map, cache->name));
		mshr_status = mshr_set(cache, miss_status_packet);

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, mshr_status);

		if(mshr_status == 2)
		{
			//access was coalesced
			//remove the message packet on coalesce, but don't send to next cache
			list_remove(cache->last_queue, message_packet);

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu coalesced packet removed removed from %s size %d\n",
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

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu l2 queue free size %d\n",
				cache->name, access_id, P_TIME, list_count(l2_caches[cache->id].Rx_queue_top));

			/*change the access type for the coherence protocol and drop into the L2's queue
			remove the access from the l1 cache queue and place it in the l2 cache ctrl queue*/

			list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
				cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			list_enqueue(l2_caches[cache->id].Rx_queue_top, message_packet);

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu %s as %s\n",
				cache->name, access_id, P_TIME, l2_caches[cache->id].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));


			CGM_DEBUG(protocol_debug_file, "%s Access_id %llu cycle %llu %s miss SEND %s %s\n",
				cache->name, access_id, P_TIME, cache->name, l2_caches[cache->id].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			//advance the L2 cache adding some wire delay time.
			future_advance(&l2_cache[cache->id], WIRE_DELAY(l2_caches[cache->id].wire_latency));
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

	return;
}


void l1_d_cache_access_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

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

	//store the decode for later
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	//////testing
	//cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_shared);
	//////testing


	CGM_DEBUG(CPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//get the block and the state of the block and charge cycles
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(cache->latency);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{
		/*cache_block_invalid = 0,
		cache_block_noncoherent,
		cache_block_modified,
		cache_block_owned,
		cache_block_exclusive,
		cache_block_shared*/

		//check state of the block
		//block is valid

		assert(*state_ptr != cache_block_invalid);

		if(*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared)
		{
			cache->hits++;

			list_remove(cache->last_queue, message_packet);
			linked_list_add(message_packet->event_queue, message_packet->data);

		}
		else
		{
			fatal("l1_d_cache_access_load(): incorrect block state set");
		}

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu hit state %s\n", cache->name, access_id, P_TIME, str_map_value(&cache_block_state_map, *state_ptr));
	}

	//Cache Miss!
	if(cache_status == 0 || *state_ptr == 0)
	{
		//on both a miss and invalid hit the state_ptr should be zero

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		message_packet->cpu_access_type = cgm_access_load;
		message_packet->access_type = cgm_access_gets_d;
		message_packet->l1_access_type = cgm_access_gets_d;

		miss_status_packet = miss_status_packet_copy(message_packet, *set_ptr, *tag_ptr, *offset_ptr, str_map_string(&l1_strn_map, cache->name));
		mshr_status = mshr_set(cache, miss_status_packet);

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, mshr_status);

		if(mshr_status == 2)
		{
			//access was coalesced
			//remove the message packet on coalesce, but don't send to next cache
			list_remove(cache->last_queue, message_packet);

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu coalesced packet removed removed from %s size %d\n",
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

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu l2 queue free size %d\n",
					cache->name, access_id, P_TIME, list_count(l2_caches[cache->id].Rx_queue_top));

			/*change the access type for the coherence protocol and drop into the L2's queue
			remove the access from the l1 cache queue and place it in the l2 cache ctrl queue*/

			//message_packet->cpu_access_type = cgm_access_store;
			//message_packet->access_type = cgm_access_gets_d;
			list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			list_enqueue(l2_caches[cache->id].Rx_queue_top, message_packet);

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu %s as %s\n",
					cache->name, access_id, P_TIME, l2_caches[cache->id].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));


			CGM_DEBUG(protocol_debug_file, "%s Access_id %llu cycle %llu %s miss SEND %s %s\n",
					cache->name, access_id, P_TIME, cache->name, l2_caches[cache->id].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			//advance the L2 cache adding some wire delay time.
			future_advance(&l2_cache[cache->id], WIRE_DELAY(l2_caches[cache->id].wire_latency));
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

	return;
}

void l1_d_cache_access_puts(struct cache_t *cache, struct cgm_packet_t *message_packet){

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


	//printf(" l1 D puts name %s\n", message_packet->name);

	//the packet is from the L2 cache
	access_type = message_packet->access_type;
	addr = message_packet->address;
	access_id = message_packet->access_id;

	//probe the address for set, tag, and offset.
	assert(addr != 0);
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu puts\n", cache->name, access_id, P_TIME);

	//charge the delay for writing cache block

	//star todo insert a function to set the correct block state here
	cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_shared);
	P_PAUSE(cache->latency);

	//get the mshr status
	mshr_row = mshr_get(cache, set_ptr, tag_ptr, access_id);
	assert(mshr_row != -1);

	//check the number of entries in the mshr row
	assert(list_count(cache->mshrs[mshr_row].entires) == cache->mshrs[mshr_row].num_entries);
	assert(cache->mshrs[mshr_row].num_entries > 0);

	CGM_DEBUG(mshr_debug_file, "%s access_id %llu cycle %llu mshr_row %d num_entries %d\n",
			cache->name, access_id, P_TIME, mshr_row, cache->mshrs[mshr_row].num_entries);

	//move them to the retry queueS
	for(i = 0; i < cache->mshrs[mshr_row].num_entries; i++)
	{

		miss_status_packet = list_dequeue(cache->mshrs[mshr_row].entires);

		//printf(" l1 D puts name %s access_id %llu\n", miss_status_packet->name, access_id);


		CGM_DEBUG(mshr_debug_file, "%s access_id %llu coalesced %d tag %d set %d\n",
				cache->name, miss_status_packet->access_id, miss_status_packet->coalesced, miss_status_packet->tag, miss_status_packet->set);

		assert(miss_status_packet != NULL);
		assert(miss_status_packet->address != 0);


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
				printf("Crashing access_id %llu cycle %llu\n", access_id, P_TIME);
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
		future_advance(&l1_d_cache[cache->id], time);
	}

	//clear the mshr row for future use
	mshr_clear(&(cache->mshrs[mshr_row]));


	return;
}


void l1_d_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet){

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

	CGM_DEBUG(CPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
		cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//get the block and the state of the block and charge a cycle
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	//P_PAUSE(2);

	//Cache Hit!
	if(message_packet->cpu_access_type == cgm_access_store)
	{
		if(cache_status == 1 && *state_ptr != 0)
		{
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

			//clear the access
			list_remove(cache->retry_queue, message_packet);
			linked_list_add(message_packet->event_queue, message_packet->data);
		}
		else
		{
			fatal("cache_access_retry(): %s miss on retry cycle %llu access_id %llu\n", cache->name, P_TIME, access_id);
		}
	}
	else
	{
		if(cache_status == 1 && *state_ptr != 0)
		{
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

			//clear the access
			list_remove(cache->retry_queue, message_packet);
			linked_list_add(message_packet->event_queue, message_packet->data);
		}
		else
		{
			fatal("cache_access_retry(): %s miss on retry cycle %llu access_id %llu\n", cache->name, P_TIME, access_id);
		}
	}

	return;
}


void l1_d_cache_ctrl(void){

	int my_pid = l1_d_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

	//struct cgm_packet_status_t *mshr_packet;

	//unsigned int addr = 0;

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

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{
		//wait here until there is a job to do.
		await(&l1_d_cache[my_pid], step);
		step++;

		//get the message out of the queue
		message_packet = cache_get_message(&(l1_d_caches[my_pid]));
		//message_packet = list_get(l1_d_caches[my_pid].Rx_queue_top, 0);
		//assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		//addr = message_packet->address;

		//probe the address for set, tag, and offset.
		//cgm_cache_decode_address(&(l1_d_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);

		/////////testing
		//list_remove(l1_d_caches[my_pid].Rx_queue_top, message_packet);
		//linked_list_add(message_packet->event_queue, message_packet->data);
		//continue;
		/////////testing

		//printf(" l1 D start name %s type %s\n", message_packet->name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));


		if (access_type == cgm_access_load)
		{
			l1_d_cache_access_load(&(l1_d_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_store)
		{
			l1_d_cache_access_store(&(l1_d_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_retry)
		{
			l1_d_cache_access_retry(&(l1_d_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_puts)
		{
			l1_d_cache_access_puts(&(l1_d_caches[my_pid]), message_packet);
		}
		else
		{
			fatal("l1_d_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
				access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
		}



		//service requests from CPU
		/*if (access_type == cgm_access_load)
		{
			cache_status = cache_mesi_load(&(l1_d_caches[my_pid]), cgm_access_load, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);

			//load hit with M, E, or S state.
			if(cache_status == 1)
			{
				//remove packet from cache queue and add to to commit stage input
				list_remove(l1_d_caches[my_pid].Rx_queue_top, message_packet);
				linked_list_add(message_packet->event_queue, message_packet->data);
				free(message_packet);
			}

			//load invalid hit or miss
			else if(cache_status == 2 || cache_status == 3)
			{


				//remove packet from cache queue and add to to commit stage input
				list_remove(l1_d_caches[my_pid].Rx_queue_top, message_packet);
				linked_list_add(message_packet->event_queue, message_packet->data);
				free(message_packet);

				mshr_packet = status_packet_create();

				//drop a token in the mshr queue
				//star todo add some detail to this so we can include coalescing
				mshr_packet->access_type = message_packet->access_type;
				mshr_packet->access_id = message_packet->access_id;
				mshr_packet->in_flight = message_packet->in_flight;
				list_enqueue(l1_d_caches[my_pid].mshr, mshr_packet);

				//Check if the cache queue is full if so leave the packet in the l1 cache and advance the l1 cache again.
				if(list_count(l2_caches[my_pid].Rx_queue_top) <= QueueSize)
				{
					//printf("in if\n");
					//remove the access from the l1 cache queue and place it in the l2 cache ctrl queue
					list_remove(l1_d_caches[my_pid].Rx_queue_top, message_packet);
					list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);

					//Advance the l2 cache controller
					l2_caches_data[my_pid]++;
					future_advance(l2_cache, (etime.count + l2_caches[my_pid].wire_latency));
				}
				else
				{
					//the l2 rx queue is full try again next cycle.
					l1_d_caches_data[my_pid]++;
					advance(l1_d_cache);
				}
			}
			else
			{
				fatal("l1_d_cache_ctrl() unexpected cache_status\n");
			}

		}
		else if (access_type == cgm_access_store)
		{

			//printf("Entered l1 d cache store\n");
			//getchar();
			//star todo evict old block this is where the LRU, FIFO stuff comes into play
			//this needs some work to get it right

			//stats
			l1_d_caches[my_pid].stores++;

			cache_status = cgm_cache_find_block(&(l1_d_caches[my_pid]), tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);

			// L1 D Cache Hit!
			if(cache_status == 1)
			{
				//ok, on a hit this means there is a block of old memory in the cache (i.e. to be over written).
				l1_d_caches[my_pid].hits++;

				//for now just set it so things will run
				cgm_cache_set_block(&(l1_i_caches[my_pid]), *set_ptr, *way_ptr, tag, 4);

				//remove packet from cache queue and add to commit stage input
				list_remove(l1_d_caches[my_pid].Rx_queue_top, message_packet);
				linked_list_add(message_packet->event_queue, message_packet->data);
				free(message_packet);

			}
			//L1 D Cache Miss!
			else if(cache_status == 0)
			{

				//remove packet from cache queue and add to commit stage input
				list_remove(l1_d_caches[my_pid].Rx_queue_top, message_packet);
				linked_list_add(message_packet->event_queue, message_packet->data);
				free(message_packet);

				l1_d_caches[my_pid].misses++;

				mshr_packet = status_packet_create();

				//drop a token in the mshr queue
				//star todo add some detail to this so we can include coalescing
				mshr_packet->access_type = message_packet->access_type;
				mshr_packet->access_id = message_packet->access_id;
				mshr_packet->in_flight = message_packet->in_flight;
				list_enqueue(l1_d_caches[my_pid].mshr, mshr_packet);

				//remove the access from the l1 cache queue and place it in the l2 cache ctrl queue
				list_remove(l1_d_caches[my_pid].Rx_queue_top, message_packet);
				list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);

				//Advance the l2 cache controller
				//4 clocks for wire delay.
				//advance(l2_cache_0);

				l2_caches_data[my_pid]++;
				future_advance(l2_cache, (etime.count + l2_caches[my_pid].wire_latency));
			}
		}*/

		//replies from L2
		/*else if(access_type == cgm_access_l2_load_reply)
		{
			//set the block in the L1 I cache
			cgm_cache_set_block(&(l1_d_caches[my_pid]), *set_ptr, *way_ptr, tag, 4);

			//service the mshr request
			mshr_remove(&(l1_d_caches[my_pid]), access_id);

			//remove the message from the in queue
			list_remove(l1_d_caches[my_pid].Rx_queue_top, message_packet);

			//dump in the done queue for the writeback stage, this is a simulator-ism.
			linked_list_add(message_packet->event_queue, message_packet->data);
		}
		else if(access_type == cgm_access_l2_store_reply)
		{
			//set the block in the L1 I cache
			cgm_cache_set_block(&(l1_d_caches[my_pid]), *set_ptr, *way_ptr, tag, 4);

			//service the mshr request
			mshr_remove(&(l1_d_caches[my_pid]), access_id);

			//remove the message from the in queue
			list_remove(l1_d_caches[my_pid].Rx_queue_top, message_packet);

			//dump in the done queue for the writeback stage, this is a simulator-ism.
			linked_list_add(message_packet->event_queue, message_packet->data);
		}
		else
		{
			fatal("l1_d_cache_ctrl_0(): unknown L2 message type = %d\n", message_packet->access_type);
		}*/

	}

	//should never get here
	fatal("l1_d_cache_ctrl task is broken\n");
	return;
}
