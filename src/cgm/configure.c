/*
 * configure.c
 *
 *  Created on: Nov 26, 2014
 *      Author: stardica
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/config.h>
#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <arch/common/arch.h>
#include <arch/x86/timing/core.h>
#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/thread.h>

#include <cgm/configure.h>
#include <cgm/cgm.h>
#include <cgm/ini-parse.h>
#include <cgm/queue.h>
#include <cgm/cache.h>
#include <cgm/mem-ctrl.h>



int cgmmem_check_config = 0;
/*struct queue_config_t *q_config;
struct cache_config_t *c_config;*/

int cgm_mem_configure(void){

	int error = 0;

	//configure the memory controller
	error = ini_parse(cgm_config_file_name_and_path, memctrl_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for memctrl configuration.\n");
		return 1;
	}

	//get some host sim configuration stats
	/*error = ini_parse(HOSTSIMCONFIGPATH, cpu_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for cpu configuration.\n");
		return 1;
	}*/

	//get check value
	/*error = ini_parse(CGMMEMCONFIGPATH, check_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for check configuration.\n");
		return 1;
	}*/

	//get queue configuration
	/*error = ini_parse(CGMMEMCONFIGPATH, queue_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for queue configuration.\n");
		return 1;
	}*/

	//get cache configuration
	/*error = ini_parse(CGMMEMCONFIGPATH, cache_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for cache configuration.\n");
		return 1;
	}*/

	//get sysagent configuration
	/*error = ini_parse(CONFIGPATH, sysagent_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for sysagent configuration.\n");
		return 1;
	}*/


	return 0;

}

int cgm_cpu_configure(void){

	//star todo run the CPU mem config call back function.

	Timing *timing;
	struct arch_t *arch;
	//char arch_name_trimmed = "x86";
	arch = arch_get("x86");
	timing = arch->timing;

	//star >> uses the call back pointer for MemConfigDefault, but runs our cpu_configure function.
	//we don't use the config struct, so pass null and set everything by hand.

	if (MSG==1)
	{
		printf("timing->MemConfigDefault(timing, NULL);\n");
		fflush(stdout);
		getchar();
	}

	timing->MemConfigDefault(timing, NULL);


	return 1;
}

int cgm_gpu_configure(void){

	return 1;
}


int cpu_configure(Timing *self, struct config_t *config){

	//star todo configure the CPU/Core/Thread here
	//this function does all the stuff that arch/x86/timing/mem-config.h/c has to do.

	if (MSG ==1)
	{
		printf("cpu_configure start\n");
		fflush(stdout);
		getchar();
	}

	X86Cpu *cpu = asX86Cpu(self);
	X86Core *core;
	X86Thread *thread;

	int core_index;
	int thread_index;

	//star todo pull these automatically.
	core_index = 0;
	thread_index = 0;

	core = cpu->cores[core_index];
	thread = core->threads[thread_index];

	//star todo link memory modules here.
	//do this somewhere else?

	//assign entry into memory system
	thread->mem_ctrl_ptr = mem_ctrl;

	//add to memory entry list list?
	linked_list_add(arch_x86->mem_entry_mod_list, thread->mem_ctrl_ptr);

	if(MSG==1)
	{
		printf("cpu_configure end\n");
		printf("Thread mem entry name is %s", thread->mem_ctrl_ptr->name);
		fflush(stdout);
		getchar();
	}

	return 1;
}

int gpu_configure(Timing *self, struct config_t *config){

	//star todo configure the GPU here. LDS and other memory archs


	return 1;
}














int check_config(void* user, const char* section, const char* name, const char* value){

	//this is used if you want to store the ini values in our configuration struct.
	//struct queue_config_t *temp = (struct queue_config_t *)user;

	if(MATCH("Check", "Check"))
	{
		//temp->size = atoi(value);
		cgmmem_check_config = atoi(value);

	}

	return 0;
}

int queue_config(void* user, const char* section, const char* name, const char* value){

	//this is used if you want to store the ini values in our configuration struct.
	//struct queue_config_t *temp = (struct queue_config_t *)user;

	if(MATCH("Queue", "Size"))
	{
		//temp->size = atoi(value);
		queue_size = atoi(value);

	}

	return 0;
}

