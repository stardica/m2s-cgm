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
	error = ini_parse(cgm_config_file_name_and_path, cache_read_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for cache configuration.\n");
		return 1;
	}


	cache_finish_create();


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
		//getchar();
	}

	timing->MemConfigDefault(timing, NULL);


	return 1;
}

int cpu_configure(Timing *self, struct config_t *config){

	int i, j = 0;
	int num_cores = x86_cpu_num_cores;
	int num_threads = x86_cpu_num_threads;

	if (MSG ==1)
	{
		printf("cpu_configure start\n");
		printf("number of cores %d\n", x86_cpu_num_cores);
		printf("number of threads %d\n", x86_cpu_num_threads);
		fflush(stdout);
		//getchar();
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

			//assign entry into memory system
			thread->i_cache_ptr = l1_i_caches;
			thread->d_cache_ptr = l1_d_caches;

			if (MSG ==1)
			{
				printf("thread %d i_cache mem entry id is %d\n", thread->id_in_cpu, thread->i_cache_ptr[thread->core->id].id);
				printf("thread %d d_cache mem entry id is %d\n", thread->id_in_cpu, thread->d_cache_ptr[thread->core->id].id);
			}

		}

	}

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
		//getchar();
	}

	timing->MemConfigDefault(timing, NULL);

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

int cache_read_config(void* user, const char* section, const char* name, const char* value){

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


	/*get max queue size*/
	if(MATCH("Queue", "Size"))
	{
		QueueSize = atoi(value);
	}


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
			//printf("block size %d\n", BlockSize);
			//printf("block size %d\n", l1_d_caches[i].block_size);
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
		/*l3_s0_cache->block_size = BlockSize;
		l3_s1_cache->block_size = BlockSize;
		l3_s2_cache->block_size = BlockSize;
		l3_s3_cache->block_size = BlockSize;*/
	}

	if(MATCH("CPU_L3_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].latency = Latency;
		}

		/*l3_s0_cache->latency = Latency;
		l3_s1_cache->latency = Latency;
		l3_s2_cache->latency = Latency;
		l3_s3_cache->latency = Latency;*/
	}

	if(MATCH("CPU_L3_Cache", "Policy"))
	{
		Policy = strdup(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].policy = Policy;
		}
		/*l3_s0_cache->policy = Policy;
		l3_s1_cache->policy = Policy;
		l3_s2_cache->policy = Policy;
		l3_s3_cache->policy = Policy;*/

	}

	if(MATCH("CPU_L3_Cache", "MSHR"))
	{
		MSHR = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].mshr_size = MSHR;
		}

		/*l3_s0_cache->mshr_size = MSHR;
		l3_s1_cache->mshr_size = MSHR;
		l3_s2_cache->mshr_size = MSHR;
		l3_s3_cache->mshr_size = MSHR;*/
	}

	if(MATCH("CPU_L3_Cache", "DirectoryLatency"))
	{
		DirectoryLatency = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].directory_latency = DirectoryLatency;
		}
		/*l3_s0_cache->directory_latency = DirectoryLatency;
		l3_s1_cache->directory_latency = DirectoryLatency;
		l3_s2_cache->directory_latency = DirectoryLatency;
		l3_s3_cache->directory_latency = DirectoryLatency;*/
	}

	if(MATCH("CPU_L3_Cache", "Ports"))
	{
		Ports = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].num_ports = Ports;
		}
		/*l3_s0_cache->num_ports = Ports;
		l3_s1_cache->num_ports = Ports;
		l3_s2_cache->num_ports = Ports;
		l3_s3_cache->num_ports = Ports;*/
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


int cache_finish_create(){

	int num_cores = x86_cpu_num_cores;
	//int num_cus = si_gpu_num_compute_units;
	struct cache_block_t *block;
	int i, set, way = 0;
	char buff[100];




	//set log_block_size and block_mask
	for(i = 0; i < num_cores ; i++ )
	{
		l1_i_caches[i].id = i;
		l1_i_caches[i].log_block_size = LOG2(l1_i_caches[i].block_size);
		l1_i_caches[i].block_mask = l1_i_caches[i].block_size - 1;
		l1_i_caches[i].Rx_queue = list_create();
		l1_i_caches[i].snoop_queue = list_create();

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d]", i);
		l1_i_caches[i].name = strdup(buff);

		//set rx queue name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].Rx", i);
		l1_i_caches[i].Rx_queue->name = strdup(buff);

		//set rx queue name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].Snoop", i);
		l1_i_caches[i].snoop_queue->name = strdup(buff);


		l1_d_caches[i].id = i;
		l1_d_caches[i].log_block_size = LOG2(l1_d_caches[i].block_size);
		l1_d_caches[i].block_mask = l1_d_caches[i].block_size - 1;
		l1_d_caches[i].Rx_queue = list_create();
		l1_d_caches[i].snoop_queue = list_create();

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d]", i);
		l1_d_caches[i].name = strdup(buff);

		//set rx queue name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].Rx", i);
		l1_d_caches[i].Rx_queue->name = strdup(buff);

		//set rx queue name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].Snoop", i);
		l1_d_caches[i].snoop_queue->name = strdup(buff);

		l2_caches[i].id = i;
		l2_caches[i].log_block_size = LOG2(l2_caches[i].block_size);
		l2_caches[i].block_mask = l2_caches[i].block_size - 1;
		l2_caches[i].Rx_queue = list_create();
		l2_caches[i].snoop_queue = list_create();

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l2_caches[%d]", i);
		l2_caches[i].name = strdup(buff);

		//set rx queue name
		memset (buff, '\0', sizeof(buff));
		snprintf(buff,100, "l2_caches[%d].Rx", i);
		l2_caches[i].Rx_queue->name = strdup(buff);

		memset (buff, '\0', sizeof(buff));
		snprintf(buff,100, "l2_caches[%d].Snoop", i);
		l2_caches[i].snoop_queue->name = strdup(buff);



		/*printf("%s ----> %s\n", l1_i_caches[i].name, l1_i_caches[i].Rx_queue->name);
		printf("%s ----> %s\n", l1_d_caches[i].name, l1_d_caches[i].Rx_queue->name);
		printf("%s ----> %s\n", l2_caches[i].name, l2_caches[i].Rx_queue->name);*/



		//Initialize array of sets
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

			//printf("l1_i_caches[%d].sets[%d].id = %d\n", i, set, l1_i_caches[i].sets[set].id);
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

			//printf("l1_d_caches[%d].sets[%d].id = %d\n", i, set, l1_d_caches[i].sets[set].id);
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

			//printf("l2_caches[%d].sets[%d].id = %d\n", i, set, l2_caches[i].sets[set].id);
		}

	}

	//fatal("stop here\n");
	//star todo finish configuring the rest of the caches.

	//System caches
	//l3_caches;

	//GPU caches
	//l1_v_caches;
	//l1_s_caches;
	//l2_caches;
	//lds_units;

	return 0;
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
