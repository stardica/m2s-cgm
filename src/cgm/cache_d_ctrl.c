/*
 * cache_d_ctrl.c
 *
 *  Created on: Apr 22, 2015
 *      Author: stardica
 */

#include <cgm/cache.h>
#include <arch/x86/timing/cpu.h>







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


		/////////testing
		list_remove(l1_d_caches[my_pid].Rx_queue_top, message_packet);
		linked_list_add(message_packet->event_queue, message_packet->data);
		/////////testing


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
