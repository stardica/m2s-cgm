/*
 * directory.c
 *
 *  Created on: Mar 30, 2015
 *      Author: stardica
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cgm/directory.h>
#include <cgm/tasking.h>

#include <arch/x86/timing/cpu.h>

#include <lib/util/list.h>



//globals
struct directory_t *directory;

int *dir_data;
eventcount volatile *dir;

void directory_init(void){

	directory_create();
	directory_create_tasks();

	return;
}

void directory_create(void){

	int num_cores = x86_cpu_num_cores;

	//init the directory struct
	directory = (void *) calloc(1, sizeof(struct directory_t));
	dir_data = (void *) calloc(num_cores, sizeof(int));

	return;
}

void directory_create_tasks(void){


	char buff[100];

	//event count
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "dir");
	dir = new_eventcount(strdup(buff));

	//tasks
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "dir_ctrl");
	create_task(directory_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	return;
}

void directory_ctrl(void){

	long long step = 1;
	int num_cores = x86_cpu_num_cores;
	int id = 0;

	while(1)
		{
			/*wait here until there is a job to do.
			In any given cycle I might have to service 1 to N number of caches*/
			await(dir, step);
			step++;

			//set id to 0
			id = 0;

			while (id < num_cores)
			{

				if(dir_data[id] > 0)
				{//then there is a task to be done in this unit.

					//decrement the counter
					dir_data[id]--;
				}

				id++; //go to the next dir unit
			}
		}
	return;
}

unsigned long long directory_map_block_number(unsigned int addr){

	unsigned long long block_number = (addr & ~(directory->block_mask))/(directory->block_size);

	return block_number;
}
