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
#include <cgm/packet.h>
#include <cgm/cgm.h>



#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>
#include <lib/util/debug.h>


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


void cache_init(void){

	//star todo make this automatic
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
	l3_caches = (void *) calloc(num_cores, sizeof(struct cache_t));
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

	return;
}


int i_cache_ctrl(int id, enum cgm_access_kind_t task){


	printf("queue name = %s\n", l1_i_caches[id].Rx_queue->name);
	printf("task = %u\n", task);
	//getchar();

	unsigned int addr;
	struct cgm_packet_t *packet;
	int status;

	int *set_ptr;
	int *pway;
	int *state_ptr;


	//fetch access
	if (task == cgm_access_load)
	{

		packet = list_dequeue(l1_i_caches[id].Rx_queue);

		printf("access id = %llu\n", packet->access_id);
		printf("in flight = %d\n", packet->in_flight);
		printf("address 0x%08x\n", packet->address);
		fflush(stdout);
		getchar();

		if(!packet)
		{
			fatal("l1_i_cache no packet\n");
		}
		else
		{


			status = cache_find_block(l1_i_caches[id], addr, set_ptr, pway, state_ptr);

			printf("Status %d\n", status);

		}


	}
	else if (task == cgm_access_nc_load)
	{
		fatal("Unsupported i_cache task = cgm_access_nc_load\n");
	}
	else
	{
		fatal("Unsupported i_cache task = all else\n");
	}


	//retire access in master list.
	//list_dequeue(cgm_access_record);

	return 0;
}



/*long long i = 1;

	while(1)
	{

		await(queue_has_data, i);

		printf("cache_ctrl\n");

		advance(stop);

	}
*/
