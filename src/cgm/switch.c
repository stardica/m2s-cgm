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

#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>

#include <cgm/tasking.h>
#include <cgm/switch.h>


struct switch_t *switches;
eventcount volatile *switches_ec;
task *switches_tasks;
int switch_pid = 0;

int *route_table;

//supports quad core with ring bus
struct str_map_t node_map =
{ 	node_number, {
		{ "l2_caches[0]", l2_cache_0},
		{ "l2_caches[1]", l2_cache_1},
		{ "l2_caches[2]", l2_cache_2},
		{ "l2_caches[3]", l2_cache_3},
		{ "l2_caches[4]", l2_cache_4},
		{ "switch[0]", switch_0},
		{ "switch[1]", switch_1},
		{ "switch[2]", switch_2},
		{ "switch[3]", switch_3},
		{ "switch[4]", switch_4},
		{ "l3_caches[0]", l3_cache_0},
		{ "l3_caches[1]", l3_cache_1},
		{ "l3_caches[2]", l3_cache_2},
		{ "l3_caches[3]", l3_cache_3},
		{ "sys_agent", sys_agent},
		}
};


void switch_init(void){

	switch_create();
	route_create();
	switch_create_tasks();


	return;
}


void switch_create(void){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;

	//for now model a ring bus on each CPU
	switches = (void *) calloc((num_cores + num_cus), sizeof(struct switch_t));

	return;
}

void route_create(void){

	int i = 0;

	//star todo get routes for each switch.

	//get possible routes
	for(i = 0; i < node_number; i ++)
	{
		//route_table[i]

	}


	return;
}


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

	while(1)
	{
		await(&switches_ec[my_pid], step);
		step++;

	}

	return;
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

	#define infinity 999

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
	}

	return;
}
