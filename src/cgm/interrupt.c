/*
 * interrupt.c
 *
 *  Created on: Mar 28, 2015
 *      Author: stardica
 */



#include <cgm/interrupt.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cgm/cgm.h>
#include <cgm/interrupt.h>
#include <cgm/tasking.h>


#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/uop.h>
#include <lib/util/list.h>
#include <lib/util/debug.h>


#define BASELAT 4000
#define OCLLAT 1000
#define SYSLAT 1000

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

void interrupt_service_routine_destroy(struct interrupt_t *isr){

	isr->thread = NULL;
	isr->uop = NULL;

	free(isr);

	return;
}

void cgm_interrupt(X86Thread *self, struct x86_uop_t *uop){

	//star todo
	//create the memory system accesses
	//check if we can access both i and d caches.
	//fetch and data caches need to be accessed.

	X86Core *core = self->core;
	//struct x86_uop_t *interrupt_uop;
	int id = core->id;
	int num_cores = x86_cpu_num_cores;

	struct interrupt_t *isr = interrupt_service_routine_create();

	isr->uop = uop;
	isr->core_id = id;
	isr->thread = self;

	//put the uop on the interrupt list
	list_enqueue(interrupt_list, isr);

	//set the flag for the right core
	interrupt_cores[id]++;
	assert(id >= 0 && id < num_cores);

	//advance the ISR
	advance(interrupt);

	return;
}


void interrupt_service_request(void){

	long long step = 1;
	int num_cores = x86_cpu_num_cores;
	int id = 0;
	long long lat = 0;

	X86Core *core;

	struct interrupt_t *isr;
	struct x86_uop_t *uop;

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
			{

				//decrement the counter
				interrupt_cores[id]--;

				isr = list_dequeue(interrupt_list);
				assert(isr);

				uop = isr->uop;
				core = isr->thread->core;

				assert(uop);
				assert(core);

				//star todo advance the memory system with ISR related accesses.
				if(uop->interrupt > 0 && uop->interrupt_type == opencl_interrupt) //an OpenCL related syscall occurred
				{
					/*if(INT == 1)
					{
						printf("interrupt_service_request() Caught OpenCL interrupt code %d in issue at cycle %llu\n", uop->interrupt, P_TIME);
					}*/

					if(uop->interrupt == 2) //GPU malloc
					{
						lat = BASELAT;

					}
					else if(uop->interrupt == 4) //GPU memcpy
					{
						//if we make it here we should have these fields
						assert(uop->int_src_ptr);
						//assert(uop->int_dest_ptr);
						assert(uop->int_size);

						//printf("interrupt memcpy size %d src 0x%08X dest 0x%08X\n", uop->int_size, uop->int_src_ptr, uop->int_dest_ptr);
						lat = BASELAT;
					}
					else //others we don't care about
					{
						lat = BASELAT;
					}
				}
				else if(uop->interrupt > 0 && uop->interrupt_type == system_interrupt)
				{
					//printf(" sys interrupt %d\n", uop->interrupt );
					lat = BASELAT;
				}
				else //everything else
				{
					//this is what m2s originally had for all system interrupts
					lat = BASELAT;
				}

				//set when the interrupt should complete
				uop = linked_list_find(core->event_queue, uop);
				uop->interrupt_start = P_TIME;
				uop->interrupt_lat = lat;
				uop->when = P_TIME + lat;
				lat = 0;

				//the interrupt is complete we can free it now. list_dequeue free's the element.
				interrupt_service_routine_destroy(isr);
			}
			id++; //go to the next core
		}
	}
	/* should never get here*/
	fatal("interrupt_service_request task is broken\n");
	return;
}
