/*
 * cache.c
 *
 *  Created on: Nov 25, 2014
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
#include <cgm/queue.h>
#include <cgm/tasking.h>
#include <cgm/packet.h>


#include <instrumentation/stats.h>


struct str_map_t cgm_cache_policy_map =
{
	3, 	{
		{ "LRU", cache_policy_lru },
		{ "FIFO", cache_policy_fifo },
		{ "Random", cache_policy_random }
		}
};

struct str_map_t cgm_cache_block_state_map =
{ 	6, 	{
		{ "N", cache_block_noncoherent },
		{ "M", cache_block_modified },
		{ "O", cache_block_owned },
		{ "E", cache_block_exclusive },
		{ "S", cache_block_shared },
		{ "I", cache_block_invalid }
		}
};

int QueueSize;

//CPU caches
struct cache_t *l1_i_caches;
struct cache_t *l1_d_caches;
struct cache_t *l2_caches;
struct cache_t *l3_caches;
/*struct cache_t *l3_s0_cache;
struct cache_t *l3_s1_cache;
struct cache_t *l3_s2_cache;
struct cache_t *l3_s3_cache;*/

//GPU caches
struct cache_t *l1_v_caches;
struct cache_t *l1_s_caches;
struct cache_t *gpu_l2_caches;
struct cache_t *lds_units;

//event counts
eventcount volatile *l1_i_cache_0;
eventcount volatile *l1_i_cache_1;
eventcount volatile *l1_i_cache_2;
eventcount volatile *l1_i_cache_3;
eventcount volatile *l1_d_cache_0;
eventcount volatile *l1_d_cache_1;
eventcount volatile *l1_d_cache_2;
eventcount volatile *l1_d_cache_3;
eventcount volatile *l2_cache_0;
eventcount volatile *l2_cache_1;
eventcount volatile *l2_cache_2;
eventcount volatile *l2_cache_3;


int mem_miss = 100;



void cache_init(void){

	cache_create();
	cache_create_tasks();

	return;
}

void cache_create(void){

	//star todo make this automatic
	//star todo make defaults
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int l3_slices = num_cores/4;
	int gpu_group_cache_num = num_cus/4;


	l1_i_caches = (void *) calloc(num_cores, sizeof(struct cache_t));
	//initialize the CPU L1D caches

	l1_d_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	//initialize the CPU L2 caches
	l2_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	//initialize the L3 caches (4 slices).

	//just one cache for now
	l3_caches = (void *) calloc(1, sizeof(struct cache_t));

	//l3_caches = (void *) calloc(num_cores, sizeof(struct cache_t));
	/*l3_s0_cache = (void *) calloc(l3_slices, sizeof(struct cache_t));
	l3_s1_cache = (void *) calloc(l3_slices, sizeof(struct cache_t));
	l3_s2_cache = (void *) calloc(l3_slices, sizeof(struct cache_t));
	l3_s3_cache = (void *) calloc(l3_slices, sizeof(struct cache_t));*/

	//initialize the GPU L1V caches
	l1_v_caches = (void *) calloc(num_cus, sizeof(struct cache_t));

	//initialize the GPU L1S caches
	l1_s_caches = (void *) calloc(num_cus, sizeof(struct cache_t));

	//initialize the GPU L2 caches.
	gpu_l2_caches = (void *) calloc(gpu_group_cache_num, sizeof(struct cache_t));

	//initialize the GPU LDS
	lds_units = (void *) calloc(num_cus, sizeof(struct cache_t));

	return ;
}

