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
#include <cgm/sys-agent.h>
#include <cgm/protocol.h>
#include <cgm/mshr.h>


#include <instrumentation/stats.h>

struct str_map_t cgm_mem_access_strn_map =
{ 	num_access_types, 	{
		{"cgm_access_invalid" ,cgm_access_invalid},
		{"cgm_access_fetch" ,cgm_access_fetch},
		{"cgm_access_load" ,cgm_access_load},
		{"cgm_access_store" ,cgm_access_store},
		{"cgm_access_nc_store" ,cgm_access_nc_store},
		{"cgm_access_nc_load" ,cgm_access_nc_load},
		{"cgm_access_prefetch" ,cgm_access_prefetch},
		{"cgm_access_gets" ,cgm_access_gets},
		{"cgm_access_gets_i" ,cgm_access_gets_i},
		{"cgm_access_gets_d", cgm_access_gets_d},
		{"cgm_access_gets_s", cgm_access_gets_s},
		{"cgm_access_gets_v", cgm_access_gets_v},
		{"cgm_access_getx" ,cgm_access_getx},
		{"cgm_access_inv" ,cgm_access_inv},
		{"cgm_access_putx" ,cgm_access_putx},
		{"cgm_access_puts" ,cgm_access_puts},
		{"cgm_access_puto" ,cgm_access_puto},
		{"cgm_access_puto_shared" ,cgm_access_puto_shared},
		{"cgm_access_unblock" ,cgm_access_unblock},
		{"cgm_access_retry" ,cgm_access_retry},
		{"cgm_access_retry_i" ,cgm_access_retry_i},
		{"num_access_types", num_access_types}
		}
};

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
		{ "I", cache_block_invalid},
		{ "N", cache_block_noncoherent },
		{ "M", cache_block_modified },
		{ "O", cache_block_owned },
		{ "E", cache_block_exclusive },
		{ "S", cache_block_shared },
		}
};

int QueueSize;
int gpu_l2_qty;
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
	int gpu_group_cache_num = (num_cus/4);


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
	gpu_l2_caches = (void *) calloc(gpu_group_cache_num, sizeof(struct cache_t));

	//initialize the GPU LDS
	gpu_lds_units = (void *) calloc(num_cus, sizeof(struct cache_t));

	return ;
}

