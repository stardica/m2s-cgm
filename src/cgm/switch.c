/*
 * switch.c
 *
 *  Created on: Feb 9, 2015
 *      Author: stardica
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


#include <lib/util/list.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>

#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>

#include <cgm/cgm.h>
#include <cgm/tasking.h>
#include <cgm/switch.h>
#include <cgm/hub-iommu.h>
#include <cgm/packet.h>
#include <cgm/cache.h>
#include <cgm/sys-agent.h>


struct switch_t *switches;
eventcount volatile *switches_ec;
task *switches_tasks;

int switch_pid = 0;


/*int *ring_adj_mat[node_number][node_number] =
{
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};*/

//supports quad core with ring bus

struct str_map_t l1_strn_map =
{ 	l1_number, {
		{ "l1_i_caches[0]", l1_i_cache_0},
		{ "l1_d_caches[0]", l1_d_cache_0},
		{ "l1_i_caches[1]", l1_i_cache_1},
		{ "l1_d_caches[1]", l1_d_cache_1},
		{ "l1_i_caches[2]", l1_i_cache_2},
		{ "l1_d_caches[2]", l1_d_cache_2},
		{ "l1_i_caches[3]", l1_i_cache_3},
		{ "l1_d_caches[3]", l1_d_cache_3},
		}
};


struct str_map_t gpu_l1_strn_map =
{ gpu_l1_number, {
		{ "gpu_s_caches[0]", gpu_s_cache_0},
		{ "gpu_v_caches[0]", gpu_v_cache_0},
		{ "gpu_s_caches[1]", gpu_s_cache_1},
		{ "gpu_v_caches[1]", gpu_v_cache_1},
		{ "gpu_s_caches[2]", gpu_s_cache_2},
		{ "gpu_v_caches[2]", gpu_v_cache_2},
		{ "gpu_s_caches[3]", gpu_s_cache_3},
		{ "gpu_v_caches[3]", gpu_v_cache_3},
		{ "gpu_s_caches[4]", gpu_s_cache_4},
		{ "gpu_v_caches[4]", gpu_v_cache_4},
		{ "gpu_s_caches[5]", gpu_s_cache_5},
		{ "gpu_v_caches[5]", gpu_v_cache_5},
		{ "gpu_s_caches[6]", gpu_s_cache_6},
		{ "gpu_v_caches[6]", gpu_v_cache_6},
		{ "gpu_s_caches[7]", gpu_s_cache_7},
		{ "gpu_v_caches[7]", gpu_v_cache_7},
		{ "gpu_s_caches[8]", gpu_s_cache_8},
		{ "gpu_v_caches[8]", gpu_v_cache_8},
		{ "gpu_s_caches[9]", gpu_s_cache_9},
		{ "gpu_v_caches[9]", gpu_v_cache_9},
		{ "gpu_s_caches[10]", gpu_s_cache_10},
		{ "gpu_v_caches[10]", gpu_v_cache_10},
		{ "gpu_s_caches[11]", gpu_s_cache_11},
		{ "gpu_v_caches[11]", gpu_v_cache_11},
		{ "gpu_s_caches[12]", gpu_s_cache_12},
		{ "gpu_v_caches[12]", gpu_v_cache_12},
		{ "gpu_s_caches[13]", gpu_s_cache_13},
		{ "gpu_v_caches[13]", gpu_v_cache_13},
		{ "gpu_s_caches[14]", gpu_s_cache_14},
		{ "gpu_v_caches[14]", gpu_v_cache_14},
		{ "gpu_s_caches[15]", gpu_s_cache_15},
		{ "gpu_v_caches[15]", gpu_v_cache_15},
		{ "gpu_s_caches[16]", gpu_s_cache_16},
		{ "gpu_v_caches[16]", gpu_v_cache_16},
		{ "gpu_s_caches[17]", gpu_s_cache_17},
		{ "gpu_v_caches[17]", gpu_v_cache_17},
		{ "gpu_s_caches[18]", gpu_s_cache_18},
		{ "gpu_v_caches[18]", gpu_v_cache_18},
		{ "gpu_s_caches[19]", gpu_s_cache_19},
		{ "gpu_v_caches[19]", gpu_v_cache_19},
		{ "gpu_s_caches[20]", gpu_s_cache_20},
		{ "gpu_v_caches[20]", gpu_v_cache_20},
		{ "gpu_s_caches[21]", gpu_s_cache_21},
		{ "gpu_v_caches[21]", gpu_v_cache_21},
		{ "gpu_s_caches[22]", gpu_s_cache_22},
		{ "gpu_v_caches[22]", gpu_v_cache_22},
		{ "gpu_s_caches[23]", gpu_s_cache_23},
		{ "gpu_v_caches[23]", gpu_v_cache_23},
		{ "gpu_s_caches[24]", gpu_s_cache_24},
		{ "gpu_v_caches[24]", gpu_v_cache_24},
		{ "gpu_s_caches[25]", gpu_s_cache_25},
		{ "gpu_v_caches[25]", gpu_v_cache_25},
		{ "gpu_s_caches[26]", gpu_s_cache_26},
		{ "gpu_v_caches[26]", gpu_v_cache_26},
		{ "gpu_s_caches[27]", gpu_s_cache_27},
		{ "gpu_v_caches[27]", gpu_v_cache_27},
		{ "gpu_s_caches[28]", gpu_s_cache_28},
		{ "gpu_v_caches[28]", gpu_v_cache_28},
		{ "gpu_s_caches[29]", gpu_s_cache_29},
		{ "gpu_v_caches[29]", gpu_v_cache_29},
		{ "gpu_s_caches[30]", gpu_s_cache_30},
		{ "gpu_v_caches[30]", gpu_v_cache_30},
		{ "gpu_s_caches[31]", gpu_s_cache_31},
		{ "gpu_v_caches[31]", gpu_v_cache_31},
		}
};


