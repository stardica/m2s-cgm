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

void l1_i_cache_ctrl(void){

	//my_pid increments for the number of CPU cores. i.e. 0 - 4 for a quad core
	int my_pid = l1_i_pid++;
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

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;

	int mshr_status = 0;
	int retry = 0;
	int *retry_ptr = &retry;

	int i = 0;
	long long advance_time = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{

		//wait here until there is a job to do
		await(&l1_i_cache[my_pid], step);
		step++;

		//check the top or bottom rx queues for messages.
		message_packet = get_message(&(l1_i_caches[my_pid]), retry_ptr);

		access_type = message_packet->access_type;

		if (access_type == cgm_access_fetch || access_type == cgm_access_retry)
		{//then the access is from the CPU

			//stats
			if(access_type == cgm_access_fetch)
				l1_i_caches[my_pid].fetches++;

			if(access_type == cgm_access_retry)
				l1_i_caches[my_pid].retries++;


			//memory acess from CPU
			addr = message_packet->address;
			access_id = message_packet->access_id;

			//probe the address for set, tag, and offset.
			cgm_cache_decode_address(&(l1_i_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);

			fprintf(cgm_debug,"l1_i_cache[%d] access_id %llu cycle %llu\n", my_pid, access_id, P_TIME);
			fprintf(cgm_debug,"%s, addr 0x%08u, tag %d, set %d, offset %u\n\n", (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

			//get the block and the state of the block and charge a cycle
			cache_status = cgm_cache_find_block(&(l1_i_caches[my_pid]), tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
			P_PAUSE(1);

			//L1 I Cache Hit!
			if(cache_status == 1 && *state_ptr != 0)
			{
				fprintf(cgm_debug, "l1_i_cache[%d] access_id %llu cycle %llu\n", my_pid, access_id, P_TIME);
				fprintf(cgm_debug, "l1_i_cache[%d] Hit\n\n", my_pid);

				if(access_type == cgm_access_retry)
					retry_ptr--;

				if(access_type == cgm_access_fetch)
					l1_i_caches[my_pid].hits++;

				//remove packet from cache queue, global queue, and simulator memory
				//note cycle already charged

				if(access_type == cgm_access_fetch)
				{
					list_remove(l1_i_caches[my_pid].next_queue, message_packet);
				}
				else if (access_type == cgm_access_retry)
				{
					list_remove(l1_i_caches[my_pid].retry_queue, message_packet);

				}

				remove_from_global(access_id);
				free(message_packet);

			}

			//L1 I Cache Miss!
			else if(cache_status == 0 || *state_ptr == 0)
			{

				//all mshr based retires should be hits
				//star todo there is a bug here 1 access fails retry in our MM.
				assert(message_packet->access_type != cgm_access_retry);

				if(access_type == cgm_access_fetch)
					l1_i_caches[my_pid].misses++;

				fprintf(cgm_debug, "l1_i_cache[%d] access_id %llu cycle %llu\n", my_pid, access_id, P_TIME);
				fprintf(cgm_debug, "l1_i_cache[%d] Miss\n\n", my_pid);

				miss_status_packet = miss_status_packet_create(message_packet->access_id, message_packet->access_type, set, tag, offset);

				mshr_status = mshr_set(&(l1_i_caches[my_pid]), miss_status_packet, message_packet);

				if(mshr_status == 1)
				{
					//access is unique in the MSHR
					//while the next level of cache's in queue is full stall
					while(!cache_can_access_top(&l2_caches[my_pid]))
					{
						P_PAUSE(1);
					}

					/*change the access type for the coherence protocol and drop into the L2's queue
					remove the access from the l1 cache queue and place it in the l2 cache ctrl queue*/
					message_packet->access_type = cgm_access_gets_i;
					list_remove(l1_i_caches[my_pid].Rx_queue_top, message_packet);
					list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);

					fprintf(cgm_debug, "l1_i_cache[%d] access_id %llu cycle %llu\n", my_pid, access_id, P_TIME);
					fprintf(cgm_debug, "l2_cache[%d] access as %s\n\n", my_pid, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

					//advance the L2 cache adding some wire delay time.
					future_advance(&l2_cache[my_pid], (etime.count + l2_caches[my_pid].wire_latency));
				}
				else if(mshr_status == 0)
				{
					//mshr is full so we can't progress, retry.
					message_packet->access_type = cgm_access_retry;
					future_advance(&l1_i_cache[my_pid], (etime.count + 2));

				}
				else
				{
					//access was coalesced so do nothing until later.
				}

				//done
			}
		}

		else if(access_type == cgm_access_puts)
		{
			//the packet is from the L2 cache
			addr = message_packet->address;
			access_id = message_packet->access_id;

			//probe the address for set, tag, and offset.
			cgm_cache_decode_address(&(l1_i_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);


			fprintf(cgm_debug, "l1_i_cache[%d] access_id %llu cycle %llu\n", my_pid, access_id, P_TIME);
			fprintf(cgm_debug, "l1_i_cache[%d] puts\n\n", my_pid);

			//charge the delay for writing cache block
			cgm_cache_set_block(&l1_i_caches[my_pid], *set_ptr, *way_ptr, tag, cache_block_shared);
			P_PAUSE(1);

			//get the mshr status
			mshr_status = mshr_get(&l1_i_caches[my_pid], set_ptr, tag_ptr);
			assert(mshr_status != -1);

			if(mshr_status >= 0)
			{
				/*we have outstanding mshr requests so set the retry state bit*/
				*retry_ptr = l1_i_caches[my_pid].mshrs[mshr_status].num_entries;
				//printf("retry_ptr %d\n", *retry_ptr);
				assert(*retry_ptr > 0);
			}

			advance_time = etime.count + 2;

			//move the access and any coalesced accesses to the retry queue.
			for(i = 0; i < *retry_ptr; i++)
			{
				if( i == 0)
				{
					//move current message_packet to retry queue
					message_packet->access_type = cgm_access_retry;
					list_remove(l1_i_caches[my_pid].next_queue, message_packet);
					list_enqueue(l1_i_caches[my_pid].retry_queue, message_packet);

					printf("list count %d\n", list_count(l1_i_caches[my_pid].retry_queue));

					advance(&l1_i_cache[my_pid]);
				}
				else if( i > 0)
				{
					miss_status_packet = list_remove_at(l1_i_caches[my_pid].mshrs[mshr_status].entires, i);
					list_enqueue(l1_i_caches[my_pid].retry_queue, miss_status_packet->coalesced_packet);
					free(miss_status_packet);
					advance_time += 2;
					advance(&l1_i_cache[my_pid]);
				}
			}

			//clear the mshr row for future use
			mshr_clear(&l1_i_caches[my_pid].mshrs[mshr_status]);
			//done.
		}
		else
		{
			fatal("l1_i_cache_ctrl_0(): unknown L2 message type = %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
		}

	}

	/* should never get here*/
	fatal("l1_i_cache_ctrl task is broken\n");
	return;
}
