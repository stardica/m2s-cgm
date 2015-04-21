/*
 * cache_ctrl.h
 *
 *  Created on: Apr 21, 2015
 *      Author: stardica
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <lib/util/string.h>
#include <lib/util/misc.h>

#include <cgm/cgm.h>
#include <cgm/cache.h>
#include <cgm/tasking.h>
#include <cgm/packet.h>
#include <cgm/switch.h>
#include <cgm/protocol.h>
#include <cgm/mshr.h>


void l1_d_cache_ctrl(void){


	int my_pid = l1_d_pid++;
	long long step = 1;

	int num_cores = x86_cpu_num_cores;
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

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{
		//wait here until there is a job to do.
		await(&l1_d_cache[my_pid], step);
		step++;

		//get the message out of the queue
		message_packet = list_get(l1_d_caches[my_pid].Rx_queue_top, 0);
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		addr = message_packet->address;

		//probe the address for set, tag, and offset.
		cgm_cache_decode_address(&(l1_d_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);


		//for testing
		list_remove(l1_d_caches[my_pid].Rx_queue_top, message_packet);
		linked_list_add(message_packet->event_queue, message_packet->data);
		free(message_packet);


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


void l3_cache_ctrl(void){

	int my_pid = l3_pid++;
	long long step = 1;

	int num_cores = x86_cpu_num_cores;
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

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);


	while(1)
	{
		/*wait here until there is a job to do.*/
		await(&l3_cache[my_pid], step);
		step++;

		//get the message out of the queue
		message_packet = list_get(l3_caches[my_pid].Rx_queue_top, 0);
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		addr = message_packet->address;

		//probe the address for set, tag, and offset.
		cgm_cache_decode_address(&(l2_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);

		if(access_id == 1)
		{
			printf("L3\n");
			printf("access id %llu\n", access_id);
			printf("access type %d\n", access_type);
			printf("addr 0x%08u\n", addr);
			printf("set = %d\n", *set_ptr);
			printf("tag = %d\n", *tag_ptr);
			printf("offset = %u\n", *offset_ptr);
			getchar();
		}


		if (access_type == cgm_access_gets_i  || access_type == cgm_access_retry_i)
		{

			//stats
			if (access_type == cgm_access_gets_i)
				l3_caches[my_pid].loads++;

			if (access_type == cgm_access_retry_i)
				l3_caches[my_pid].retries++;


			//charge the cycle for the look up.
			cache_status = cgm_cache_find_block(&l3_caches[my_pid], tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
			P_PAUSE(1);


			//L3 Cache Hit!
			if(cache_status == 1 && *state_ptr != 0)
			{

				if(access_id == 1)
				{
					printf("access id %llu l3 hit\n", access_id);
					getchar();
				}

				if (access_type == cgm_access_gets_i)
					l3_caches[my_pid].hits++;

				//This is a hit in the L3 cache, send up to L2 cache

				//while the next level of cache's in queue is full stall
				//star todo possible deadlock situation if both the l2 and core are trying to fill a full queue
				while(!switch_can_access(switches[my_pid].south_queue))
				{
					P_PAUSE(1);
				}

				//success
				//remove packet from l3 cache in queue
				//change access type, i cache only ever reads so puts is fine.
				message_packet->access_type = cgm_access_puts;
				message_packet->dest_name = message_packet->src_name;
				message_packet->dest_id = str_map_string(&node_strn_map, message_packet->src_name);
				message_packet->src_name = l3_caches[my_pid].name;
				message_packet->source_id = str_map_string(&node_strn_map, l3_caches[my_pid].name);

				list_remove(l3_caches[my_pid].Rx_queue_top, message_packet);
				list_enqueue(switches[my_pid].south_queue, message_packet);
				future_advance(&switches_ec[my_pid], (etime.count + switches[my_pid].wire_latency));
				//done

			}
			//L3 Cache Miss!
			else if(cache_status == 0 || *state_ptr == 0)
			{

				if(access_id == 1)
				{
					printf("access id %llu l3 miss\n", access_id);
					getchar();
				}

				if (access_type == cgm_access_gets_i)
					l3_caches[my_pid].misses++;


				//star todo check on size of MSHR
				mshr_packet = status_packet_create();

				//drop a token in the mshr queue
				//star todo add some detail to this so we can include coalescing
				//star todo have an MSHR hit advance the cache and clear out the request.
				mshr_packet->access_type = message_packet->access_type;
				mshr_packet->access_id = message_packet->access_id;
				mshr_packet->in_flight = message_packet->in_flight;
				mshr_packet->source_id = str_map_string(&node_strn_map, l1_i_caches[my_pid].name);
				list_enqueue(l2_caches[my_pid].mshr, mshr_packet);


				//send to L3 cache over switching network
				//add source and dest
				//star todo make a function to get the string for the l3 slices.
				message_packet->access_type = cgm_access_gets;
				message_packet->src_name = l3_caches[my_pid].name;
				message_packet->source_id = str_map_string(&node_strn_map, l3_caches[my_pid].name);

				message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
				message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);


				while(!switch_can_access(switches[my_pid].south_queue))
				{
					P_PAUSE(1);
				}

				//success
				list_remove(l3_caches[my_pid].Rx_queue_top, message_packet);
				list_enqueue(switches[my_pid].south_queue, message_packet);

				future_advance(&switches_ec[my_pid], (etime.count + switches[my_pid].wire_latency));
				//done
			}
		}
		else if(access_type == cgm_access_puts)
		{
			//reply from L3
			//charge the delay for writing cache block
			//P_PAUSE(1);
			cgm_cache_set_block(&(l3_caches[my_pid]), *set_ptr, *way_ptr, tag, cache_block_shared);

			//star todo what should I put for the state?
			//star todo service the mshr requests (this is for coalescing)

			//current just removes 1 element at a time,
			mshr_packet = mshr_remove(&l3_caches[my_pid], access_id);
			assert(mshr_packet); //we better find a token
			//charge the delay for servicing the older request in the MSHR
			//advance the l1_i_cache, on the next cycle the request should be a hit
			//set to fetch for retry

			//star todo check with Dr. H on this
			message_packet->access_type = cgm_access_retry_i;
			message_packet->src_name = mshr_packet->src_name;
			advance(&l3_cache[my_pid]);
			//done.

						//remove the message from the in queue
						//list_remove(l1_i_caches[my_pid].Rx_queue_top, message_packet);
						//remove from the access tracker, this is a simulator-ism.
						//remove_from_global(access_id);
		}

	}

	/* should never get here*/
	fatal("l3_cache_ctrl task is broken\n");
	return;
}

