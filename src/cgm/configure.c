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
#include <lib/util/string.h>

#include <arch/common/arch.h>

#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/core.h>
#include <arch/x86/timing/thread.h>

#include <arch/si/timing/gpu.h>
#include <arch/si/timing/compute-unit.h>

#include <cgm/configure.h>
#include <cgm/cgm.h>
#include <cgm/ini-parse.h>
#include <cgm/cache.h>
#include <cgm/tasking.h>
#include <cgm/mem-ctrl.h>
#include <cgm/directory.h>
#include <cgm/switch.h>


int cgmmem_check_config = 0;


int cgm_mem_configure(void){

	int error = 0;

	//configure the caches
	error = ini_parse(cgm_config_file_name_and_path, cache_read_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for cache configuration.\n");
		return 1;
	}

	cache_finish_create();

	error = ini_parse(cgm_config_file_name_and_path, directory_read_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for dir configuration.\n");
		return 1;
	}

	directory_finish_create();

	error = ini_parse(cgm_config_file_name_and_path, switch_read_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for switch configuration.\n");
		return 1;
	}

	switch_finish_create();

	//get sysagent configuration
	error = ini_parse(cgm_config_file_name_and_path, sys_agent_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for sysagent configuration.\n");
		return 1;
	}

	sys_agent_finish_create();

	//configure the memory controller
	error = ini_parse(cgm_config_file_name_and_path, mem_ctrl_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for memctrl configuration.\n");
		return 1;
	}

	mem_ctrl_finish_create();

	return 0;

}

int sys_agent_config(void* user, const char* section, const char* name, const char* value){



	return 0;
}

int sys_agent_finish_create(void){


	return 0;
}

int mem_ctrl_config(void* user, const char* section, const char* name, const char* value){


	return 0;
}

int mem_ctrl_finish_create(void){

	return 0;
}


int cgm_cpu_configure(void){

	//star >> uses the call back pointer for MemConfigDefault, but runs our cpu_configure function.
	//we don't use the config struct, so pass null and set everything by hand for now.

	Timing *timing;
	struct arch_t *arch;
	//char arch_name_trimmed = "x86";
	arch = arch_get("x86");
	timing = arch->timing;

	if (MSG==1)
	{
		printf("CPU timing->MemConfigDefault(timing, NULL);\n");
		fflush(stdout);
	}

	timing->MemConfigDefault(timing, NULL);

	return 1;
}

int cpu_configure(Timing *self, struct config_t *config){

	//star set the thread pointer to our memory system entries.

	int i, j = 0;
	int num_cores = x86_cpu_num_cores;
	int num_threads = x86_cpu_num_threads;

	if (MSG ==1)
	{
		printf("cpu_configure start\n");
		printf("number of cores %d\n", x86_cpu_num_cores);
		printf("number of threads %d\n", x86_cpu_num_threads);
		fflush(stdout);
	}

	X86Cpu *cpu = asX86Cpu(self);
	X86Core *core;
	X86Thread *thread;

	for(i = 0; i < num_cores; i++)
	{
		for(j = 0; j < num_threads; j++)
		{
			core = cpu->cores[i];
			thread = core->threads[j];

			//printf("thread assign core %d  thread %d\n", core->id, thread->id_in_core);

			//assert(core);
			//assert(thread);

			//assign entry into memory system
			thread->i_cache_ptr = l1_i_caches;
			thread->d_cache_ptr = l1_d_caches;
			//core->threads->i_cache_ptr = &l1_i_caches[core->id];
			//core->threads->d_cache_ptr = &l1_d_caches[core->id];

			//core->threads[j]->i_cache_ptr = &l1_i_caches[core->id];
			//core->threads[j]->d_cache_ptr = &l1_d_caches[core->id];

		}

		/*core = cpu->cores[i];
		core->threads[0]->i_cache_ptr = &l1_i_caches[core->id];
		core->threads[0]->d_cache_ptr = &l1_d_caches[core->id];*/

	}

	//getchar();
	return 1;
}


int cgm_gpu_configure(void){

	//star >> uses the call back pointer for MemConfigDefault, but runs our cpu_configure function.
	//we don't use the config struct, so pass null and set everything by hand.

	Timing *timing;
	struct arch_t *arch;
	//char arch_name_trimmed = "x86";
	arch = arch_get("SouthernIslands");
	timing = arch->timing;


	if (MSG==1)
	{
		printf("GPU timing->MemConfigDefault(timing, NULL);\n");
		fflush(stdout);
		//getchar();
	}

	timing->MemConfigDefault(timing, NULL);

	return 1;
}