void cache_create_tasks(void){

	//star todo make this dynamic
	char buff[100];

	/////////////
	//eventcounts
	/////////////

	//l1 i caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_0");
	l1_i_cache_0 = new_eventcount(strdup(buff));

	/*memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_1");
	l1_i_cache_1 = new_eventcount(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_2");
	l1_i_cache_2 = new_eventcount(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_3");
	l1_i_cache_3 = new_eventcount(strdup(buff));*/


	//l1 d caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_0");
	l1_d_cache_0 = new_eventcount(strdup(buff));

	/*memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_1");
	l1_d_cache_1 = new_eventcount(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_2");
	l1_d_cache_2 = new_eventcount(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_3");
	l1_d_cache_3 = new_eventcount(strdup(buff));*/


	//l2 caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_0");
	l2_cache_0 = new_eventcount(strdup(buff));

	/*memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_1");
	l2_cache_1 = new_eventcount(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_2");
	l2_cache_2 = new_eventcount(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_3");
	l2_cache_3 = new_eventcount(strdup(buff));*/


	////////////////////
	//tasks
	////////////////////

	//l1 i caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_ctrl_0");
	create_task(l1_i_cache_ctrl_0, DEFAULT_STACK_SIZE, strdup(buff));

	/*memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_ctrl_1");
	create_task(l1_i_cache_ctrl_1, DEFAULT_STACK_SIZE, buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_ctrl_2");
	create_task(l1_i_cache_ctrl_2, DEFAULT_STACK_SIZE, buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_ctrl_3");
	create_task(l1_i_cache_ctrl_3, DEFAULT_STACK_SIZE, buff);*/


	//l1 d caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_ctrl_0");
	create_task(l1_d_cache_ctrl_0, DEFAULT_STACK_SIZE, strdup(buff));

	/*memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_ctrl_1");
	create_task(l1_d_cache_ctrl_1, DEFAULT_STACK_SIZE, buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_ctrl_2");
	create_task(l1_d_cache_ctrl_2, DEFAULT_STACK_SIZE, buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_ctrl_0");
	create_task(l1_d_cache_ctrl_0, DEFAULT_STACK_SIZE, buff);*/


	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_ctrl_0");
	create_task(l2_cache_ctrl_0, DEFAULT_STACK_SIZE, strdup(buff));

	/*memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_ctrl_1");
	create_task(l2_cache_ctrl_1, DEFAULT_STACK_SIZE, buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_ctrl_2");
	create_task(l2_cache_ctrl_2, DEFAULT_STACK_SIZE, buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_ctrl_3");
	create_task(l2_cache_ctrl_3, DEFAULT_STACK_SIZE, buff);*/

	return;
}


void l1_i_cache_ctrl_0(void){

	struct cgm_packet_t *message_packet;
	struct cgm_packet_status_t *mshr_packet;
	long long step = 1;
	int list_index = 0;
	int i = 0;


	enum cgm_access_kind_t access_type;
	unsigned int addr = 0;
	long long access_id = 0;
	int set = 0;
	int tag = 0;
	unsigned int offset = 0;

	int cache_status;
	int way = 0;
	int state = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;

	while(1)
	{
		//wait here until there is a job to do.
		await(l1_i_cache_0, step);
		step++;


		//message received check queue there should be a message there.
		assert(list_count(l1_i_caches[0].Rx_queue_top) >= 1);

		//get the message out of the queue
		message_packet = list_get(l1_i_caches[0].Rx_queue_top, list_index);
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		addr = message_packet->address;

		//decode the address to set, tag, and offset
		cgm_cache_decode_address(&(l1_i_caches[0]), addr, set_ptr, tag_ptr, offset_ptr);

		/*printf("address = 0x%08u\n", addr);
		printf("set = %d\n", *set_ptr);
		printf("tag = %d\n", *tag_ptr);
		printf("offset = %u\n", *offset_ptr);
		getchar();*/

		//set = (addr >> l1_i_caches[0].log_block_size) % l1_i_caches[0].num_sets;
		//tag = addr & ~(l1_i_caches[0].block_mask);

		//check if the block is in the cache


		//request from CPU
		if (message_packet->access_type == cgm_access_fetch)
		{
			//test

			//stats
			l1_i_caches[0].fetches++;

			cache_status = cgm_cache_find_block(&(l1_i_caches[0]), addr, set_ptr, way_ptr, state_ptr);

			// L1 I Cache Hit!
			if(cache_status == 1)
			{
				l1_i_caches[0].hits++;

				//Mr. CPU, go about your business...
				//remove packet from cache and global queues
				list_remove(l1_i_caches[0].Rx_queue_top, message_packet);
				remove_from_global(access_id);

			}
			//L1 I Cache Miss!
			else if(cache_status == 0)
			{
				// L1 I Cache Miss!
				l1_i_caches[0].misses++;

				mshr_packet = status_packet_create();

				//drop a token in the mshr queue
				//star todo add some detail to this so we can include coalescing
				mshr_packet->access_type = message_packet->access_type;
				mshr_packet->access_id = message_packet->access_id;
				mshr_packet->in_flight = message_packet->in_flight;
				list_enqueue(l1_i_caches[0].mshr, mshr_packet);

				//remove the access from the l1 cache queue and place it in the l2 cache ctrl queue
				list_remove(l1_i_caches[0].Rx_queue_top, message_packet);
				list_enqueue(l2_caches[0].Rx_queue_top, message_packet);


				future_advance(l2_cache_0, (etime.count + 10));

			}

			continue;
		}

		//replies from L2
		if(message_packet->access_type == cgm_access_l2_load_reply)
		{
			//set the block in the L1 I cache
			cgm_cache_set_block(&(l1_i_caches[0]), *set_ptr, *way_ptr, tag, 1);

			//service the mshr request
			mshr_remove(&(l1_i_caches[0]), access_id);

			//remove the message from the in queue
			list_remove(l1_i_caches[0].Rx_queue_top, message_packet);

			//remove from the access tracker, this is a simulator-ism.
			remove_from_global(access_id);

			continue;
		}
		else
		{
			fatal("l1_i_cache_ctrl_0(): unknown L2 message type = %d\n", message_packet->access_type);
		}

	}

	/* should never get here*/
	fatal("l1_i_cache_ctrl_0 task is broken\n");
	return;
}