void gpu_v_cache_ctrl(void){

	int my_pid = gpu_v_pid++;
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
		//wait here until there is a job to do.
		//In any given cycle I might have to service 1 to N number of caches
		await(&gpu_v_cache[my_pid], step);
		step++;

		//get the message out of the unit's queue
		message_packet = list_get(gpu_v_caches[my_pid].Rx_queue_top, 0);
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		addr = message_packet->address;

		//probe the address for set, tag, and offset.
		cgm_cache_decode_address(&(gpu_v_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);


		//star todo figure out what to do with this.
		if(access_type == cgm_access_load || access_type == cgm_access_store || access_type == cgm_access_nc_store)
		{//then the packet is from the L2 cache

			P_PAUSE(mem_miss);

			//clear the gpu uop witness_ptr
			(*message_packet->witness_ptr)++;

			list_remove(gpu_v_caches[my_pid].Rx_queue_top, message_packet);
			free(message_packet);

		}
		else
		{
			fatal("gpu_v_cache_ctrl(): unknown access type = %d\n", message_packet->access_type);
		}

	}
	//should never get here
	fatal("gpu_v_cache_ctrl task is broken\n");
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
		//wait here until there is a job to do.
		//In any given cycle I might have to service 1 to N number of caches
		await(&gpu_s_cache[my_pid], step);
		step++;

		//get the message out of the unit's queue
		message_packet = list_get(gpu_s_caches[my_pid].Rx_queue_top, 0);
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		addr = message_packet->address;

		//probe the address for set, tag, and offset.
		cgm_cache_decode_address(&(gpu_s_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);


		//star todo figure out what to do with this.
		if(access_type == cgm_access_load)
		{//then the packet is from the L2 cache

			P_PAUSE(mem_miss);

			//clear the gpu uop witness_ptr
			(*message_packet->witness_ptr)++;

			list_remove(gpu_s_caches[my_pid].Rx_queue_top, message_packet);
			free(message_packet);

		}
		else
		{
			fatal("gpu_s_cache_ctrl(): unknown access type = %d\n", message_packet->access_type);
		}

	}
	 //should never get here
	fatal("gpu_s_cache_ctrl task is broken\n");
	return;
}