int gpu_configure(Timing *self, struct config_t *config){

	int num_cus = si_gpu_num_compute_units;
	int i = 0;

	if (MSG ==1)
	{
		printf("gpu_configure start\n");
		fflush(stdout);
	}


	//star todo pull compute_unit_index automatically.
	//i think getting the size of the arrays will work?
	//other option is to set manually when we have fully defined the processor and memory system configurations.
	if (MSG ==1)
	{
		printf("number of si_gpu->compute_units %d\n", list_count(si_gpu->available_compute_units));
		fflush(stdout);
	}

	//for now make sure number of compute units is 1
	/*if(list_count(si_gpu->available_compute_units) > 1)
	{
		fatal("number of si_gpu->compute_units > 1 STOP\n");
	}*/

	int compute_unit_id;
	struct si_compute_unit_t *compute_unit;


	//star todo set this up for multiple CUs
	for(i = 0; i < num_cus; i++)
	{
		compute_unit = si_gpu->compute_units[i];
		compute_unit->gpu_v_cache_ptr = gpu_v_caches;
		compute_unit->gpu_s_cache_ptr = gpu_s_caches;
		compute_unit->gpu_lds_unit_ptr = gpu_lds_units;
	}

	/*if(MSG==1)
	{
		printf("gpu_configure end\n");
		printf("GPU mem entry name is %s", compute_unit->mem_ctrl_ptr->name);
		fflush(stdout);
	}*/

	return 1;
}

int cache_read_config(void* user, const char* section, const char* name, const char* value){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int i = 0;

	int Slices = 0;
	int Sets = 0;
	int Assoc = 0;
	int BlockSize = 0;
	int Latency = 0;
	const char* Policy = "";
	int Ports = 0;
	int MSHR = 0;
	int DirectoryLatency = 0;
	int WireLatency = 0;


	/*get max queue size*/
	if(MATCH("Queue", "Size"))
	{
		QueueSize = atoi(value);
	}


	////////////
	//CPU Caches
	////////////

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

	if(MATCH("CPU_L1_D_Cache", "WireLatency"))
	{
		WireLatency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].wire_latency = WireLatency;
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
			l1_i_caches[i].num_sets = Sets;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_i_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_i_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_i_caches[i].latency = Latency;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "Policy"))
	{
		Policy = strdup(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_i_caches[i].policy = Policy;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "MSHR"))
	{
		MSHR = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_i_caches[i].mshr_size = MSHR;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "DirectoryLatency"))
	{
		DirectoryLatency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_i_caches[i].directory_latency = DirectoryLatency;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "WireLatency"))
	{
		WireLatency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].wire_latency = WireLatency;
		}
	}

	if(MATCH("CPU_L1_I_Cache", "Ports"))
	{
		Ports = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_i_caches[i].num_ports = Ports;
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

	if(MATCH("CPU_L2_Cache", "WireLatency"))
	{
		WireLatency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].wire_latency = WireLatency;
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
	if(MATCH("CPU_L3_Cache", "Slices"))
	{
		Slices = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].num_slices = Slices;
		}
	}


	/*configure CPU L3 caches*/
	if(MATCH("CPU_L3_Cache", "Sets"))
	{
		Sets = atoi(value);
		int slice_size = (Sets / l3_caches[i].num_slices);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].num_sets = slice_size;
		}
	}

	if(MATCH("CPU_L3_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("CPU_L3_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("CPU_L3_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].latency = Latency;
		}
	}

	if(MATCH("CPU_L3_Cache", "Policy"))
	{
		Policy = strdup(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].policy = Policy;
		}
	}

	if(MATCH("CPU_L3_Cache", "MSHR"))
	{
		MSHR = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].mshr_size = MSHR;
		}
	}

	if(MATCH("CPU_L3_Cache", "DirectoryLatency"))
	{
		DirectoryLatency = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].directory_latency = DirectoryLatency;
		}
	}

	if(MATCH("CPU_L3_Cache", "WireLatency"))
	{
		WireLatency = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].wire_latency = WireLatency;
		}
	}

	if(MATCH("CPU_L3_Cache", "Ports"))
	{
		Ports = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].num_ports = Ports;
		}
	}



	////////////
	//GPU Caches
	////////////

	/*configure GPU V caches*/
	if(MATCH("GPU_V_Cache", "Sets"))
	{
		Sets = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_v_caches[i].num_sets = Sets;
		}
	}

	if(MATCH("GPU_V_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_v_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("GPU_V_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_v_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("GPU_V_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_v_caches[i].latency = Latency;
		}
	}


	/*configure GPU S caches*/
	if(MATCH("GPU_S_Cache", "Sets"))
	{
		Sets = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_s_caches[i].num_sets = Sets;
		}
	}

	if(MATCH("GPU_S_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_s_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("GPU_S_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_s_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("GPU_S_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_s_caches[i].latency = Latency;
		}
	}


	/*configure GPU L2 caches*/
	if(MATCH("GPU_L2_Cache", "Sets"))
	{
		Sets = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_l2_caches[i].num_sets = Sets;
		}
	}

	if(MATCH("GPU_L2_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_l2_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("GPU_L2_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_l2_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("GPU_L2_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_l2_caches[i].latency = Latency;
		}
	}

	/*configure GPU LDS units*/
	if(MATCH("GPU_LDS", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_lds_units[i].block_size = BlockSize;
		}
	}

	if(MATCH("GPU_LDS", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_lds_units[i].latency = Latency;
		}
	}

	if(MATCH("GPU_LDS", "Ports"))
	{
		Ports = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_lds_units[i].num_ports = Ports;
		}
	}

	return 0;
}

