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
#include <cgm/tasking.h>
#include <cgm/packet.h>
#include <cgm/switch.h>
#include <cgm/protocol.h>
#include <cgm/mshr.h>


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
int mem_miss = 100;


//CPU caches
struct cache_t *l1_i_caches;
struct cache_t *l1_d_caches;
struct cache_t *l2_caches;
struct cache_t *l3_caches;

//GPU caches
struct cache_t *gpu_v_caches;
struct cache_t *gpu_s_caches;
struct cache_t *gpu_l2_caches;
struct cache_t *gpu_lds_units;

//global tasking related
int l1_i_pid = 0;
int l1_d_pid = 0;
int l2_pid = 0;
int l3_pid = 0;
int gpu_v_pid = 0;
int gpu_s_pid = 0;
int gpu_l2_pid = 0;
int gpu_lds_pid = 0;


//event counts
eventcount volatile *l1_i_cache;
eventcount volatile *l1_d_cache;
eventcount volatile *l2_cache;
eventcount volatile *l3_cache;
eventcount volatile *gpu_l2_cache;
eventcount volatile *gpu_v_cache;
eventcount volatile *gpu_s_cache;
eventcount volatile *gpu_lds_unit;

task *l1_i_cache_tasks;
task *l1_d_cache_tasks;
task *l2_cache_tasks;
task *l3_cache_tasks;
task *gpu_l2_cache_tasks;
task *gpu_v_cache_tasks;
task *gpu_s_cache_tasks;
task *gpu_lds_tasks;

void cache_init(void){

	cache_create();
	cache_create_tasks();

	return;
}

void cache_create(void){


	//star todo make defaults so we don't always have to include cgm_config.ini
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	//int gpu_group_cache_num = (num_cus/4);


	////////////
	//CPU Caches
	////////////

	//initialize the CPU L1 I Caches
	l1_i_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	//initialize the CPU L1 D Caches
	l1_d_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	//initialize the CPU L2 caches
	l2_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	//initialize the L3 caches (1 slice per core).
	l3_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	////////////
	//GPU Caches
	////////////

	//initialize the GPU L1V caches
	gpu_v_caches = (void *) calloc(num_cus, sizeof(struct cache_t));

	//initialize the GPU L1S caches
	gpu_s_caches = (void *) calloc(num_cus, sizeof(struct cache_t));

	//initialize the GPU L2 caches.
	gpu_l2_caches = (void *) calloc(num_cus, sizeof(struct cache_t));

	//initialize the GPU LDS
	gpu_lds_units = (void *) calloc(num_cus, sizeof(struct cache_t));

	return ;
}