void gpu_l2_cache_ctrl(void){

	long long step = 1;


	while(1)
	{
		//wait here until there is a job to do.
		await(gpu_l2_cache, step);
		step++;

	}

	//should never get here
	fatal("gpu_l2_cache_ctrl task is broken\n");
	return;

}

void gpu_lds_unit_ctrl(void){

	int my_pid = gpu_lds_pid++;
	long long step = 1;

	int num_cus = si_gpu_num_compute_units;
	struct cgm_packet_t *message_packet;

	enum cgm_access_kind_t access_type;

	assert(my_pid <= num_cus);
	set_id((unsigned int)my_pid);

	while(1)
	{
		/*wait here until there is a job to do.
		In any given cycle I might have to service 1 to N number of units*/
		await(&gpu_lds_unit[my_pid], step);
		step++;

		//get the message out of the unit's queue
		message_packet = list_get(gpu_lds_units[my_pid].Rx_queue_top, 0);
		assert(message_packet);

		access_type = message_packet->access_type;
		//for now treat the LDS unit as a register and charge a small amount of cycles for it's access.

		//star todo figure out what to do with this.
		if(access_type == cgm_access_load || access_type == cgm_access_store)
		{//then the packet is from the L2 cache

			//LDS is close to the CU so delay two cycles for now
			P_PAUSE(etime.count + 2);

			//clear the gpu uop witness_ptr
			(*message_packet->witness_ptr)++;

			list_remove(gpu_lds_units[my_pid].Rx_queue_top, message_packet);
			free(message_packet);
		}
		else
		{
			fatal("gpu_lds_unit_ctrl(): unknown L2 message type = %d\n", message_packet->access_type);
		}
	}
	/* should never get here*/
	fatal("gpu_lds_unit_ctrl task is broken\n");
	return;
}



void cgm_cache_update_waylist(struct cache_set_t *set, struct cache_block_t *blk, enum cache_waylist_enum where){
	if (!blk->way_prev && !blk->way_next)
	{
		assert(set->way_head == blk && set->way_tail == blk);
		return;

	}
	else if (!blk->way_prev)
	{
		assert(set->way_head == blk && set->way_tail != blk);
		if (where == cache_waylist_head)
			return;
		set->way_head = blk->way_next;
		blk->way_next->way_prev = NULL;

	}
	else if (!blk->way_next)
	{
		assert(set->way_head != blk && set->way_tail == blk);
		if (where == cache_waylist_tail)
			return;
		set->way_tail = blk->way_prev;
		blk->way_prev->way_next = NULL;

	}
	else
	{
		assert(set->way_head != blk && set->way_tail != blk);
		blk->way_prev->way_next = blk->way_next;
		blk->way_next->way_prev = blk->way_prev;
	}

	if (where == cache_waylist_head)
	{
		blk->way_next = set->way_head;
		blk->way_prev = NULL;
		set->way_head->way_prev = blk;
		set->way_head = blk;
	}
	else
	{
		blk->way_prev = set->way_tail;
		blk->way_next = NULL;
		set->way_tail->way_next = blk;
		set->way_tail = blk;
	}
}