int cache_finish_create(){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	//int num_l2_groups = (si_gpu_num_compute_units/4);
	struct cache_block_t *block;
	int i, set, way = 0;
	char buff[100];


	//finish creating the CPU caches
	for(i = 0; i < num_cores ; i++ )
	{
		l1_i_caches[i].id = i;
		l1_i_caches[i].log_block_size = LOG2(l1_i_caches[i].block_size);
		l1_i_caches[i].log_set_size = LOG2(l1_i_caches[i].num_sets);
		l1_i_caches[i].block_mask = l1_i_caches[i].block_size - 1;
		l1_i_caches[i].set_mask = l1_i_caches[i].num_sets - 1;
		l1_i_caches[i].hits = 0;
		l1_i_caches[i].invalid_hits = 0;
		l1_i_caches[i].misses = 0;
		l1_i_caches[i].fetches = 0;
		l1_i_caches[i].Rx_queue_top = list_create();
		l1_i_caches[i].Rx_queue_bottom = list_create();
		l1_i_caches[i].mshr = list_create();

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d]", i);
		l1_i_caches[i].name = strdup(buff);

		//set rx queue names
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].Rx_queue_top", i);
		l1_i_caches[i].Rx_queue_top->name = strdup(buff);

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].Rx_queue_bottom", i);
		l1_i_caches[i].Rx_queue_bottom->name = strdup(buff);

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].mshr", i);
		l1_i_caches[i].mshr->name = strdup(buff);


		l1_d_caches[i].id = i;
		l1_d_caches[i].log_block_size = LOG2(l1_d_caches[i].block_size);
		l1_d_caches[i].log_set_size = LOG2(l1_d_caches[i].num_sets);
		l1_d_caches[i].block_mask = l1_d_caches[i].block_size - 1;
		l1_d_caches[i].set_mask = l1_d_caches[i].num_sets - 1;
		l1_d_caches[i].hits = 0;
		l1_d_caches[i].invalid_hits = 0;
		l1_d_caches[i].misses = 0;
		l1_d_caches[i].loads = 0;
		l1_d_caches[i].stores = 0;
		l1_d_caches[i].Rx_queue_top = list_create();
		l1_d_caches[i].Rx_queue_bottom = list_create();
		l1_d_caches[i].mshr = list_create();

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d]", i);
		l1_d_caches[i].name = strdup(buff);

		//set rx queue name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].Rx_queue_top", i);
		l1_d_caches[i].Rx_queue_top->name = strdup(buff);


		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].Rx_queue_bottom", i);
		l1_d_caches[i].Rx_queue_bottom->name = strdup(buff);


		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].mshr", i);
		l1_d_caches[i].mshr->name = strdup(buff);


		l2_caches[i].id = i;
		l2_caches[i].log_block_size = LOG2(l2_caches[i].block_size);
		l2_caches[i].log_set_size = LOG2(l2_caches[i].num_sets);
		l2_caches[i].block_mask = l2_caches[i].block_size - 1;
		l2_caches[i].set_mask = l2_caches[i].num_sets - 1;
		l2_caches[i].hits = 0;
		l2_caches[i].invalid_hits = 0;
		l2_caches[i].misses = 0;
		l2_caches[i].Rx_queue_top = list_create();
		l2_caches[i].Rx_queue_bottom = list_create();
		l2_caches[i].mshr = list_create();

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l2_caches[%d]", i);
		l2_caches[i].name = strdup(buff);

		//set rx queue name
		memset (buff, '\0', sizeof(buff));
		snprintf(buff,100, "l2_caches[%d].Rx_queue_top", i);
		l2_caches[i].Rx_queue_top->name = strdup(buff);

		memset (buff, '\0', sizeof(buff));
		snprintf(buff,100, "l2_caches[%d].Rx_queue_bottom", i);
		l2_caches[i].Rx_queue_bottom->name = strdup(buff);

		memset (buff, '\0', sizeof(buff));
		snprintf(buff,100, "l2_caches[%d].mshr", i);
		l2_caches[i].mshr->name = strdup(buff);


		l3_caches[i].id = 1;
		l3_caches[i].log_block_size = LOG2(l3_caches[i].block_size);
		l3_caches[i].log_set_size = LOG2(l3_caches[i].num_sets);
		l3_caches[i].block_mask = l3_caches[i].block_size - 1;
		l3_caches[i].set_mask = l3_caches[i].num_sets - 1;
		l3_caches[i].hits = 0;
		l3_caches[i].invalid_hits = 0;
		l3_caches[i].misses = 0;
		l3_caches[i].Rx_queue_top = list_create();
		l3_caches[i].Rx_queue_bottom = list_create();
		l3_caches[i].mshr = list_create();

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d]", i);
		l3_caches[i].name = strdup(buff);

		//set rx queue name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].Rx_queue_top", i);
		l3_caches[i].Rx_queue_top->name = strdup(buff);

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].Rx_queue_bottom", i);
		l3_caches[i].Rx_queue_bottom->name = strdup(buff);

		//set rx queue name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].mshr", i);
		l3_caches[i].mshr->name = strdup(buff);

		//Initialize array of sets
		//l1_i_caches[i].num_sets = (l1_i_caches[i].num_sets / l1_i_caches[i].assoc);
		l1_i_caches[i].sets = calloc(l1_i_caches[i].num_sets, sizeof(struct cache_set_t));
		for (set = 0; set < l1_i_caches[i].num_sets; set++)
		{
			//Initialize array of blocks
			l1_i_caches[i].sets[set].id = set;
			l1_i_caches[i].sets[set].blocks = calloc(l1_i_caches[i].assoc, sizeof(struct cache_block_t));
			l1_i_caches[i].sets[set].way_head = &l1_i_caches[i].sets[set].blocks[0];
			l1_i_caches[i].sets[set].way_tail = &l1_i_caches[i].sets[set].blocks[l1_i_caches[i].assoc - 1];
			for (way = 0; way < l1_i_caches[i].assoc; way++)
			{
				block = &l1_i_caches[i].sets[set].blocks[way];
				block->way = way;
				block->way_prev = way ? &l1_i_caches[i].sets[set].blocks[way - 1] : NULL;
				block->way_next = way < l1_i_caches[i].assoc - 1 ? &l1_i_caches[i].sets[set].blocks[way + 1] : NULL;
			}

		}

		l1_d_caches[i].sets = calloc(l1_i_caches[i].num_sets, sizeof(struct cache_set_t));
		for (set = 0; set < l1_d_caches[i].num_sets; set++)
		{
			//Initialize array of blocks
			l1_d_caches[i].sets[set].id = set;
			l1_d_caches[i].sets[set].blocks = calloc(l1_d_caches[i].assoc, sizeof(struct cache_block_t));
			l1_d_caches[i].sets[set].way_head = &l1_d_caches[i].sets[set].blocks[0];
			l1_d_caches[i].sets[set].way_tail = &l1_d_caches[i].sets[set].blocks[l1_d_caches[i].assoc - 1];
			for (way = 0; way < l1_d_caches[i].assoc; way++)
			{
				block = &l1_d_caches[i].sets[set].blocks[way];
				block->way = way;
				block->way_prev = way ? &l1_d_caches[i].sets[set].blocks[way - 1] : NULL;
				block->way_next = way < l1_d_caches[i].assoc - 1 ? &l1_d_caches[i].sets[set].blocks[way + 1] : NULL;
			}

		}

		l2_caches[i].sets = calloc(l2_caches[i].num_sets, sizeof(struct cache_set_t));
		for (set = 0; set < l2_caches[i].num_sets; set++)
		{
			//Initialize array of blocks
			l2_caches[i].sets[set].id = set;
			l2_caches[i].sets[set].blocks = calloc(l2_caches[i].assoc, sizeof(struct cache_block_t));
			l2_caches[i].sets[set].way_head = &l2_caches[i].sets[set].blocks[0];
			l2_caches[i].sets[set].way_tail = &l2_caches[i].sets[set].blocks[l2_caches[i].assoc - 1];
			for (way = 0; way < l2_caches[i].assoc; way++)
			{
				block = &l2_caches[i].sets[set].blocks[way];
				block->way = way;
				block->way_prev = way ? &l2_caches[i].sets[set].blocks[way - 1] : NULL;
				block->way_next = way < l2_caches[i].assoc - 1 ? &l2_caches[i].sets[set].blocks[way + 1] : NULL;
			}

		}

		l3_caches[i].sets = calloc(l3_caches[i].num_sets, sizeof(struct cache_set_t));
		for (set = 0; set < l3_caches[i].num_sets; set++)
		{
			//Initialize array of blocks
			l3_caches[i].sets[set].id = set;
			l3_caches[i].sets[set].blocks = calloc(l3_caches[i].assoc, sizeof(struct cache_block_t));
			l3_caches[i].sets[set].way_head = &l3_caches[i].sets[set].blocks[0];
			l3_caches[i].sets[set].way_tail = &l3_caches[i].sets[set].blocks[l3_caches[i].assoc - 1];

			for (way = 0; way < l3_caches[i].assoc; way++)
			{
				block = &l3_caches[i].sets[set].blocks[way];
				block->way = way;
				block->way_prev = way ? &l3_caches[i].sets[set].blocks[way - 1] : NULL;
				block->way_next = way < l3_caches[i].assoc - 1 ? &l3_caches[i].sets[set].blocks[way + 1] : NULL;
			}
		}


		//star todo init directory here?

	}


	//finish creating the GPU caches
	for(i = 0 ; i < num_cus; i++)
	{

		//vector caches
		gpu_v_caches[i].id = i;
		gpu_v_caches[i].log_block_size = LOG2(gpu_v_caches[i].block_size);
		gpu_v_caches[i].log_set_size = LOG2(gpu_v_caches[i].num_sets);
		gpu_v_caches[i].block_mask = gpu_v_caches[i].block_size - 1;
		gpu_v_caches[i].set_mask = gpu_v_caches[i].num_sets - 1;
		gpu_v_caches[i].hits = 0;
		gpu_v_caches[i].invalid_hits = 0;
		gpu_v_caches[i].misses = 0;
		//gpu_v_caches[i].fetches = 0;
		gpu_v_caches[i].Rx_queue_top = list_create();
		gpu_v_caches[i].Rx_queue_bottom = list_create();
		gpu_v_caches[i].mshr = list_create();

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d]", i);
		gpu_v_caches[i].name = strdup(buff);

		//set rx queue names
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].Rx_queue_top", i);
		gpu_v_caches[i].Rx_queue_top->name = strdup(buff);

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].Rx_queue_bottom", i);
		gpu_v_caches[i].Rx_queue_bottom->name = strdup(buff);

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].mshr", i);
		gpu_v_caches[i].mshr->name = strdup(buff);


		//scalar caches
		gpu_s_caches[i].id = i;
		gpu_s_caches[i].log_block_size = LOG2(gpu_s_caches[i].block_size);
		gpu_s_caches[i].log_set_size = LOG2(gpu_s_caches[i].num_sets);
		gpu_s_caches[i].block_mask = gpu_s_caches[i].block_size - 1;
		gpu_s_caches[i].set_mask = gpu_s_caches[i].num_sets - 1;
		gpu_s_caches[i].hits = 0;
		gpu_s_caches[i].invalid_hits = 0;
		gpu_s_caches[i].misses = 0;
		//gpu_s_caches[i].fetches = 0;
		gpu_s_caches[i].Rx_queue_top = list_create();
		gpu_s_caches[i].Rx_queue_bottom = list_create();
		gpu_s_caches[i].mshr = list_create();

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d]", i);
		gpu_s_caches[i].name = strdup(buff);

		//set rx queue names
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].Rx_queue_top", i);
		gpu_s_caches[i].Rx_queue_top->name = strdup(buff);

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].Rx_queue_bottom", i);
		gpu_s_caches[i].Rx_queue_bottom->name = strdup(buff);

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].mshr", i);
		gpu_s_caches[i].mshr->name = strdup(buff);


		//LDS caches
		gpu_lds_units[i].id = i;
		gpu_lds_units[i].log_block_size = LOG2(gpu_lds_units[i].block_size);
		gpu_lds_units[i].log_set_size = LOG2(gpu_lds_units[i].num_sets);
		gpu_lds_units[i].block_mask = gpu_lds_units[i].block_size - 1;
		gpu_lds_units[i].set_mask = gpu_lds_units[i].num_sets - 1;
		gpu_lds_units[i].hits = 0;
		gpu_lds_units[i].invalid_hits = 0;
		gpu_lds_units[i].misses = 0;
		//gpu_v_caches[i].fetches = 0;
		gpu_lds_units[i].Rx_queue_top = list_create();
		gpu_lds_units[i].Rx_queue_bottom = list_create();
		gpu_lds_units[i].mshr = list_create();

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_lds_units[%d]", i);
		gpu_lds_units[i].name = strdup(buff);

		//set rx queue names
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_lds_units[%d].Rx_queue_top", i);
		gpu_lds_units[i].Rx_queue_top->name = strdup(buff);

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_lds_units[%d].Rx_queue_bottom", i);
		gpu_lds_units[i].Rx_queue_bottom->name = strdup(buff);

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_lds_units[%d].mshr", i);
		gpu_lds_units[i].mshr->name = strdup(buff);


		//Initialize array of sets
		gpu_v_caches[i].sets = calloc(gpu_v_caches[i].num_sets, sizeof(struct cache_set_t));
		for (set = 0; set < gpu_v_caches[i].num_sets; set++)
		{
			//Initialize array of blocks
			gpu_v_caches[i].sets[set].id = set;
			gpu_v_caches[i].sets[set].blocks = calloc(gpu_v_caches[i].assoc, sizeof(struct cache_block_t));
			gpu_v_caches[i].sets[set].way_head = &gpu_v_caches[i].sets[set].blocks[0];
			gpu_v_caches[i].sets[set].way_tail = &gpu_v_caches[i].sets[set].blocks[gpu_v_caches[i].assoc - 1];
			for (way = 0; way < gpu_v_caches[i].assoc; way++)
			{
				block = &gpu_v_caches[i].sets[set].blocks[way];
				block->way = way;
				block->way_prev = way ? &gpu_v_caches[i].sets[set].blocks[way - 1] : NULL;
				block->way_next = way < gpu_v_caches[i].assoc - 1 ? &gpu_v_caches[i].sets[set].blocks[way + 1] : NULL;
			}
		}

		gpu_s_caches[i].sets = calloc(gpu_s_caches[i].num_sets, sizeof(struct cache_set_t));
		for (set = 0; set < gpu_s_caches[i].num_sets; set++)
		{
			//Initialize array of blocks
			gpu_s_caches[i].sets[set].id = set;
			gpu_s_caches[i].sets[set].blocks = calloc(gpu_s_caches[i].assoc, sizeof(struct cache_block_t));
			gpu_s_caches[i].sets[set].way_head = &gpu_s_caches[i].sets[set].blocks[0];
			gpu_s_caches[i].sets[set].way_tail = &gpu_s_caches[i].sets[set].blocks[gpu_s_caches[i].assoc - 1];
			for (way = 0; way < gpu_s_caches[i].assoc; way++)
			{
				block = &gpu_s_caches[i].sets[set].blocks[way];
				block->way = way;
				block->way_prev = way ? &gpu_s_caches[i].sets[set].blocks[way - 1] : NULL;
				block->way_next = way < gpu_s_caches[i].assoc - 1 ? &gpu_s_caches[i].sets[set].blocks[way + 1] : NULL;
			}
		}


		gpu_lds_units[i].sets = calloc(gpu_lds_units[i].num_sets, sizeof(struct cache_set_t));
		for (set = 0; set < gpu_lds_units[i].num_sets; set++)
		{
			//Initialize array of blocks
			gpu_lds_units[i].sets[set].id = set;
			gpu_lds_units[i].sets[set].blocks = calloc(gpu_lds_units[i].assoc, sizeof(struct cache_block_t));
			gpu_lds_units[i].sets[set].way_head = &gpu_lds_units[i].sets[set].blocks[0];
			gpu_lds_units[i].sets[set].way_tail = &gpu_lds_units[i].sets[set].blocks[gpu_lds_units[i].assoc - 1];
			for (way = 0; way < gpu_lds_units[i].assoc; way++)
			{
				block = &gpu_lds_units[i].sets[set].blocks[way];
				block->way = way;
				block->way_prev = way ? &gpu_lds_units[i].sets[set].blocks[way - 1] : NULL;
				block->way_next = way < gpu_lds_units[i].assoc - 1 ? &gpu_lds_units[i].sets[set].blocks[way + 1] : NULL;
			}
		}


		//set up one cache for every four CUs
		//mod 3 because i starts at zero.
		//if((i % 3) == 0)
		//{

			gpu_l2_caches[i].id = i;
			gpu_l2_caches[i].log_block_size = LOG2(gpu_l2_caches[i].block_size);
			gpu_l2_caches[i].log_set_size = LOG2(gpu_l2_caches[i].num_sets);
			gpu_l2_caches[i].block_mask = gpu_l2_caches[i].block_size - 1;
			gpu_l2_caches[i].set_mask = gpu_l2_caches[i].num_sets - 1;
			gpu_l2_caches[i].hits = 0;
			gpu_l2_caches[i].invalid_hits = 0;
			gpu_l2_caches[i].misses = 0;
			//gpu_v_caches[i].fetches = 0;
			gpu_l2_caches[i].Rx_queue_top = list_create();
			gpu_l2_caches[i].Rx_queue_bottom = list_create();
			gpu_l2_caches[i].mshr = list_create();

			//set cache name
			memset (buff,'\0' , 100);
			snprintf(buff, 100, "gpu_l2_caches[%d]", i);
			gpu_l2_caches[i].name = strdup(buff);

			//set rx queue names
			memset (buff,'\0' , 100);
			snprintf(buff, 100, "gpu_l2_caches[%d].Rx_queue_top", i);
			gpu_l2_caches[i].Rx_queue_top->name = strdup(buff);

			memset (buff,'\0' , 100);
			snprintf(buff, 100, "gpu_l2_caches[%d].Rx_queue_bottom", i);
			gpu_l2_caches[i].Rx_queue_bottom->name = strdup(buff);

			memset (buff,'\0' , 100);
			snprintf(buff, 100, "gpu_l2_caches[%d].mshr", i);
			gpu_l2_caches[i].mshr->name = strdup(buff);


			gpu_l2_caches[i].sets = calloc(gpu_l2_caches[i].num_sets, sizeof(struct cache_set_t));
			for (set = 0; set < gpu_l2_caches[i].num_sets; set++)
			{
				//Initialize array of blocks
				gpu_l2_caches[i].sets[set].id = set;
				gpu_l2_caches[i].sets[set].blocks = calloc(gpu_l2_caches[i].assoc, sizeof(struct cache_block_t));
				gpu_l2_caches[i].sets[set].way_head = &gpu_l2_caches[i].sets[set].blocks[0];
				gpu_l2_caches[i].sets[set].way_tail = &gpu_l2_caches[i].sets[set].blocks[gpu_l2_caches[i].assoc - 1];
				for (way = 0; way < gpu_l2_caches[i].assoc; way++)
				{
					block = &gpu_l2_caches[i].sets[set].blocks[way];
					block->way = way;
					block->way_prev = way ? &gpu_l2_caches[i].sets[set].blocks[way - 1] : NULL;
					block->way_next = way < gpu_l2_caches[i].assoc - 1 ? &gpu_l2_caches[i].sets[set].blocks[way + 1] : NULL;
				}
			}
		//}
	}
	return 0;
}