void cache_create_tasks(void){

	//star todo make this dynamic
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
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
	gpu_l2_cache = (void *) calloc(gpu_group_cache_num, sizeof(eventcount));
	for(i = 0; i < gpu_group_cache_num; i++)
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
	gpu_l2_cache_tasks = (void *) calloc(gpu_group_cache_num, sizeof(task));
	for(i = 0; i < gpu_group_cache_num; i++)
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

struct cgm_packet_t *cache_get_message(struct cache_t *cache){

	//star this is round robin
	struct cgm_packet_t *new_message;
	//printf ("cache->name %s\n", cache->name);

	int retry_queue_size = list_count(cache->retry_queue);

	//pull from the retry queue first.
	if(retry_queue_size > 0)
	{
		new_message = list_get(cache->retry_queue, 0);
		cache->last_queue = cache->retry_queue;
	}
	else
	{
		new_message = list_get(cache->next_queue, 0);

		//keep pointer to last queue
		cache->last_queue = cache->next_queue;

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
		if(new_message == NULL)
		{

			new_message = list_get(cache->next_queue, 0);

			//keep pointer to last queue
			cache->last_queue = cache->next_queue;

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

	CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu pulled from %s queue size %d\n",
			cache->name, new_message->access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

	//shouldn't be exiting without a message
	assert(new_message != NULL);
	return new_message;
}

void cpu_l1_cache_access_load(struct cache_t *cache, struct cgm_packet_t *message_packet){

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
	int mshr_status = -1;
	int mshr_row = -1;

	int i = 0;

	//stats
	cache->loads++;

	//access information
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	//store the decode
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;


	CGM_DEBUG(CPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//////testing
	if(cache->cache_type == l1_d_cache_t)// || cache->cache_type == l1_i_cache_t)
	{
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_shared);
	}
	//////testing

	//get the block and the state of the block and charge cycles
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(cache->latency);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{
		assert(*state_ptr != cache_block_invalid);
		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

		if(*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
		{
			cache->hits++;

			//CPU L1 I cache
			if(message_packet->access_type == cgm_access_fetch)
			{
				//remove packet from cache queue, global queue, and simulator memory
				list_remove(cache->last_queue, message_packet);
				remove_from_global(access_id);
			}
			//CPU L1 D cache
			if(message_packet->access_type == cgm_access_load)
			{
				list_remove(cache->last_queue, message_packet);
				linked_list_add(message_packet->event_queue, message_packet->data);
			}

		}
		else
		{
			fatal("cpu_l1_cache_access_load(): incorrect block state set");
		}

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu hit state %s\n", cache->name, access_id, P_TIME, str_map_value(&cache_block_state_map, *state_ptr));
	}
	//Cache miss or cache block is invalid
	else if(cache_status == 0 || *state_ptr == 0)
	{
		cache->misses++;

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		//CPU L1 I cache
		if(message_packet->access_type == cgm_access_fetch)
		{
			message_packet->cpu_access_type = cgm_access_fetch;
			message_packet->access_type = cgm_access_gets_i;
			message_packet->l1_access_type = cgm_access_gets_i;
		}
		//CPU L1 D cache
		else if(message_packet->access_type == cgm_access_load)
		{
			message_packet->cpu_access_type = cgm_access_load;
			message_packet->access_type = cgm_access_gets_d;
			message_packet->l1_access_type = cgm_access_gets_d;
		}
		else
		{
			fatal("cpu_l1_cache_access_load(): invalid CPU l1 cache access type access_id %llu cycle %llu", access_id, P_TIME);
		}


		//miss so check ORT status
		for (i = 0; i <  cache->mshr_size; i++)
		{
			if(cache->ort[i][0] == tag && cache->ort[i][1] == set && cache->ort[i][2] == 1)
			{
				//hit in the ORT table
				break;
			}
		}



		//entry was not found
		if(i == cache->mshr_size)
		{
			//get an empty row
			for (i = 0; i <  cache->mshr_size; i++)
			{
				if(cache->ort[i][0] == -1 && cache->ort[i][1] == -1 && cache->ort[i][2] == -1)
				{
					//found empty row
					break;
				}
			}

			//sanity check the table row
			assert(i < cache->mshr_size);
			assert(cache->ort[i][0] == -1);
			assert(cache->ort[i][1] == -1);
			assert(cache->ort[i][2] == -1);

			//insert into table
			cache->ort[i][0] = tag;
			cache->ort[i][1] = set;
			cache->ort[i][2] = 1;

			//forward message_packet
			while(!cache_can_access_top(&l2_caches[cache->id]))
			{
				//printf("stall\n");
				P_PAUSE(1);
			}

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu l2 queue free size %d\n",
					cache->name, access_id, P_TIME, list_count(l2_caches[cache->id].Rx_queue_top));

			P_PAUSE(l2_caches[cache->id].wire_latency);

			/*change the access type for the coherence protocol and drop into the L2's queue
			remove the access from the l1 cache queue and place it in the l2 cache ctrl queue*/
			list_remove(cache->last_queue, message_packet);
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
			list_enqueue(l2_caches[cache->id].Rx_queue_top, message_packet);
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu l2_cache[%d] as %s\n",
					cache->name, access_id, P_TIME, cache->id, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
			CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu %s Miss SEND %s %s\n",
					access_id, P_TIME, cache->name, l2_caches[cache->id].name, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

			//advance the L2 cache adding some wire delay time.
			//future_advance(&l2_cache[cache->id], WIRE_DELAY(l2_caches[cache->id].wire_latency));
			advance(&l2_cache[cache->id]);

		}
		else
		{
			//entry found in ORT so coalesce access
			assert(cache->ort[i][0] == tag && cache->ort[i][1] == set && cache->ort[i][2] == 1);

			list_remove(cache->last_queue, message_packet);
			list_enqueue(cache->ort_list, message_packet);
		}


		//check MSHR status
		//mshr_status = mshr_get_status(cache, tag_ptr, set_ptr, access_id);

		//CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, mshr_status);


		//if the status comes back -1 there is no entry in the MSHR for this access
		/*if(mshr_status == -1)
		{
			//store the entry in the MSHR
			//find an empty row
			mshr_row = mshr_get_empty_row(cache, tag_ptr, set_ptr, access_id);

			//set it
			l1_mshr_set(cache, mshr_row, tag_ptr, set_ptr, access_id);*/

			//access is unique in the MSHR so send forward
			//while the next level of cache's in queue is full stall


		/*}
		//coalesce
		else
		{
			//previous access to same tag and set is in the MSHR
			list_remove(cache->last_queue, message_packet);

			l1_mshr_coalesce(cache, message_packet, mshr_status);

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu coalesced packet removed removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

		}*/

		/*}
		else if(mshr_status == 1)
		{

		}
		else //mshr == 0
		{
			printf("breaking MSHR full\n");
			mshr_dump(cache);
			STOP;

			//mshr is full so we can't progress, retry.
			fatal("l1_i_cache_access_load(): MSHR full\n");
		}
	}*/

	}

	return;
}

void cpu_l1_cache_access_store(struct cache_t *cache, struct cgm_packet_t *message_packet){

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
	if(cache->cache_type == l1_d_cache_t)
	{
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_shared);
	}
	//////testing

	CGM_DEBUG(CPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);


	//get the block and the state of the block and charge cycles
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(cache->latency);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)// && *state_ptr != cache_block_shared)
	{
		//check state of the block
		//block is valid

		assert(*state_ptr != cache_block_invalid);

		//star todo this is wrong
		if(*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
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
	else if(cache_status == 0 || *state_ptr == 0)
	{
		//on both a miss and invalid hit the state_ptr should be zero

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		//only the D$ stores
		message_packet->cpu_access_type = cgm_access_store;
		message_packet->access_type = cgm_access_gets_d;
		message_packet->l1_access_type = cgm_access_gets_d;


	/*	miss_status_packet = miss_status_packet_copy(message_packet, *set_ptr, *tag_ptr, *offset_ptr, str_map_string(&l1_strn_map, cache->name));
		mshr_status = mshr_set(cache, miss_status_packet);

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, mshr_status);*/

		/*if(mshr_status == 2)
		{
			//access was coalesced
			//remove the message packet on coalesce, but don't send to next cache
			list_remove(cache->last_queue, message_packet);

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu coalesced packet removed removed from %s size %d\n",
				cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
		}
		else if(mshr_status == 1)
		{*/
			//access is unique in the MSHR so send forward
			//while the next level of cache's in queue is full stall
			while(!cache_can_access_top(&l2_caches[cache->id]))
			{
				//printf("stall\n");
				P_PAUSE(1);
			}

			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu l2 queue free size %d\n",
				cache->name, access_id, P_TIME, list_count(l2_caches[cache->id].Rx_queue_top));

			P_PAUSE(l2_caches[cache->id].wire_latency);

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
			//future_advance(&l2_cache[cache->id], WIRE_DELAY(l2_caches[cache->id].wire_latency));

			advance(&l2_cache[cache->id]);
		/*}
		else //mshr == 0
		{
			printf("breaking\n");
			mshr_dump(cache);
			STOP;
			//mshr is full so we can't progress, retry.
			fatal("l1_d_cache_access_load(): MSHR full\n");
		}
	}*/
	}

	return;
}


/*void cpu_l1_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet){

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
	if(access_type == cgm_access_retry)
	cache->retries++;

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(CPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
		cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//get the block and the state of the block and charge a cycle
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	//P_PAUSE(1);

	//L1 Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

		if(*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
		{
			//CPU L1 I cache
			if(message_packet->cpu_access_type == cgm_access_fetch)
			{
				//remove packet from cache queue, global queue, and simulator memory
				list_remove(cache->last_queue, message_packet);
				remove_from_global(access_id);
			}
			//CPU L1 D cache
			if(message_packet->cpu_access_type == cgm_access_load || message_packet->cpu_access_type == cgm_access_store)
			{
				list_remove(cache->last_queue, message_packet);
				linked_list_add(message_packet->event_queue, message_packet->data);
			}

		}
		else
		{
			fatal("l1_d_cache_access_load(): incorrect block state set");
		}
	}
	else
	{
		printf("breaking\n");
		STOP;
		fatal("cache_access_retry(): miss on retry cycle %llu access_id %llu\n", P_TIME, access_id);
	}

	return;
}*/

void cpu_cache_access_get(struct cache_t *cache, struct cgm_packet_t *message_packet){

	struct cgm_packet_status_t *miss_status_packet;
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

	int cache_status;
	int mshr_status = 0;
	int l3_map = -1;

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
	if(cache->cache_type == l2_cache_t) //cache->cache_type == l3_cache_t // ||
	{
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_shared);
	}
	//////testing


	//look up, and charge a cycle.
	cache_status = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, state_ptr);
	P_PAUSE(cache->latency);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu hit\n", cache->name, access_id, P_TIME);

		assert(*state_ptr != cache_block_invalid);

		if(*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
		{

			cache->hits++;


			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
					cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			if(cache->cache_type == l2_cache_t)
			{
				//send to correct l1 cache and change access type
				if (message_packet->access_type == cgm_access_gets_i)
				{
					//while the next level of cache's in queue is full stall
					while(!cache_can_access_bottom(&l1_i_caches[cache->id]))
					{
						//printf("stall\n");
						P_PAUSE(1);
					}

					CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu L1 bottom queue free size %d\n",
							cache->name, access_id, P_TIME, list_count(l1_i_caches[cache->id].Rx_queue_bottom));

					P_PAUSE(l1_i_caches[cache->id].wire_latency);

					message_packet->access_type = cgm_access_puts;
					list_remove(cache->last_queue, message_packet);
					list_enqueue(l1_i_caches[cache->id].Rx_queue_bottom, message_packet);

					//future_advance(&l1_i_cache[cache->id], WIRE_DELAY(l1_i_caches[cache->id].wire_latency));
					advance(&l1_i_cache[cache->id]);

				}
				else if (message_packet->access_type == cgm_access_gets_d)
				{
					//while the next level of cache's in queue is full stall
					while(!cache_can_access_bottom(&l1_d_caches[cache->id]))
					{
						//printf("stall\n");
						P_PAUSE(1);
					}

					CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu %s free size %d\n",
							cache->name, access_id, P_TIME, l1_d_caches[cache->id].Rx_queue_bottom->name, list_count(l1_d_caches[cache->id].Rx_queue_bottom));

					P_PAUSE(l1_d_caches[cache->id].wire_latency);

					message_packet->access_type = cgm_access_puts;
					list_remove(cache->last_queue, message_packet);
					list_enqueue(l1_d_caches[cache->id].Rx_queue_bottom, message_packet);

					//future_advance(&l1_d_cache[cache->id], WIRE_DELAY(l1_d_caches[cache->id].wire_latency));
					advance(&l1_d_cache[cache->id]);
				}
				else
				{
					fatal("l2_cache_access_gets(): %s access_id %llu cycle %llu incorrect access type %s\n",
							cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
				}

			}
			else if(cache->cache_type == l3_cache_t)
			{
				//This is a hit in the L3 cache, send up to L2 cache
				//while the next level of cache's in queue is full stall
				//star todo possible deadlock situation if both the l2 and core are trying to fill a full queue
				while(!switch_can_access(switches[cache->id].south_queue))
				{
					//printf("stall\n");
					P_PAUSE(1);
				}

				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu switch south queue free size %d\n",
						cache->name, access_id, P_TIME, list_count(switches[cache->id].south_queue));

				P_PAUSE(switches[cache->id].wire_latency);

				//success
				//remove packet from l3 cache in queue
				message_packet->access_type = cgm_access_puts;
				message_packet->dest_name = message_packet->src_name;
				message_packet->dest_id = message_packet->src_id;
				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);

				list_remove(cache->last_queue, message_packet);
				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
						cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

				list_enqueue(switches[cache->id].south_queue, message_packet);
				advance(&switches_ec[cache->id]);
				//future_advance(&switches_ec[cache->id], WIRE_DELAY(switches[cache->id].wire_latency));
				//done
			}
			else
			{
				fatal("cpu_cache_access_get(): hit bad cache type access_id %llu cycle %llu\n", access_id, P_TIME);
			}

		}
		else
		{
			fatal("cpu_cache_access_load(): incorrect block state set");
		}

		CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu %s Hit SEND %s to %s\n",
			access_id, P_TIME, cache->name, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), l1_i_caches[cache->id].name);

	}
	//Cache Miss!
	else if(cache_status == 0 || *state_ptr == 0)
	{

		/*printf("cpu_cache_access_get() MISS!!\n");
		getchar();*/

		cache->misses++;

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss\n", cache->name, access_id, P_TIME);

		//message_packet->access_type = cgm_access_gets;

		/*miss_status_packet = miss_status_packet_copy(message_packet, set, tag, offset, str_map_string(&node_strn_map, cache->name));
		mshr_status = mshr_set(cache, miss_status_packet);

		printf("%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, mshr_status);
		mshr_dump(cache);
		getchar();

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
		{*/
			//access is unique in the MSHR so send forward
			//while the next level of cache's in queue is full stall

			//======================

			if(cache->cache_type == l2_cache_t)
			{
				while(!switch_can_access(switches[cache->id].north_queue))
				{
					//printf("stall\n");
					P_PAUSE(1);
				}

				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss switch north queue free size %d\n",
						cache->name, access_id, P_TIME, list_count(switches[cache->id].north_queue));

				P_PAUSE(switches[cache->id].wire_latency);
				//printf(" l2 miss type %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

				l3_map = cgm_l3_cache_map(set_ptr);

				message_packet->access_type = cgm_access_gets;
				message_packet->l2_cache_id = cache->id;
				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);
				message_packet->dest_name = l3_caches[l3_map].name;
				message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

				list_remove(cache->last_queue, message_packet);
				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
						cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

				list_enqueue(switches[cache->id].north_queue, message_packet);

				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu l3_cache[%d] send %s\n",
						cache->name, access_id, P_TIME, l3_map, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

				CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu %s Miss SEND %s %s\n",
						access_id, P_TIME, cache->name, l3_caches[l3_map].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

				//advance the L2 cache adding some wire delay time.
				advance(&switches_ec[cache->id]);
				//future_advance(&switches_ec[cache->id], WIRE_DELAY(switches[cache->id].wire_latency));
			}
			else if(cache->cache_type == l3_cache_t)
			{
				while(!switch_can_access(switches[cache->id].south_queue))
				{
					//printf("stall\n");
					P_PAUSE(1);
				}

				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu %s free size %d\n",
						cache->name, access_id, P_TIME, switches[cache->id].south_queue->name, list_count(switches[cache->id].south_queue));

				P_PAUSE(switches[cache->id].wire_latency);

				//message_packet->access_type = cgm_access_gets;
				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);
				message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
				message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

				//success
				list_remove(cache->last_queue, message_packet);
				list_enqueue(switches[cache->id].south_queue, message_packet);

				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu l3_cache[%d] as %s\n",
						cache->name, access_id, P_TIME, cache->id, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

				CGM_DEBUG(protocol_debug_file, "Access_id %llu cycle %llu %s Miss SEND %s %s\n",
					access_id, P_TIME, cache->name, system_agent->name, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

				advance(&switches_ec[cache->id]);
				//future_advance(&switches_ec[cache->id], WIRE_DELAY(switches[cache->id].wire_latency));

			}
			else
			{
				fatal("cpu_cache_access_get(): miss bad cache type access_id %llu cycle %llu\n", access_id, P_TIME);
			}

		/*}
		else //mshr == 0 || -1
		{
			//mshr is full so we can't progress, retry.

			printf("%s breaking MSHR full\n", cache->name);
			mshr_dump(cache);
			STOP;

			fatal("cpu_cache_access_load(): MSHR full\n");
		}*/

	}
	return;
}

