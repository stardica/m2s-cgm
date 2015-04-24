/*
 * cache_l2_ctrl.c
 *
 *  Created on: Apr 22, 2015
 *      Author: stardica
 */

#include <arch/x86/timing/cpu.h>

#include <cgm/cache.h>
#include <cgm/switch.h>
#include <cgm/protocol.h>
#include <cgm/tasking.h>



void l2_cache_ctrl(void){

	int my_pid = l2_pid++;
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

	int mshr_status = 0;
	int retry = 0;
	int *retry_ptr = &retry;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{

		/*wait here until there is a job to do.*/
		await(&l2_cache[my_pid], step);
		step++;

		printf("L2\n");

		//check the top or bottom rx queues for messages.
		message_packet = get_message(&(l2_caches[my_pid]), retry_ptr);

		access_type = message_packet->access_type;

		//Messages from L1_I_Cache
		if (access_type == cgm_access_gets_i || cgm_access_retry)
		{
			//stats
			if(access_type == cgm_access_gets_i)
				l2_caches[my_pid].loads++;

			if(access_type == cgm_access_retry)
				l2_caches[my_pid].retries++;

			access_id = message_packet->access_id;
			addr = message_packet->address;

			//probe the address for set, tag, and offset.
			cgm_cache_decode_address(&(l2_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);

			CGM_DEBUG(cache_debug_file,"l2_cache[%d] access_id %llu cycle %llu as %s at addr 0x%08u, tag %d, set %d, offset %u\n",
					my_pid, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

			CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu l2_cache[%d]\tRECIEVE l1_i_cache[%d] %s\n",
								access_id, P_TIME, my_pid, my_pid, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			/////////
			cgm_cache_set_block(&(l2_caches[my_pid]), *set_ptr, *way_ptr, tag, cache_block_shared);
			/////////

			//look up, and charge a cycle.
			cache_status = cgm_cache_find_block(&l2_caches[my_pid], tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
			P_PAUSE(1);

			// L2 Cache Hit!
			if(cache_status == 1 && *state_ptr != 0)
			{

				//fprintf(cgm_debug, "l2_cache[%d] access_id %llu cycle %llu\n", my_pid, access_id, P_TIME);
				//fprintf(cgm_debug, "l2_cache[%d] Hit\n\n", my_pid);


				if(access_type == cgm_access_retry_i)
					l2_caches[my_pid].retries++;

				if(access_type == cgm_access_gets_i)
					l2_caches[my_pid].hits++;


				//while the next level of cache's in queue is full stall
				while(!cache_can_access_bottom(&l1_i_caches[my_pid]))
				{
					P_PAUSE(1);
				}

				//success, remove packet from l2 cache in queue
				list_remove(l2_caches[my_pid].Rx_queue_top, message_packet);

				//change access type, i cache only ever reads so puts is ok.
				message_packet->access_type = cgm_access_puts;
				list_enqueue(l1_i_caches[my_pid].Rx_queue_bottom, message_packet);
				future_advance(&l1_i_cache[my_pid], (etime.count + l1_i_caches[my_pid].wire_latency));

				//fprintf(cgm_debug, "l2_cache[%d] access_id %llu cycle %llu\n", my_pid, access_id, P_TIME);
			//	fprintf(cgm_debug, "l1_i_cache[%d] access as %s\n\n", my_pid, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
			}
			// L2 Cache Miss!
			else if(cache_status == 0 || *state_ptr == 0)
			{
				if(access_id == 1)
				{
					printf("access id %llu l2 miss\n", access_id);
					getchar();
				}

				if(access_type == cgm_access_gets_i)
					l2_caches[my_pid].misses++;

				//star todo check on size of MSHR
				mshr_packet = status_packet_create();

				//drop a token in the mshr queue
				//star todo add some detail to this so we can include coalescing
				//star todo have an MSHR hit advance the cache and clear out the request.
				mshr_packet->access_type = message_packet->access_type;
				mshr_packet->access_id = message_packet->access_id;
				mshr_packet->in_flight = message_packet->in_flight;
				mshr_packet->src_name = l1_i_caches[my_pid].name;
				list_enqueue(l2_caches[my_pid].mshr, mshr_packet);


				//send to L3 cache over switching network add source and dest here
				message_packet->access_type = cgm_access_gets_i;
				message_packet->src_name = l2_caches[my_pid].name;
				message_packet->source_id = str_map_string(&node_strn_map, l2_caches[my_pid].name);
				message_packet->dest_name = l3_caches[my_pid].name;
				message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[my_pid].name);

				while(!switch_can_access(switches[my_pid].north_queue))
				{
					P_PAUSE(1);
				}

				//success
				list_remove(l2_caches[my_pid].Rx_queue_top, message_packet);
				list_enqueue(switches[my_pid].north_queue, message_packet);

				future_advance(&switches_ec[my_pid], (etime.count + switches[my_pid].wire_latency));
				//done
			}
		}
		else if(access_type == cgm_access_puts)
		{
			//reply from L3
			//charge the delay for writing cache block
			P_PAUSE(1);
			cgm_cache_set_block(&(l2_caches[my_pid]), *set_ptr, *way_ptr, tag, cache_block_shared);

			//star todo what should I put for the state?

			//star todo service the mshr requests (this is for coalescing)
			//current just removes 1 element at a time,
			//mshr_packet = mshr_remove(&l2_caches[my_pid]);
			assert(mshr_packet); //we better find a token

			//charge the delay for servicing the older request in the MSHR
			//advance the l1_i_cache, on the next cycle the request should be a hit

			//set to fetch for retry

			//star todo check with Dr. H on this
			message_packet->access_type = cgm_access_retry_i;
			advance(&l2_cache[my_pid]);
			//done.

			//remove the message from the in queue
			//list_remove(l1_i_caches[my_pid].Rx_queue_top, message_packet);
			//remove from the access tracker, this is a simulator-ism.
			//remove_from_global(access_id);

		}

		/*//Messages from L1_D_Cache
		else if (access_type == cgm_access_load)
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

		}
		else if (access_type == cgm_access_store)
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
	}
	/* should never get here*/
	fatal("l2_cache_ctrl task is broken\n");
	return;
}