void l1_d_cache_ctrl_0(void){

	long long step = 1;
	long long access_id = 0;
	unsigned int addr = 0;
	int list_index = 0;
	int cache_status;
	int i = 0;
	struct cgm_packet_t *message_packet;
	struct cgm_packet_status_t *mshr_packet;

	enum cgm_access_kind_t access_type;
	int tag = 0;
	int offset = 0;
	int set = 0;
	int way = 0;
	int state = 0;


	while(1)
	{
		//wait here until there is a job to do.
		await(l1_d_cache_0, step);
		step++;

		//message received check queue there should be a message there.
		assert(list_count(l1_d_caches[0].Rx_queue_top) >= 1);

		//get the message out of the queue
		message_packet = list_get(l1_d_caches[0].Rx_queue_top, list_index);
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		addr = message_packet->address;


		set = (addr >> l1_d_caches[0].log_block_size) % l1_i_caches[0].num_sets;
		tag = addr & ~(l1_d_caches[0].block_mask);


		//check if the block is in the cache
		int *set_ptr = &set;
		int *way_ptr = &way;
		int *state_ptr = &state;

		//request from CPU
		if (message_packet->access_type == cgm_access_load)
		{

			//stats
			l1_d_caches[0].loads++;

			cache_status = cgm_cache_find_block(&(l1_d_caches[0]), addr, set_ptr, way_ptr, state_ptr);

			// L1 D Cache Hit!
			if(cache_status == 1)
			{
				l1_d_caches[0].hits++;

				//Mr. CPU, go about your business...
				//remove packet from cache queue and add to to commit stage input
				list_remove(l1_d_caches[0].Rx_queue_top, message_packet);
				linked_list_add(message_packet->event_queue, message_packet->data);

			}
			//L1 D Cache Miss!
			else if(cache_status == 0)
			{
				// L1 I Cache Miss!
				l1_d_caches[0].misses++;

				mshr_packet = status_packet_create();

				//drop a token in the mshr queue
				//star todo add some detail to this so we can include coalescing
				mshr_packet->access_type = message_packet->access_type;
				mshr_packet->access_id = message_packet->access_id;
				mshr_packet->in_flight = message_packet->in_flight;
				list_enqueue(l1_d_caches[0].mshr, mshr_packet);

				//remove the access from the l1 cache queue and place it in the l2 cache ctrl queue
				list_remove(l1_d_caches[0].Rx_queue_top, message_packet);
				list_enqueue(l2_caches[0].Rx_queue_top, message_packet);

				//Advance the l2 cache controller
				//4 clocks for wire delay.
				//advance(l2_cache_0);
				future_advance(l2_cache_0, (etime.count + 10));

			}

			continue;
		}

		//request from CPU
		else if (message_packet->access_type == cgm_access_store)
		{

			//star todo evict old block this is where the LRU, FIFO stuff comes into play
			//this needs some work to get it right

			//stats
			l1_d_caches[0].stores++;

			cache_status = cgm_cache_find_block(&(l1_d_caches[0]), addr, set_ptr, way_ptr, state_ptr);

			// L1 D Cache Hit!
			if(cache_status == 1)
			{
				//ok, on a hit this means there is a block of old memory in the cache (i.e. to be over written).
				l1_d_caches[0].hits++;

				//for now just set it so things will run
				cgm_cache_set_block(&(l1_i_caches[0]), *set_ptr, *way_ptr, tag, 1);

				//Mr. CPU, go about your business...
				//remove packet from cache queue and add to commit stage input
				list_remove(l1_d_caches[0].Rx_queue_top, message_packet);
				linked_list_add(message_packet->event_queue, message_packet->data);

			}
			//L1 D Cache Miss!
			else if(cache_status == 0)
			{
				// L1 I Cache Miss!
				l1_d_caches[0].misses++;

				mshr_packet = status_packet_create();

				//drop a token in the mshr queue
				//star todo add some detail to this so we can include coalescing
				mshr_packet->access_type = message_packet->access_type;
				mshr_packet->access_id = message_packet->access_id;
				mshr_packet->in_flight = message_packet->in_flight;
				list_enqueue(l1_d_caches[0].mshr, mshr_packet);

				//remove the access from the l1 cache queue and place it in the l2 cache ctrl queue
				list_remove(l1_d_caches[0].Rx_queue_top, message_packet);
				list_enqueue(l2_caches[0].Rx_queue_top, message_packet);

				//Advance the l2 cache controller
				//4 clocks for wire delay.
				//advance(l2_cache_0);
				future_advance(l2_cache_0, (etime.count + 10));

			}


			continue;
		}

		//replies from L2
		if(message_packet->access_type == cgm_access_l2_load_reply)
		{
			//set the block in the L1 I cache
			cgm_cache_set_block(&(l1_d_caches[0]), *set_ptr, *way_ptr, tag, 1);

			//service the mshr request
			mshr_remove(&(l1_d_caches[0]), access_id);

			//remove the message from the in queue
			list_remove(l1_d_caches[0].Rx_queue_top, message_packet);

			//dump in the done queue for the writeback stage, this is a simulator-ism.
			linked_list_add(message_packet->event_queue, message_packet->data);
		}

		else if(message_packet->access_type == cgm_access_l2_store_reply)
		{
			//set the block in the L1 I cache
			cgm_cache_set_block(&(l1_d_caches[0]), *set_ptr, *way_ptr, tag, 1);

			//service the mshr request
			mshr_remove(&(l1_d_caches[0]), access_id);

			//remove the message from the in queue
			list_remove(l1_d_caches[0].Rx_queue_top, message_packet);

			//dump in the done queue for the writeback stage, this is a simulator-ism.
			linked_list_add(message_packet->event_queue, message_packet->data);
		}
		else
		{
			fatal("l1_d_cache_ctrl_0(): unknown L2 message type = %d\n", message_packet->access_type);
		}


	}

	//should never get here
	fatal("l1_d_cache_ctrl_0 task is broken\n");
	return;
}