void cpu_cache_access_put(struct cache_t *cache, struct cgm_packet_t *message_packet){

	struct cgm_packet_t *ort_packet;

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
	int adv = 0;

	//the packet is from the L2 cache
	access_type = message_packet->access_type;
	addr = message_packet->address;
	access_id = message_packet->access_id;

	//probe the address for set, tag, and offset.
	cgm_cache_decode_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu puts\n", cache->name, access_id, P_TIME);

	//charge the delay for writing cache block
	//star todo add LRU evict here
	cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_shared);
	P_PAUSE(cache->latency);

	//get the mshr status
	/*mshr_row = mshr_get_status(cache, tag_ptr, set_ptr, access_id);
	if(mshr_row == -1)
	{
		printf("%s crashing %llu access_id %llu type %s\n", cache->name, P_TIME, access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->cpu_access_type));
		mshr_dump(cache);
		assert(mshr_row != -1);
	}

	CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu miss mshr status %d\n", cache->name, access_id, P_TIME, mshr_row);
	CGM_DEBUG(mshr_debug_file, "%s access_id %llu cycle %llu mshr_row %d num_entries %d\n", cache->name, access_id, P_TIME, mshr_row, cache->mshrs[mshr_row].num_entries);

	//check the number of entries in the mshr row
	assert(list_count(cache->mshrs[mshr_row].entires) == (cache->mshrs[mshr_row].num_entries - 1));
	assert(cache->mshrs[mshr_row].num_entries >= 0);*/

	/*printf("cache->mshrs[mshr_row].num_entries %d\n", cache->mshrs[mshr_row].num_entries);
	printf("made it here\n");
	STOP;*/


	//block is returned so find it in the ORT
	for (i = 0; i < cache->mshr_size; i++)
	{
		if(cache->ort[i][0] == tag && cache->ort[i][1] == set && cache->ort[i][2] == 1)
		{
			//hit in the ORT table
			break;
		}
	}

	//if we didn't find it there is a problem;
	assert(cache->ort[i][0] == tag && cache->ort[i][1] == set && cache->ort[i][2] == 1);

	//clear the ORT now
	cache->ort[i][0] = -1;
	cache->ort[i][1] = -1;
	cache->ort[i][2] = -1;

	//printf("block is here %d tag %d set %d \n", i, tag, set);
	//printf("ort is %d tag %d set %d \n", i, cache->ort[i][0], cache->ort[i][1]);

	//move returned message and coalesced messages to retry queue

	message_packet->access_type = cgm_access_retry;
	list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);

	////////testing
	advance(cache->ec_ptr);
	////////testing

	i = 0;
	LIST_FOR_EACH(cache->ort_list, i)
	{
		ort_packet = list_get(cache->ort_list, i);
		printf("tag %d set %d access_id %llu\n", ort_packet->tag, ort_packet->set, ort_packet->access_id);
	}



	i = 0;
	LIST_FOR_EACH(cache->ort_list, i)
	{
		//get pointer to access in queue and check it's status.
		ort_packet = list_get(cache->ort_list, i);

		//printf("made it here\n");
		//printf("tag %d set %d access_id %llu\n", ort_packet->tag, ort_packet->set, ort_packet->access_id);

		if (!ort_packet)
		{
			//no coalesced packets.
			break;
		}
		else if(ort_packet->tag == tag && ort_packet->set == set)
		{
			//printf("ort_packet is tag %d set %d \n", i, cache->ort[i][0], cache->ort[i][1]);

			ort_packet->access_type = cgm_access_retry;
			list_remove_at(cache->ort_list, i);
			list_enqueue(cache->retry_queue, ort_packet);

			////////testing
			advance(cache->ec_ptr);
			////////testing
		}
	}

	/*adv ++;

	//if there are any coalesced accesses move those as well.
	if(list_count(cache->mshrs[mshr_row].entires) > 0)//(cache->mshrs[mshr_row].num_entries - 1) > 0)
	{
		for(i = 0; i < list_count(cache->mshrs[mshr_row].entires); i++)
		{
			miss_status_packet = list_dequeue(cache->mshrs[mshr_row].entires);

			//this is a coalesced packet
			if(miss_status_packet->coalesced != 1)
			{
				printf("breaking access_id %llu cycle %llu\n", access_id, P_TIME);
				printf("i %d miss sp %llu, coalesced %d\n", i, miss_status_packet->access_id, miss_status_packet->coalesced);
				mshr_dump(cache);
				STOP;
			}

			assert(miss_status_packet != NULL);

			CGM_DEBUG(mshr_debug_file, "%s access_id %llu coalesced %d tag %d set %d\n",
					cache->name, miss_status_packet->access_id, miss_status_packet->coalesced, miss_status_packet->tag, miss_status_packet->set);

			//drop it into the retry queue
			miss_status_packet->access_type = cgm_access_retry;
			list_enqueue(cache->retry_queue, miss_status_packet);
			adv ++;
		}
	}

	//advance myself by the number of packets.
	long long time = etime.count;  :-P
	for(i = 0; i < adv; i ++)
	{
		time += 2;
		future_advance(cache->ec_ptr, time);
	}

	//clear the mshr row for future use
	mshr_clear(&(cache->mshrs[mshr_row]));*/

	return;
}

