/*
 * interrupt.c
 *
 *  Created on: Mar 28, 2015
 *      Author: stardica
 */

#include <stdio.h>
#include <string.h>

#include <cgm/interrupt.h>
#include <cgm/tasking.h>

#include <arch/x86/timing/cpu.h>
#include <lib/util/list.h>


//global flags
//catching the interrupts is a little challenging.
int opencl_syscall_flag = 0;
int syscall_flag = 0;
unsigned int int_src_ptr = 0;
unsigned int int_dest_ptr = 0;
unsigned int int_size = 0;
int *interrupt_cores;

struct list_t *interrupt_list;

eventcount volatile *interrupt;

void interrupt_init(void){

	interrupt_create();
	interrupt_create_tasks();

	return;
}

void interrupt_create(void){

	char buff[100];
	interrupt_list = list_create();

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "interrupt_list");
	interrupt_list->name = strdup(buff);

	int num_cores = x86_cpu_num_cores;
	interrupt_cores = (void *) calloc(num_cores, sizeof(int));

	return;
}

void interrupt_create_tasks(void){

	char buff[100];

	//event count
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "interrupt");
	interrupt = new_eventcount(strdup(buff));

	//task
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "interrupt");
	create_task(interrupt_service_request, DEFAULT_STACK_SIZE, strdup(buff));

	return;
}

struct interrupt_t *interrupt_service_routine_create(void){

	struct interrupt_t *isr = (void*) malloc(sizeof(struct interrupt_t));

	return isr;
}

void interrupt_service_request(void){

	long long step = 1;
	int num_cores = x86_cpu_num_cores;
	int id = 0;

	while(1)
	{
		/*wait here until there is a job to do.
		In any given cycle I might have to service 1 to N number of caches*/
		await(interrupt, step);
		step++;

		//set id to 0
		id = 0;

		while (id < num_cores)
		{

			if(interrupt_cores[id] > 0)
			{//then there is a task to be done in this unit.

				//decrement the counter
				interrupt_cores[id]--;


				/*//star todo figure out what to do with this.
				if(access_type == cgm_access_load || access_type == cgm_access_store || access_type == cgm_access_nc_store)
				{//then the packet is from the L2 cache

					P_PAUSE(mem_miss);

					//clear the gpu uop witness_ptr
					(*message_packet->witness_ptr)++;

					list_remove(gpu_v_caches[id].Rx_queue_top, message_packet);

				}
				else
				{
					fatal("gpu_v_cache_ctrl(): unknown access type = %d\n", message_packet->access_type);
				}*/
			}

			id++; //go to the next lds unit
		}

	}

	return;
}
