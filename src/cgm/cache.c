/*
 * cache.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */

#include <stdlib.h>
#include <stdio.h>
#include <cgm/cache.h>
#include <lib/util/list.h>
#include <cgm/queue.h>
#include <cgm/tasking.h>
#include <cgm/cgm.h>



#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>
#include <lib/util/debug.h>

//CPU caches
struct cache_t *l1_i_caches;
struct cache_t *l1_d_caches;
struct cache_t *l2_caches;
struct cache_t *l3_s0_cache;
struct cache_t *l3_s1_cache;
struct cache_t *l3_s2_cache;
struct cache_t *l3_s3_cache;

//GPU caches
struct cache_t *l1_v_caches;
struct cache_t *l1_s_caches;
struct cache_t *l2_caches;
struct cache_t *lds_units;


void cache_init(void){

	//star todo make this automatic
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int l3_slices = num_cores/4;
	int gpu_group_cache_num = num_cus/4;

	//initialize the CPU L1I caches
	l1_i_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	//initialize the CPU L1D caches
	l1_d_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	//initialize the CPU L2 caches
	l2_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	//initialize the L3 caches (sliced).
	l3_s0_cache = (void *) calloc(l3_slices, sizeof(struct cache_t));
	l3_s1_cache = (void *) calloc(l3_slices, sizeof(struct cache_t));
	l3_s2_cache = (void *) calloc(l3_slices, sizeof(struct cache_t));
	l3_s3_cache = (void *) calloc(l3_slices, sizeof(struct cache_t));

	//initialize the GPU L1V caches
	l1_v_caches = (void *) calloc(num_cus, sizeof(struct cache_t));

	//initialize the GPU L1S caches
	l1_s_caches = (void *) calloc(num_cus, sizeof(struct cache_t));

	//initialize the GPU L2 caches.
	l2_caches = (void *) calloc(gpu_group_cache_num, sizeof(struct cache_t));

	//initialize the GPU LDS
	lds_units = (void *) calloc(num_cus, sizeof(struct cache_t));

	return;
}

struct cache_t *cgm_cache_create(void){

	struct cache_t *new_cache;

	new_cache = (void *) calloc(1, sizeof(struct cache_t));

	return new_cache;
}

//todo fix the arguments
/*void cgm_cache_configure(void){

	struct cache_t *cache;
	struct cache_block_t *block;
	unsigned int set, way;

	 Initialize
	cache = xcalloc(1, sizeof(struct cache_t));
	cache->name = xstrdup(name);
	cache->num_sets = num_sets;
	cache->block_size = block_size;
	cache->assoc = assoc;
	cache->policy = policy;

	 Derived fields
	assert(!(num_sets & (num_sets - 1)));
	assert(!(block_size & (block_size - 1)));
	assert(!(assoc & (assoc - 1)));
	cache->log_block_size = log_base2(block_size);
	cache->block_mask = block_size - 1;

	 Initialize array of sets
	cache->sets = xcalloc(num_sets, sizeof(struct cache_set_t));
	for (set = 0; set < num_sets; set++)
	{
		 Initialize array of blocks
		cache->sets[set].blocks = xcalloc(assoc, sizeof(struct cache_block_t));
		cache->sets[set].way_head = &cache->sets[set].blocks[0];
		cache->sets[set].way_tail = &cache->sets[set].blocks[assoc - 1];
		for (way = 0; way < assoc; way++)
		{
			block = &cache->sets[set].blocks[way];
			block->way = way;
			block->way_prev = way ? &cache->sets[set].blocks[way - 1] : NULL;
			block->way_next = way < assoc - 1 ? &cache->sets[set].blocks[way + 1] : NULL;
		}
	}

	return;
}*/

//star >> todo automate queue connection here
void connect_queue(struct list_t *queue){


	return;
}


//star >> need to know what cache reads from what queue and reads what data
/*void cache_poll_queues(void){

	struct cache_t *cache;
	struct list_t *queue;

	int cache_iterator, queue_iterator, i = 0;
	int num_ports = 0;
	int num_queues = 0;

	int message = 0;

	//iterate through each cache structure
	LIST_FOR_EACH(cache_list, cache_iterator)
	{
		cache = list_get(cache_list, cache_iterator);

		//iterate through each queue in each cache structure.
		LIST_FOR_EACH(cache->in_queues, queue_iterator)
		{
			queue = list_get(cache->in_queues, queue_iterator);

			//message = cache_poll_queue(queue);

			//debug
			printf("Cache %s queue %s size is %d\n", cache->name, queue->name, list_count(queue));
			//printf("message %d\n", message);

			//star >> todo add arbitration.
		}

	}

	return;
}*/

int cache_ctrl(struct list_t *queue){

	/*long long i = 1;

	while(1)
	{

		await(queue_has_data, i);

		printf("cache_ctrl\n");

		advance(stop);

	}*/

	return 0;
}