void cpu_cache_access_retry(struct cache_t *cache, struct cgm_packet_t *message_packet){

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
	//P_PAUSE(cache->latency);

	//Cache Hit!
	if(cache_status == 1 && *state_ptr != 0)
	{
		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu retry hit\n", cache->name, access_id, P_TIME);

		if(*state_ptr == cache_block_modified || *state_ptr == cache_block_exclusive || *state_ptr == cache_block_shared || *state_ptr == cache_block_noncoherent)
		{

			list_remove(cache->retry_queue, message_packet);
			CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
				cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

			//send to correct l1 cache and change access type
			if(cache->cache_type == l1_i_cache_t || cache->cache_type == l1_d_cache_t)
			{
				//CPU L1 I cache
				if(message_packet->cpu_access_type == cgm_access_fetch)
				{
					//remove packet from cache queue, global queue, and simulator memory
					list_remove(cache->last_queue, message_packet);
					remove_from_global(access_id);
				}
				//CPU L1 D cache
				if(message_packet->cpu_access_type == cgm_access_load || message_packet->cpu_access_type == cgm_access_store)
				{
					list_remove(cache->last_queue, message_packet);
					linked_list_add(message_packet->event_queue, message_packet->data);
				}
			}
			else if(cache->cache_type == l2_cache_t)
			{

				if (message_packet->l1_access_type == cgm_access_gets_i)
				{
					//while the next level of cache's in queue is full stall
					while(!cache_can_access_bottom(&l1_i_caches[cache->id]))
					{
						//printf("stall\n");
						P_PAUSE(1);
					}

						CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu L1 bottom queue free size %d\n",
								cache->name, access_id, P_TIME, list_count(l1_i_caches[cache->id].Rx_queue_bottom));

						P_PAUSE(l1_i_caches[cache->id].wire_latency);

						message_packet->access_type = cgm_access_puts;
						list_enqueue(l1_i_caches[cache->id].Rx_queue_bottom, message_packet);

						advance(&l1_i_cache[cache->id]);
						//future_advance(&l1_i_cache[cache->id], WIRE_DELAY(l1_i_caches[cache->id].wire_latency));
				}
				else if (message_packet->l1_access_type == cgm_access_gets_d)
				{
					//while the next level of cache's in queue is full stall
					while(!cache_can_access_bottom(&l1_d_caches[cache->id]))
					{
						//printf("stall\n");
						P_PAUSE(1);
					}

					P_PAUSE(l1_d_caches[cache->id].wire_latency);

					CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu %s free size %d\n",
							cache->name, access_id, P_TIME, l1_d_caches[cache->id].Rx_queue_bottom->name, list_count(l1_d_caches[cache->id].Rx_queue_bottom));

					message_packet->access_type = cgm_access_puts;
					list_enqueue(l1_d_caches[cache->id].Rx_queue_bottom, message_packet);

					advance(&l1_d_cache[cache->id]);
					//future_advance(&l1_d_cache[cache->id], WIRE_DELAY(l1_d_caches[cache->id].wire_latency));
				}
				else
				{
					fatal("l2_cache_access_gets(): %s access_id %llu cycle %llu incorrect l1 access type %s\n",
							cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, message_packet->l1_access_type));
				}
			}
			else if(cache->cache_type == l3_cache_t)
			{

				//Send up to L2 cache
				while(!switch_can_access(switches[cache->id].south_queue))
				{
					//printf("stall\n");
					P_PAUSE(1);
				}

				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu switch south queue free size %d\n",
						cache->name, access_id, P_TIME, list_count(switches[cache->id].south_queue));

				P_PAUSE(switches[cache->id].wire_latency);

				//success
				//remove packet from l3 cache in queue
				message_packet->access_type = cgm_access_puts;
				message_packet->dest_id = message_packet->l2_cache_id;
				message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);
				message_packet->src_name = cache->name;
				message_packet->src_id = str_map_string(&node_strn_map, cache->name);

				list_remove(cache->last_queue, message_packet);
				CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
						cache->name, access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));

				list_enqueue(switches[cache->id].south_queue, message_packet);

				advance(&switches_ec[cache->id]);
				//future_advance(&switches_ec[cache->id], WIRE_DELAY(switches[cache->id].wire_latency));
			}
		}
		else
		{
			fatal("l1_d_cache_access_load(): incorrect block state set");
		}
	}
	return;
}