int directory_read_config(void* user, const char* section, const char* name, const char* value){


	if(MATCH("Directory", "MemSize"))
	{
		directory->mem_image_size = atoll(value);//4GB;
	}

	if(MATCH("Directory", "BlockSize"))
	{
		directory->block_size = atoi(value);
	}

	if(MATCH("Directory", "Mode"))
	{
		directory->mode = atoi(value);
	}

	if(MATCH("Directory", "VectorSize"))
	{
		directory->vector_size = atoi(value);
	}


	return 0;
}

int directory_finish_create(void){

	int num_cores = x86_cpu_num_cores;
	char buff[100];
	int i = 0;

	// create the number of queues we need.
	// for our base system one entry per l3 cache slice.
	// this can also be set up as one entry per system agent.

	//do some checks
	if(directory->block_size != l3_caches->block_size)
	{
		fatal("directory_finish_create() block size needs to match l3 block size. Check the cgm_config.ini file");
	}

	//init the data vectors
	//m2s doesn't natively account for dir memory overhead IN the memory image
	//get the number of blocks in the system
	directory->num_blocks = (directory->mem_image_size/directory->block_size);

	//this accounts for directory overhead
	//directory->num_blocks = (directory->mem_image_size/(directory->block_size + directory->vector_size));

	//get the block mask
	directory->block_mask = directory->block_size - 1; // this is 0 - N


	if(directory->vector_size == 1)
	{
		directory->bit_vector_8 = (void *) calloc(directory->num_blocks, sizeof(unsigned char));
	}
	else if(directory->vector_size == 2)
	{
		directory->bit_vector_16 = (void *) calloc(directory->num_blocks, sizeof(unsigned short));
	}
	else if(directory->vector_size == 4)
	{
		directory->bit_vector_24 = (void *) calloc(directory->num_blocks, sizeof(unsigned long));
	}
	else if (directory->vector_size == 8)
	{
		directory->bit_vector_64 = (void *) calloc(directory->num_blocks, sizeof(unsigned long long));
	}
	else
	{
		fatal("directory_finish_create() invalid vector size. Check the cgm_config.ini file");
	}


	//set up input queues
	if (directory->mode)
	{
		directory->Rx_queue = (void *) calloc(num_cores, sizeof(struct list_t));

		for(i = 0; i < num_cores ; i++)
		{
			directory->Rx_queue[i] = list_create();
			memset(buff,'\0' , 100);
			snprintf(buff, 100, "Rx_queue.%d", i);
			directory->Rx_queue[i]->name = strdup(buff);
			//printf("directory->Rx_queue[%d] name = %s\n", i, directory->Rx_queue[i]->name);
		}
	}
	else
	{
		fatal("directory_finish_create() currently only support distributed mode. Check the cgm_config.ini file");
	}

	/*printf("directory->mem_image_size %llu\n", directory->mem_image_size);
	printf("directory->block_size %llu\n", directory->block_size);
	printf("num_blocks %llu\n", num_blocks);
	getchar();*/

	return 0;

}