struct str_map_t gpu_l2_strn_map =
{ gpu_l2_number, {
		{ "gpu_l2_caches[0]", gpu_l2_caches_0},
		{ "gpu_l2_caches[1]", gpu_l2_caches_1},
		{ "gpu_l2_caches[2]", gpu_l2_caches_2},
		{ "gpu_l2_caches[3]", gpu_l2_caches_3},
		{ "gpu_l2_caches[4]", gpu_l2_caches_4},
		{ "gpu_l2_caches[5]", gpu_l2_caches_5},
		{ "gpu_l2_caches[6]", gpu_l2_caches_6},
		{ "gpu_l2_caches[7]", gpu_l2_caches_7},
		}
};

struct str_map_t node_strn_map =
{ node_number, {
		{ "l2_caches[0]", l2_cache_0},
		{ "switch[0]", switch_0},
		{ "l3_caches[0]", l3_cache_0},
		{ "l2_caches[1]", l2_cache_1},
		{ "switch[1]", switch_1},
		{ "l3_caches[1]", l3_cache_1},
		{ "l2_caches[2]", l2_cache_2},
		{ "switch[2]", switch_2},
		{ "l3_caches[2]", l3_cache_2},
		{ "l2_caches[3]", l2_cache_3},
		{ "switch[3]", switch_3},
		{ "l3_caches[3]", l3_cache_3},
		{ "hub_iommu", hub_iommu_4},
		{ "switch[4]", switch_4},
		{ "sys_agent", sys_agent_4},
		}
};

struct str_map_t port_name_map =
{ 	port_num, {
		{ "north_queue", north_queue},
		{ "east_queue", east_queue},
		{ "south_queue", south_queue},
		{ "west_queue", west_queue},
		}
};


void switch_init(void){

	switch_create();
	//route_create();
	switch_create_tasks();
	return;
}


void switch_create(void){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;

	//for now the number of GPU connected switches is hard coded
	//this one switch for all of the GPU.
	//star todo fix this
	int extras = 1;

	//for now model a ring bus on each CPU
	switches = (void *) calloc((num_cores + extras), sizeof(struct switch_t));

	return;
}

