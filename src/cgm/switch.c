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
#include <cgm/switch.h>

#include <lib/util/list.h>

#include <arch/si/timing/gpu.h>
#include <arch/x86/timing/cpu.h>

#include <cgm/tasking.h>


struct switch_t *switches;
eventcount volatile *switches_ec;
task *switches_tasks;
int switch_pid = 0;


void switch_init(void){

	switch_create();
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

	//todo need priority queue

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
	}

	return;
}
