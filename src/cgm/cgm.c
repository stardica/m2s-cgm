/*
 * queue.c
 *
 *  Created on: Nov 23, 2014
 *      Author: stardica
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//star todo take any borrowed files and move then to our cgm-mem directory.
#include <cgm/cgm.h>
#include <cgm/queue.h>
#include <cgm/cache.h>
#include <cgm/mem-ctrl.h>
#include <cgm/configure.h>
#include <cgm/sys-agent.h>
#include <cgm/ini-parse.h>
#include <cgm/tasking.h>
#include <cgm/packet.h>



char test_mem[256] = {NULL};
int host_sim_cpu_num = 1; //Statically set to 1 for now.
int host_sim_cpu_core_num;
int host_sim_cpu_thread_num;
int host_sim_cpu_fetch_queue_size;
int host_sim_cpu_lsq_queue_size;

char *cgm_config_file_name_and_path;

long long int mem_cycle = 0;

//globals for tasking
eventcount *start;
eventcount *stop;

/*star todo:
 (2) integrate with M2S single core CPU, then multicore CPU, then multicore CPU with GPU.
*/

void cgm_init(void){

	//star todo add error checking.
	memctrl_init();

	return;
}

void cgm_configure(void){

	//star todo add error checking.
	cgm_mem_configure();
	print_config();

	//star todo use the call back functions for each arch (x86Cpu and SIGpu)
	cgm_cpu_configure();
	cgm_gpu_configure();

	return;
}

/*int main(int argc, char **argv){

	//initialize memory structures and tasks
	printf("---CGM-MEM Init()---\n");
	cgm_mem_structure_init();
	cgm_mem_threads_init();

	//run the main loop
	printf("---CGM-MEM Simulation Loop()---\n");
	//cgm_mem_sim_loop();
	simulate(cleanup);

	//Will never get here if running correctly.
	printf("---Done---\n");
	return 1;

}*/

void cgm_mem_task_init(void){

	long long i = 1;
	//star the threads simulation.

	advance(start);

	printf("in task init\n");

	await(stop, i);

	return;
}



void cgm_mem_structure_init(void){

	int error = 0;

	//create queue structures
	queue_init();

	//create cache structures
	cache_init();

	//create system agent structures
	sysagent_init();

	//create microcontroller structure
	//memctrl_init();

	//star todo create memory image, mshr, and directory table.
	//testmem_init();

	//configure memory structures
	//error = cgmmem_configure();

	if(error == 1)
	{

		printf("Error in cgm_mem_init()\n");
		exit(0);
	}

	//Check configuration
	if(cgmmem_check_config == 1)
	{
		print_config();
		//mem_dump();
	}


	return;
}

void cgm_mem_threads_init(void){

	char *taskname = NULL;
	char *eventname = NULL;

	//create task
	taskname = "cgm_mem_task_init";
	create_task(cgm_mem_task_init, DEFAULT_STACK_SIZE, taskname);
	taskname = "queue_ctrl";
	create_task(queue_ctrl, DEFAULT_STACK_SIZE, taskname);


	//create eventcounts
	eventname = "start";
	start = new_eventcount(eventname);
	eventname = "stop";
	stop = new_eventcount(eventname);

	return;
}


void cgm_mem_sim_loop(void){

	int error = 0;

	/*error = system_test();
	if(error == 1)
	{
		printf("failed in system_test()\n");
	}*/


	return;

}



void testmem_init(void){

	/*star todo initialize array to random chars... or not.
	For testing purposes all NULL maybe OK.
	Just use the script to load a few initial values.*/

	//test_mem[254] = 'a';
	//test_mem[255] = 'b';

	//int i = sizeof(test_mem);
	//printf("memsize %d\n", i);

	//int j = 0;
	//char tempchar = (char)(((int)'0') +i);
	//for(j=0; i < j; i++)
	//{
	//	test_mem[i] = tempchar;
	//}

	return;
}

