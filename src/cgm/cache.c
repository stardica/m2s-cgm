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
long long wire_delay = 10;


//CPU caches
struct cache_t *l1_i_caches;
struct cache_t *l1_d_caches;
struct cache_t *l2_caches;
struct cache_t *l3_caches;
//CPU cache flags
int *l1_i_caches_data;
int *l1_d_caches_data;
int *l2_caches_data;
int *l3_caches_data;


//GPU caches
struct cache_t *gpu_v_caches;
struct cache_t *gpu_s_caches;
struct cache_t *gpu_l2_caches;
struct cache_t *gpu_lds_units;
//GPU cache flags
int *gpu_l2_caches_data;
int *gpu_v_caches_data;
int *gpu_s_caches_data;
int *gpu_lds_units_data;


//event counts
eventcount volatile *l1_i_cache;
eventcount volatile *l1_d_cache;
eventcount volatile *l2_cache;
eventcount volatile *l3_cache;
eventcount volatile *gpu_l2_cache;
eventcount volatile *gpu_v_cache;
eventcount volatile *gpu_s_cache;
eventcount volatile *gpu_lds_unit;


void cache_init(void){

	cache_create();
	cache_create_tasks();

	return;
}

void cache_create(void){


	//star todo make defaults so we don't always have to include cgm_config.ini
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);


	////////////
	//CPU Caches
	////////////

	//initialize the CPU L1 I Caches
	l1_i_caches = (void *) calloc(num_cores, sizeof(struct cache_t));
	l1_i_caches_data = (void *) calloc(num_cores, sizeof(int));

	//initialize the CPU L1 D Caches
	l1_d_caches = (void *) calloc(num_cores, sizeof(struct cache_t));
	l1_d_caches_data = (void *) calloc(num_cores, sizeof(int));

	//initialize the CPU L2 caches
	l2_caches = (void *) calloc(num_cores, sizeof(struct cache_t));
	l2_caches_data = (void *) calloc(num_cores, sizeof(int));

	//initialize the L3 caches (1 slice per core).
	l3_caches = (void *) calloc(num_cores, sizeof(struct cache_t));
	l3_caches_data = (void *) calloc(num_cores, sizeof(int));



	////////////
	//GPU Caches
	////////////


	//initialize the GPU L1V caches
	gpu_v_caches = (void *) calloc(num_cus, sizeof(struct cache_t));
	gpu_v_caches_data = (void *) calloc(num_cus, sizeof(int));

	//initialize the GPU L1S caches
	gpu_s_caches = (void *) calloc(num_cus, sizeof(struct cache_t));
	gpu_s_caches_data = (void *) calloc(num_cus, sizeof(int));

	//initialize the GPU L2 caches.
	gpu_l2_caches = (void *) calloc(gpu_group_cache_num, sizeof(struct cache_t));
	gpu_l2_caches_data = (void *) calloc(gpu_group_cache_num, sizeof(int));

	//initialize the GPU LDS
	gpu_lds_units = (void *) calloc(num_cus, sizeof(struct cache_t));
	gpu_lds_units_data = (void *) calloc(num_cus, sizeof(int));

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
	snprintf(buff, 100, "l1_i_cache");
	l1_i_cache = new_eventcount(strdup(buff));


	//l1 d caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache");
	l1_d_cache = new_eventcount(strdup(buff));


	//l2 caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache");
	l2_cache = new_eventcount(strdup(buff));

	//l3 caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l3_cache");
	l3_cache = new_eventcount(strdup(buff));

	//GPU L2
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "gpu_l2_cache");
	gpu_l2_cache = new_eventcount(strdup(buff));

	//GPU vector
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "gpu_v_cache");
	gpu_v_cache = new_eventcount(strdup(buff));

	//GPU scalar
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "gpu_s_cache");
	gpu_s_cache = new_eventcount(strdup(buff));

	//GPU scalar
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "gpu_lds_unit");
	gpu_lds_unit = new_eventcount(strdup(buff));

	////////////////////
	//tasks
	////////////////////

	//l1 i caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_ctrl");
	create_task(l1_i_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff));


	//l1 d caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_ctrl");
	create_task(l1_d_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	//l2 caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_ctrl");
	create_task(l2_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	//l3 caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l3_cache_ctrl");
	create_task(l3_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	//gpu l2 caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "gpu_l2_cache_ctrl");
	create_task(gpu_l2_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	//gpu v caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "gpu_v_cache_ctrl");
	create_task(gpu_v_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	//gpu s caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "gpu_s_cache_ctrl");
	create_task(gpu_s_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	//gpu lds unit
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "gpu_lds_unit_ctrl");
	create_task(gpu_lds_unit_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	return;
}

void gpu_lds_unit_ctrl(void){

	long long step = 1;

	while(1)
	{
		/*wait here until there is a job to do.
		In any given cycle I might have to service 1 to N number of caches*/
		await(gpu_lds_unit, step);
		step++;

	}

	/* should never get here*/
	fatal("gpu_lds_unit_ctrl task is broken\n");
	return;
}

void gpu_s_cache_ctrl(void){

	long long step = 1;

	while(1)
	{
		/*wait here until there is a job to do.
		In any given cycle I might have to service 1 to N number of caches*/
		await(gpu_s_cache, step);
		step++;

	}

	/* should never get here*/
	fatal("gpu_s_cache_ctrl task is broken\n");
	return;
}


void gpu_v_cache_ctrl(void){

	long long step = 1;


	while(1)
	{
		/*wait here until there is a job to do.
		In any given cycle I might have to service 1 to N number of caches*/
		await(gpu_v_cache, step);
		step++;
	}

	/* should never get here*/
	fatal("gpu_v_cache_ctrl task is broken\n");
	return;
}


void gpu_l2_cache_ctrl(void){

	long long step = 1;


	while(1)
	{
		/*wait here until there is a job to do.
		In any given cycle I might have to service 1 to N number of caches*/
		await(gpu_l2_cache, step);
		step++;

	}

	/* should never get here*/
	fatal("gpu_l2_cache_ctrl task is broken\n");
	return;

}


void l3_cache_ctrl(void){

	long long step = 1;


	while(1)
	{
		/*wait here until there is a job to do.
		In any given cycle I might have to service 1 to N number of caches*/
		await(l3_cache, step);
		step++;

	}

	/* should never get here*/
	fatal("l3_cache_ctrl task is broken\n");
	return;
}


void l1_i_cache_ctrl(void){

	long long step = 1;
	int list_index = 0;
	int num_cores = x86_cpu_num_cores;
	struct cgm_packet_t *message_packet;
	struct cgm_packet_status_t *mshr_packet;
	int id = 0;
	int i = 0;

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

	while(1)
	{
		/*wait here until there is a job to do.
		In any given cycle I might have to service 1 to N number of caches*/
		await(l1_i_cache, step);
		step++;

		//set id to 0
		id = 0;

		//to get here at least one cache has been advanced
		//run through each cache looking for work this cycle
		//note only process one packet per cycle
		while (id < num_cores)
		{

			//printf(" number is is %d round %d\n", l1_i_caches_data[id], id);

			if(l1_i_caches_data[id] > 0)
			{//then there is a task to be done in this cache.

				//decrement the counter
				l1_i_caches_data[id]--;

				//get the message out of the queue
				message_packet = list_get(l1_i_caches[id].Rx_queue_top, list_index);
				assert(message_packet);

				access_type = message_packet->access_type;
				access_id = message_packet->access_id;
				addr = message_packet->address;

				//probe the address for set, tag, and offset.

				//printf("Before probe addr 0x%08x\n", addr);

				cgm_cache_decode_address(&(l1_i_caches[id]), addr, set_ptr, tag_ptr, offset_ptr);

				/*printf("access type %d\n", access_type);
				printf("access id %d\n", access_id);
				printf("addr 0x%08u\n", addr);
				printf("set = %d\n", *set_ptr);
				printf("tag = %d\n", *tag_ptr);
				printf("offset = %u\n", *offset_ptr);
				getchar();*/

				if (access_type == cgm_access_fetch)
				{//then the packet is from the CPU

					//stats
					l1_i_caches[id].fetches++;

					cache_status = cgm_cache_find_block(&(l1_i_caches[id]), addr, set_ptr, way_ptr, state_ptr);

					// L1 I Cache Hit!
					if(cache_status == 1)
					{
						l1_i_caches[id].hits++;

						//Mr. CPU, go about your business...
						//remove packet from cache and global queues
						list_remove(l1_i_caches[id].Rx_queue_top, message_packet);
						remove_from_global(access_id);

					}
					//L1 I Cache Miss!
					else if(cache_status == 0)
					{
						// L1 I Cache Miss!
						l1_i_caches[id].misses++;

						mshr_packet = status_packet_create();

						//drop a token in the mshr queue
						//star todo add some detail to this so we can include coalescing
						mshr_packet->access_type = message_packet->access_type;
						mshr_packet->access_id = message_packet->access_id;
						mshr_packet->in_flight = message_packet->in_flight;
						list_enqueue(l1_i_caches[id].mshr, mshr_packet);

						//remove the access from the l1 cache queue and place it in the l2 cache ctrl queue
						list_remove(l1_i_caches[id].Rx_queue_top, message_packet);
						list_enqueue(l2_caches[id].Rx_queue_top, message_packet);

						//advance the L2 cache adding some wire delay time.
						l2_caches_data[id]++;

						future_advance(l2_cache, (etime.count + l2_caches[id].wire_latency));
					}

				}
				//replies from L2
				else if(access_type == cgm_access_l2_load_reply)
				{//then the packet is from the L2 cache

					//set the block in the L1 I cache
					cgm_cache_set_block(&(l1_i_caches[id]), *set_ptr, *way_ptr, tag, 1);

					//service the mshr request
					mshr_remove(&(l1_i_caches[id]), access_id);

					//remove the message from the in queue
					list_remove(l1_i_caches[id].Rx_queue_top, message_packet);

					//remove from the access tracker, this is a simulator-ism.
					remove_from_global(access_id);

					continue;
				}
				else
				{
					fatal("l1_i_cache_ctrl_0(): unknown L2 message type = %d\n", message_packet->access_type);
				}
			}

			//go to the next cache
			id++;
		}

	}

	/* should never get here*/
	fatal("l1_i_cache_ctrl task is broken\n");
	return;
}

void l2_cache_ctrl(void){

	long long step = 1;
	int list_index = 0;
	int num_cores = x86_cpu_num_cores;
	struct cgm_packet_t *message_packet;
	struct cgm_packet_status_t *mshr_packet;
	int id = 0;

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

	while(1)
	{

		/*wait here until there is a job to do.
		In any given cycle I might have to service 1 to N number of caches*/
		await(l2_cache, step);
		step++;

		//set id to 0
		id = 0;

		//to get here at least one cache has been advanced
		//run through each cache looking for work this cycle
		//note only process one packet per cycle
		while (id < num_cores)
		{

			if(l2_caches_data[id] > 0)
			{//then there is a task to be done in this cache.

				//decrement the counter
				l2_caches_data[id]--;

				//get the message out of the queue
				message_packet = list_get(l2_caches[id].Rx_queue_top, list_index);
				assert(message_packet);

				access_type = message_packet->access_type;
				access_id = message_packet->access_id;
				addr = message_packet->address;

				//probe the address for set, tag, and offset.
				cgm_cache_decode_address(&(l2_caches[id]), addr, set_ptr, tag_ptr, offset_ptr);

				/*printf("access type %d\n", access_type);
				printf("access id %d\n", access_id);
				printf("addr 0x%08u\n", addr);
				printf("set = %d\n", *set_ptr);
				printf("tag = %d\n", *tag_ptr);
				printf("offset = %u\n", *offset_ptr);
				getchar();*/

				//Messages from L1_I_Cache
				if (access_type == cgm_access_fetch)
				{

					//stats
					l2_caches[id].fetches++;

					cache_status = cgm_cache_find_block(&(l2_caches[id]), addr, set_ptr, way_ptr, state_ptr);

					// L2 Cache Hit!
					if(cache_status == 1)
					{
						l2_caches[id].hits++;

						//This is a hit in the L2 cache need to send up to L1 cache
						//remove packet from l2 cache in queue

						message_packet->access_type = cgm_access_l2_load_reply;

						list_remove(l2_caches[id].Rx_queue_top, message_packet);
						list_enqueue(l1_i_caches[id].Rx_queue_top, message_packet);
						//cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);
						l1_i_caches_data[id]++;
						future_advance(l1_i_cache, (etime.count + l1_i_caches[id].wire_latency));

					}
					else if(cache_status == 0)
					{

						l2_caches[id].misses++;
						//for now pretend that it is the last level of cache and memory ctrl.
						P_PAUSE(mem_miss);

						message_packet->access_type = cgm_access_l2_load_reply;


						cgm_cache_set_block(&(l2_caches[id]), *set_ptr, *way_ptr, tag, 1);

						list_remove(l2_caches[id].Rx_queue_top, message_packet);
						list_enqueue(l1_i_caches[id].Rx_queue_top, message_packet);

						l1_i_caches_data[id]++;
						future_advance(l1_i_cache, (etime.count + l1_i_caches[id].wire_latency));
					}
				}
				//Messages from L1_D_Cache
				else if (access_type == cgm_access_load)
				{
					//stats
					l2_caches[id].loads++;

					cache_status = cgm_cache_find_block(&(l2_caches[id]), addr, set_ptr, way_ptr, state_ptr);

					// L2 Cache Hit!
					if(cache_status == 1)
					{
						//stats
						l2_caches[id].hits++;

						//This is a hit in the L2 cache need to send up to L1 cache
						//remove packet from l2 cache in queue
						message_packet->access_type = cgm_access_l2_load_reply;

						list_remove(l2_caches[id].Rx_queue_top, message_packet);
						list_enqueue(l1_d_caches[id].Rx_queue_top, message_packet);
						//cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);

						l1_d_caches_data[id]++;
						future_advance(l1_d_cache, (etime.count + l1_d_caches[id].wire_latency));
					}
					// L2 Cache Miss!
					else if(cache_status == 0)
					{
						//stats
						l2_caches[id].misses++;
						//for now pretend that it is the last level of cache and memory ctrl.
						P_PAUSE(mem_miss);

						message_packet->access_type = cgm_access_l2_load_reply;


						cgm_cache_set_block(&(l2_caches[id]), *set_ptr, *way_ptr, tag, 1);

						list_remove(l2_caches[id].Rx_queue_top, message_packet);
						list_enqueue(l1_d_caches[id].Rx_queue_top, message_packet);

						l1_d_caches_data[id]++;
						future_advance(l1_d_cache, (etime.count + l1_d_caches[id].wire_latency));
					}

				}
				else if (access_type == cgm_access_store)
				{
					//stats
					l2_caches[id].stores++;
					cache_status = cgm_cache_find_block(&(l2_caches[id]), addr, set_ptr, way_ptr, state_ptr);

					// L2 Cache Hit!
					if(cache_status == 1)
					{
						//stats
						l2_caches[id].hits++;

						cgm_cache_set_block(&(l2_caches[id]), *set_ptr, *way_ptr, tag, 1);

						//This is a hit in the L2 cache need to send up to L1 cache
						//remove packet from l2 cache in queue
						message_packet->access_type = cgm_access_l2_store_reply;

						list_remove(l2_caches[id].Rx_queue_top, message_packet);
						list_enqueue(l1_d_caches[id].Rx_queue_top, message_packet);
						//cgm_cache_set_block(&(l2_caches[0]), *set_ptr, *way_ptr, tag, 1);

						l1_d_caches_data[id]++;
						future_advance(l1_d_cache, (etime.count + l1_d_caches[id].wire_latency));
					}
					// L2 Cache Miss!
					else if(cache_status == 0)
					{
						//stats
						l2_caches[id].misses++;

						//for now pretend that it is the last level of cache and memory ctrl.
						P_PAUSE(mem_miss);

						message_packet->access_type = cgm_access_l2_store_reply;

						cgm_cache_set_block(&(l2_caches[id]), *set_ptr, *way_ptr, tag, 1);

						list_remove(l2_caches[id].Rx_queue_top, message_packet);
						list_enqueue(l1_d_caches[id].Rx_queue_top, message_packet);

						l1_d_caches_data[id]++;
						future_advance(l1_d_cache, (etime.count + l1_d_caches[id].wire_latency));
					}

				}

			}

			//go to the next cache
			id++;
		}

	}
	/* should never get here*/
	fatal("l2_cache_ctrl task is broken\n");
	return;
}

void l1_d_cache_ctrl(void){

	long long step = 1;
	int list_index = 0;
	int num_cores = x86_cpu_num_cores;
	struct cgm_packet_t *message_packet;
	struct cgm_packet_status_t *mshr_packet;
	int id = 0;

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

	while(1)
	{
		/*wait here until there is a job to do.
		In any given cycle I might have to service 1 to N number of caches*/
		await(l1_d_cache, step);
		step++;

		//set id to 0
		id = 0;

		//to get here at least one cache has been advanced
		//run through each cache looking for work this cycle
		//note only process one packet per cycle
		while (id < num_cores)
		{

			//printf("loop id %d\n", id);

			if(l1_d_caches_data[id] > 0)
			{//then there is a task to be done in this cache.

				//decrement the counter
				l1_d_caches_data[id]--;

				//get the message out of the queue
				message_packet = list_get(l1_d_caches[id].Rx_queue_top, list_index);
				assert(message_packet);

				access_type = message_packet->access_type;
				access_id = message_packet->access_id;
				addr = message_packet->address;

				/*printf("access type %d\n", access_type);
				printf("access id %d\n", access_id);
				printf("addr 0x%08u\n", addr);
				getchar();*/

				//probe the address for set, tag, and offset.
				cgm_cache_decode_address(&(l1_d_caches[id]), addr, set_ptr, tag_ptr, offset_ptr);

				//request from CPU
				if (access_type == cgm_access_load)
				{//then the packet is from the CPU

					//stats
					l1_d_caches[id].loads++;
					cache_status = cgm_cache_find_block(&(l1_d_caches[id]), addr, set_ptr, way_ptr, state_ptr);

					if(cache_status == 1)
					{//then L1 D Cache Hit!

						l1_d_caches[id].hits++;

						//Mr. CPU, go about your business...
						//remove packet from cache queue and add to to commit stage input
						list_remove(l1_d_caches[id].Rx_queue_top, message_packet);
						linked_list_add(message_packet->event_queue, message_packet->data);

					}
					else if(cache_status == 0)
					{//then L1 D Cache Miss!

						l1_d_caches[id].misses++;

						mshr_packet = status_packet_create();

						//drop a token in the mshr queue
						//star todo add some detail to this so we can include coalescing
						mshr_packet->access_type = message_packet->access_type;
						mshr_packet->access_id = message_packet->access_id;
						mshr_packet->in_flight = message_packet->in_flight;
						list_enqueue(l1_d_caches[id].mshr, mshr_packet);

						//remove the access from the l1 cache queue and place it in the l2 cache ctrl queue
						list_remove(l1_d_caches[id].Rx_queue_top, message_packet);
						list_enqueue(l2_caches[id].Rx_queue_top, message_packet);

						//Advance the l2 cache controller
						//4 clocks for wire delay.
						//advance(l2_cache_0);
						l2_caches_data[id]++;
						future_advance(l2_cache, (etime.count + l2_caches[id].wire_latency));
					}

				}
				else if (access_type == cgm_access_store)
				{

					//star todo evict old block this is where the LRU, FIFO stuff comes into play
					//this needs some work to get it right

					//stats
					l1_d_caches[id].stores++;

					cache_status = cgm_cache_find_block(&(l1_d_caches[id]), addr, set_ptr, way_ptr, state_ptr);

					// L1 D Cache Hit!
					if(cache_status == 1)
					{
						//ok, on a hit this means there is a block of old memory in the cache (i.e. to be over written).
						l1_d_caches[id].hits++;

						//for now just set it so things will run
						cgm_cache_set_block(&(l1_i_caches[id]), *set_ptr, *way_ptr, tag, 1);

						//Mr. CPU, go about your business...
						//remove packet from cache queue and add to commit stage input
						list_remove(l1_d_caches[id].Rx_queue_top, message_packet);
						linked_list_add(message_packet->event_queue, message_packet->data);

					}
					//L1 D Cache Miss!
					else if(cache_status == 0)
					{
						// L1 I Cache Miss!
						l1_d_caches[id].misses++;

						mshr_packet = status_packet_create();

						//drop a token in the mshr queue
						//star todo add some detail to this so we can include coalescing
						mshr_packet->access_type = message_packet->access_type;
						mshr_packet->access_id = message_packet->access_id;
						mshr_packet->in_flight = message_packet->in_flight;
						list_enqueue(l1_d_caches[id].mshr, mshr_packet);

						//remove the access from the l1 cache queue and place it in the l2 cache ctrl queue
						list_remove(l1_d_caches[id].Rx_queue_top, message_packet);
						list_enqueue(l2_caches[id].Rx_queue_top, message_packet);

						//Advance the l2 cache controller
						//4 clocks for wire delay.
						//advance(l2_cache_0);

						l2_caches_data[id]++;
						future_advance(l2_cache, (etime.count + l2_caches[id].wire_latency));
					}
				}
				//replies from L2
				else if(access_type == cgm_access_l2_load_reply)
				{
					//set the block in the L1 I cache
					cgm_cache_set_block(&(l1_d_caches[id]), *set_ptr, *way_ptr, tag, 1);

					//service the mshr request
					mshr_remove(&(l1_d_caches[id]), access_id);

					//remove the message from the in queue
					list_remove(l1_d_caches[id].Rx_queue_top, message_packet);

					//dump in the done queue for the writeback stage, this is a simulator-ism.
					linked_list_add(message_packet->event_queue, message_packet->data);
				}
				else if(access_type == cgm_access_l2_store_reply)
				{
					//set the block in the L1 I cache
					cgm_cache_set_block(&(l1_d_caches[id]), *set_ptr, *way_ptr, tag, 1);

					//service the mshr request
					mshr_remove(&(l1_d_caches[id]), access_id);

					//remove the message from the in queue
					list_remove(l1_d_caches[id].Rx_queue_top, message_packet);

					//dump in the done queue for the writeback stage, this is a simulator-ism.
					linked_list_add(message_packet->event_queue, message_packet->data);
				}
				else
				{
					fatal("l1_d_cache_ctrl_0(): unknown L2 message type = %d\n", message_packet->access_type);
				}

			}

			//go to the next cache
			id++;
		}

	}

	//should never get here
	fatal("l1_d_cache_ctrl task is broken\n");
	return;
}



int mshr_remove(struct cache_t *cache, long long access_id){

	int i = 0;
	struct cgm_packet_status_t *mshr_packet;

	LIST_FOR_EACH(cache->mshr, i)
	{
		mshr_packet = list_get(cache->mshr, i);

		if (mshr_packet->access_id == access_id)
		{
			list_remove_at(cache->mshr, i);
			return 1;
		}
	}

	return 0;
}

/* Return {set, tag, offset} for a given address */
void cgm_cache_decode_address(struct cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr)
{

	/*printf("After probe addr 0x%08x\n", addr);

	printf("cache->log_block_size = %d\n",cache->log_block_size);
	printf("cache->block_mask %d\n", cache->block_mask);
	printf("\n");*/

	*(set_ptr) = (addr >> cache->log_block_size) % cache->num_sets;
	*(tag_ptr) = addr & ~(cache->block_mask);
	*(offset_ptr) = addr & (cache->block_mask);

	//notes this is useing the tag and indx to calculate set location.

	/*printf("---set_ptr---\n");
	printf("Addr 0x%08x\n", addr);
	printf("(addr >> cache->log_block_size) = 0x%08x\n", addr >> cache->log_block_size);
	printf("set_ptr %d\n", (addr >> cache->log_block_size) % cache->num_sets);
	printf("---set_ptr---\n");

	printf("---tag_ptr---\n");
	printf("Addr 0x%08x\n", addr);
	printf("~(cache->block_mask) 0x%08x\n", ~(cache->block_mask));
	printf("addr & ~(cache->block_mask) 0x%08x\n", addr & ~(cache->block_mask));
	printf("---tag_ptr---\n");

	printf("---offset_ptr---\n");
	printf("Addr 0x%08x\n", addr);
	printf("(cache->block_mask) 0x%08x\n", (cache->block_mask));
	printf("addr & (cache->block_mask) 0x%08x\n", addr & addr & (cache->block_mask));
	printf("---offset_ptr---\n");
	getchar();*/


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
