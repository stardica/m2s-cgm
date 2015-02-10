/*
 * configure.c
 *
 *  Created on: Nov 26, 2014
 *      Author: stardica
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/debug.h>

#include <lib/util/config.h>
#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <arch/common/arch.h>

#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/core.h>
#include <arch/x86/timing/thread.h>

#include <arch/si/timing/gpu.h>
#include <arch/si/timing/compute-unit.h>

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

	//configure the caches
	error = ini_parse(cgm_config_file_name_and_path, cache_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for cache configuration.\n");
		return 1;
	}

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
	//we don't use the config struct, so pass null and set everything by hand for now.

	if (MSG==1)
	{
		printf("CPU timing->MemConfigDefault(timing, NULL);\n");
		fflush(stdout);
		getchar();
	}

	timing->MemConfigDefault(timing, NULL);


	return 1;
}

int cgm_gpu_configure(void){

	Timing *timing;
	struct arch_t *arch;
	//char arch_name_trimmed = "x86";
	arch = arch_get("SouthernIslands");

	timing = arch->timing;


	//star >> uses the call back pointer for MemConfigDefault, but runs our cpu_configure function.
	//we don't use the config struct, so pass null and set everything by hand.

	if (MSG==1)
	{
		printf("GPU timing->MemConfigDefault(timing, NULL);\n");
		fflush(stdout);
		getchar();
	}

	timing->MemConfigDefault(timing, NULL);

	return 1;
}


int cpu_configure(Timing *self, struct config_t *config){

	//star todo (1) configure the CPU/Core/Thread here
	//			(2) get the number of cores/threads and configure each.
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

	//star todo pull core_index and thread_index automatically.
	//i think getting the size of the arrays will work.
	//other option is to set manually when we have fully defined the processor and memory system configurations.

	if (MSG ==1)
	{
		printf("number of cores %d\n", x86_cpu_num_cores);
		printf("number of threads %d\n", x86_cpu_num_threads);

		fflush(stdout);
		getchar();
	}

	//for now make sure number of cores and threads are 1
	if(x86_cpu_num_cores > 1)
	{
		fatal("Number of core > 1 STOP\n");
	}
	if(x86_cpu_num_threads > 1)
	{
		fatal("Number of threads > 1 STOP\n");
	}


	//star todo change this to connect to each cache.
	core_index = 0;
	thread_index = 0;
	core = cpu->cores[core_index];
	thread = core->threads[thread_index];

	//star todo link memory modules here.
	//do this somewhere else?

	//assign entry into memory system
	thread->mem_ctrl_ptr = mem_ctrl;


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

	//star todo  (1) configure the GPU here.
	//			 (2) determine the number of compute units and configure them all.

	if (MSG ==1)
	{
		printf("gpu_configure start\n");
		fflush(stdout);
		getchar();
	}

	int compute_unit_id;
	struct si_compute_unit_t *compute_unit;

	//star todo pull compute_unit_index automatically.
	//i think getting the size of the arrays will work?
	//other option is to set manually when we have fully defined the processor and memory system configurations.
	if (MSG ==1)
	{
		printf("number of si_gpu->compute_units %d\n", list_count(si_gpu->available_compute_units));
		fflush(stdout);
		getchar();
	}

	//for now make sure number of compute units is 1
	if(list_count(si_gpu->available_compute_units) > 1)
	{
		fatal("number of si_gpu->compute_units > 1 STOP\n");
	}


	compute_unit_id = 0;
	compute_unit = si_gpu->compute_units[compute_unit_id];

	compute_unit->mem_ctrl_ptr = mem_ctrl;

	if(MSG==1)
	{
		printf("gpu_configure end\n");
		printf("GPU mem entry name is %s", compute_unit->mem_ctrl_ptr->name);
		fflush(stdout);
		getchar();
	}

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

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int i = 0;

	int Sets = 0;
	int Assoc = 0;
	int BlockSize = 0;
	int Latency = 0;
	const char* Policy = "";
	int Ports = 0;
	int MSHR = 0;
	int DirectoryLatency = 0;



	/*configure CPU D caches*/
	if(MATCH("CPU_L1_D_Cache", "Sets"))
	{
		Sets = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].num_sets = Sets;
		}
	}

	if(MATCH("CPU_L1_D_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("CPU_L1_D_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("CPU_L1_D_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].latency = Latency;
		}
	}

	if(MATCH("CPU_L1_D_Cache", "Policy"))
	{
		Policy = strdup(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].policy = Policy;
		}
	}

	if(MATCH("CPU_L1_D_Cache", "MSHR"))
	{
		MSHR = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].mshr_size = MSHR;
		}
	}
	if(MATCH("CPU_L1_D_Cache", "DirectoryLatency"))
	{
		DirectoryLatency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].directory_latency = DirectoryLatency;
		}
	}

	if(MATCH("CPU_L1_D_Cache", "Ports"))
	{
		Ports = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].num_ports = Ports;
		}
	}

	/*configure CPU I caches*/
	if(MATCH("CPU_L1_I_Cache", "Sets"))
	{
		Sets = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].num_sets = Sets;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].latency = Latency;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "Policy"))
	{
		Policy = strdup(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].policy = Policy;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "MSHR"))
	{
		MSHR = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].mshr_size = MSHR;
		}
	}
	if(MATCH("CPU_L1_I_Cache", "DirectoryLatency"))
	{
		DirectoryLatency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].directory_latency = DirectoryLatency;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "Ports"))
	{
		Ports = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].num_ports = Ports;
		}
	}


	/*configure CPU L2 caches*/
	if(MATCH("CPU_L2_Cache", "Sets"))
	{
		Sets = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].num_sets = Sets;
		}
	}

	if(MATCH("CPU_L2_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("CPU_L2_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("CPU_L2_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].latency = Latency;
		}
	}

	if(MATCH("CPU_L2_Cache", "Policy"))
	{
		Policy = strdup(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].policy = Policy;
		}
	}

	if(MATCH("CPU_L2_Cache", "MSHR"))
	{
		MSHR = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].mshr_size = MSHR;
		}
	}
	if(MATCH("CPU_L2_Cache", "DirectoryLatency"))
	{
		DirectoryLatency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].directory_latency = DirectoryLatency;
		}
	}

	if(MATCH("CPU_L2_Cache", "Ports"))
	{
		Ports = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].num_ports = Ports;
		}
	}


	/*configure CPU L3 caches*/
	if(MATCH("CPU_L3_Cache", "Sets"))
	{
		Sets = atoi(value);
		int slice_size = Sets / 4;

		l3_s0_cache->num_sets = slice_size;
		l3_s1_cache->num_sets = slice_size;
		l3_s2_cache->num_sets = slice_size;
		l3_s3_cache->num_sets = slice_size;
	}

	if(MATCH("CPU_L3_Cache", "Assoc"))
	{
		Assoc = atoi(value);

		l3_s0_cache->assoc = Assoc;
		l3_s1_cache->assoc = Assoc;
		l3_s2_cache->assoc = Assoc;
		l3_s3_cache->assoc = Assoc;
	}

	if(MATCH("CPU_L3_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);

		l3_s0_cache->block_size = BlockSize;
		l3_s1_cache->block_size = BlockSize;
		l3_s2_cache->block_size = BlockSize;
		l3_s3_cache->block_size = BlockSize;
	}

	if(MATCH("CPU_L3_Cache", "Latency"))
	{
		Latency = atoi(value);
		l3_s0_cache->latency = Latency;
		l3_s1_cache->latency = Latency;
		l3_s2_cache->latency = Latency;
		l3_s3_cache->latency = Latency;
	}

	if(MATCH("CPU_L3_Cache", "Policy"))
	{
		Policy = strdup(value);

		l3_s0_cache->policy = Policy;
		l3_s1_cache->policy = Policy;
		l3_s2_cache->policy = Policy;
		l3_s3_cache->policy = Policy;

	}

	if(MATCH("CPU_L3_Cache", "MSHR"))
	{
		MSHR = atoi(value);

		l3_s0_cache->mshr_size = MSHR;
		l3_s1_cache->mshr_size = MSHR;
		l3_s2_cache->mshr_size = MSHR;
		l3_s3_cache->mshr_size = MSHR;
	}

	if(MATCH("CPU_L3_Cache", "DirectoryLatency"))
	{
		DirectoryLatency = atoi(value);

		l3_s0_cache->directory_latency = DirectoryLatency;
		l3_s1_cache->directory_latency = DirectoryLatency;
		l3_s2_cache->directory_latency = DirectoryLatency;
		l3_s3_cache->directory_latency = DirectoryLatency;
	}

	if(MATCH("CPU_L3_Cache", "Ports"))
	{
		Ports = atoi(value);

		l3_s0_cache->num_ports = Ports;
		l3_s1_cache->num_ports = Ports;
		l3_s2_cache->num_ports = Ports;
		l3_s3_cache->num_ports = Ports;
	}


	/*configure GPU V caches*/
	if(MATCH("GPU_V_Cache", "Sets"))
	{
		Sets = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l1_v_caches[i].num_sets = Sets;
		}
	}

	if(MATCH("GPU_V_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l1_v_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("GPU_V_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l1_v_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("GPU_V_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l1_v_caches[i].latency = Latency;
		}
	}

	/*configure GPU S caches*/
	if(MATCH("GPU_S_Cache", "Sets"))
	{
		Sets = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l1_s_caches[i].num_sets = Sets;
		}
	}

	if(MATCH("GPU_S_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l1_s_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("GPU_S_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l1_s_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("GPU_S_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l1_s_caches[i].latency = Latency;
		}
	}


	/*configure GPU L2 caches*/
	if(MATCH("GPU_L2_Cache", "Sets"))
	{
		Sets = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l2_caches[i].num_sets = Sets;
		}
	}

	if(MATCH("GPU_L2_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l2_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("GPU_L2_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l2_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("GPU_L2_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			l2_caches[i].latency = Latency;
		}
	}

	/*configure GPU LDS units*/
	if(MATCH("GPU_LDS", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			lds_units[i].block_size = BlockSize;
		}
	}

	if(MATCH("GPU_LDS", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			lds_units[i].latency = Latency;
		}
	}

	if(MATCH("GPU_LDS", "Ports"))
	{
		Ports = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			lds_units[i].num_ports = Ports;
		}
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
	getchar();
	return;
}