void cgm_mem_dump(void){

	int i = sizeof(test_mem);
	int j = 0;

	printf("---Memory Dump---\n");
	for(j = 0; j < i; j++)
	{

		printf("%d \t", j);
		if (test_mem[j] == NULL)
		{
			printf("NULL\n");
		}
		else
		{
			printf("%c\n", test_mem[j]);
		}

	}

	return;
}

/*int system_test(void){

	//star todo read and run scripted memory interaction for testing.
	int error = 0;

	while (mem_cycle <= 12)
	{
		//printf("in while\n");

		//error = ini_parse(TESTSCRIPTPATH, test_run, NULL);
		if (error < 0)
		{
			printf("Unable to open Config.ini for queue configuration.\n");
			return 1;
		}

		//star >> todo memory structures need to check their queues (down and up).
		cache_poll_queues();
		//sysagent_poll_queues();
		//memctrl_poll_queues();



		//caches and memctrl check place data on their out queues (down and up).
		//star todo each structure needs to read queue on clock cycle

		mem_cycle++;
	}

	return 0;
}*/

/*void test_run(void* user, const char* section, const char* name, const char* value){

	char *inst_string = NULL;

	char cycle[256];
	sprintf(cycle, "%lld", mem_cycle);
	//printf("%s\n", cycle);

	const char delim[] = " \0";
	char *token = NULL;

	if(MATCH("TestScript", cycle))
	{

		inst_string = strdup(value);
		token = strtok(inst_string, delim);

		if (strcmp(token, "store") == 0)
		{

			store_issue(strdup(value));

		}else if (strcmp(token, "load") == 0)
		{

			load_issue(strdup(value));

		}else if (strcmp(token, "fetch") == 0)
		{

			load_fetch(strdup(value));

		}
	}

	return;
}*/

/*void store_issue(char * string){


	//star todo initialize data_packet
	struct cgm_packet_t * data_packet;
	data_packet = data_packet_create();

	char *inst = NULL;
	char *address = NULL;
	char *data = NULL;
	const char delim[] = " \0";

	inst = strtok(string, delim);
	address = strtok(NULL, delim);
	data = strtok(NULL, delim);

	//debug
	//printf("%s\n", inst);
	//printf("%s\n", address);
	//printf("%s\n", data);

	data_packet->task = inst;
	data_packet->address = atoi(address);
	data_packet->data = data[0];

	//debug
	//printf("%s\n", data_packet->task);
	//printf("%d\n", data_packet->address);
	//printf("%c\n", data_packet->data);

	//star todo add error checking here to queue_insert
	//returns 1 on fail and 0 on success.
	queue_insert(q_l1d_0_CoreRequest, data_packet);

	return;
}*/

/*void load_issue(char * string){

	struct cgm_packet_t * data_packet;
	data_packet = data_packet_create();

	char *inst = NULL;
	char *address = NULL;
	const char delim[] = " \0";

	inst = strtok(string, delim);
	address = strtok(NULL, delim);

	data_packet->task = inst;
	data_packet->address = atoi(address);
	data_packet->data = NULL;

	//star todo add error checking here to queue_insert
	//returns 1 on fail and 0 on success.
	queue_insert(q_l1d_0_CoreRequest, data_packet);

	return;
}*/



/*void load_fetch(char * string){

	struct cgm_packet_t * data_packet;
	data_packet = data_packet_create();

	char *inst = NULL;
	char *address = NULL;
	const char delim[] = " \0";

	inst = strtok(string, delim);
	address = strtok(NULL, delim);

	data_packet->task = inst;
	data_packet->address = atoi(address);
	data_packet->data = NULL;

	//star todo add error checking here to queue_insert.
	//returns 1 on fail and 0 on success.
	queue_insert(q_l1i_0_CoreRequest, data_packet);

	return;
}*/

void cleanup(void){

	/*star >> todo:
	(1) print stats or put in a file or something for later.
	(2) clean up all global memory objects by running finish functions.*/
	printf("---CGM-MEM Cleanup()---\n");
	fflush(stdout);
}
