/*
 * cache_l3_ctrl.c
 *
 *  Created on: Apr 22, 2015
 *      Author: stardica
 */

#include <cgm/cache.h>
#include <cgm/switch.h>
#include <arch/x86/timing/cpu.h>

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
			//mshr_packet = mshr_remove(&l3_caches[my_pid]);
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
