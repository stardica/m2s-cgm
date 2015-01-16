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


//L1 caches
struct cache_t *l1_inst_cache;
struct cache_t *l1_data_cache;

//L2 caches
struct cache_t *l2_cache;

//L3 caches
struct cache_t *l3_cache;

struct list_t *cache_list;

	/*
	//Memory controller
	q_mc_0_L3Request = list_create();
	q_mc_0_L3Reply = list_create();*/



void cache_init(void){

	//core 1
	//Create L1 caches
	l1_inst_cache = cgm_cache_create();
	l1_inst_cache->name = "l1_inst_cache";
	l1_data_cache = cgm_cache_create();
	l1_data_cache->name = "l1_data_cache";

	//Create L2 caches
	l2_cache = cgm_cache_create();
	l2_cache->name = "l2_cache";

	//Create L3 caches
	l3_cache  = cgm_cache_create();
	l3_cache->name = "l3_cache";

	//star >> put the caches in a list for easy access later on. list is global.
	cache_list = list_create();
	list_add(cache_list, l1_inst_cache);
	list_add(cache_list, l1_data_cache);
	list_add(cache_list, l2_cache);
	list_add(cache_list, l3_cache);

	//star connect queues to caches
	//star todo automate this, the queue names are going to be a problem...
	//L1 inst cache
	list_add(l1_inst_cache->in_queues, q_l1i_0_CoreRequest);
	list_add(l1_inst_cache->out_queues, q_l1i_0_L1iReply);

	//L1 data cache
	list_add(l1_data_cache->in_queues, q_l1d_0_CoreRequest);
	list_add(l1_data_cache->out_queues, q_l1d_0_L1dReply);

	//L2 cache
	list_add(l2_cache->in_queues, q_l2_0_L1iRequest);
	list_add(l2_cache->in_queues, q_l2_0_L1dRequest);
	list_add(l2_cache->out_queues, q_l2_0_L2L1iReply);
	list_add(l2_cache->out_queues, q_l2_0_L2L1dReply);

	//L3 cache
	list_add(l3_cache->in_queues, q_l3_0_L2Request);
	list_add(l3_cache->in_queues, q_l3_0_L3Reply);


	/*//create tasks
	char * taskname = "cache_ctrl";
	create_task(cache_ctrl, DEFAULT_STACK_SIZE, taskname);*/

	return;

}

struct cache_t *cgm_cache_create(void){

	struct cache_t *new_cache;

	new_cache = (void *) malloc(sizeof(struct cache_t));

	//star todo finish initializing cache state here.
	new_cache->in_queues = list_create();
	new_cache->out_queues = list_create();

	return new_cache;

}

//star >> todo automate queue connection here
void connect_queue(struct list_t *queue){


	return;
}


//star >> need to know what cache reads from what queue and reads what data
void cache_poll_queues(void){

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
}

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


