/*
 * ptw.c
 *
 *  Created on: May 16, 2017
 *      Author: stardica
 */

#include <mem-image/ptw.h>


eventcount volatile *ptw_ec;
task *ptw_task;

long long ptw_pid = 0;


void ptw_init(void){

	int num_cores = x86_cpu_num_cores;
	char buff[100];
	int i = 0;

	//create event counts
	ptw_ec = (void *) calloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "ptw_ec_%d", i);
		ptw_ec[i] = *(new_eventcount(strdup(buff)));
	}

	//create tasks
	ptw_task = (void *) calloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "ptw_task_%d", i);
		ptw_task[i] = *(create_task(ptw_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	return;
}


void ptw_ctrl(void){

	//my_pid increments for the number of CPU cores. i.e. 0 - 4 for a quad core
	int my_pid = ptw_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;
	//long long occ_start = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	//int i = 0;
	//int num_translations = 0;
	//int num_requests = 0;

	//int tlb_block_hit;
	//int tlb_block_state;
	//int *tlb_block_state_ptr = &tlb_block_state;

	//int set = 0;
	//int tag = 0;
	//unsigned int offset = 0;
	//int way = 0;

	//int *set_ptr = &set;
	//int *tag_ptr = &tag;
	//unsigned int *offset_ptr = &offset;
	//int *way_ptr = &way;

	//int victim_way = 0;

	while(1)
	{
		//wait here until there is a job to do
		await(&ptw_ec[my_pid], step); //&mmu_ec[my_pid], step);


		//check each mmu fault bit, resolve address translations, reply accesses (advance MMU again?)




		step++;

		fatal("PTW\n");

	}



	return;
}
