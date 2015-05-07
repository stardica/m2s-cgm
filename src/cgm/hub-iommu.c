/*
 * IOMMU.c
 *
 *  Created on: May 2, 2015
 *      Author: stardica
 */

#include <stdio.h>
#include <stdlib.h>

#include <arch/si/timing/gpu.h>

#include <lib/util/string.h>
#include <lib/util/debug.h>

#include <cgm/tasking.h>
#include <cgm/hub-iommu.h>
#include <cgm/switch.h>
#include <cgm/packet.h>



//max number of GPU hub queues 8:1
struct str_map_t queue_strn_map =
{ 	queue_num, {

		{ "hub_iommu.Rx_queue_top[0]", Rx_queue_top_0},
		{ "hub_iommu.Rx_queue_top[1]", Rx_queue_top_1},
		{ "hub_iommu.Rx_queue_top[2]", Rx_queue_top_2},
		{ "hub_iommu.Rx_queue_top[3]", Rx_queue_top_3},
		{ "hub_iommu.Rx_queue_top[4]", Rx_queue_top_4},
		{ "hub_iommu.Rx_queue_top[5]", Rx_queue_top_5},
		{ "hub_iommu.Rx_queue_top[6]", Rx_queue_top_6},
		{ "hub_iommu.Rx_queue_top[7]", Rx_queue_top_7},
		{ "hub_iommu.Rx_queue_bottom", Rx_queue_bottom},
		}
};


struct hub_iommu_t *hub_iommu;
eventcount volatile *hub_iommu_ec;
task *hub_iommu_tasks;
int hub_iommu_pid = 0;


void hub_iommu_init(void){

	hub_iommu_create();
	hub_iommu_create_tasks();

	return;
}

void hub_iommu_create(void){

	hub_iommu = (void *) calloc(1, sizeof(struct hub_iommu_t));

	return;
}

void hub_iommu_create_tasks(void){

	char buff[100];
	//int i = 0;

	hub_iommu_ec = (void *) calloc(1, sizeof(eventcount));
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "hub_iommu");
	hub_iommu_ec = new_eventcount(strdup(buff));


	hub_iommu_tasks = (void *) calloc(1, sizeof(task));
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "hub_iommu_ctrl");
	hub_iommu_tasks = create_task(hub_iommu_ctrl, DEFAULT_STACK_SIZE, strdup(buff));


	return;
}

struct cgm_packet_t *hub_iommu_get_from_queue(){


	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	int current_queue_num = -1;

	struct cgm_packet_t *new_message;

	//rotate the queues, of which we don't really know how many we have until runtime.
	//current_queue_num = str_map_string(&queue_strn_map, hub_iommu->last_queue->name);

	//at least one of the queues has a message packet.
	do
	{
		//round robin
		new_message = list_get(hub_iommu->next_queue, 0);

		//keep pointer to last queue
		hub_iommu->last_queue = hub_iommu->next_queue;

		//rotate
		current_queue_num = str_map_string(&queue_strn_map, hub_iommu->last_queue->name);

		if(current_queue_num < (gpu_group_cache_num -1))
		{
			//go to next queue
			current_queue_num ++;
			hub_iommu->next_queue = hub_iommu->Rx_queue_top[current_queue_num];
		}
		else if(current_queue_num == (gpu_group_cache_num -1))
		{
			//at the last top queue
			hub_iommu->next_queue = hub_iommu->Rx_queue_bottom;
		}
		else if(current_queue_num > gpu_group_cache_num)
		{
			//start at beginning.
			hub_iommu->next_queue = hub_iommu->Rx_queue_top[0];
		}
		else
		{
			fatal("hub_iommu_get_from_queue(): unexpected queuing behavior\n");

		}

	}while(new_message == NULL);


	//star todo change this to hub_iommu_debug_file
	/*CGM_DEBUG(hub_iommu_debug_file, "%s access_id %llu cycle %llu pulled from %s queue size %d\n",
			cache->name, new_message->access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));*/

	//shouldn't be exiting without a message
	assert(new_message != NULL);
	return new_message;

}

void hub_iommu_ctrl(void){

	int my_pid = hub_iommu_pid++;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	struct cgm_packet_t *message_packet;
	long long step = 1;

	while(1)
	{
		//we have received a packet
		await(hub_iommu_ec, step);
		step++;






	}
	fatal("hub_iommu_ctrl() quit\n");
	return;
}