int cgm_l3_cache_map(int *set){

	int num_cores = x86_cpu_num_cores;
	int map = -1;

	//star todo fix this
	int map_type = 0;

	//stripe or block
	if (map_type == 0)
	{
		//map = *(set) % num_cores;
		//star this is a faster way to do the look up.
		map = (unsigned int) *set & (unsigned int) (num_cores - 1);

	}
	else if (map_type == 1)
	{
		//get the address range
		fatal("cgm_l3_cache_map(): invalid map_type set\n");

	}

	/*printf("l3 cache map() set %d map %d", *set, map);
	getchar();*/

	assert(map >= 0 && map <= (num_cores - 1));
	return map;
}

int cgm_gpu_cache_map(int cache_id){

	int map;

	if(cache_id < 4)
	{
		map = 0;
	}
	else if(cache_id >= 4 && cache_id < 8)
	{
		map = 1;
	}
	else if(cache_id >= 8 && cache_id < 12)
	{
		map = 2;
	}
	else if(cache_id >= 12 && cache_id < 16)
	{
		map = 3;
	}
	else if(cache_id >= 16 && cache_id < 20)
	{
		map = 4;
	}
	else if(cache_id >= 20 && cache_id < 24)
	{
		map = 5;
	}
	else if(cache_id >= 24 && cache_id < 28)
	{
		map = 6;
	}
	else if(cache_id >= 28 && cache_id < 32)
	{
		map = 7;
	}
	else if (cache_id < 0 && cache_id > 31)
	{
		fatal("cgm_gpu_cache_map(): cache_id out of bounds\n");
	}

	return map;
}



