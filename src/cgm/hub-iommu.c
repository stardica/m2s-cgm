/*
 * IOMMU.c
 *
 *  Created on: May 2, 2015
 *      Author: stardica
 */



#include <cgm/hub-iommu.h>


/*
#include <stdio.h>
#include <stdlib.h>

#include <arch/si/timing/gpu.h>

#include <lib/util/list.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>

#include <cgm/cgm.h>
#include <cgm/tasking.h>
#include <cgm/hub-iommu.h>
#include <cgm/switch.h>
#include <cgm/packet.h>
*/


//max number of GPU hub queues 8:1
struct str_map_t Rx_queue_strn_map =
{ 	Rx_queue_num, {

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

struct str_map_t Tx_queue_strn_map =
{ 	Rx_queue_num, {

		{ "hub_iommu.Tx_queue_top[0]", Tx_queue_top_0},
		{ "hub_iommu.Tx_queue_top[1]", Tx_queue_top_1},
		{ "hub_iommu.Tx_queue_top[2]", Tx_queue_top_2},
		{ "hub_iommu.Tx_queue_top[3]", Tx_queue_top_3},
		{ "hub_iommu.Tx_queue_top[4]", Tx_queue_top_4},
		{ "hub_iommu.Tx_queue_top[5]", Tx_queue_top_5},
		{ "hub_iommu.Tx_queue_top[6]", Tx_queue_top_6},
		{ "hub_iommu.Tx_queue_top[7]", Tx_queue_top_7},
		{ "hub_iommu.Tx_queue_bottom", Tx_queue_bottom},
		}
};


struct hub_iommu_t *hub_iommu;
eventcount volatile *hub_iommu_ec;
task *hub_iommu_tasks;
int hub_iommu_pid = 0;
int hub_iommu_io_up_pid = 0;
int hub_iommu_io_down_pid = 0;


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

int hub_iommu_can_access(struct list_t *queue){

	//check if in queue is full
	if(QueueSize <= list_count(queue))
	{
		return 0;
	}

	return 1;
}

struct cgm_packet_t *hub_iommu_get_from_queue(void){


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
		current_queue_num = str_map_string(&Rx_queue_strn_map, hub_iommu->next_queue->name);

		if(current_queue_num < (gpu_group_cache_num -1))
		{
			//go to next queue
			current_queue_num++;
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

	CGM_DEBUG(hub_iommu_debug_file, "%s access_id %llu cycle %llu pulled from %s queue size %d\n",
			hub_iommu->name, new_message->access_id, P_TIME,hub_iommu->last_queue->name, list_count(hub_iommu->last_queue));

	//shouldn't be exiting without a message
	return new_message;
}

void hub_iommu_put_next_queue(struct cgm_packet_t *message_packet){

	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	int last_queue_num = -1;
	int l2_src_id = -1;

	//get the number of the last queue
	last_queue_num = str_map_string(&Rx_queue_strn_map, hub_iommu->last_queue->name);

	//if we are pointing to one of the top queues put the packet on the switch.
	if(last_queue_num >= 0 && last_queue_num <= (gpu_group_cache_num -1))
	{

		/*while(!switch_can_access(hub_iommu->switch_queue))
		{
			printf("hub stalling\n");
			P_PAUSE(1);
		}*/

		//switch queue has a slot

		//save the gpu l2 cache id
		message_packet->l2_cache_id = message_packet->src_id;
		message_packet->l2_cache_name = message_packet->src_name;

		//change src name and id
		message_packet->src_name = hub_iommu->name;
		message_packet->src_id = str_map_string(&node_strn_map, hub_iommu->name);

		message_packet = list_remove(hub_iommu->last_queue, message_packet);
		assert(message_packet);

		//list_enqueue(hub_iommu->switch_queue, message_packet);
		//advance(&switches_ec[hub_iommu->switch_id]);
		//future_advance(&switches_ec[hub_iommu->switch_id], WIRE_DELAY(switches[hub_iommu->switch_id].wire_latency));

		list_enqueue(hub_iommu->Tx_queue_bottom, message_packet);
		advance(hub_iommu->hub_iommu_io_down_ec);

		/*printf("hub_iommu ctrl send size %d\n", list_count(hub_iommu->Tx_queue_bottom));*/

		CGM_DEBUG(hub_iommu_debug_file,"%s access_id %llu cycle %llu delivered\n",
				hub_iommu->name, message_packet->access_id, P_TIME);

	}
	//if we are pointing to the bottom queue route to the correct GPU l2 cache
	else if(last_queue_num == Rx_queue_bottom)
	{
		//return trip for memory access
		l2_src_id = str_map_string(&gpu_l2_strn_map, message_packet->l2_cache_name);

		//star todo fix this
		while(!cache_can_access_bottom(&gpu_l2_caches[l2_src_id]))
		{
			printf("hub stalling up\n");
			P_PAUSE(1);
		}

		message_packet = list_remove(hub_iommu->last_queue, message_packet);

		//list_enqueue(gpu_l2_caches[l2_src_id].Rx_queue_bottom, message_packet);
		//advance(&gpu_l2_cache[l2_src_id]);
		//future_advance(&gpu_l2_cache[l2_src_id], WIRE_DELAY(gpu_l2_caches[l2_src_id].wire_latency));

		list_enqueue(hub_iommu->Tx_queue_top[l2_src_id], message_packet);
		advance(hub_iommu->hub_iommu_io_up_ec[l2_src_id]);

	}
	else
	{
		fatal("hub_iommu_put_next_queue(): got a queue id that is out of range\n");

	}

	return;
}

void hub_iommu_ctrl(void){

	int my_pid = hub_iommu_pid++;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	struct cgm_packet_t *message_packet;

	long long step = 1;

	set_id((unsigned int)my_pid);

	while(1)
	{
		//we have received a packet
		await(hub_iommu_ec, step);

		//star todo figure out a way to check the l2 in queue
		if(!switch_can_access(hub_iommu->switch_queue))// && !cache_can_access_bottom(&gpu_l2_caches[l2_src_id]))
		{
			P_PAUSE(1);
		}
		else
		{
			step++;

			//if we made it here we should have a packet.
			message_packet = hub_iommu_get_from_queue();
			assert(message_packet);

			CGM_DEBUG(hub_iommu_debug_file,"%s access_id %llu cycle %llu src %s dest %s\n",
					hub_iommu->name, message_packet->access_id, P_TIME, message_packet->src_name, message_packet->dest_name);

			//star todo add GPU virtual to physical translation here
			P_PAUSE(hub_iommu->latency);

			//star the hub mostly multiplexes GPU memory requests.
			hub_iommu_put_next_queue(message_packet);
		}
	}

	fatal("hub_iommu_ctrl(): reached end of func\n");
	return;
}



void hub_iommu_io_up_ctrl(void){

	int my_pid = hub_iommu_io_up_pid++;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	struct cgm_packet_t *message_packet;

	long long access_id = 0;
	int transfer_time = 0;
	long long step = 1;


	set_id((unsigned int)my_pid);

	while(1)
	{
		await(hub_iommu->hub_iommu_io_up_ec[my_pid], step);
		step++;

		message_packet = list_dequeue(hub_iommu->Tx_queue_top[my_pid]);
		assert(message_packet);

		access_id = message_packet->access_id;
		transfer_time = (message_packet->size/hub_iommu->bus_width);

		P_PAUSE(transfer_time);

		list_enqueue(gpu_l2_caches[my_pid].Rx_queue_bottom, message_packet);
		advance(&gpu_l2_cache[my_pid]);
	}
	return;
}

void hub_iommu_io_down_ctrl(void){

	int my_pid = hub_iommu_io_down_pid++;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	struct cgm_packet_t *message_packet;

	long long access_id = 0;
	int transfer_time = 0;
	long long step = 1;

	set_id((unsigned int)my_pid);


	while(1)
	{

		await(hub_iommu->hub_iommu_io_down_ec, step);
		step++;

		message_packet = list_dequeue(hub_iommu->Tx_queue_bottom);
		assert(message_packet);

		access_id = message_packet->access_id;
		transfer_time = (message_packet->size/hub_iommu->bus_width);

		P_PAUSE(transfer_time);

		list_enqueue(hub_iommu->switch_queue, message_packet);
		advance(&switches_ec[hub_iommu->switch_id]);
	}

	return;
}