int cache_config(void* user, const char* section, const char* name, const char* value){

	//L1 caches

	if(MATCH("CacheL1d", "Sets"))
	{
		l1_data_cache->num_sets = atoi(value);
	}
	if(MATCH("CacheL1d", "Assoc"))
	{
		l1_data_cache->assoc = atoi(value);
	}
	if(MATCH("CacheL1d", "BlockSize"))
	{
		l1_data_cache->block_size = atoi(value);
	}
	if(MATCH("CacheL1d", "Latency"))
	{
		l1_data_cache->latency = atoi(value);
	}
	if(MATCH("CacheL1d", "Policy"))
	{
		l1_data_cache->policy = strdup(value);
	}
	if(MATCH("CacheL1d", "MSHR"))
	{
		l1_data_cache->mshr_size = atoi(value);
	}
	if(MATCH("CacheL1d", "DirectoryLatency"))
	{
		l1_data_cache->directory_latency = atoi(value);
	}
	if(MATCH("CacheL1d", "Ports"))
	{
		l1_data_cache->num_ports = atoi(value);
	}


	if(MATCH("CacheL1i", "Sets"))
	{
		l1_inst_cache->num_sets = atoi(value);
	}
	if(MATCH("CacheL1i", "Assoc"))
	{
		l1_inst_cache->assoc = atoi(value);
	}
	if(MATCH("CacheL1i", "BlockSize"))
	{
		l1_inst_cache->block_size = atoi(value);
	}
	if(MATCH("CacheL1i", "Latency"))
	{
		l1_inst_cache->latency = atoi(value);
	}
	if(MATCH("CacheL1i", "Policy"))
	{
		l1_inst_cache->policy = strdup(value);
	}
	if(MATCH("CacheL1i", "MSHR"))
	{
		l1_inst_cache->mshr_size = atoi(value);
	}
	if(MATCH("CacheL1i", "DirectoryLatency"))
	{
		l1_inst_cache->directory_latency = atoi(value);
	}
	if(MATCH("CacheL1i", "Ports"))
	{
		l1_inst_cache->num_ports = atoi(value);
	}

	/*L2 caches
	struct cache_t *l2_cache;*/

	if(MATCH("CacheL2", "Sets"))
	{
		l2_cache->num_sets = atoi(value);
	}
	if(MATCH("CacheL2", "Assoc"))
	{
		l2_cache->assoc = atoi(value);
	}
	if(MATCH("CacheL2", "BlockSize"))
	{
		l2_cache->block_size = atoi(value);
	}
	if(MATCH("CacheL2", "Latency"))
	{
		l2_cache->latency = atoi(value);
	}
	if(MATCH("CacheL2", "Policy"))
	{
		l2_cache->policy = strdup(value);
	}
	if(MATCH("CacheL2", "MSHR"))
	{
		l2_cache->mshr_size = atoi(value);
	}
	if(MATCH("CacheL2", "DirectoryLatency"))
	{
		l2_cache->directory_latency = atoi(value);
	}
	if(MATCH("CacheL2", "Ports"))
	{
		l2_cache->num_ports = atoi(value);
	}

	/*L3 caches
	struct cache_t *l3_cache;*/

	if(MATCH("CacheL3", "Sets"))
	{
		l3_cache->num_sets = atoi(value);
	}
	if(MATCH("CacheL3", "Assoc"))
	{
		l3_cache->assoc = atoi(value);
	}
	if(MATCH("CacheL3", "BlockSize"))
	{
		l3_cache->block_size = atoi(value);
	}
	if(MATCH("CacheL3", "Latency"))
	{
		l3_cache->latency = atoi(value);
	}
	if(MATCH("CacheL3", "Policy"))
	{
		l3_cache->policy = strdup(value);
	}
	if(MATCH("CacheL3", "MSHR"))
	{
		l3_cache->mshr_size = atoi(value);
	}
	if(MATCH("CacheL3", "DirectoryLatency"))
	{
		l3_cache->directory_latency = atoi(value);
	}
	if(MATCH("CacheL3", "Ports"))
	{
		l3_cache->num_ports = atoi(value);
	}


	return 0;
}

int sysagent_config(void* user, const char* section, const char* name, const char* value){


	return 0;
}


