/*
 * cache.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>

#include <cgm/cache.h>
#include <cgm/queue.h>
#include <cgm/tasking.h>
#include <cgm/packet.h>
#include <cgm/cgm.h>


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
	struct cgm_packet_t *Rx_packet;

	while(1)
	{

		await(l1_i_cache_0, step);
		step++;

		Rx_packet = list_dequeue(cgm_access_record);

		if(TSK == 1)
		{
			printf("l1_i_cache_ctrl_0\n");
		}

		advance(l2_cache_0);

		//change this to something like if mem access complete then dequeue from the global list.
		if(1)
		{

		}

	}
		/*unsigned int addr;
		struct cgm_packet_t *packet;
		enum cgm_access_kind_t task;
		int hit;

		int *set_ptr = NULL;
		int *pway = NULL;
		int *state_ptr = NULL;


		//packet = list_dequeue(l1_i_caches[id].Rx_queue);
		if(!packet)
		{
			//fatal("l1_i_cache no packet\n");
		}
		else
		{
			addr = packet->address;
			task = packet->access_type;
		}


		if (task == cgm_access_load)
		{

			// 0 = miss 1 = hit
			//hit = cgm_cache_find_block(&(l1_i_caches[id]), addr, set_ptr, pway, state_ptr);

			printf("address 0x%08x\n", packet->address);
			printf("hit or miss %d\n", status);

			//remove this.
			hit = 1;
			if(hit)
			{
				//retire access in master list.
				list_dequeue(cgm_access_record);

			}
			else if (task == cgm_access_nc_load)
			{
				fatal("Unsupported i_cache task = cgm_access_nc_load\n");
			}
			else
			{
				fatal("Unsupported i_cache task = all else\n");
			}

		}*/

	return;
}

void l2_cache_ctrl_0(void){

	long long step = 1;
	struct cgm_packet_t *Rx_packet;

	while(1)
	{

		await(l2_cache_0, step);
		step++;

		if(TSK == 1)
		{
			printf("l2_cache_ctrl_0\n");

		}


		//change this to something like if mem access complete then dequeue from the global list.
		if(1)
		{

		}

	}

	return;
}

void l1_d_cache_ctrl(void){

	return;
}

int cgm_cache_find_block(struct cache_t *cache, unsigned int addr, int *set_ptr, int *way_ptr, int *state_ptr){


	//printf("cgm_cache_find_block()\n");
	//printf("working with %s\n", cache->name);
	//fatal("stop here\n");

	int set, tag, way;

	/* Locate block */
	tag = addr & ~cache->block_mask;
	set = (addr >> cache->log_block_size) % cache->num_sets;
	PTR_ASSIGN(set_ptr, set);
	PTR_ASSIGN(state_ptr, 0);  /* Invalid */
	for (way = 0; way < cache->assoc; way++)
		if (cache->sets[set].blocks[way].tag == tag && cache->sets[set].blocks[way].state)
			break;

	/* Block not found */
	if (way == cache->assoc)
		return 0;

	/* Block found */
	PTR_ASSIGN(way_ptr, way);
	PTR_ASSIGN(state_ptr, cache->sets[set].blocks[way].state);
	return 1;
}