void cache_create_tasks(void){

	//star todo make this dynamic
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	//int num_cus_4 = (num_cus / 4);
	char buff[100];
	int i = 0;

	/////////////
	//eventcounts
	/////////////

	//l1_i_caches
	l1_i_cache = (void *) calloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_cache_%d", i);
		l1_i_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//l1 d caches
	l1_d_cache = (void *) calloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_cache_%d", i);
		l1_d_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//l2 caches
	l2_cache = (void *) calloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l2_cache_%d", i);
		l2_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//l3 caches
	l3_cache = (void *) calloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l3_cache_%d", i);
		l3_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//GPU L2
	gpu_l2_cache = (void *) calloc(num_cus, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_cache_%d", i);
		gpu_l2_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//GPU vector
	gpu_v_cache = (void *) calloc(num_cus, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_cache_%d", i);
		gpu_v_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//GPU scalar
	gpu_s_cache = (void *) calloc(num_cus, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_cache_%d", i);
		gpu_s_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//GPU lds
	gpu_lds_unit = (void *) calloc(num_cus, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_lds_unit_%d", i);
		gpu_lds_unit[i] = *(new_eventcount(strdup(buff)));
	}


	////////////////////
	//tasks
	////////////////////


	//l1 i caches
	l1_i_cache_tasks = (void *) calloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_cache_ctrl_%d", i);
		l1_i_cache_tasks[i] = *(create_task(l1_i_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
		//printf("l1_i_cache_tasks[i].id = %d name = %s\n", l1_i_cache_tasks[i].id, l1_i_cache_tasks[i].name);
	}

	//getchar();

	//l1 d caches
	l1_d_cache_tasks = (void *) calloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_cache_ctrl_%d", i);
		l1_d_cache_tasks[i] = *(create_task(l1_d_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//l2 caches
	l2_cache_tasks = (void *) calloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l2_cache_ctrl_%d", i);
		l2_cache_tasks[i] = *(create_task(l2_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//l3 caches
	l3_cache_tasks = (void *) calloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l3_cache_ctrl_%d", i);
		l3_cache_tasks[i] = *(create_task(l3_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//gpu l2 caches
	gpu_l2_cache_tasks = (void *) calloc(num_cus, sizeof(task));
	for(i = 0; i < num_cus; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_cache_ctrl");
		gpu_l2_cache_tasks[i] = *(create_task(gpu_l2_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//gpu v caches
	gpu_v_cache_tasks = (void *) calloc(num_cus, sizeof(task));
	for(i = 0; i < num_cus; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_cache_ctrl");
		gpu_v_cache_tasks[i] = *(create_task(gpu_v_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//gpu s caches
	gpu_s_cache_tasks = (void *) calloc(num_cus, sizeof(task));
	for(i = 0; i < num_cus; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_cache_ctrl");
		gpu_s_cache_tasks[i] = *(create_task(gpu_s_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//gpu lds unit
	gpu_lds_tasks = (void *) calloc(num_cus, sizeof(task));
	for(i = 0; i < num_cus; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_lds_unit_ctrl");
		gpu_lds_tasks[i] = *(create_task(gpu_lds_unit_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	return;
}


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
	int mshr_status = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;

	int retry = 0;
	int *retry_ptr = &retry;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{

		//wait here until there is a job to do
		await(&l1_i_cache[my_pid], step);
		step++;

		//check the top or bottom rx queues for messages.
		message_packet = get_message(&(l1_i_caches[my_pid]), retry_ptr, mshr_status);

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

			if (access_id == 1)
			{
				printf("L1\n");
				printf("access id %llu\n", access_id);
				printf("access type %d\n", access_type);
				printf("addr 0x%08u\n", addr);
				printf("set = %d\n", *set_ptr);
				printf("tag = %d\n", *tag_ptr);
				printf("offset = %u\n", *offset_ptr);
				getchar();
			}

			//get the block and the state of the block and charge a cycle
			cache_status = cgm_cache_find_block(&(l1_i_caches[my_pid]), tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
			P_PAUSE(1);

			//L1 I Cache Hit!
			if(cache_status == 1 && *state_ptr != 0)
			{

				if (access_id == 1)
				{
					printf("access id %llu l1 hit\n", access_id);
					getchar();
				}

				if(access_type == cgm_access_fetch)
					l1_i_caches[my_pid].hits++;

				//remove packet from cache queue, global queue, and simulator memory
				//note cycle already charged
				list_remove(l1_i_caches[my_pid].Rx_queue_top, message_packet);
				remove_from_global(access_id);
				free(message_packet);

			}

			//L1 I Cache Miss!
			else if(cache_status == 0 || *state_ptr == 0)
			{

				//star todo there is a bug here 1 access fails retry in our MM.
				//assert(access_type != cgm_access_retry);
				if(access_type == cgm_access_fetch)
					l1_i_caches[my_pid].misses++;

				if (access_id == 1)
				{
					printf("access id %llu l1 miss\n", access_id);
					getchar();
				}

				miss_status_packet = miss_status_packet_create(message_packet->access_id, message_packet->access_type, set, tag, offset);

				if(mshr_set(&(l1_i_caches[my_pid]), miss_status_packet))
				{

					//while the next level of cache's in queue is full stall
					while(!cache_can_access(&l2_caches[my_pid]))
					{
						/*printf("access id %llu l1 miss in while\n", access_id);
						getchar();*/
						P_PAUSE(1);
					}

					//change the access type for the coherence protocol and drop into the L2's queue
					//remove the access from the l1 cache queue and place it in the l2 cache ctrl queue
					message_packet->access_type = cgm_access_gets_i;
					list_remove(l1_i_caches[my_pid].Rx_queue_top, message_packet);
					list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);

					//advance the L2 cache adding some wire delay time.
					future_advance(&l2_cache[my_pid], (etime.count + l2_caches[my_pid].wire_latency));

					/*if (access_id == 1)
					{
						printf("access id %llu l1 miss 2\n", access_id);
						getchar();
					}*/

				}
				else
				{
					//mshr is full so we can't progress, retry.
					message_packet->access_type = cgm_access_retry;
					future_advance(&l1_i_cache[my_pid], (etime.count + 2));
				}

				//done

			}
		}

		else if(access_type == cgm_access_puts)
		{//then the packet is from the L2 cache

			//probe the address for set, tag, and offset.
			addr = message_packet->address;
			access_id = message_packet->access_id;
			cgm_cache_decode_address(&(l1_i_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);

			//charge the delay for writing cache block
			cgm_cache_set_block(&l1_i_caches[my_pid], *set_ptr, *way_ptr, tag, cache_block_shared);
			P_PAUSE(1);

			//get the mshr status
			mshr_status = mshr_get(&l1_i_caches[my_pid], set_ptr, tag_ptr);

			if(mshr_status)
			{
				//we have outstanding mshr requests
				message_packet->

			}
			//mshr_remove(&l1_i_caches[my_pid], access_id);

			//charge the delay for servicing the older request in the MSHR
			//advance the l1_i_cache, on the next cycle the request should be a hit




			message_packet->access_type = cgm_access_retry;
			advance(&l1_i_cache[my_pid]);
			//done.

		}

		else
		{
			fatal("l1_i_cache_ctrl_0(): unknown L2 message type = %d\n", message_packet->access_type);
		}

	}

	/* should never get here*/
	fatal("l1_i_cache_ctrl task is broken\n");
	return;
}

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


	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{

		/*wait here until there is a job to do.*/
		await(&l2_cache[my_pid], step);
		step++;

		//get the message out of the queue
		message_packet = list_get(l2_caches[my_pid].Rx_queue_top, 0);
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		addr = message_packet->address;

		//probe the address for set, tag, and offset.
		cgm_cache_decode_address(&(l2_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);

		if(access_id == 1)
		{
			printf("L2\n");
			printf("access id %llu\n", access_id);
			printf("access type %d\n", access_type);
			printf("addr 0x%08u\n", addr);
			printf("set = %d\n", *set_ptr);
			printf("tag = %d\n", *tag_ptr);
			printf("offset = %u\n", *offset_ptr);
			getchar();
		}

		//Messages from L1_I_Cache
		//star todo join this with l1 d cache gets somehow
		if (access_type == cgm_access_gets_i || cgm_access_retry_i)
		{

			//stats
			if(access_type == cgm_access_gets_i)
				l2_caches[my_pid].loads++;

			if(access_type == cgm_access_retry_i)
				l2_caches[my_pid].retries;


			//look up, and charge a cycle.
			cache_status = cgm_cache_find_block(&l2_caches[my_pid], tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
			P_PAUSE(1);

			// L2 Cache Hit!
			if(cache_status == 1 && *state_ptr != 0)
			{

				if(access_id == 1)
				{
					printf("access id %llu l2 hit\n", access_id);
					getchar();
				}


				if(access_type == cgm_access_gets_i)
					l2_caches[my_pid].hits++;


				//This is a hit in the L2 cache need to send up to L1 cache

				//while the next level of cache's in queue is full stall
				//star todo possible deadlock situation if both the l2 and core are trying to fill a full queue
				while(!cache_can_access(&l1_i_caches[my_pid]))
				{
					P_PAUSE(1);
				}

				//success
				//remove packet from l2 cache in queue
				list_remove(l2_caches[my_pid].Rx_queue_top, message_packet);

				//change access type, i cache only ever reads so puts is ok.
				message_packet->access_type = cgm_access_puts;
				list_enqueue(l1_i_caches[my_pid].Rx_queue_top, message_packet);
				future_advance(&l1_i_cache[my_pid], (etime.count + l1_i_caches[my_pid].wire_latency));

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
			mshr_packet = mshr_remove(&l2_caches[my_pid], access_id);
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

struct cgm_packet_t *get_message(struct cache_t *cache, int *retry_ptr, int mshr_status){

	//star this is round robin
	struct cgm_packet_t *new_message;
	int i = 0;

	if(*retry_ptr > 0)
	{
		//retry the outstanding misses in the mshr
		new_message = mshr_remove(

				list_dequeue(cache->mshrs[mshr_status].entires, 0);



		*retry_ptr--;

	}
	else if(*retry_ptr == 0)
	{
		//check for a message
		new_message = list_get(cache->next_queue, 0);

		//rotate the queues
		if(cache->next_queue == cache->Rx_queue_top)
		{
			cache->next_queue = cache->Rx_queue_bottom;
		}
		else if(cache->next_queue == cache->Rx_queue_bottom)
		{
			cache->next_queue = cache->Rx_queue_top;
		}
		else
		{
			fatal("get_message() pointers arn't working");
		}

		//if we didn't get a message try again (now that the queues are rotated)
		if(!new_message)
		{
			//no message from last queue, try again
			new_message = list_get(cache->next_queue, 0);

			//rotate the queues.
			if(cache->next_queue == cache->Rx_queue_top)
			{
				cache->next_queue = cache->Rx_queue_bottom;
			}
			else if(cache->next_queue == cache->Rx_queue_bottom)
			{
				cache->next_queue = cache->Rx_queue_top;
			}
			else
			{
				fatal("get_message() pointers arn't working");
			}

		}

	}

	//shouldn't be exiting without a message
	assert(new_message);

	return new_message;
}




//star todo this isn't for here, but remember to change the access type when needed getx putx etc.
int cache_can_access(struct cache_t *cache){


	//check if in queue is full
	if(QueueSize <= list_count(cache->Rx_queue_top))
	{
		return 0;
	}

	//cache queue is accessible.
	return 1;
}


/* Return {tag, set, offset} for a given address */
void cgm_cache_decode_address(struct cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr)
{
	//star i reworked this a little
	*(tag_ptr) = (addr >> (cache->log_block_size + cache->log_set_size));//addr & ~(cache->block_mask);
	*(set_ptr) =  (addr >> (cache->log_block_size) & (cache->set_mask));//(addr >> cache->log_block_size) % cache->num_sets;
	*(offset_ptr) = addr & (cache->block_mask);

}


/* Look for a block in the cache. If it is found and its state is other than 0,
 * the function returns 1 and the state and way of the block are also returned.
 * The set where the address would belong is returned anyways. */
int cgm_cache_find_block(struct cache_t *cache, int *tag_ptr, int *set_ptr, unsigned int *offset_ptr, int *way_ptr, int *state_ptr){

	int set, tag, way;
	//unsigned int offset;

	/* Locate block */
	tag = *(tag_ptr);
	set = *(set_ptr);
	//offset = *(offset_ptr);

	*(state_ptr) = 10;

	for (way = 0; way < cache->assoc; way++)
	{
		//if (cache->sets[set].blocks[way].tag == tag && cache->sets[set].blocks[way].state)
		if (cache->sets[set].blocks[way].tag == tag)
		{
			/* Block found */
			*(way_ptr) = way;
			*(state_ptr) = cache->sets[set].blocks[way].state;
			return 1;
		}
	}

	/* Block not found */
	if (way == cache->assoc)
	{
		return 0;
	}
	else
	{
		fatal("cgm_cache_find_block() incorrect behavior\n");
	}

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
