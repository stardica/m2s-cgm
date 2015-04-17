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
struct str_map_t node_strn_map =
{ 	node_number, {
		{ "l2_caches[0]", l2_cache_0},
		{ "switch[0]", switch_0},
		{ "l3_caches[0]", l3_cache_0},
		{ "l2_caches[1]", l2_cache_1},
		{ "switch[1]", switch_1},
		{ "l3_caches[1]", l3_cache_1},
		{ "l2_caches[2]", l2_cache_2},
		{ "switch[2]", switch_2},
		{ "l2_caches[3]", l2_cache_3},
		{ "switch[3]", switch_3},
		{ "l3_caches[2]", l3_cache_2},
		{ "l2_caches[4]", l2_cache_4},
		{ "switch[4]", switch_4},
		{ "l3_caches[3]", l3_cache_3},
		{ "sys_agent", sys_agent},
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


void switch_ctrl(void){

	int my_pid = switch_pid++;
	long long step = 1;

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;

	struct cgm_packet_t *message_packet;
	char *dest;
	int i = 0;
	int dest_node_number;
	float distance;
	int queue_status;

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
		dest = message_packet->dest_name;
		dest_node_number = str_map_string(&node_strn_map, dest);

		//if dest is the L2/L3/SA connected to this switch.
		if(dest_node_number == (switches[my_pid].switch_node_number - 1) || dest_node_number == (switches[my_pid].switch_node_number +1))
		{

			//if the node number is lower this means it is an L2 cache
			if(dest_node_number < switches[my_pid].switch_node_number)
			{
				//for CPU L2s
				if(my_pid < num_cores)
				{
					//make sure we can access the cache
					//star todo add the ability to do something else if we can access the target cache
					while(!cache_can_access(&l2_caches[my_pid]))
					{
						//the L2 cache queue is full try again next cycle
						P_PAUSE(1);
					}

					//success, remove packet from the switche's queue
					remove_from_queue(&switches[my_pid], message_packet);

					//drop the packet into the cache's queue
					list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);
					future_advance(&l2_cache[my_pid], (etime.count + l2_caches[my_pid].wire_latency));
					//done with this access
				}
				//GPU and other L2 caches
				else if(my_pid >= num_cores)
				{
					while(!cache_can_access(&gpu_l2_caches[my_pid]))
					{
						//the L2 cache queue is full try again next cycle
						P_PAUSE(1);
					}

					//success, remove packet from the switche's queue
					remove_from_queue(&switches[my_pid], message_packet);

					//drop the packet into the cache's queue
					list_enqueue(gpu_l2_caches[my_pid].Rx_queue_top, message_packet);
					future_advance(&gpu_l2_cache[my_pid], (etime.count + gpu_l2_caches[my_pid].wire_latency));
					//done with this access
				}

			}
			//if the node number is high this means it is an L3 cache or the sys agent
			else if(dest_node_number > switches[my_pid].switch_node_number)
			{

				//for CPU L3 caches
				if(my_pid < num_cores)
				{
					//make sure we can access the cache
					//star todo add the ability to do something else if we can access the target cache
					while(!cache_can_access(&l3_caches[my_pid]))
					{
						//the L2 cache queue is full try again next cycle
						P_PAUSE(1);
					}

					//success, remove packet from the switche's queue
					remove_from_queue(&switches[my_pid], message_packet);

					//drop the packet into the cache's queue
					list_enqueue(l3_caches[my_pid].Rx_queue_top, message_packet);
					future_advance(&l3_cache[my_pid], (etime.count + l3_caches[my_pid].wire_latency));
					//done with this access
				}
				//for the system agent
				else if(my_pid >= num_cores)
				{
					while(!sys_agent_can_access())
					{
						//the sys agent queue is full try again next cycle
						P_PAUSE(1);
					}

					//success, remove packet from the switche's queue
					remove_from_queue(&switches[my_pid], message_packet);

					//drop the packet into the sys agent queue
					list_enqueue(system_agent->Rx_queue_top, message_packet);
					future_advance(system_agent_ec, (etime.count + system_agent->wire_latency));
					//done with this access

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
				if(dest_node_number > switches[my_pid].switch_node_number)
				{
					//get the distance from this switch to the destination (left and right)
					distance = dest_node_number - switches[my_pid].switch_node_number;

					//go in the direction with the shortest number of hops.
					if(distance <= switches[my_pid].switch_median_node_num)
					{//go right

						//special cases are the two end switches.
						//if(switches[my_pid].switch_node_number == 2)
						//if(switches[my_pid].switch_node_number == (node_number - 2))

					}
					else
					{//go left


					}
				}
				else if(dest_node_number < switches[my_pid].switch_node_number)
				{
					distance = switches[my_pid].switch_node_number - dest_node_number;

					//go in the direction with the shortest number of hops.
					if(distance <= switches[my_pid].switch_median_node_num)
					{//go left

					}
					else
					{//go right

					}

				}
			}
			else if(switches[my_pid].queue == east_queue || switches[my_pid].queue == west_queue)
			{
				//packet came from another switch, but needs to continue on.

				if(switches[my_pid].queue == east_queue)
				{//go left (even if it is less efficient)



				}
				else
				{//go right


				}

			}
		}

		//end, clear the message_packet ptr
		//this should be getting set up above in list_get(), but just for safe measure.
		message_packet = NULL;
	}

	return;
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
			}
			else if(switches->queue == east_queue)
			{
				new_packet = list_get(switches->east_queue, 0);
			}
			else if(switches->queue == south_queue)
			{
				new_packet = list_get(switches->south_queue, 0);
			}
			else if(switches->queue == west_queue)
			{
				new_packet = list_get(switches->west_queue, 0);
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



void get_path(void){

	/*//todo need priority queue
	int cost[10][10], path[10][10], i, j, n, p, min, v, distance[10], row, column;
	int index = 1;

	//Dijkstra's algorithm
	for (i=1; i <=p; i ++)
	{
		distance[i] = 0;
		row = 1;
		for(j=1; j <=n; j++)
		{
			if(row!=v)
			{
				column=path[i][j+1];
				distance[i] = distance[i] + cost[row][column];
			}
			row = column;

		}
	}

	min = distance[1];

	for(i=1; i<=p; i++)
	{
		if(distance[i]<=min)
		{
			min=distance[i];
			index = i;
		}
	}*/



	/***********************************************************
	* You can use all the programs on  www.c-program-example.com
	* for personal and learning purposes. For permissions to use the
	* programs for commercial purposes,
	* contact info@c-program-example.com
	* To find more C programs, do visit www.c-program-example.com
	* and browse!
	*
	*                      Happy Coding
	***********************************************************/

/*	#define infinity 999

	void dij(int n,int v,int cost[10][10],int dist[]){

		int i,u,count,w,flag[10],min;

		for(i=1;i<=n;i++)
		{
			flag[i]=0,dist[i]=cost[v][i];
		}

		count=2;

		while(count<=n)
		{
			min=99;
			for(w=1;w<=n;w++)
			{
				if(dist[w]<min && !flag[w])
				{
					min=dist[w],u=w;
				}

				flag[u]=1;
				count++;
			}
			for(w=1;w<=n;w++)
			{
				if((dist[u]+cost[u][w]<dist[w]) && !flag[w])
				{
					dist[w]=dist[u]+cost[u][w];
				}
			}
		}
	}

	void main()
	{
	 int n,v,i,j,cost[10][10],dist[10];

	 printf("\n Enter the number of nodes:");
	 scanf("%d",&n);

	 printf("\n Enter the cost matrix:\n");
	 for(i=1;i<=n;i++)
	  for(j=1;j<=n;j++)
	  {
	   scanf("%d",&cost[i][j]);
	   if(cost[i][j]==0)
	    cost[i][j]=infinity;
	  }
	 printf("\n Enter the source node:");
	 scanf("%d",&v);

	 dij(n,v,cost,dist);

	 printf("\n Shortest path:\n");
	 for(i=1;i<=n;i++)
	  if(i!=v)
	   printf("%d->%d,cost=%d\n",v,i,dist[i]);

	 getchar();
	}*/

	return;
}