int switch_read_config(void* user, const char* section, const char* name, const char* value){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int Ports = 0;
	int i = 0;
	int WireLatency = 0;

	//star todo fix this
	int extras = 1;

	if(MATCH("Switch", "Ports"))
	{
		Ports = atoi(value);
		for (i = 0; i < (num_cores + extras); i++)
		{
			switches[i].port_num = Ports;
		}
	}

	if(MATCH("Switch", "WireLatency"))
	{
		WireLatency = atoi(value);

		for (i = 0; i < (num_cores + extras); i++)
		{
			switches[i].wire_latency = WireLatency;
		}
	}


	return 0;
}

int switch_finish_create(void){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;

	//star todo fix this
	int extras = 1;

	char buff[100];
	int i = 0;
	float median = 0;

	//create the queues
	if(switches[0].port_num == 4)
	{
		for (i = 0; i < (num_cores + extras); i++)
		{

			memset (buff,'\0' , 100);
			snprintf(buff, 100, "switch[%d]", i);
			switches[i].name = strdup(buff);

			switches[i].north_queue = list_create();
			switches[i].east_queue = list_create();
			switches[i].south_queue = list_create();
			switches[i].west_queue = list_create();

			memset (buff,'\0' , 100);
			snprintf(buff, 100, "switch[%d].north_queue", i);
			switches[i].north_queue->name = strdup(buff);

			memset (buff,'\0' , 100);
			snprintf(buff, 100, "switch[%d].east_queue", i);
			switches[i].east_queue->name = strdup(buff);

			memset (buff,'\0' , 100);
			snprintf(buff, 100, "switch[%d].south_queue", i);
			switches[i].south_queue->name = strdup(buff);

			memset (buff,'\0' , 100);
			snprintf(buff, 100, "switch[%d].west_queue", i);
			switches[i].west_queue->name = strdup(buff);

			//init the queue pointer
			switches[i].queue = west_queue;

			//init the switche's network node number
			switches[i].switch_node_number = str_map_string(&node_strn_map, switches[i].name);
			median += switches[i].switch_node_number;

		}
	}
	else if(switches[0].port_num == 6)
	{
		fatal("switch_finish_create() 6 ports currently unsupported\n");
	}
	else
	{
		fatal("switch_finish_create() port_num error\n");
	}


	//init the median node number on all switches
	for (i = 0; i < (num_cores + extras); i++)
	{
		//get the average
		switches[i].switch_median_node_num = (median/(num_cores + extras));
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
	printf("\n");
	printf("---Memory Controller Initialized---\n");
	printf("block_size = %d\n", mem_ctrl->block_size);
	printf("log_block_size = %d\n", mem_ctrl->log_block_size);
	printf("latency = %d\n", mem_ctrl->latency);
	printf("dir_latency = %d\n", mem_ctrl->dir_latency);
	printf("Number of queues = %d\n", mem_ctrl->ports);
	printf("Queue name = %s\n", mem_ctrl->fetch_request_queue->name);
	printf("Queue name = %s\n", mem_ctrl->issue_request_queue->name);
	fflush(stdout);
	getchar();*/
	return;
}