void switch_create_tasks(void){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;

	//star todo fix this
	int extras = 1;

	char buff[100];
	int i = 0;

	switches_ec = (void *) calloc((num_cores + extras), sizeof(eventcount));
	for(i = 0; i < (num_cores + extras); i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "switch_%d", i);
		switches_ec[i] = *(new_eventcount(strdup(buff)));
	}

	switches_tasks = (void *) calloc((num_cores + extras), sizeof(task));
	for(i = 0; i < (num_cores + extras); i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "switch_ctrl_%d", i);
		switches_tasks[i] = *(create_task(switch_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	return;
}

int switch_can_access(struct list_t *queue){

	//check if in queue is full
	if(QueueSize <= list_count(queue))
	{
		return 0;
	}

	return 1;
}


void switch_ctrl(void){

	int my_pid = switch_pid++;
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	struct cgm_packet_t *message_packet;
	long long step = 1;
	int next_switch = 0;
	int queue_status;

	char *dest_name;
	int dest_node;
	char *src_name;
	int src_node;
	int switch_node = switches[my_pid].switch_node_number;
	float distance;

	assert(my_pid <= (num_cores + num_cus));

	set_id((unsigned int)my_pid);

	while(1)
	{

		//we have received a packet
		await(&switches_ec[my_pid], step);
		step++;

		//if we made it here we should have a packet.
		message_packet = get_from_queue(&switches[my_pid]);
		assert(message_packet);

		//send the packet to it's destination OR on to the next hop
		//look up the node number of the destination
		dest_name = message_packet->dest_name;
		dest_node = message_packet->dest_id;

		src_name = message_packet->src_name;
		src_node = message_packet->src_id;

		CGM_DEBUG(switch_debug_file,"%s access_id %llu cycle %llu src %s dest %s\n",
			switches[my_pid].name, message_packet->access_id, P_TIME, message_packet->src_name, message_packet->dest_name);

		P_PAUSE(switches[my_pid].latency);

		//if dest is the L2/L3/SA connected to this switch.
		if(dest_node == (switch_node - 1) || dest_node == (switch_node +1))
		{
			//if the node number is lower this means it is an L2 cache
			if(dest_node < switch_node)
			{
				//for CPU L2s
				if(my_pid < num_cores)
				{
					//make sure we can access the cache
					//star todo add the ability to do something else if we can access the target cache
					while(!cache_can_access_bottom(&l2_caches[my_pid]))
					{
						//the L2 cache queue is full try again next cycle
						P_PAUSE(1);
					}

					//success, remove packet from the switche's queue
					remove_from_queue(&switches[my_pid], message_packet);

					//drop the packet into the cache's queue
					list_enqueue(l2_caches[my_pid].Rx_queue_bottom, message_packet);
					future_advance(&l2_cache[my_pid], WIRE_DELAY(l2_caches[my_pid].wire_latency));

					//done with this access
					CGM_DEBUG(switch_debug_file,"%s access_id %llu cycle %llu delivered\n", switches[my_pid].name, message_packet->access_id, P_TIME);
				}
				//GPU and other L2 caches
				else if(my_pid >= num_cores)
				{
					while(!hub_iommu_can_access(hub_iommu->Rx_queue_bottom))
					{
						//the hub-iommu cache queue is full try again next cycle
						P_PAUSE(1);
					}

					//success, remove packet from the switche's queue
					remove_from_queue(&switches[my_pid], message_packet);

					//drop the packet into the hub's
					list_enqueue(hub_iommu->Rx_queue_bottom, message_packet);
					future_advance(hub_iommu_ec, WIRE_DELAY(hub_iommu->wire_latency));
					//done with this access
					CGM_DEBUG(switch_debug_file,"%s access_id %llu cycle %llu delivered\n", switches[my_pid].name, message_packet->access_id, P_TIME);
				}

			}
			//if the node number is high this means it is an L3 cache or the sys agent
			else if(dest_node > switch_node)
			{
				//for CPU L3 caches
				if(my_pid < num_cores)
				{
					//make sure we can access the cache
					//star todo add the ability to do something else if we can access the target cache
					while(!cache_can_access_top(&l3_caches[my_pid]))
					{
						//the L2 cache queue is full try again next cycle
						P_PAUSE(1);
					}

					//success, remove packet from the switche's queue
					remove_from_queue(&switches[my_pid], message_packet);

					//message_packet->access_type = cgm_access_puts;


					/////////test code
					/*message_packet->access_type = cgm_access_puts;
					message_packet->dest_name = message_packet->src_name;
					message_packet->dest_id = str_map_string(&node_strn_map, message_packet->src_name);
					message_packet->src_name = l3_caches[0].name;
					message_packet->src_id = str_map_string(&node_strn_map, l3_caches[0].name);
					list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);
					future_advance(&l2_cache[my_pid], WIRE_DELAY(l2_caches[my_pid].wire_latency));*/
					/////////test code


					//old code
					//drop the packet into the cache's queue
					list_enqueue(l3_caches[my_pid].Rx_queue_top, message_packet);
					future_advance(&l3_cache[my_pid], WIRE_DELAY(l3_caches[my_pid].wire_latency));
					//done with this access
					CGM_DEBUG(switch_debug_file,"%s access_id %llu cycle %llu delivered\n", switches[my_pid].name, message_packet->access_id, P_TIME);

				}
				//for the system agent
				else if(my_pid >= num_cores)
				{

					while(!sys_agent_can_access_top())
					{
						//the sys agent queue is full try again next cycle
						P_PAUSE(1);
					}

					//success, remove packet from the switche's queue
					remove_from_queue(&switches[my_pid], message_packet);

					//drop the packet into the sys agent queue
					list_enqueue(system_agent->Rx_queue_top, message_packet);
					future_advance(system_agent_ec, WIRE_DELAY(system_agent->wire_latency));
					//done with this access

					CGM_DEBUG(switch_debug_file,"%s access_id %llu cycle %llu delivered\n", switches[my_pid].name, message_packet->access_id, P_TIME);
				}
			}
			else
			{
				fatal("switch_ctrl() switch % d: Invalid dest_node_number \n", my_pid);
			}

		}
		else
		{
			//send packet to adjacent switch
			//there is no transfer direction established.
			if(switches[my_pid].queue == north_queue || switches[my_pid].queue == south_queue)
			{
				//new packets from connected L2 or L3 cache.
				if(dest_node > src_node)
				{
					distance = switch_get_distance(dest_node, src_node);

					/*//get the distance from this switch to the destination (left to right)
					if(dest_node % 3 == 0 && src_node % 3 == 0)
					{
						//L2 to L2
						distance = (dest_node - src_node)/3;
					}
					else if(dest_node % 3 != 0 && src_node % 3 == 0)
					{
						//L2 to L3/SA || L3 to L2 (works for both ways)
						distance = (dest_node - (src_node + 2))/3;
					}
					else
					{
						//L3 to L3/SA
						distance = (dest_node - src_node)/3;
					}*/

					//go in the direction with the shortest number of hops.
					if(distance <= switches[my_pid].switch_median_node)
					{//go east

						while(!switch_can_access(switches[my_pid].next_east))
						{
							//the switch queue is full try again next cycle
							P_PAUSE(1);
						}

						//success, remove packet from the switche's queue
						remove_from_queue(&switches[my_pid], message_packet);

						//drop the packet into the next switche's queue
						list_enqueue(switches[my_pid].next_east, message_packet);
						future_advance(&switches_ec[switches[my_pid].next_east_id], WIRE_DELAY(switches[switches[my_pid].next_east_id].wire_latency));

						CGM_DEBUG(switch_debug_file,"%s access_id %llu cycle %llu delivered to next hop\n", switches[my_pid].name, message_packet->access_id, P_TIME);

					}
					else
					{//go west

						while(!switch_can_access(switches[my_pid].next_west))
						{
							//the switch queue is full try again next cycle
							P_PAUSE(1);
						}

						//success, remove packet from the switche's queue
						remove_from_queue(&switches[my_pid], message_packet);

						//drop the packet into the next switche's queue
						list_enqueue(switches[my_pid].next_west, message_packet);
						future_advance(&switches_ec[switches[my_pid].next_west_id], WIRE_DELAY(switches[switches[my_pid].next_west_id].wire_latency));

						CGM_DEBUG(switch_debug_file,"%s access_id %llu cycle %llu delivered to next hop\n", switches[my_pid].name, message_packet->access_id, P_TIME);
					}
				}
				else if(src_node > dest_node)
				{

					distance = switch_get_distance(dest_node, src_node);

					//go in the direction with the shortest number of hops.
					if(distance <= switches[my_pid].switch_median_node)
					{//go west

						while(!switch_can_access(switches[my_pid].next_west))
						{
							//the switch queue is full try again next cycle
							P_PAUSE(1);
						}

						//success, remove packet from the switche's queue
						remove_from_queue(&switches[my_pid], message_packet);

						//drop the packet into the next switche's queue
						list_enqueue(switches[my_pid].next_west, message_packet);
						future_advance(&switches_ec[switches[my_pid].next_west_id], WIRE_DELAY(switches[switches[my_pid].next_west_id].wire_latency));
						CGM_DEBUG(switch_debug_file,"%s access_id %llu cycle %llu delivered to next hop\n", switches[my_pid].name, message_packet->access_id, P_TIME);

					}
					else
					{//go east

						while(!switch_can_access(switches[my_pid].next_east))
						{
							//the switch queue is full try again next cycle
							P_PAUSE(1);
						}

						//success, remove packet from the switche's queue
						remove_from_queue(&switches[my_pid], message_packet);

						//drop the packet into the next switche's queue
						list_enqueue(switches[my_pid].next_east, message_packet);
						future_advance(&switches_ec[switches[my_pid].next_east_id], WIRE_DELAY(switches[switches[my_pid].next_east_id].wire_latency));
						CGM_DEBUG(switch_debug_file,"%s access_id %llu cycle %llu delivered to next hop\n", switches[my_pid].name, message_packet->access_id, P_TIME);
					}
				}
			}
			else if(switches[my_pid].queue == east_queue || switches[my_pid].queue == west_queue)
			{
				//packet came from another switch, but needs to continue on.

				if(switches[my_pid].queue == east_queue)
				{//go west

					while(!switch_can_access(switches[my_pid].next_west))
					{
						//the switch queue is full try again next cycle
						P_PAUSE(1);
					}

					//success, remove packet from the switche's queue
					remove_from_queue(&switches[my_pid], message_packet);

					//drop the packet into the next switche's queue
					list_enqueue(switches[my_pid].next_west, message_packet);
					future_advance(&switches_ec[switches[my_pid].next_west_id], WIRE_DELAY(switches[switches[my_pid].next_west_id].wire_latency));
					CGM_DEBUG(switch_debug_file,"%s access_id %llu cycle %llu delivered to next hop\n", switches[my_pid].name, message_packet->access_id, P_TIME);

				}
				else if(switches[my_pid].queue == west_queue)
				{//go east
					while(!switch_can_access(switches[my_pid].next_east))
					{
						//the switch queue is full try again next cycle
						P_PAUSE(1);
					}

					//success, remove packet from the switche's queue
					remove_from_queue(&switches[my_pid], message_packet);

					//drop the packet into the next switche's queue
					list_enqueue(switches[my_pid].next_east, message_packet);
					future_advance(&switches_ec[switches[my_pid].next_east_id], WIRE_DELAY(switches[switches[my_pid].next_east_id].wire_latency));
					CGM_DEBUG(switch_debug_file,"%s access_id %llu cycle %llu delivered to next hop\n", switches[my_pid].name, message_packet->access_id, P_TIME);

				}
				else
				{
					fatal("switch_ctrl() directional queue error.\n");

				}

			}

		}
		//end, clear the message_packet ptr
		//this should be getting set up above in list_get(), but just for safe measure.
		message_packet = NULL;

	}

	fatal("switch_ctrl() quit\n");
	return;
}


float switch_get_distance(int dest_node, int src_node){

	float distance = 0;

	if(dest_node > src_node)
	{
		//get the distance from this switch to the destination (left to right)
		if(dest_node % 3 == 0 && src_node % 3 == 0)
		{
			//L2 to L2
			distance = (dest_node - src_node)/3;
		}
		else if(dest_node % 3 != 0 && src_node % 3 == 0)
		{
			//L2 to L3/SA || L3 to L2 (works for both ways)
			distance = (dest_node - (src_node + 2))/3;
		}
		else
		{
			//L3 to L3/SA
			distance = (dest_node - src_node)/3;
		}
	}
	else
	{
		//get the distance from this switch to the destination (right to left)
		if(src_node % 3 == 0 && dest_node % 3 == 0)
		{
			//L2 to L2
			distance = (src_node - dest_node)/3;
		}
		else if(src_node % 3 == 0 && dest_node % 3 != 0)
		{
			//L2 to L3/SA || L3 to L2 (works for both ways)
			distance = ((src_node + 2) - dest_node)/3;
		}
		else
		{
			//L3 to L3/SA
			distance = (src_node - dest_node)/3;
		}
	}
	return distance;
}


struct cgm_packet_t *get_from_queue(struct switch_t *switches){

	struct cgm_packet_t *new_packet;
	int i = 0;

	//choose a port this cycle to work from
	if(switches->arb_style == round_robin)
	{
		for(i = 0; i < switches->port_num; i++)
		{
			//set switches->queue to the next queue.
			switches->queue = get_next_queue_rb(switches->queue);

			//if we don't have a message go on to the next.
			if(switches->queue == north_queue)
			{
				new_packet = list_get(switches->north_queue, 0);
				switches->current_queue = switches->north_queue;
			}
			else if(switches->queue == east_queue)
			{
				new_packet = list_get(switches->east_queue, 0);
				switches->current_queue = switches->east_queue;
			}
			else if(switches->queue == south_queue)
			{
				new_packet = list_get(switches->south_queue, 0);
				switches->current_queue = switches->south_queue;
			}
			else if(switches->queue == west_queue)
			{
				new_packet = list_get(switches->west_queue, 0);
				switches->current_queue = switches->west_queue;
			}

			//when we have a packet break out.
			//next advance start with the next queue
			if(new_packet)
			{
				i = 0;
				break;
			}

		}

	}
	else
	{
		fatal("get_from_queue() invalid arbitration set switch %s\n", switches->name);
	}

	CGM_DEBUG(switch_debug_file, "%s access_id %llu cycle %llu ptr get from %s with size %d\n",
			switches->name, new_packet->access_id, P_TIME, (char *)str_map_value(&port_name_map, switches->queue), list_count(switches->current_queue));

	return new_packet;
}



void remove_from_queue(struct switch_t *switches, struct cgm_packet_t *message_packet){


	if(switches->queue == north_queue)
	{
		list_remove(switches->north_queue, message_packet);
	}
	else if(switches->queue == east_queue)
	{
		list_remove(switches->east_queue, message_packet);
	}
	else if(switches->queue == south_queue)
	{
		list_remove(switches->south_queue, message_packet);
	}
	else if(switches->queue == west_queue)
	{
		list_remove(switches->west_queue, message_packet);
	}
	else
	{
		fatal("remove_from_queue() invalid port name\n");
	}

	CGM_DEBUG(switch_debug_file, "%s access_id %llu cycle %llu removed from %s with size %d\n",
			switches->name, message_packet->access_id, P_TIME, (char *)str_map_value(&port_name_map, switches->queue), list_count(switches->current_queue));

	return;
}

enum port_name get_next_queue_rb(enum port_name queue){

	enum port_name next_queue;

	if(queue == west_queue)
	{
		next_queue = north_queue;
	}
	else if(queue == north_queue)
	{
		next_queue = east_queue;
	}
	else if(queue == east_queue)
	{
		next_queue = south_queue;
	}
	else if(queue == south_queue)
	{
		next_queue = west_queue;
	}
	else
	{
		fatal("get_next_queue() Invalid port name\n");
	}

	return next_queue;
}