int cache_can_access_top(struct cache_t *cache){


	//check if in queue is full
	if(QueueSize <= list_count(cache->Rx_queue_top))
	{
		return 0;
	}

	//cache queue is accessible.
	return 1;
}


int cache_can_access_bottom(struct cache_t *cache){


	//check if in queue is full
	if(QueueSize <= list_count(cache->Rx_queue_bottom))
	{
		return 0;
	}

	//cache queue is accessible.
	return 1;
}


/* Return {tag, set, offset} for a given address */
void cgm_cache_decode_address(struct cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr)
{
	//star i reworked this a little//addr = message_packet->address;
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

	*(state_ptr) = 0;

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

	fatal("cgm_cache_find_block() reached bottom\n");
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

	CGM_STATS(cgm_stats_file, "[General]\n");
	CGM_STATS(cgm_stats_file, "NumCores = %d\n", num_cores);
	CGM_STATS(cgm_stats_file, "ThreadsPerCore = %d\n", num_threads);
	CGM_STATS(cgm_stats_file, "TotalCycles = %lld\n", P_TIME);
	CGM_STATS(cgm_stats_file, "\n");

	for(i = 0; i < num_cores; i++)
	{
		CGM_STATS(cgm_stats_file, "[L1_I_Cache_%d]\n", i);
		CGM_STATS(cgm_stats_file, "Sets = %d\n", l1_i_caches[i].num_sets);
		CGM_STATS(cgm_stats_file, "BlockSize = %d\n", l1_i_caches[i].block_size);
		CGM_STATS(cgm_stats_file, "Fetches = %lld\n", l1_i_caches[i].loads);
		CGM_STATS(cgm_stats_file, "Hits = %lld\n", l1_i_caches[i].hits);
		CGM_STATS(cgm_stats_file, "Misses = %lld\n", l1_i_caches[i].misses);
		CGM_STATS(cgm_stats_file, "\n");

		CGM_STATS(cgm_stats_file, "[L1_D_Cache_%d]\n", i);
		CGM_STATS(cgm_stats_file, "Sets = %d\n", l1_d_caches[i].num_sets);
		CGM_STATS(cgm_stats_file, "BlockSize = %d\n", l1_d_caches[i].block_size);
		CGM_STATS(cgm_stats_file, "Loads = %lld\n", l1_d_caches[i].loads);
		CGM_STATS(cgm_stats_file, "Stores = %lld\n", l1_d_caches[i].stores);
		CGM_STATS(cgm_stats_file, "Hits = %lld\n", l1_d_caches[i].hits);
		CGM_STATS(cgm_stats_file, "Misses = %lld\n", l1_d_caches[i].misses);
		CGM_STATS(cgm_stats_file, "\n");

		CGM_STATS(cgm_stats_file, "[L2_Cache_%d]\n", i);
		CGM_STATS(cgm_stats_file, "Sets = %d\n", l2_caches[i].num_sets);
		CGM_STATS(cgm_stats_file, "BlockSize = %d\n", l2_caches[i].block_size);
		CGM_STATS(cgm_stats_file, "Accesses = %lld\n", (l2_caches[i].fetches + l2_caches[i].loads + l2_caches[i].stores));
		CGM_STATS(cgm_stats_file, "Hits = %lld\n", l2_caches[i].hits);
		CGM_STATS(cgm_stats_file, "Misses = %lld\n", l2_caches[i].misses);
		CGM_STATS(cgm_stats_file, "\n");
	}

	return;
}

