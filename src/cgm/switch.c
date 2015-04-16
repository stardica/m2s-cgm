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

#include <cgm/tasking.h>
#include <cgm/switch.h>
#include <cgm/packet.h>


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

/*void route_create(void){

	int i = 0;

	//star todo get routes for each switch.
	route_table = (void *) malloc(node_number * node_number * sizeof(int));


	//get possible routes
	for(i = 0; i < node_number; i ++)
	{
		//route_table[i]
	}


	return;
}*/


void switch_create_tasks(void){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;

	char buff[100];
	int i = 0;

	switches_ec = (void *) calloc(num_cores + num_cus, sizeof(eventcount));
	for(i = 0; i < (num_cores + num_cus); i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "switch_%d", i);
		switches_ec[i] = *(new_eventcount(strdup(buff)));
	}

	switches_tasks = (void *) calloc((num_cores + num_cus), sizeof(task));
	for(i = 0; i < (num_cores + num_cus); i++)
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
	unsigned int addr = 0;
	long long access_id = 0;
	char *dest;
	int i = 0;
	int dest_node_number;

	assert(my_pid <= (num_cores + num_cus));

	set_id((unsigned int)my_pid);


	while(1)
	{

		//we have received a packet
		await(&switches_ec[my_pid], step);
		step++;


		//choose a port this cycle to work from
		if(switches[my_pid].arb_style == round_robin)
		{
			for(i = 0; i < switches[my_pid].port_num; i++)
			{
				//switches[my_pid].queue is now the next queue.
				switches[my_pid].queue = get_next_queue_rb(switches[my_pid].queue);

				//if we don't have a message go to the next queue.
				if(switches[my_pid].queue == north_queue)
				{
					message_packet = list_get(switches[my_pid].north_queue, 0);
				}
				else if(switches[my_pid].queue == east_queue)
				{
					message_packet = list_get(switches[my_pid].east_queue, 0);
				}
				else if(switches[my_pid].queue == south_queue)
				{
					message_packet = list_get(switches[my_pid].south_queue, 0);
				}
				else if(switches[my_pid].queue == west_queue)
				{
					message_packet = list_get(switches[my_pid].west_queue, 0);
				}

				//if we have a packet break out.
				//next advance start with the next queue
				if(message_packet)
				{
					i = 0;
					break;
				}

			}


		}
		else
		{
			fatal("switch_ctrl() invalid arbitration set switch %d\n", my_pid);
		}

		//if we made it here we should have a packet.
		assert(message_packet);

		//send the packet to it's destination OR on to the next hop

		//look up the node number of the destination
		dest = message_packet->dest_name;
		dest_node_number = str_map_string(&node_strn_map, dest);


		//if dest is the L2 or L3 cache connected to this switch.
		if(dest_node_number == (switches[my_pid].switch_node_number - 1) || dest_node_number == (switches[my_pid].switch_node_number +1))
		{

			if(dest_node_number < switches[my_pid].switch_node_number)
			{
				//drop in L2_x in queue and advance

			}
			else if(dest_node_number > switches[my_pid].switch_node_number)
			{
				//drop in L3_x in queue and advnace
			}
			else
			{
				fatal("switch_ctrl() switch % d: Invalid dest_node_number \n", my_pid);
			}

		}
		else
		{
			//packet came from connected L2 or L3 cache
			//there is no bias on direction established.
			if(switches[my_pid].queue == north_queue || switches[my_pid].queue == south_queue)
			{
				//send the packet to an adjacent switch




				if(dest_node_number > switches[my_pid].switch_node_number && dest_node_number > (switches[my_pid].switch_median_node_num + switches[my_pid].switch_node_number))
				{

					//check the status of neighbor switche's queue

				}
				else if(dest_node_number > switches[my_pid].switch_median_node_num)
				{
					//check the status of neighbor switche's queue

				}



			}
			//packet came from east queue
			else if(switches[my_pid].queue == east_queue)
			{

			}
			else if(switches[my_pid].queue == west_queue)
			{


			}

		}


		//special case R0 and R4
		//if switch number = 2 or  switch number = (node_number - 2)

		//for the special case of GPU L2 and system Agent.




		//end, clear the message_packet ptr
		//this should be getting set up above in list_get(), but just for safe measure.
		message_packet = NULL;
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