int memctrl_config(void* user, const char* section, const char* name, const char* value){

	if(MATCH("MemCtrl", "Name"))
	{
		mem_ctrl->name = strdup(value);
	}

	if(MATCH("MemCtrl", "BlockSize"))
	{
		mem_ctrl->block_size = atoi(value);
		mem_ctrl->log_block_size = LOG2(mem_ctrl->block_size);
	}

	if(MATCH("MemCtrl", "Latency"))
	{
		mem_ctrl->latency = atoi(value);
	}

	if(MATCH("MemCtrl", "DirLatency"))
	{
		mem_ctrl->dir_latency = atoi(value);
	}

	if(MATCH("MemCtrl", "Ports"))
	{
		mem_ctrl->ports = atoi(value);
	}

	if(MATCH("MemCtrl", "QueueSize"))
	{
		mem_ctrl->queue_size = atoi(value);
	}
	if(MATCH("MemCtrl", "MSHRSize"))
	{
		mem_ctrl->mshr_size = atoi(value);
	}

	return 0;
}

void print_config(void){

	//print config before runtime
	/*printf("CPU:\n");
	printf("Number of cpus %d\n", host_sim_cpu_num);
	printf("Cores per cpu %d\n", host_sim_cpu_core_num);
	printf("Threads per core %d\n", host_sim_cpu_thread_num);
	printf("Fetch queue size %d\n", host_sim_cpu_fetch_queue_size);
	printf("Load and store queue size %d\n", host_sim_cpu_lsq_queue_size);
	printf("\n");
	printf("Memory Queues:\n");
	printf("queue size %d\n", queue_size);
	printf("\n");
	printf("Caches:\n");
	printf("Cache L1D sets = %d\n", l1_data_cache->num_sets);
	printf("Cache L1D assoc = %d\n", l1_data_cache->assoc);
	printf("Cache L1D block size = %d\n", l1_data_cache->block_size);
	printf("Cache L1D latency = %d\n", l1_data_cache->latency);
	printf("Cache L1D policy = %s\n", l1_data_cache->policy);
	printf("Cache L1D mshr size = %d\n", l1_data_cache->mshr_size);
	printf("Cache L1D directory latency = %d\n", l1_data_cache->directory_latency);
	printf("Cache L1D Ports = %d\n", l1_data_cache->num_ports);
	printf("\n");
	printf("Cache L1I sets = %d\n", l1_inst_cache->num_sets);
	printf("Cache L1I assoc = %d\n", l1_inst_cache->assoc);
	printf("Cache L1I block size = %d\n", l1_inst_cache->block_size);
	printf("Cache L1I latency = %d\n", l1_inst_cache->latency);
	printf("Cache L1I policy = %s\n", l1_inst_cache->policy);
	printf("Cache L1I mshr size = %d\n", l1_inst_cache->mshr_size);
	printf("Cache L1I directory latency = %d\n", l1_inst_cache->directory_latency);
	printf("Cache L1I Ports = %d\n", l1_inst_cache->num_ports);
	printf("\n");
	printf("Cache L2 sets = %d\n", l2_cache->num_sets);
	printf("Cache L2 assoc = %d\n", l2_cache->assoc);
	printf("Cache L2 block size = %d\n", l2_cache->block_size);
	printf("Cache L2 latency = %d\n", l2_cache->latency);
	printf("Cache L2 policy = %s\n", l2_cache->policy);
	printf("Cache L2 mshr size = %d\n", l2_cache->mshr_size);
	printf("Cache L2 directory latency = %d\n", l2_cache->directory_latency);
	printf("Cache L2 Ports = %d\n", l2_cache->num_ports);
	printf("\n");
	printf("Cache L3 sets = %d\n", l3_cache->num_sets);
	printf("Cache L3 assoc = %d\n", l3_cache->assoc);
	printf("Cache L3 block size = %d\n", l3_cache->block_size);
	printf("Cache L3 latency = %d\n", l3_cache->latency);
	printf("Cache L3 policy = %s\n", l3_cache->policy);
	printf("Cache L3 mshr size = %d\n", l3_cache->mshr_size);
	printf("Cache L3 directory latency = %d\n", l3_cache->directory_latency);
	printf("Cache L3 Ports = %d\n", l3_cache->num_ports);
	printf("\n");*/
	printf("---Memory Controller Initialized---\n");
	printf("block_size = %d\n", mem_ctrl->block_size);
	printf("log_block_size = %d\n", mem_ctrl->log_block_size);
	printf("latency = %d\n", mem_ctrl->latency);
	printf("dir_latency = %d\n", mem_ctrl->dir_latency);
	printf("Number of queues = %d\n", mem_ctrl->ports);
	printf("Queue name = %s\n", mem_ctrl->fetch_request_queue->name);
	printf("Queue name = %s\n", mem_ctrl->issue_request_queue->name);
	fflush(stdout);
	return;
}