void l2_cache_ctrl_0(void){

	long long step = 1;
	long long access_id = 0;
	unsigned int addr = 0;
	int list_index = 0;
	int cache_status;
	int i = 0;
	struct cgm_packet_t *message_packet;
	struct cgm_packet_status_t *mshr_packet;

	enum cgm_access_kind_t access_type;
	int tag = 0;
	int set = 0;
	int way = 0;
	int state = 0;

	while(1)
	{


		await(l2_cache_0, step);
		step++;
		//printf("l2_cache_0 time is %lld\n", P_TIME);
		//getchar();

		//message received check queue there should be a message there.
		assert(list_count(l2_caches[0].Rx_queue_top) >= 1);

		//get the message out of the queue
		message_packet = list_get(l2_caches[0].Rx_queue_top, list_index);
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		addr = message_packet->address;
		set = (addr >> l2_caches[0].log_block_size) % l2_caches[0].num_sets;
		tag = addr & ~(l2_caches[0].block_mask);

		/*list_remove(l2_caches[0].Rx_queue_top, message_packet);
		remove_from_global(access_id);*/


		//list_enqueue(l1_i_caches[0].Rx_queue_top, message_packet);
		//check if the block is in the cache
		int *set_ptr = &set;
		int *way_ptr = &way;
		int *state_ptr = &state;


		//Messages from L1_I_Cache
		if (message_packet->access_type == cgm_access_fetch)
		{

			//stats
			l2_caches[0].fetches++;

			cache_status = cgm_cache_find_block(&(l2_caches[0]), addr, set_ptr, way_ptr, state_ptr);

			// L2 Cache Hit!
			if(cache_status == 1)
			{
				l2_caches[0].hits++;

				//This is a hit in the L2 cache need to send up to L1 cache
				//remove packet from l2 cache in queue

				message_packet->access_type = cgm_access_l2_load_reply;

				list_remove(l2_caches[0].Rx_queue_top, message_packet);
				list_enqueue(l1_i_caches[0].Rx_queue_top, message_packet);
				//cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);
				future_advance(l1_i_cache_0, (etime.count + 10));

			}
			else if(cache_status == 0)
			{

				l2_caches[0].misses++;
				//for now pretend that it is the last level of cache and memory ctrl.
				P_PAUSE(mem_miss);

				message_packet->access_type = cgm_access_l2_load_reply;


				cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);

				list_remove(l2_caches[0].Rx_queue_top, message_packet);
				list_enqueue(l1_i_caches[0].Rx_queue_top, message_packet);

				future_advance(l1_i_cache_0, (etime.count + 10));

			}

			continue;
		}

		//Messages from L1_D_Cache
		if (message_packet->access_type == cgm_access_load)
		{
			//stats
			l2_caches[0].loads++;

			cache_status = cgm_cache_find_block(&(l2_caches[0]), addr, set_ptr, way_ptr, state_ptr);

			// L2 Cache Hit!
			if(cache_status == 1)
			{
				//stats
				l2_caches[0].hits++;

				//This is a hit in the L2 cache need to send up to L1 cache
				//remove packet from l2 cache in queue
				message_packet->access_type = cgm_access_l2_load_reply;

				list_remove(l2_caches[0].Rx_queue_top, message_packet);
				list_enqueue(l1_d_caches[0].Rx_queue_top, message_packet);
				//cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);
				future_advance(l1_d_cache_0, (etime.count + 10));
			}
			// L2 Cache Miss!
			else if(cache_status == 0)
			{
				//stats
				l2_caches[0].misses++;
				//for now pretend that it is the last level of cache and memory ctrl.
				P_PAUSE(mem_miss);

				message_packet->access_type = cgm_access_l2_load_reply;


				cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);

				list_remove(l2_caches[0].Rx_queue_top, message_packet);
				list_enqueue(l1_d_caches[0].Rx_queue_top, message_packet);

				future_advance(l1_d_cache_0, (etime.count + 10));

			}

		}
		else if (message_packet->access_type == cgm_access_store)
		{
			//stats
			l2_caches[0].stores++;
			cache_status = cgm_cache_find_block(&(l2_caches[0]), addr, set_ptr, way_ptr, state_ptr);

			// L2 Cache Hit!
			if(cache_status == 1)
			{
				//stats
				l2_caches[0].hits++;

				cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);

				//This is a hit in the L2 cache need to send up to L1 cache
				//remove packet from l2 cache in queue
				message_packet->access_type = cgm_access_l2_store_reply;

				list_remove(l2_caches[0].Rx_queue_top, message_packet);
				list_enqueue(l1_d_caches[0].Rx_queue_top, message_packet);
				//cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);
				future_advance(l1_d_cache_0, (etime.count + 10));
			}
			// L2 Cache Miss!
			else if(cache_status == 0)
			{
				//stats
				l2_caches[0].misses++;

				//for now pretend that it is the last level of cache and memory ctrl.
				P_PAUSE(mem_miss);

				message_packet->access_type = cgm_access_l2_store_reply;

				cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);

				list_remove(l2_caches[0].Rx_queue_top, message_packet);
				list_enqueue(l1_d_caches[0].Rx_queue_top, message_packet);

				future_advance(l1_d_cache_0, (etime.count + 10));
			}

		}

	}

	/* should never get here*/
	fatal("l2_cache_ctrl_0 task is broken\n");
	return;
}

