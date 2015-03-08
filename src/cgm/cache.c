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

//statistics

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
	l1_i_cache_0 = new_eventcount(buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_1");
	l1_i_cache_1 = new_eventcount(buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_2");
	l1_i_cache_2 = new_eventcount(buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_3");
	l1_i_cache_3 = new_eventcount(buff);


	//l1 d caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_0");
	l1_d_cache_0 = new_eventcount(buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_1");
	l1_d_cache_1 = new_eventcount(buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_2");
	l1_d_cache_2 = new_eventcount(buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_3");
	l1_d_cache_3 = new_eventcount(buff);


	//l2 caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_0");
	l2_cache_0 = new_eventcount(buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_1");
	l2_cache_1 = new_eventcount(buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_2");
	l2_cache_2 = new_eventcount(buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l2_cache_3");
	l2_cache_3 = new_eventcount(buff);


	////////////////////
	//tasks
	////////////////////

	//l1 i caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_ctrl_0");
	create_task(l1_i_cache_ctrl_0, DEFAULT_STACK_SIZE, buff);

	/*memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_ctrl_1");
	create_task(l1_i_cache_ctrl_1, DEFAULT_STACK_SIZE, buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_ctrl_2");
	create_task(l1_i_cache_ctrl_2, DEFAULT_STACK_SIZE, buff);

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_i_cache_ctrl_3");
	create_task(l1_i_cache_ctrl_3, DEFAULT_STACK_SIZE, buff);


	//l1 d caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "l1_d_cache_ctrl_0");
	create_task(l1_d_cache_ctrl_0, DEFAULT_STACK_SIZE, buff);

	memset(buff,'\0' , 100);
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
	create_task(l2_cache_ctrl_0, DEFAULT_STACK_SIZE, buff);

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

	long long step = 1;
	long long access_id = 0;
	unsigned int addr = 0;
	int list_index = 0;
	int cache_status;
	int i = 0;
	struct cgm_packet_t *new_packet;
	struct cgm_packet_t *mshr_packet;

	enum cgm_access_kind_t access_type;
	int tag = 0;
	int set = 0;
	int way = 0;
	int state = 0;


	while(1)
	{
		//waits here until it has a job to do.
		await(l1_i_cache_0, step);
		step++;

		//message received check queue there should be a message there.
		assert(list_count(l1_i_caches[0].Rx_queue_top) >= 1);


		//printf("list size %d\n", list_count(l1_i_caches[0].Rx_queue));
		new_packet = list_get(l1_i_caches[0].Rx_queue_top, list_index);
		assert(new_packet);

		/*printf("access_id %lld\n", packet->access_id);
		printf("addr 0x%08u\n", packet->address);
		getchar();*/

		access_type = new_packet->access_type;
		access_id = new_packet->access_id;
		addr = new_packet->address;
		tag = new_packet->tag;

	/*	printf("access_id %lld\n", access_id);
		printf("addr 0x%08u\n", addr);
		getchar();*/


		//some cache work
		int *set_ptr = &set;
		int *way_ptr = &way;
		int *state_ptr = &state;

		/*printf("state = %d\n", state);
		*state_ptr = 35;
		printf("state = %d\n", state);*/

		cache_status = cgm_cache_find_block(&(l1_i_caches[0]), addr, set_ptr, way_ptr, state_ptr);

		/*printf("cache_status %d\n", cache_status);
		printf("set = %d\n", set);
		printf("way = %d\n", way);
		printf("state = %d\n", state);
		getchar();*/




		if(cache_status == 0 && new_packet->access_type == cgm_access_load) //miss
		{
			// L1 I Cache Miss!
			l1_i_caches[0].misses++;

			//star todo check for coalesce
			//if(list_count(l1_i_caches[0].mshr) >= 0)
			//{

				/*LIST_FOR_EACH(l1_i_caches[0].mshr, i)
				{
					//get pointer to access in queue and check it's status.
					mshr_packet = list_get(cgm_access_record, i);

					if(mshr_packet->tag == new_packet->tag)
					{


					}

				}*/

			//	list_enqueue(l1_i_caches[0].mshr, new_packet);

			//}
			//else
			//{

				//remove from rx queue and insert into mshr
				//in the real world these are parallel task. for timing its ok to put them here
				//because this occurs in the same cycle.

			//}



			list_remove(l1_i_caches[0].Rx_queue_top, new_packet);
			list_enqueue(l1_i_caches[0].mshr, new_packet);

			cgm_cache_set_block(&(l1_i_caches[0]), *set_ptr, *way_ptr, tag, 1);
			remove_from_global(new_packet->access_id);


			/*printf("list size %d\n", list_count(l1_i_caches[0].Rx_queue));
			printf("list size %d\n", list_count(l1_i_caches[0].Rx_queue));
			getchar();*/

		}
		else if (cache_status == 1 && new_packet->access_type == cgm_access_load)
		{
			// L1 I Cache Hit!
			l1_i_caches[0].hits++;

			//Mr. CPU, go about your business...
			//remove packet from cache and global queues
			list_remove(l1_i_caches[0].Rx_queue_top, new_packet);
			remove_from_global(access_id);
		}
	}

	/* should never get here*/
	fatal("l1_i_cache_ctrl_0 task is broken\n");
	return;
}


void l2_cache_ctrl_0(void){

	long long step = 1;
	long long access_id = 0;
	unsigned int addr = 0;
	enum cgm_access_kind_t access_type;
	struct cgm_packet_t *packet;

	while(1)
	{

		await(l2_cache_0, step);
		step++;

		if(TSK == 1)
		{
			printf("l2_cache_ctrl_0\n");

		}

		//message received check queue there should be a message there.
		assert(list_count(l2_caches[0].Rx_queue_top) >= 1);


		//get the packet
		if(list_count(l2_caches[0].Rx_queue_top) >= 1)
		{
			packet = list_dequeue(l2_caches[0].Rx_queue_top);
		}

		access_type = packet->access_type;
		access_id = packet->access_id;
		addr = packet->address;

		if(1) //hit
		{
			remove_from_global(access_id);
		}
		else
		{


		}

	}

	return;
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
		cgm_cache_update_waylist(&cache->sets[set], &cache->sets[set].blocks[way], cache_waylist_head);

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

	printf("Total Cycles %lld\n\n", P_TIME + 1);

	for(i = 0; i < num_cores; i++)
	{
		printf("li_i_cache_%d\n", i);
		printf("Number of set %d\n", l1_i_caches[i].num_sets);
		printf("Block size = %d\n", l1_i_caches[i].block_size);
		printf("* hits %lld\n", l1_i_caches[i].hits);
		printf("* misses %lld\n", l1_i_caches[i].misses);
		printf("\n");
	}

	for(i = 0; i < num_cores; i++)
	{
		printf("li_2_cache_%d\n", i);
		printf("Number of set %d\n", l2_caches[i].num_sets);
		printf("Block size = %d\n", l2_caches[i].block_size);
		printf("* hits %lld\n", l2_caches[i].hits);
		printf("* misses %lld\n", l2_caches[i].misses);
		printf("\n");
	}

	return;
}