void cache_dump_stats(void){

	int num_cores = x86_cpu_num_cores;
	int num_threads = x86_cpu_num_threads;
	int i = 0;

	fprintf(cgm_stats, "[General]\n");
	fprintf(cgm_stats, "NumCores = %d\n", num_cores);
	fprintf(cgm_stats, "ThreadsPerCore = %d\n", num_threads);
	fprintf(cgm_stats, "TotalCycles = %lld\n", P_TIME);
	fprintf(cgm_stats, "\n");

	for(i = 0; i < num_cores; i++)
	{
		fprintf(cgm_stats, "[L1_I_Cache_%d]\n", i);
		fprintf(cgm_stats, "Sets = %d\n", l1_i_caches[i].num_sets);
		fprintf(cgm_stats, "BlockSize = %d\n", l1_i_caches[i].block_size);
		fprintf(cgm_stats, "Fetches = %lld\n", l1_i_caches[i].fetches);
		fprintf(cgm_stats, "Hits = %lld\n", l1_i_caches[i].hits);
		fprintf(cgm_stats, "Misses = %lld\n", l1_i_caches[i].misses);
		fprintf(cgm_stats, "\n");

		fprintf(cgm_stats, "[L1_D_Cache_%d]\n", i);
		fprintf(cgm_stats, "Sets = %d\n", l1_d_caches[i].num_sets);
		fprintf(cgm_stats, "BlockSize = %d\n", l1_d_caches[i].block_size);
		fprintf(cgm_stats, "Loads = %lld\n", l1_d_caches[i].loads);
		fprintf(cgm_stats, "Stores = %lld\n", l1_d_caches[i].stores);
		fprintf(cgm_stats, "Hits = %lld\n", l1_d_caches[i].hits);
		fprintf(cgm_stats, "Misses = %lld\n", l1_d_caches[i].misses);
		fprintf(cgm_stats, "\n");

		fprintf(cgm_stats, "[L2_Cache_%d]\n", i);
		fprintf(cgm_stats, "Sets = %d\n", l2_caches[i].num_sets);
		fprintf(cgm_stats, "BlockSize = %d\n", l2_caches[i].block_size);
		fprintf(cgm_stats, "Accesses = %lld\n", (l2_caches[i].fetches + l2_caches[i].loads + l2_caches[i].stores));
		fprintf(cgm_stats, "Hits = %lld\n", l2_caches[i].hits);
		fprintf(cgm_stats, "Misses = %lld\n", l2_caches[i].misses);
		fprintf(cgm_stats, "\n");
	}

	return;
}

/*int cache_mesi_load(struct cache_t *cache, enum cgm_access_kind_t access_type, int *tag_ptr, int *set_ptr, unsigned int *offset_ptr, int *way_ptr, int *state_ptr){

	cache_block_invalid = 0
	cache_block_noncoherent = 1
	cache_block_modified = 2
	cache_block_owned = 3
	cache_block_exclusive = 4
	cache_block_shared = 5

	int cache_status;

	//stats
	cache->loads++;

	//find the block in the cache and get it's state
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);

	printf("cache_status %d\n", cache_status);
	getchar();

	//hit and state is M, E, or S we are done at this level of cache
	if((cache_status == 1 && *state_ptr == 2) || (cache_status == 1 && *state_ptr == 4 ) || (cache_status == 1 && *state_ptr == 5))
	{
		//stats
		cache->hits++;

		//done, respond to requester.
		return 1;
	}
	//hit and state is invalid (miss)
	else if(cache_status == 1 && *state_ptr == 0)
	{
		//stats
		cache->invalid_hits++;

		//treat this like a miss
		return 2;

	}
	//the cache block is not present m the cache (miss)
	else if(cache_status == 0)
	{
		//stats
		cache->misses++;

		return 3;
	}
	else if (cache_status == 1 && *state_ptr == 1)
	{
		printf("CRASHING cache_status %d state_ptr %d\n", cache_status, *state_ptr);
		getchar();
		fatal("cache_mesi_load() non cached state\n");
	}
	else
	{
		printf("CRASHING cache_status %d state_ptr %d\n", cache_status, *state_ptr);
		getchar();
		fatal("cache_mesi_load() something went wrong here\n");
	}

	return 0;

}*/