int mshr_remove(struct cache_t *cache, long long access_id){

	int i = 0;
	struct cgm_packet_status_t *mshr_packet;

	LIST_FOR_EACH(cache[0].mshr, i)
	{
		mshr_packet = list_get(cache[0].mshr, i);

		if (mshr_packet->access_id == access_id)
		{
			list_remove_at(cache[0].mshr, i);
			return 1;
		}
	}

	return 0;
}

/* Return {set, tag, offset} for a given address */
void cgm_cache_decode_address(struct cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr)
{
	*(set_ptr) = (addr >> cache->log_block_size) % cache->num_sets;
	*(tag_ptr) = addr & ~(cache->block_mask);
	*(offset_ptr) = addr & (cache->block_mask);
}


/* Look for a block in the cache. If it is found and its state is other than 0,
 * the function returns 1 and the state and way of the block are also returned.
 * The set where the address would belong is returned anyways. */
int cgm_cache_find_block(struct cache_t *cache, unsigned int addr, int *set_ptr, int *way_ptr, int *state_ptr){

	int set, tag, way;

	/* Locate block */
	tag = addr & ~cache->block_mask;
	set = (addr >> cache->log_block_size) % cache->num_sets;

	*(set_ptr) = set;
	*(state_ptr) = 0;

	for (way = 0; way < cache->assoc; way++)
	{
		if (cache->sets[set].blocks[way].tag == tag && cache->sets[set].blocks[way].state)
		{
			break;
		}
	}

	/* Block not found */
	if (way == cache->assoc)
	{
		return 0;
	}

	/* Block found */
	*(way_ptr) = way;
	*(state_ptr) = cache->sets[set].blocks[way].state;
	return 1;
}