void l1_i_cache_ctrl(void){

	//my_pid increments for the number of CPU cores. i.e. 0 - 4 for a quad core
	int my_pid = l1_i_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{

		//wait here until there is a job to do
		await(&l1_i_cache[my_pid], step);
		step++;

		//get a message from the top or bottom queues.
		message_packet = cache_get_message(&(l1_i_caches[my_pid]));

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;

		if (access_type == cgm_access_fetch)
		{
			cpu_l1_cache_access_load(&(l1_i_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_puts)
		{
			cpu_cache_access_put(&(l1_i_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_retry)
		{
			cpu_cache_access_retry(&(l1_i_caches[my_pid]), message_packet);
		}
		else
		{
			fatal("l1_i_cache_ctrl(): %s access_id %llu bad access type %s at cycle %llu\n",
				l1_i_caches[my_pid].name, access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
		}
	}

	/* should never get here*/
	fatal("l1_i_cache_ctrl task is broken\n");
	return;
}

void l1_d_cache_ctrl(void){

	int my_pid = l1_d_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{
		//wait here until there is a job to do.
		await(&l1_d_cache[my_pid], step);
		step++;

		//get the message out of the queue
		message_packet = cache_get_message(&(l1_d_caches[my_pid]));

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;

		//probe the address for set, tag, and offset.
		//cgm_cache_decode_address(&(l1_d_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);

		if (access_type == cgm_access_load)
		{
			cpu_l1_cache_access_load(&(l1_d_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_store)
		{
			cpu_l1_cache_access_store(&(l1_d_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_puts)
		{
			cpu_cache_access_put(&(l1_d_caches[my_pid]), message_packet);
		}
		else if (access_type == cgm_access_retry)
		{
			cpu_cache_access_retry(&(l1_d_caches[my_pid]), message_packet);
		}
		else
		{
			fatal("l1_d_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
				access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
		}
	}
	//should never get here
	fatal("l1_d_cache_ctrl task is broken\n");
	return;
}

void l2_cache_ctrl(void){

	int my_pid = l2_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

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
			cpu_cache_access_get(&l2_caches[my_pid], message_packet);
		}
		else if(access_type == cgm_access_puts)
		{
			cpu_cache_access_put(&l2_caches[my_pid], message_packet);
		}
		else if (access_type == cgm_access_retry)
		{
			cpu_cache_access_retry(&l2_caches[my_pid], message_packet);
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

void l3_cache_ctrl(void){

	int my_pid = l3_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;
	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{
		/*wait here until there is a job to do.*/
		await(&l3_cache[my_pid], step);
		step++;

		//get the message out of the queue
		message_packet = cache_get_message(&(l3_caches[my_pid]));

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;

		if (access_type == cgm_access_gets)
		{
			cpu_cache_access_get(&l3_caches[my_pid], message_packet);
		}
		else if (access_type == cgm_access_puts)
		{
			cpu_cache_access_put(&l3_caches[my_pid], message_packet);
		}
		else if (access_type == cgm_access_retry)
		{
			cpu_cache_access_retry(&l3_caches[my_pid], message_packet);
		}
		else
		{
			fatal("l3_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
				access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
		}

	}
	/* should never get here*/
	fatal("l3_cache_ctrl task is broken\n");
	return;
}