/* Set the tag and state of a block.
 * If replacement policy is FIFO, update linked list in case a new
 * block is brought to cache, i.e., a new tag is set. */
void cgm_cache_set_block(struct cache_t *cache, int set, int way, int tag, int state)
{
	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);

	if (cache->policy == cache_policy_fifo && cache->sets[set].blocks[way].tag != tag)
	{
		cgm_cache_update_waylist(&cache->sets[set], &cache->sets[set].blocks[way], cache_waylist_head);
	}

	cache->sets[set].blocks[way].tag = tag;
	cache->sets[set].blocks[way].state = state;
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

	printf("\n---stats---\n\n");

	int num_cores = x86_cpu_num_cores;
	int i = 0;

	printf("Total Cycles %lld\n\n", P_TIME); //P_TIME + 1);
	//printf("current cycle%lld\n\n", Current_Cycle); //P_TIME + 1);

	/*for(i = 0; i < num_cores; i++)
	{*/
		printf("li_i_cache_%d\n", i);
		printf("Number of set %d\n", l1_i_caches[i].num_sets);
		printf("Block size = %d\n", l1_i_caches[i].block_size);
		printf("* cpu accesses %lld\n", l1_i_caches[i].fetches);
		printf("* hits %lld\n", l1_i_caches[i].hits);
		printf("* misses %lld\n", l1_i_caches[i].misses);
		printf("\n");

	/*}

	 for(i = 0; i < num_cores; i++)
	{*/
		printf("li_d_cache_%d\n", i);
		printf("Number of set %d\n", l1_d_caches[i].num_sets);
		printf("Block size = %d\n", l1_d_caches[i].block_size);
		printf("* cpu accesses %lld\n", l1_d_caches[i].loads);
		printf("* hits %lld\n", l1_d_caches[i].hits);
		printf("* misses %lld\n", l1_d_caches[i].misses);
		printf("\n");
	/*}

	for(i = 0; i < num_cores; i++)
	{*/
		printf("li_2_cache_%d\n", i);
		printf("Number of set %d\n", l2_caches[i].num_sets);
		printf("Block size = %d\n", l2_caches[i].block_size);
		//printf("* L1 accesses %lld\n", l2_caches[i]);
		printf("* hits %lld\n", l2_caches[i].hits);
		printf("* misses %lld\n", l2_caches[i].misses);
		printf("\n");
	/*}*/

	return;
}
