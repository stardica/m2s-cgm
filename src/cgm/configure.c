/*
 * configure.c
 *
 *  Created on: Nov 26, 2014
 *      Author: stardica
 */

#include <stdio.h>
#include <cgm/configure.h>
#include <mem-image/memory.h>
#include <limits.h>

int cgmmem_check_config = 0;

int cgm_mem_configure(struct mem_t *mem){

	int error = 0;

	error = ini_parse(cgm_config_file_name_and_path, debug_read_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for debug configuration.\n");
		return 1;
	}

	debug_finish_create();

	error = ini_parse(cgm_config_file_name_and_path, stats_read_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for stats configuration.\n");
		return 1;
	}

	stats_finish_create();


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

	//get sys agent configuration
	error = ini_parse(cgm_config_file_name_and_path, sys_agent_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for sys agent configuration.\n");
		return 1;
	}

	sys_agent_finish_create();

	//configure the memory controller
	error = ini_parse(cgm_config_file_name_and_path, mem_ctrl_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for mem ctrl configuration.\n");
		return 1;
	}

	mem_ctrl_finish_create(mem);

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

void cpu_configure(Timing *self, struct config_t *config){

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

	if(num_cores <= 0 || num_cores > 4)
	{
		fatal("For now, number of cores must be between 1 - 4\n");
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
	return;
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


void gpu_configure(Timing *self, struct config_t *config){

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

	/*int compute_unit_id;*/
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

	return;
}

int debug_read_config(void* user, const char* section, const char* name, const char* value){

	//star todo add debug category for protocol
	if(MATCH("Debug", "CPU_cache_Debug"))
	{
		CPU_cache_debug = atoi(value);
	}

	if(MATCH("Debug", "GPU_cache_Debug"))
	{
		GPU_cache_debug = atoi(value);
	}

	if(MATCH("Debug", "Switch_Debug"))
	{
		switch_debug = atoi(value);
	}

	if(MATCH("Debug", "Hub_IOMMU_Debug"))
	{
		hub_iommu_debug = atoi(value);
	}

	if(MATCH("Debug", "SysAgent_Debug"))
	{
		sysagent_debug = atoi(value);
	}

	if(MATCH("Debug", "MemCtrl_Debug"))
	{
		memctrl_debug = atoi(value);
	}

	if(MATCH("Debug", "Protocol_Debug"))
	{
		protocol_debug = atoi(value);
	}

	if(MATCH("Debug", "ORT_Debug"))
	{
		ort_debug = atoi(value);
	}

	if(MATCH("Debug", "Load_Store_Debug"))
	{
		load_store_debug = atoi(value);
	}

	if(MATCH("Debug", "Path"))
	{
		cgm_debug_output_path = strdup(value);
	}

	if(MATCH("Debug", "Watch_Dog"))
	{
		watch_dog = atoi(value);
	}

	return 1;
}

int debug_finish_create(void){

	char buff[250];

	if (CPU_cache_debug == 1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_debug_output_path);
		sprintf(buff + strlen(buff), "/CPU_cache_debug.out");
		CPU_cache_debug_file = fopen (buff, "w+");
	}

	if (GPU_cache_debug == 1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_debug_output_path);
		sprintf(buff + strlen(buff), "/GPU_cache_debug.out");
		GPU_cache_debug_file = fopen (buff, "w+");
	}

	if(switch_debug == 1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_debug_output_path);
		sprintf(buff + strlen(buff), "/switch_debug.out");
		switch_debug_file = fopen (buff, "w+");
	}

	if(hub_iommu_debug == 1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_debug_output_path);
		sprintf(buff + strlen(buff), "/hub_iommu_debug.out");
		hub_iommu_debug_file = fopen (buff, "w+");
	}

	if(sysagent_debug ==1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_debug_output_path);
		sprintf(buff + strlen(buff), "/sysagent_debug.out");
		sysagent_debug_file = fopen (buff, "w+");
	}

	if(memctrl_debug ==1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_debug_output_path);
		sprintf(buff + strlen(buff), "/memctrl_debug.out");
		memctrl_debug_file = fopen (buff, "w+");
	}

	if(protocol_debug ==1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_debug_output_path);
		sprintf(buff + strlen(buff), "/protocol_debug.out");
		protocol_debug_file = fopen (buff, "w+");
	}

	if(mshr_debug ==1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_debug_output_path);
		sprintf(buff + strlen(buff), "/mshr_debug.out");
		mshr_debug_file = fopen (buff, "w+");
	}

	if(ort_debug ==1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_debug_output_path);
		sprintf(buff + strlen(buff), "/ort_debug.out");
		ort_debug_file = fopen (buff, "w+");
	}

	if(load_store_debug ==1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_debug_output_path);
		sprintf(buff + strlen(buff), "/load_store_debug.out");
		load_store_log_file = fopen (buff, "w+");
	}

	return 1;
}

int stats_read_config(void* user, const char* section, const char* name, const char* value){

	if(MATCH("Stats", "CGM_Stats"))
	{
		cgm_stats = atoi(value);
	}

	if(MATCH("Stats", "MEM_Trace"))
	{
		mem_trace = atoi(value);
	}

	if(MATCH("Stats", "Path"))
	{
		cgm_stats_output_path = strdup(value);
	}

	if(MATCH("Stats", "File_Name"))
	{
		cgm_stats_file_name = strdup(value);
	}

	return 1;
}

int stats_finish_create(void){

	char buff[250];
	int num_cores = x86_cpu_num_cores;

	if (cgm_stats == 1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_stats_output_path);
		/*sprintf(buff + strlen(buff), "%s_p%d", cgm_stats_file_name, num_cores);*/
		sprintf(buff + strlen(buff), "%s", cgm_stats_file_name);
		cgm_stats_file = fopen (buff, "w+");
	}

	if (mem_trace == 1)
	{
		memset (buff,'\0' , 250);
		sprintf(buff, "%s", cgm_stats_output_path);
		sprintf(buff + strlen(buff), "/mem_trace.out");
		mem_trace_file = fopen (buff, "w+");
	}

	return 1;
}

int cache_read_config(void* user, const char* section, const char* name, const char* value){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	int i = 0;

	int Sliced = 0;
	int Sets = 0;
	/*int Qty = 0;*/
	int Assoc = 0;
	int BlockSize = 0;
	int Latency = 0;
	char *Policy = 0;
	int Ports = 0;
	int MSHR = 0;
	int DirectoryLatency = 0;
	int WireLatency = 0;
	int maxcoal = 0;
	int Bus_width = 0;
	char *temp_strn;

	////////////////////////
	//MISC Checks
	////////////////////////

	if( num_cus % 4 != 0)
	{
		fatal("cache_read_config(): invalid number of compute units set for GPU, must be divisible by 4\n");
	}


	////////////////////////
	//MISC Settings
	////////////////////////

	/*get max queue size*/
	if(MATCH("Queue", "Size"))
	{
		QueueSize = atoi(value);
	}

	if(MATCH("Debug", "MEM_SYSTEM_OFF"))
	{
		mem_system_off = atoi(value);
	}

	if(MATCH("Debug", "L1_I_INF"))
	{
		l1_i_inf = atoi(value);
	}

	if(MATCH("Debug", "L1_D_INF"))
	{
		l1_d_inf = atoi(value);
	}

	if(MATCH("Debug", "L2_INF"))
	{
		l2_inf = atoi(value);
	}

	if(MATCH("Debug", "L3_INF"))
	{
		l3_inf = atoi(value);
	}

	if(MATCH("Debug", "L1_I_MISS"))
	{
		l1_i_miss = atoi(value);
	}

	if(MATCH("Debug", "L1_D_MISS"))
	{
		l1_d_miss = atoi(value);
	}

	if(MATCH("Debug", "L2_MISS"))
	{
		l2_miss = atoi(value);
	}

	if(MATCH("Debug", "L3_MISS"))
	{
		l3_miss = atoi(value);
	}

	if(MATCH("Debug", "GPU_L1_INF"))
	{
		gpu_l1_inf = atoi(value);
	}

	if(MATCH("Debug", "GPU_L2_INF"))
	{
		gpu_l2_inf = atoi(value);
	}

	////////////////////////
	//Protocol
	////////////////////////
	if(MATCH("Protocol", "CPU_Protocol_type"))
	{
		temp_strn = strdup(value);

		if(strcmp(temp_strn, "BT") == 0)
		{
			cgm_cache_protocol = cgm_protocol_bt;
			printf("---CPU memory protocol is BT---\n");
		}
		else if(strcmp(temp_strn, "MESI") == 0)
		{
			cgm_cache_protocol = cgm_protocol_mesi;
			printf("---CPU memory protocol is MESI---\n");
		}
		else if(strcmp(temp_strn, "MOESI") == 0)
		{
			fatal("cache_read_config(): MOESI protocol not yet supported, check config file\n");
		}
		else
		{
			fatal("cache_read_config(): invalid protocol, check config file\n");
		}
	}

	if(MATCH("Protocol", "GPU_Protocol_type"))
	{
		temp_strn = strdup(value);

		if(strcmp(temp_strn, "NC") == 0)
		{
			cgm_gpu_cache_protocol = cgm_protocol_non_coherent;
			printf("---GPU memory protocol is NC---\n");
		}
		else if(strcmp(temp_strn, "MESI") == 0)
		{
			cgm_gpu_cache_protocol = cgm_protocol_mesi;
			printf("---GPU memory protocol is MESI---\n");
		}
		else if(strcmp(temp_strn, "GMESI") == 0)
		{
			fatal("cache_read_config(): GMESI protocol not yet supported, check config file\n");
		}
		else
		{
			fatal("cache_read_config(): invalid protocol, check config file\n");
		}
	}

	if(MATCH("Protocol", "GPU_Connection_type"))
	{
		temp_strn = strdup(value);

		if(strcmp(temp_strn, "MC") == 0)
		{
			hub_iommu_connection_type = hub_to_mc;

			if(cgm_gpu_cache_protocol != cgm_protocol_non_coherent)
			{
				fatal("cache_read_config(): invalid gpu nc protocol configuration\n");
			}
		}
		else if(strcmp(temp_strn, "L3") == 0)
		{
			hub_iommu_connection_type = hub_to_l3;

			if (!(cgm_gpu_cache_protocol != cgm_protocol_non_coherent || cgm_gpu_cache_protocol != cgm_protocol_mesi))
			{
				fatal("cache_read_config(): invalid gpu mesi protocol configuration\n");
			}
		}
		else
		{
			fatal("cache_read_config(): invalid gpu connection, check config file\n");
		}
	}


	////////////////////////
	//l1_d_caches
	////////////////////////

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
			if(strcmp(Policy, "LRU") == 0)
			{
				l1_d_caches[i].policy = cache_policy_lru;
			}
			else if(strcmp(Policy, "FA") == 0)
			{
				l1_d_caches[i].policy = cache_policy_first_available;
			}
			else
			{
				fatal("cache_read_config(): invalid cache policy, check config file\n");
			}
		}
	}

	if(MATCH("CPU_L1_D_Cache", "MaxCoalesce"))
	{
		maxcoal = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].max_coal = maxcoal;
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

	if(MATCH("Bus", "CPUL2-CPUL1"))
	{
		Bus_width = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_d_caches[i].bus_width = Bus_width;

			if(l1_d_caches[i].bus_width == 0)
			{
				fatal("cache_read_config(): d cache bus width is out of bounds %d\n", l1_d_caches[i].bus_width);
			}
		}
	}

	////////////////////////
	//l1_i_caches
	////////////////////////

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

	/*if(MATCH("CPU_L1_I_Cache", "Policy"))
	{
		Policy = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_i_caches[i].policy_type = Policy;
		}
	}*/

	if(MATCH("CPU_L1_I_Cache", "Policy"))
	{
		Policy = strdup(value);
		for (i = 0; i < num_cores; i++)
		{
			if(strcmp(Policy, "LRU") == 0)
			{
				l1_i_caches[i].policy = cache_policy_lru;
			}
			else if(strcmp(Policy, "FA") == 0)
			{
				l1_i_caches[i].policy = cache_policy_first_available;
			}
			else
			{
				fatal("cache_read_config(): invalid cache policy, check config file\n");
			}
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

	if(MATCH("CPU_L1_I_Cache", "MaxCoalesce"))
	{
		maxcoal = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_i_caches[i].max_coal = maxcoal;
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
			l1_i_caches[i].wire_latency = WireLatency;
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

	if(MATCH("Bus", "CPUL2-CPUL1"))
	{
		Bus_width = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l1_i_caches[i].bus_width = Bus_width;

			if(l1_i_caches[i].bus_width == 0)
			{
				fatal("cache_read_config(): i cache bus width is out of bounds %d\n", l1_i_caches[i].bus_width);
			}
		}
	}

	////////////////////////
	//l2_caches
	////////////////////////

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
			if(strcmp(Policy, "LRU") == 0)
			{
				l2_caches[i].policy = cache_policy_lru;
			}
			else if(strcmp(Policy, "FA") == 0)
			{
				l2_caches[i].policy = cache_policy_first_available;
			}
			else
			{
				fatal("cache_read_config(): invalid cache policy, check config file\n");
			}
		}
	}


	/*if(MATCH("CPU_L2_Cache", "Policy"))
	{
		Policy = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].policy_type = Policy;
		}
	}*/

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

	if(MATCH("CPU_L2_Cache", "MaxCoalesce"))
	{
		maxcoal = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].max_coal = maxcoal;
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

	if(MATCH("Bus", "Switches"))
	{
		Bus_width = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l2_caches[i].bus_width = Bus_width;

			if(l2_caches[i].bus_width == 0)
			{
				fatal("cache_read_config(): l2 cache bus width is out of bounds %d\n", l2_caches[i].bus_width);
			}
		}
	}

	////////////////////////
	//l3_caches
	////////////////////////

	/*configure CPU L3 caches*/
	if(MATCH("CPU_L3_Cache", "Sliced"))
	{
		Sliced = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].slice_type = Sliced;
		}
	}


	/*configure CPU L3 caches
	if(MATCH("CPU_L3_Cache", "Slices"))
	{
		Slices = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].num_slices = Slices;
		}
	}*/


	if(MATCH("CPU_L3_Cache", "Sets"))
	{
		Sets = atoi(value);
		//int slice_size = (Sets / l3_caches[i].num_slices);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].num_sets = Sets;
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
			if(strcmp(Policy, "LRU") == 0)
			{
				l3_caches[i].policy = cache_policy_lru;
			}
			else if(strcmp(Policy, "FA") == 0)
			{
				l3_caches[i].policy = cache_policy_first_available;
			}
			else
			{
				fatal("cache_read_config(): invalid cache policy, check config file\n");
			}
		}
	}

	/*if(MATCH("CPU_L3_Cache", "Policy"))
	{
		Policy = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].policy_type = Policy;
		}
	}*/

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

	if(MATCH("CPU_L3_Cache", "MaxCoalesce"))
	{
		maxcoal = atoi(value);
		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].max_coal = maxcoal;
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

	if(MATCH("Bus", "Switches"))
	{
		Bus_width = atoi(value);

		for (i = 0; i < num_cores; i++)
		{
			l3_caches[i].bus_width = Bus_width;

			if(l3_caches[i].bus_width == 0)
			{
				fatal("cache_read_config(): l3 cache bus width is out of bounds %d\n",l3_caches[i].bus_width);
			}
		}
	}


	////////////
	//GPU Caches
	////////////


	////////////////////////
	//gpu_s_caches
	////////////////////////

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

/*	if(MATCH("GPU_S_Cache", "Policy"))
	{
		Policy = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_s_caches[i].policy_type = Policy;
		}
	}*/

	if(MATCH("GPU_S_Cache", "Policy"))
	{
		Policy = strdup(value);
		for (i = 0; i < num_cus; i++)
		{
			if(strcmp(Policy, "LRU") == 0)
			{
				gpu_s_caches[i].policy = cache_policy_lru;
			}
			else if(strcmp(Policy, "FA") == 0)
			{
				gpu_s_caches[i].policy = cache_policy_first_available;
			}
			else
			{
				fatal("cache_read_config(): invalid cache policy, check config file\n");
			}
		}
	}


	if(MATCH("GPU_S_Cache", "MSHR"))
	{
		MSHR = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_s_caches[i].mshr_size = MSHR;
		}
	}

/*	if(MATCH("GPU_S_Cache", "DirectoryLatency"))
	{
		DirectoryLatency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_s_caches[i].directory_latency = DirectoryLatency;
		}
	}*/

	if(MATCH("GPU_S_Cache", "MaxCoalesce"))
	{
		maxcoal = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_s_caches[i].max_coal = maxcoal;
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

	if(MATCH("GPU_S_Cache", "WireLatency"))
	{
		WireLatency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_s_caches[i].wire_latency = WireLatency;
		}
	}


	if(MATCH("Bus", "GPUL2-GPUL1"))
	{
		Bus_width = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_s_caches[i].bus_width = Bus_width;

			if(gpu_s_caches[i].bus_width == 0)
			{
				fatal("cache_read_config(): s cache bus width is out of bounds %d cache bus width %d \n", Bus_width, gpu_s_caches[i].bus_width);
			}

		}
	}

	////////////////////////
	//gpu_v_caches
	////////////////////////

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

	if(MATCH("GPU_V_Cache", "Policy"))
	{
		Policy = strdup(value);
		for (i = 0; i < num_cus; i++)
		{
			if(strcmp(Policy, "LRU") == 0)
			{
				gpu_v_caches[i].policy = cache_policy_lru;
			}
			else if(strcmp(Policy, "FA") == 0)
			{
				gpu_v_caches[i].policy = cache_policy_first_available;
			}
			else
			{
				fatal("cache_read_config(): invalid cache policy, check config file\n");
			}
		}
	}

	/*if(MATCH("GPU_V_Cache", "Policy"))
	{
		Policy = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_v_caches[i].policy_type = Policy;
		}
	}*/

	if(MATCH("GPU_V_Cache", "MSHR"))
	{
		MSHR = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_v_caches[i].mshr_size = MSHR;
		}
	}

/*	if(MATCH("GPU_V_Cache", "DirectoryLatency"))
	{
		DirectoryLatency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_v_caches[i].directory_latency = DirectoryLatency;
		}
	}*/

	if(MATCH("GPU_V_Cache", "MaxCoalesce"))
	{
		maxcoal = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_v_caches[i].max_coal = maxcoal;
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

	if(MATCH("GPU_V_Cache", "WireLatency"))
	{
		WireLatency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_v_caches[i].wire_latency = WireLatency;
		}
	}



	if(MATCH("Bus", "GPUL2-GPUL1"))
	{
		Bus_width = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_v_caches[i].bus_width = Bus_width;

			if(gpu_v_caches[i].bus_width == 0)
			{
				fatal("cache_read_config(): v cache bus width is out of bounds %d\n", Bus_width);
			}
		}
	}


	////////////////////////
	//gpu_l2_caches
	////////////////////////


	/*configure CPU L2 caches*/
	/*if(MATCH("GPU_L2_Cache", "Qty"))
	{
		gpu_l2_qty = atoi(value);
	}*/

	/*configure GPU L2 caches*/
	if(MATCH("GPU_L2_Cache", "Sets"))
	{
		Sets = atoi(value);
		for (i = 0; i < gpu_group_cache_num; i++)
		{
			gpu_l2_caches[i].num_sets = Sets;
		}
	}

	if(MATCH("GPU_L2_Cache", "Assoc"))
	{
		Assoc = atoi(value);
		for (i = 0; i < gpu_group_cache_num; i++)
		{
			gpu_l2_caches[i].assoc = Assoc;
		}
	}

	if(MATCH("GPU_L2_Cache", "BlockSize"))
	{
		BlockSize = atoi(value);
		for (i = 0; i < gpu_group_cache_num; i++)
		{
			gpu_l2_caches[i].block_size = BlockSize;
		}
	}

	if(MATCH("GPU_L2_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < gpu_group_cache_num; i++)
		{
			gpu_l2_caches[i].latency = Latency;
		}
	}

	if(MATCH("GPU_L2_Cache", "Policy"))
	{
		Policy = strdup(value);
		for (i = 0; i < gpu_group_cache_num; i++)
		{
			if(strcmp(Policy, "LRU") == 0)
			{
				gpu_l2_caches[i].policy = cache_policy_lru;
			}
			else if(strcmp(Policy, "FA") == 0)
			{
				gpu_l2_caches[i].policy = cache_policy_first_available;
			}
			else
			{
				fatal("cache_read_config(): invalid cache policy, check config file\n");
			}
		}
	}


	/*if(MATCH("GPU_L2_Cache", "Policy"))
	{
		Policy = atoi(value);
		for (i = 0; i < gpu_group_cache_num; i++)
		{
			gpu_l2_caches[i].policy_type = Policy;
		}
	}*/

	if(MATCH("GPU_L2_Cache", "MSHR"))
	{
		MSHR = atoi(value);
		for (i = 0; i < gpu_group_cache_num; i++)
		{
			gpu_l2_caches[i].mshr_size = MSHR;
		}
	}

	if(MATCH("GPU_L2_Cache", "DirectoryLatency"))
	{
		DirectoryLatency = atoi(value);
		for (i = 0; i < gpu_group_cache_num; i++)
		{
			gpu_l2_caches[i].directory_latency = DirectoryLatency;
		}
	}

	if(MATCH("GPU_L2_Cache", "MaxCoalesce"))
	{
		maxcoal = atoi(value);
		for (i = 0; i < gpu_group_cache_num; i++)
		{
			gpu_l2_caches[i].max_coal = maxcoal;
		}
	}

	if(MATCH("GPU_L2_Cache", "WireLatency"))
	{
		WireLatency = atoi(value);
		for (i = 0; i < gpu_group_cache_num; i++)
		{
			gpu_l2_caches[i].wire_latency = WireLatency;
		}
	}

	if(MATCH("Bus", "Switches"))
	{
		Bus_width = atoi(value);
		for (i = 0; i < gpu_group_cache_num; i++)
		{
			gpu_l2_caches[i].bus_width = Bus_width;

			if(gpu_l2_caches[i].bus_width == 0)
			{
				fatal("cache_read_config(): gpu l2 cache bus width is out of bounds %d\n", gpu_l2_caches[i].bus_width);
			}
		}
	}


	////////////////////////
	//gpu LDS
	////////////////////////


	if(MATCH("GPU_L2_Cache", "Latency"))
	{
		Latency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_lds_units[i].latency = Latency;
		}
	}

	if(MATCH("GPU_L2_Cache", "WireLatency"))
	{
		WireLatency = atoi(value);
		for (i = 0; i < num_cus; i++)
		{
			gpu_lds_units[i].wire_latency = WireLatency;
		}
	}


	return 0;
}

int cache_finish_create(){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	struct cache_block_t *block;
	int i = 0, j = 0, k = 0, l = 0, set = 0, way = 0;
	char buff[100];

	//finish creating the CPU caches
	for(i = 0; i < num_cores ; i++ )
	{
		/////////////
		//L1 I Cache
		/////////////

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d]", i);
		l1_i_caches[i].name = strdup(buff);
		l1_i_caches[i].id = i;
		l1_i_caches[i].cache_type = l1_i_cache_t;
		l1_i_caches[i].log_block_size = LOG2(l1_i_caches[i].block_size);
		l1_i_caches[i].log_set_size = LOG2(l1_i_caches[i].num_sets);
		l1_i_caches[i].block_mask = l1_i_caches[i].block_size - 1;
		l1_i_caches[i].set_mask = l1_i_caches[i].num_sets - 1;
		l1_i_caches[i].block_address_mask = (unsigned int) 0xFFFFFFFF ^ l1_i_caches[i].block_mask;

		if(!l1_i_caches[i].policy)
		{
			fatal("cache_finish_create(): Invalid cache policy\n");
		}

		//pointer to my own event count
		l1_i_caches[i].ec_ptr = &l1_i_cache[i];

		l1_i_caches[i].Rx_queue_top = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].Rx_queue_top", i);
		l1_i_caches[i].Rx_queue_top->name = strdup(buff);

		l1_i_caches[i].Rx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].Rx_queue_bottom", i);
		l1_i_caches[i].Rx_queue_bottom->name = strdup(buff);

		l1_i_caches[i].next_queue = l1_i_caches[i].Rx_queue_top;

		l1_i_caches[i].retry_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].retry_queue", i);
		l1_i_caches[i].retry_queue->name = strdup(buff);

		l1_i_caches[i].ort_list = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].ort_list", i);
		l1_i_caches[i].ort_list->name = strdup(buff);

		l1_i_caches[i].ort = (int **)malloc(l1_i_caches[i].mshr_size * sizeof(int *));
		for (j = 0; j < l1_i_caches[i].mshr_size; j++)
		{
			l1_i_caches[i].ort[j] = (int *)malloc(3 * sizeof(int));
		}

		//init ort
		for (j = 0; j <  l1_i_caches[i].mshr_size; j++)
		{
			for (k = 0; k < 3; k++)
			{
				l1_i_caches[i].ort[j][k] = -1;
			}
		}

		//Tx queue
		l1_i_caches[i].Tx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].Tx_queue_bottom", i);
		l1_i_caches[i].Tx_queue_bottom->name = strdup(buff);

		//WB
		l1_i_caches[i].write_back_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].write_back_buffer", i);
		l1_i_caches[i].write_back_buffer->name = strdup(buff);

		//Pending requests
		l1_i_caches[i].pending_request_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].pending_request_buffer", i);
		l1_i_caches[i].pending_request_buffer->name = strdup(buff);

		//Coherence queues
		l1_i_caches[i].Coherance_Rx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].Coherance_Rx_queue", i);
		l1_i_caches[i].Coherance_Rx_queue->name = strdup(buff);

		l1_i_caches[i].Coherance_Tx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_caches[%d].Coherance_Tx_queue", i);
		l1_i_caches[i].Coherance_Tx_queue->name = strdup(buff);

		//io ctrl
		l1_i_caches[i].cache_io_down_ec = (void *) calloc((1), sizeof(eventcount));
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_ec");
		l1_i_caches[i].cache_io_down_ec = new_eventcount(strdup(buff));

		//io tasks
		l1_i_caches[i].cache_io_down_tasks = (void *) calloc((1), sizeof(task));
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_task");
		l1_i_caches[i].cache_io_down_tasks = create_task(l1_i_cache_down_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

		/*link cache virtual functions*/
		if(cgm_cache_protocol == cgm_protocol_bt)
		{
			l1_i_caches[i].l1_i_fetch = cgm_bt_fetch;
			l1_i_caches[i].l1_i_write_block = cgm_bt_l1_i_write_block;
		}
		else if(cgm_cache_protocol == cgm_protocol_mesi)
		{
			l1_i_caches[i].l1_i_fetch = cgm_mesi_fetch;
			l1_i_caches[i].l1_i_write_block = cgm_mesi_l1_i_write_block;
		}
		else
		{
			fatal("invalid protocol at i cache init\n");
		}

		//stats
		l1_i_caches[i].fetches = 0;
		l1_i_caches[i].stores = 0;
		l1_i_caches[i].hits = 0;
		l1_i_caches[i].invalid_hits = 0;
		l1_i_caches[i].misses = 0;
		l1_i_caches[i].assoc_conflict = 0;
		l1_i_caches[i].upgrade_misses = 0;
		l1_i_caches[i].retries = 0;
		l1_i_caches[i].coalesces = 0;
		l1_i_caches[i].mshr_entries = 0;
		l1_i_caches[i].stalls = 0;


		/////////////
		//L1 D Cache
		/////////////

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d]", i);
		l1_d_caches[i].name = strdup(buff);
		l1_d_caches[i].id = i;
		l1_d_caches[i].cache_type = l1_d_cache_t;
		l1_d_caches[i].log_block_size = LOG2(l1_d_caches[i].block_size);
		l1_d_caches[i].log_set_size = LOG2(l1_d_caches[i].num_sets);
		l1_d_caches[i].block_mask = l1_d_caches[i].block_size - 1;
		l1_d_caches[i].set_mask = l1_d_caches[i].num_sets - 1;
		l1_d_caches[i].block_address_mask = (unsigned int) 0xFFFFFFFF ^ l1_d_caches[i].block_mask;


		if(!l1_d_caches[i].policy)
		{
			fatal("cache_finish_create(): Invalid cache policy\n");
		}

		//pointer to my own event count
		l1_d_caches[i].ec_ptr = &l1_d_cache[i];

		l1_d_caches[i].Rx_queue_top = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].Rx_queue_top", i);
		l1_d_caches[i].Rx_queue_top->name = strdup(buff);

		l1_d_caches[i].Rx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].Rx_queue_bottom", i);
		l1_d_caches[i].Rx_queue_bottom->name = strdup(buff);

		l1_d_caches[i].next_queue = l1_d_caches[i].Rx_queue_top;

		l1_d_caches[i].retry_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].retry_queue", i);
		l1_d_caches[i].retry_queue->name = strdup(buff);

		l1_d_caches[i].ort_list = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].ort_list", i);
		l1_d_caches[i].ort_list->name = strdup(buff);

		l1_d_caches[i].ort = (int **)malloc(l1_d_caches[i].mshr_size * sizeof(int *));
		for (j = 0; j < l1_d_caches[i].mshr_size; j++)
		{
			l1_d_caches[i].ort[j] = (int *)malloc(3 * sizeof(int));
		}

		//init ort
		for (j = 0; j <  l1_d_caches[i].mshr_size; j++)
		{
			for (k = 0; k < 3; k++)
			{
				l1_d_caches[i].ort[j][k] = -1;
			}
		}

		//Tx queue
		l1_d_caches[i].Tx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].Tx_queue_bottom", i);
		l1_d_caches[i].Tx_queue_bottom->name = strdup(buff);

		//coherance queues
		l1_d_caches[i].Coherance_Rx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].Coherance_Rx_queue", i);
		l1_d_caches[i].Coherance_Rx_queue->name = strdup(buff);

		l1_d_caches[i].Coherance_Tx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].Coherance_Tx_queue", i);
		l1_d_caches[i].Coherance_Tx_queue->name = strdup(buff);

		//WB
		l1_d_caches[i].write_back_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].write_back_buffer", i);
		l1_d_caches[i].write_back_buffer->name = strdup(buff);

		//Pending Request Buffer
		l1_d_caches[i].pending_request_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_caches[%d].pending_request_buffer", i);
		l1_d_caches[i].pending_request_buffer->name = strdup(buff);

		//io ctrl
		l1_d_caches[i].cache_io_down_ec = (void *) calloc((1), sizeof(eventcount));
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_ec");
		l1_d_caches[i].cache_io_down_ec = new_eventcount(strdup(buff));

		//io tasks
		l1_d_caches[i].cache_io_down_tasks = (void *) calloc((1), sizeof(task));
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_task");
		l1_d_caches[i].cache_io_down_tasks = create_task(l1_d_cache_down_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

		//watch dog
		/*l1_d_caches[i].outstanding_addresses = (unsigned int *) calloc(l1_d_caches[i].mshr_size, sizeof(task));*/

		/*link cache virtual functions*/
		if(cgm_cache_protocol == cgm_protocol_bt)
		{
			l1_d_caches[i].l1_d_load = cgm_bt_load;
			l1_d_caches[i].l1_d_store = cgm_bt_store;
			l1_d_caches[i].l1_d_write_block = cgm_bt_l1_d_write_block;
			l1_d_caches[i].l1_d_downgrade = cgm_bt_l1_d_downgrade;
			l1_d_caches[i].l1_d_getx_fwd_inval = cgm_bt_l1_d_getx_fwd_inval;
			l1_d_caches[i].l1_d_write_back = cgm_bt_l1_d_write_back;
			l1_d_caches[i].l1_d_inval = cgm_bt_l1_d_inval;
		}
		else if(cgm_cache_protocol == cgm_protocol_mesi)
		{
			l1_d_caches[i].l1_d_load = cgm_mesi_load;
			l1_d_caches[i].l1_d_store = cgm_mesi_store;
			l1_d_caches[i].l1_d_write_block = cgm_mesi_l1_d_write_block;
			l1_d_caches[i].l1_d_downgrade = cgm_mesi_l1_d_downgrade;
			l1_d_caches[i].l1_d_getx_fwd_inval = cgm_mesi_l1_d_getx_fwd_inval;
			l1_d_caches[i].l1_d_upgrade_inval = cgm_mesi_l1_d_upgrade_inval;
			l1_d_caches[i].l1_d_upgrade_ack = cgm_mesi_l1_d_upgrade_ack;
			l1_d_caches[i].l1_d_write_back = cgm_mesi_l1_d_write_back;
			l1_d_caches[i].l1_d_inval = cgm_mesi_l1_d_inval;
		}
		else
		{
			fatal("invalid protocol at d cache init\n");
		}

		//stats
		l1_d_caches[i].fetches = 0;
		l1_d_caches[i].loads = 0;
		l1_d_caches[i].stores = 0;
		l1_d_caches[i].hits = 0;
		l1_d_caches[i].invalid_hits = 0;
		l1_d_caches[i].misses = 0;
		l1_d_caches[i].assoc_conflict = 0;
		l1_d_caches[i].upgrade_misses = 0;
		l1_d_caches[i].retries = 0;
		l1_d_caches[i].coalesces = 0;
		l1_d_caches[i].mshr_entries = 0;
		l1_d_caches[i].stalls = 0;

		/////////////
		//L2 Cache
		/////////////

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l2_caches[%d]", i);
		l2_caches[i].name = strdup(buff);
		l2_caches[i].id = i;
		l2_caches[i].cache_type = l2_cache_t;
		l2_caches[i].log_block_size = LOG2(l2_caches[i].block_size);
		l2_caches[i].log_set_size = LOG2(l2_caches[i].num_sets);
		l2_caches[i].block_mask = l2_caches[i].block_size - 1;
		l2_caches[i].set_mask = l2_caches[i].num_sets - 1;
		l2_caches[i].block_address_mask = (unsigned int) 0xFFFFFFFF ^ l2_caches[i].block_mask;

		if(!l2_caches[i].policy)
		{
			fatal("cache_finish_create(): Invalid cache policy\n");
		}

		//pointer to my own event count
		l2_caches[i].ec_ptr = &l2_cache[i];

		l2_caches[i].Rx_queue_top = list_create();
		memset (buff, '\0', sizeof(buff));
		snprintf(buff,100, "l2_caches[%d].Rx_queue_top", i);
		l2_caches[i].Rx_queue_top->name = strdup(buff);

		l2_caches[i].Rx_queue_bottom = list_create();
		memset (buff, '\0', sizeof(buff));
		snprintf(buff,100, "l2_caches[%d].Rx_queue_bottom", i);
		l2_caches[i].Rx_queue_bottom->name = strdup(buff);

		l2_caches[i].next_queue = l2_caches[i].Rx_queue_top;

		l2_caches[i].retry_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l2_caches[%d].retry_queue", i);
		l2_caches[i].retry_queue->name = strdup(buff);

		l2_caches[i].ort_list = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l2_caches[%d].ort_list", i);
		l2_caches[i].ort_list->name = strdup(buff);

		l2_caches[i].ort = (int **)malloc(l2_caches[i].mshr_size * sizeof(int *));
		for (j = 0; j < l2_caches[i].mshr_size; j++)
		{
			l2_caches[i].ort[j] = (int *)malloc(3 * sizeof(int));
		}

		//init ort
		for (j = 0; j <  l2_caches[i].mshr_size; j++)
		{
			for (k = 0; k < 3; k++)
			{
				l2_caches[i].ort[j][k] = -1;
			}
		}

		//Tx queues
		l2_caches[i].Tx_queue_top = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l2_caches[%d].Tx_queue_top", i);
		l2_caches[i].Tx_queue_top->name = strdup(buff);

		l2_caches[i].Tx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l2_caches[%d].Tx_queue_bottom", i);
		l2_caches[i].Tx_queue_bottom->name = strdup(buff);

		//coherance queues
		l2_caches[i].Coherance_Rx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l2_caches[%d].Coherance_Rx_queue", i);
		l2_caches[i].Coherance_Rx_queue->name = strdup(buff);

		l2_caches[i].Coherance_Tx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l2_caches[%d].Coherance_Tx_queue", i);
		l2_caches[i].Coherance_Tx_queue->name = strdup(buff);

		//WB
		l2_caches[i].write_back_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l2_caches[%d].write_back_buffer", i);
		l2_caches[i].write_back_buffer->name = strdup(buff);

		//Pending Request Buffer
		l2_caches[i].pending_request_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l2_caches[%d].pending_request_buffer", i);
		l2_caches[i].pending_request_buffer->name = strdup(buff);

		//io ctrl
		l2_caches[i].cache_io_up_ec = (void *) calloc((1), sizeof(eventcount));
		l2_caches[i].cache_io_down_ec = (void *) calloc((1), sizeof(eventcount));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_up_ec");
		l2_caches[i].cache_io_up_ec = new_eventcount(strdup(buff));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_ec");
		l2_caches[i].cache_io_down_ec = new_eventcount(strdup(buff));

		//io tasks
		l2_caches[i].cache_io_up_tasks = (void *) calloc((1), sizeof(task));
		l2_caches[i].cache_io_down_tasks = (void *) calloc((1), sizeof(task));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_up_task");
		l2_caches[i].cache_io_up_tasks = create_task(l2_cache_up_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_task");
		l2_caches[i].cache_io_down_tasks = create_task(l2_cache_down_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

		/*link cache virtual functions*/
		if(cgm_cache_protocol == cgm_protocol_bt)
		{
			l2_caches[i].l2_gets = cgm_bt_l2_gets;
			l2_caches[i].l2_get = cgm_bt_l2_get;
			l2_caches[i].l2_get_nack = cgm_bt_l2_get_nack;
			l2_caches[i].l2_getx = cgm_bt_l2_getx;
			l2_caches[i].l2_getx_nack = cgm_bt_l2_getx_nack;
			l2_caches[i].l2_downgrade_ack = cgm_bt_l2_downgrade_ack;
			l2_caches[i].l2_get_fwd = cgm_bt_l2_get_fwd;
			l2_caches[i].l2_getx_fwd = cgm_bt_l2_getx_fwd;
			l2_caches[i].l2_getx_fwd_inval_ack = cgm_bt_l2_getx_fwd_inval_ack;
			l2_caches[i].l2_inval_ack = cgm_bt_l2_inval_ack;
			l2_caches[i].l2_write_block = cgm_bt_l2_write_block;
			l2_caches[i].l2_write_back = cgm_bt_l2_write_back;
			l2_caches[i].l2_inval = cgm_bt_l2_inval;
			l2_caches[i].l2_inval_ack = cgm_bt_l2_inval_ack;
		}
		else if(cgm_cache_protocol == cgm_protocol_mesi)
		{
			l2_caches[i].l2_gets = cgm_mesi_l2_gets;
			l2_caches[i].l2_get = cgm_mesi_l2_get;
			l2_caches[i].l2_get_nack = cgm_mesi_l2_get_nack;
			l2_caches[i].l2_getx = cgm_mesi_l2_getx;
			l2_caches[i].l2_getx_nack = cgm_mesi_l2_getx_nack;
			l2_caches[i].l2_downgrade_ack = cgm_mesi_l2_downgrade_ack;
			l2_caches[i].l2_get_fwd = cgm_mesi_l2_get_fwd;
			l2_caches[i].l2_getx_fwd = cgm_mesi_l2_getx_fwd;
			l2_caches[i].l2_getx_fwd_inval_ack = cgm_mesi_l2_getx_fwd_inval_ack;
			l2_caches[i].l2_upgrade = cgm_mesi_l2_upgrade;
			l2_caches[i].l2_upgrade_ack = cgm_mesi_l2_upgrade_ack;
			l2_caches[i].l2_upgrade_nack = cgm_mesi_l2_upgrade_nack;
			l2_caches[i].l2_upgrade_putx_n = cgm_mesi_l2_upgrade_putx_n;
			l2_caches[i].l2_upgrade_inval = cgm_mesi_l2_upgrade_inval;
			l2_caches[i].l2_inval_ack = cgm_mesi_l2_inval_ack;
			l2_caches[i].l2_write_block = cgm_mesi_l2_write_block;
			l2_caches[i].l2_write_back = cgm_mesi_l2_write_back;
			l2_caches[i].l2_inval = cgm_mesi_l2_inval;
			l2_caches[i].l2_inval_ack = cgm_mesi_l2_inval_ack;
		}
		else
		{
			fatal("invalid protocol at l2 cache init\n");
		}

		//stats
		l2_caches[i].fetches = 0;
		l2_caches[i].loads = 0;
		l2_caches[i].stores = 0;
		l2_caches[i].hits = 0;
		l2_caches[i].invalid_hits = 0;
		l2_caches[i].misses = 0;
		l2_caches[i].assoc_conflict = 0;
		l2_caches[i].upgrade_misses = 0;
		l2_caches[i].retries = 0;
		l2_caches[i].coalesces = 0;
		l2_caches[i].mshr_entries = 0;
		l2_caches[i].stalls = 0;

		/////////////
		//L3 Cache
		/////////////

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d]", i);
		l3_caches[i].name = strdup(buff);
		l3_caches[i].id = i;
		l3_caches[i].cache_type = l3_cache_t;
		l3_caches[i].log_block_size = LOG2(l3_caches[i].block_size);
		l3_caches[i].log_set_size = LOG2(l3_caches[i].num_sets);
		l3_caches[i].block_mask = l3_caches[i].block_size - 1;
		l3_caches[i].set_mask = l3_caches[i].num_sets - 1;
		l3_caches[i].block_address_mask = (unsigned int) 0xFFFFFFFF ^ l3_caches[i].block_mask;


		//build the share_mask
		for(l = 0; l < num_cores; l ++)
		{
			l3_caches[i].share_mask++;

			if(l < (num_cores - 1))
				l3_caches[i].share_mask = l3_caches[i].share_mask << 1;
		}

		if(!l3_caches[i].policy)
		{
			fatal("cache_finish_create(): Invalid cache policy\n");
		}

		//pointer to my own event counter
		l3_caches[i].ec_ptr = &l3_cache[i];

		l3_caches[i].Rx_queue_top = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].Rx_queue_top", i);
		l3_caches[i].Rx_queue_top->name = strdup(buff);

		l3_caches[i].Rx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].Rx_queue_bottom", i);
		l3_caches[i].Rx_queue_bottom->name = strdup(buff);

		l3_caches[i].next_queue = l3_caches[i].Rx_queue_top;

		l3_caches[i].retry_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].retry_queue", i);
		l3_caches[i].retry_queue->name = strdup(buff);

		l3_caches[i].ort_list = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].ort_list", i);
		l3_caches[i].ort_list->name = strdup(buff);

		l3_caches[i].ort = (int **)malloc(l3_caches[i].mshr_size * sizeof(int *));
		for (j = 0; j < l3_caches[i].mshr_size; j++)
		{
			l3_caches[i].ort[j] = (int *)malloc(3 * sizeof(int));
		}

		//init ort
		for (j = 0; j <  l3_caches[i].mshr_size; j++)
		{
			for (k = 0; k < 3; k++)
			{
				l3_caches[i].ort[j][k] = -1;
			}
		}

		//Tx queues
		l3_caches[i].Tx_queue_top = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].Tx_queue_top", i);
		l3_caches[i].Tx_queue_top->name = strdup(buff);

		l3_caches[i].Tx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].Tx_queue_bottom", i);
		l3_caches[i].Tx_queue_bottom->name = strdup(buff);

		//coherance queues
		l3_caches[i].Coherance_Rx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].Coherance_Rx_queue", i);
		l3_caches[i].Coherance_Rx_queue->name = strdup(buff);

		l3_caches[i].Coherance_Tx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].Coherance_Tx_queue", i);
		l3_caches[i].Coherance_Tx_queue->name = strdup(buff);

		//WB
		l3_caches[i].write_back_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].write_back_buffer", i);
		l3_caches[i].write_back_buffer->name = strdup(buff);

		//pending request buffer
		l3_caches[i].pending_request_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "l3_caches[%d].pending_request_buffer", i);
		l3_caches[i].pending_request_buffer->name = strdup(buff);

		//io ctrl
		l3_caches[i].cache_io_up_ec = (void *) calloc((1), sizeof(eventcount));
		l3_caches[i].cache_io_down_ec = (void *) calloc((1), sizeof(eventcount));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_up_ec");
		l3_caches[i].cache_io_up_ec = new_eventcount(strdup(buff));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_ec");
		l3_caches[i].cache_io_down_ec = new_eventcount(strdup(buff));

		//io tasks
		l3_caches[i].cache_io_up_tasks = (void *) calloc((1), sizeof(task));
		l3_caches[i].cache_io_down_tasks = (void *) calloc((1), sizeof(task));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_up_task");
		l3_caches[i].cache_io_up_tasks = create_task(l3_cache_up_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_task");
		l3_caches[i].cache_io_down_tasks = create_task(l3_cache_down_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

		/*link cache virtual functions*/
		if(cgm_cache_protocol == cgm_protocol_bt)
		{
			l3_caches[i].l3_gets = cgm_bt_l3_gets;
			l3_caches[i].l3_get = cgm_bt_l3_get;
			l3_caches[i].l3_getx = cgm_bt_l3_getx;
			l3_caches[i].l3_downgrade_ack = cgm_bt_l3_downgrade_ack;
			l3_caches[i].l3_downgrade_nack = cgm_bt_l3_downgrade_nack;
			l3_caches[i].l3_getx_fwd_nack = cgm_bt_l3_getx_fwd_nack;
			l3_caches[i].l3_getx_fwd_ack = cgm_bt_l3_getx_fwd_ack;
			l3_caches[i].l3_write_block = cgm_bt_l3_write_block;
			l3_caches[i].l3_write_back = cgm_bt_l3_write_back;
		}
		else if(cgm_cache_protocol == cgm_protocol_mesi)
		{
			l3_caches[i].l3_gets = cgm_mesi_l3_gets;
			l3_caches[i].l3_get = cgm_mesi_l3_get;
			l3_caches[i].l3_getx = cgm_mesi_l3_getx;
			l3_caches[i].l3_downgrade_ack = cgm_mesi_l3_downgrade_ack;
			l3_caches[i].l3_downgrade_nack = cgm_mesi_l3_downgrade_nack;
			l3_caches[i].l3_getx_fwd_nack = cgm_mesi_l3_getx_fwd_nack;
			l3_caches[i].l3_getx_fwd_upgrade_nack = cgm_mesi_l3_getx_fwd_upgrade_nack;
			l3_caches[i].l3_get_fwd_upgrade_nack = cgm_mesi_l3_get_fwd_upgrade_nack;
			l3_caches[i].l3_getx_fwd_ack = cgm_mesi_l3_getx_fwd_ack;
			l3_caches[i].l3_upgrade = cgm_mesi_l3_upgrade;
			l3_caches[i].l3_write_block = cgm_mesi_l3_write_block;
			l3_caches[i].l3_write_back = cgm_mesi_l3_write_back;
		}
		else
		{
			fatal("invalid protocol at l3 cache init\n");
		}

		//stats
		l3_caches[i].fetches = 0;
		l3_caches[i].loads = 0;
		l3_caches[i].stores = 0;
		l3_caches[i].hits = 0;
		l3_caches[i].invalid_hits = 0;
		l3_caches[i].misses = 0;
		l3_caches[i].assoc_conflict = 0;
		l3_caches[i].upgrade_misses = 0;
		l3_caches[i].retries = 0;
		l3_caches[i].coalesces = 0;
		l3_caches[i].mshr_entries = 0;
		l3_caches[i].stalls = 0;


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
	}

	//finish creating the GPU caches
	for(i = 0 ; i < num_cus; i++)
	{
		/////////////
		//GPU S Cache
		/////////////

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d]", i);
		gpu_s_caches[i].name = strdup(buff);
		gpu_s_caches[i].id = i;
		gpu_s_caches[i].cache_type = gpu_s_cache_t;
		gpu_s_caches[i].log_block_size = LOG2(gpu_s_caches[i].block_size);
		gpu_s_caches[i].log_set_size = LOG2(gpu_s_caches[i].num_sets);
		gpu_s_caches[i].block_mask = gpu_s_caches[i].block_size - 1;
		gpu_s_caches[i].set_mask = gpu_s_caches[i].num_sets - 1;
		gpu_s_caches[i].block_address_mask = (unsigned int) 0xFFFFFFFF ^ gpu_s_caches[i].block_mask;

		//fatal("gpu_s_caches[i].block_address_mask addr mask 0x%08x\n", gpu_s_caches[i].block_address_mask);

		if(!gpu_s_caches[i].policy)
		{
			fatal("cache_finish_create(): Invalid cache policy\n");
		}

		//pointer to my own event count
		gpu_s_caches[i].ec_ptr = &gpu_s_cache[i];

		gpu_s_caches[i].Rx_queue_top = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].Rx_queue_top", i);
		gpu_s_caches[i].Rx_queue_top->name = strdup(buff);

		gpu_s_caches[i].Rx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].Rx_queue_bottom", i);
		gpu_s_caches[i].Rx_queue_bottom->name = strdup(buff);

		gpu_s_caches[i].next_queue = gpu_s_caches[i].Rx_queue_top;

		gpu_s_caches[i].retry_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].retry_queue", i);
		gpu_s_caches[i].retry_queue->name = strdup(buff);

		gpu_s_caches[i].ort_list = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].ort_list", i);
		gpu_s_caches[i].ort_list->name = strdup(buff);

		gpu_s_caches[i].ort = (int **)malloc(gpu_s_caches[i].mshr_size * sizeof(int *));
		for (j = 0; j < gpu_s_caches[i].mshr_size; j++)
		{
			gpu_s_caches[i].ort[j] = (int *)malloc(3 * sizeof(int));
		}

		//init ort
		for (j = 0; j <  gpu_s_caches[i].mshr_size; j++)
		{
			for (k = 0; k < 3; k++)
			{
				gpu_s_caches[i].ort[j][k] = -1;
			}
		}

		//Tx queue
		gpu_s_caches[i].Tx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].Tx_queue_bottom", i);
		gpu_s_caches[i].Tx_queue_bottom->name = strdup(buff);

		//WB
		gpu_s_caches[i].write_back_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].write_back_buffer", i);
		gpu_s_caches[i].write_back_buffer->name = strdup(buff);

		//Pending requests
		gpu_s_caches[i].pending_request_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].pending_request_buffer", i);
		gpu_s_caches[i].pending_request_buffer->name = strdup(buff);

		//Coherence queues
		gpu_s_caches[i].Coherance_Rx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].Coherance_Rx_queue", i);
		gpu_s_caches[i].Coherance_Rx_queue->name = strdup(buff);

		gpu_s_caches[i].Coherance_Tx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_caches[%d].Coherance_Tx_queue", i);
		gpu_s_caches[i].Coherance_Tx_queue->name = strdup(buff);

		//io ctrl
		gpu_s_caches[i].cache_io_down_ec = (void *) calloc((1), sizeof(eventcount));
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_ec");
		gpu_s_caches[i].cache_io_down_ec = new_eventcount(strdup(buff));

		//io tasks
		gpu_s_caches[i].cache_io_down_tasks = (void *) calloc((1), sizeof(task));
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_task");
		gpu_s_caches[i].cache_io_down_tasks = create_task(gpu_s_cache_down_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

		/*link cache virtual functions*/
		if(cgm_gpu_cache_protocol == cgm_protocol_non_coherent)
		{
			gpu_s_caches[i].gpu_s_load = cgm_nc_gpu_s_load;
			gpu_s_caches[i].gpu_s_write_block = cgm_nc_gpu_s_write_block;
		}
		else if (cgm_gpu_cache_protocol == cgm_protocol_mesi)
		{
			gpu_s_caches[i].gpu_s_load = cgm_mesi_gpu_s_load;
			gpu_s_caches[i].gpu_s_write_block = NULL;

		}
		else
		{
			fatal("invalid protocol at GPU S cache init\n");
		}

		//stats
		gpu_s_caches[i].fetches = 0;
		gpu_s_caches[i].loads = 0;
		gpu_s_caches[i].stores = 0;
		gpu_s_caches[i].hits = 0;
		gpu_s_caches[i].invalid_hits = 0;
		gpu_s_caches[i].misses = 0;
		gpu_s_caches[i].assoc_conflict = 0;
		gpu_s_caches[i].upgrade_misses = 0;
		gpu_s_caches[i].retries = 0;
		gpu_s_caches[i].coalesces = 0;
		gpu_s_caches[i].mshr_entries = 0;
		gpu_s_caches[i].stalls = 0;

		/////////////
		//GPU V Cache
		/////////////

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d]", i);
		gpu_v_caches[i].name = strdup(buff);
		gpu_v_caches[i].id = i;
		gpu_v_caches[i].cache_type = gpu_v_cache_t;
		gpu_v_caches[i].log_block_size = LOG2(gpu_v_caches[i].block_size);
		gpu_v_caches[i].log_set_size = LOG2(gpu_v_caches[i].num_sets);
		gpu_v_caches[i].block_mask = gpu_v_caches[i].block_size - 1;
		gpu_v_caches[i].set_mask = gpu_v_caches[i].num_sets - 1;
		gpu_v_caches[i].block_address_mask = (unsigned int) 0xFFFFFFFF ^ gpu_v_caches[i].block_mask;


		if(!gpu_v_caches[i].policy)
		{
			fatal("cache_finish_create(): Invalid cache policy\n");
		}

		//pointer to my own event count
		gpu_v_caches[i].ec_ptr = &gpu_v_cache[i];

		gpu_v_caches[i].Rx_queue_top = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].Rx_queue_top", i);
		gpu_v_caches[i].Rx_queue_top->name = strdup(buff);

		gpu_v_caches[i].Rx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].Rx_queue_bottom", i);
		gpu_v_caches[i].Rx_queue_bottom->name = strdup(buff);

		gpu_v_caches[i].next_queue = gpu_v_caches[i].Rx_queue_top;

		gpu_v_caches[i].retry_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].retry_queue", i);
		gpu_v_caches[i].retry_queue->name = strdup(buff);

		gpu_v_caches[i].ort_list = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].ort_list", i);
		gpu_v_caches[i].ort_list->name = strdup(buff);

		gpu_v_caches[i].ort = (int **)malloc(gpu_v_caches[i].mshr_size * sizeof(int *));
		for (j = 0; j < gpu_v_caches[i].mshr_size; j++)
		{
			gpu_v_caches[i].ort[j] = (int *)malloc(3 * sizeof(int));
		}

		//init ort
		for (j = 0; j <  gpu_v_caches[i].mshr_size; j++)
		{
			for (k = 0; k < 3; k++)
			{
				gpu_v_caches[i].ort[j][k] = -1;
			}
		}

		//Tx queue
		gpu_v_caches[i].Tx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].Tx_queue_bottom", i);
		gpu_v_caches[i].Tx_queue_bottom->name = strdup(buff);

		//WB
		gpu_v_caches[i].write_back_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].write_back_buffer", i);
		gpu_v_caches[i].write_back_buffer->name = strdup(buff);

		//Pending requests
		gpu_v_caches[i].pending_request_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].pending_request_buffer", i);
		gpu_v_caches[i].pending_request_buffer->name = strdup(buff);

		//Coherence queues
		gpu_v_caches[i].Coherance_Rx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].Coherance_Rx_queue", i);
		gpu_v_caches[i].Coherance_Rx_queue->name = strdup(buff);

		gpu_v_caches[i].Coherance_Tx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_caches[%d].Coherance_Tx_queue", i);
		gpu_v_caches[i].Coherance_Tx_queue->name = strdup(buff);

		//io ctrl
		gpu_v_caches[i].cache_io_down_ec = (void *) calloc((1), sizeof(eventcount));
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_ec");
		gpu_v_caches[i].cache_io_down_ec = new_eventcount(strdup(buff));

		//io tasks
		gpu_v_caches[i].cache_io_down_tasks = (void *) calloc((1), sizeof(task));
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_task");
		gpu_v_caches[i].cache_io_down_tasks = create_task(gpu_v_cache_down_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

		/*link cache virtual functions*/
		if(cgm_gpu_cache_protocol == cgm_protocol_non_coherent)
		{
			gpu_v_caches[i].gpu_v_load = cgm_nc_gpu_v_load;
			gpu_v_caches[i].gpu_v_store = cgm_nc_gpu_v_store;
			gpu_v_caches[i].gpu_v_write_block = cgm_nc_gpu_v_write_block;
		}
		else if(cgm_gpu_cache_protocol == cgm_protocol_mesi)
		{
			gpu_v_caches[i].gpu_v_load = cgm_mesi_gpu_v_load;
			gpu_v_caches[i].gpu_v_store = cgm_mesi_gpu_v_store;
			gpu_v_caches[i].gpu_v_write_block = cgm_mesi_gpu_v_write_block;
			gpu_v_caches[i].gpu_v_inval = cgm_mesi_gpu_v_inval;
		}
		else
		{
			fatal("invalid protocol at GPU V cache init\n");
		}

		//stats
		gpu_v_caches[i].fetches = 0;
		gpu_v_caches[i].loads = 0;
		gpu_v_caches[i].stores = 0;
		gpu_v_caches[i].hits = 0;
		gpu_v_caches[i].invalid_hits = 0;
		gpu_v_caches[i].misses = 0;
		gpu_v_caches[i].assoc_conflict = 0;
		gpu_v_caches[i].upgrade_misses = 0;
		gpu_v_caches[i].retries = 0;
		gpu_v_caches[i].coalesces = 0;
		gpu_v_caches[i].mshr_entries = 0;
		gpu_v_caches[i].stalls = 0;


		////////////
		//LDS caches
		////////////


		gpu_lds_units[i].id = i;
		//gpu_lds_units[i].log_block_size = LOG2(gpu_lds_units[i].block_size);
		//gpu_lds_units[i].log_set_size = LOG2(gpu_lds_units[i].num_sets);
		//gpu_lds_units[i].block_mask = gpu_lds_units[i].block_size - 1;
		//gpu_lds_units[i].set_mask = gpu_lds_units[i].num_sets - 1;
		//gpu_lds_units[i].hits = 0;
		//gpu_lds_units[i].invalid_hits = 0;
		//gpu_lds_units[i].misses = 0;
		//gpu_v_caches[i].fetches = 0;
		gpu_lds_units[i].Rx_queue_top = list_create();
		//gpu_lds_units[i].Rx_queue_bottom = list_create();
		//gpu_lds_units[i].mshr = list_create();

		//set cache name
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_lds_units[%d]", i);
		gpu_lds_units[i].name = strdup(buff);

		//set rx queue names
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_lds_units[%d].Rx_queue_top", i);
		gpu_lds_units[i].Rx_queue_top->name = strdup(buff);

		//memset (buff,'\0' , 100);
		//snprintf(buff, 100, "gpu_lds_units[%d].Rx_queue_bottom", i);
		//gpu_lds_units[i].Rx_queue_bottom->name = strdup(buff);

		//memset (buff,'\0' , 100);
		//snprintf(buff, 100, "gpu_lds_units[%d].mshr", i);
		//gpu_lds_units[i].mshr->name = strdup(buff);


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


		/*gpu_lds_units[i].sets = calloc(gpu_lds_units[i].num_sets, sizeof(struct cache_set_t));
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
		}*/
	}


	///////////////
	//GPU L2 caches
	///////////////

	//set up one cache for every four CUs
	for(i = 0 ; i < gpu_group_cache_num; i++)
	{

		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_caches[%d]", i);
		gpu_l2_caches[i].name = strdup(buff);
		gpu_l2_caches[i].id = i;
		gpu_l2_caches[i].cache_type = gpu_l2_cache_t;
		gpu_l2_caches[i].log_block_size = LOG2(gpu_l2_caches[i].block_size);
		gpu_l2_caches[i].log_set_size = LOG2(gpu_l2_caches[i].num_sets);
		gpu_l2_caches[i].block_mask = gpu_l2_caches[i].block_size - 1;
		gpu_l2_caches[i].set_mask = gpu_l2_caches[i].num_sets - 1;
		gpu_l2_caches[i].block_address_mask = (unsigned int) 0xFFFFFFFF ^ gpu_l2_caches[i].block_mask;

		if(!gpu_l2_caches[i].policy)
		{
			fatal("cache_finish_create(): GPU L2 Invalid cache policy\n");
		}

		//pointer to my own event count
		gpu_l2_caches[i].ec_ptr = &gpu_l2_cache[i];

		gpu_l2_caches[i].Rx_queue_top = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_caches[%d].Rx_queue_top", i);
		gpu_l2_caches[i].Rx_queue_top->name = strdup(buff);

		gpu_l2_caches[i].Rx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_caches[%d].Rx_queue_bottom", i);
		gpu_l2_caches[i].Rx_queue_bottom->name = strdup(buff);

		gpu_l2_caches[i].next_queue = gpu_l2_caches[i].Rx_queue_top;

		gpu_l2_caches[i].retry_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_caches[%d].retry_queue", i);
		gpu_l2_caches[i].retry_queue->name = strdup(buff);

		gpu_l2_caches[i].ort_list = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_caches[%d].ort_list", i);
		gpu_l2_caches[i].ort_list->name = strdup(buff);

		gpu_l2_caches[i].ort = (int **)malloc(gpu_l2_caches[i].mshr_size * sizeof(int *));
		for (j = 0; j < gpu_l2_caches[i].mshr_size; j++)
		{
			gpu_l2_caches[i].ort[j] = (int *)malloc(3 * sizeof(int));
		}

		//init ort
		for (j = 0; j <  gpu_l2_caches[i].mshr_size; j++)
		{
			for (k = 0; k < 3; k++)
			{
				gpu_l2_caches[i].ort[j][k] = -1;
			}
		}

		//Tx queues
		gpu_l2_caches[i].Tx_queue_top = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_caches[%d].Tx_queue_top", i);
		gpu_l2_caches[i].Tx_queue_top->name = strdup(buff);

		gpu_l2_caches[i].Tx_queue_bottom = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_caches[%d].Tx_queue_bottom", i);
		gpu_l2_caches[i].Tx_queue_bottom->name = strdup(buff);

		//coherance queues
		gpu_l2_caches[i].Coherance_Rx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_caches[%d].Coherance_Rx_queue", i);
		gpu_l2_caches[i].Coherance_Rx_queue->name = strdup(buff);

		gpu_l2_caches[i].Coherance_Tx_queue = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_caches[%d].Coherance_Tx_queue", i);
		gpu_l2_caches[i].Coherance_Tx_queue->name = strdup(buff);

		//WB
		gpu_l2_caches[i].write_back_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_caches[%d].write_back_buffer", i);
		gpu_l2_caches[i].write_back_buffer->name = strdup(buff);

		//Pending Request Buffer
		gpu_l2_caches[i].pending_request_buffer = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_caches[%d].pending_request_buffer", i);
		gpu_l2_caches[i].pending_request_buffer->name = strdup(buff);

		//io ctrl
		gpu_l2_caches[i].cache_io_up_ec = (void *) calloc((1), sizeof(eventcount));
		gpu_l2_caches[i].cache_io_down_ec = (void *) calloc((1), sizeof(eventcount));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_up_ec");
		gpu_l2_caches[i].cache_io_up_ec = new_eventcount(strdup(buff));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_ec");
		gpu_l2_caches[i].cache_io_down_ec = new_eventcount(strdup(buff));

		//io tasks
		gpu_l2_caches[i].cache_io_up_tasks = (void *) calloc((1), sizeof(task));
		gpu_l2_caches[i].cache_io_down_tasks = (void *) calloc((1), sizeof(task));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_up_task");
		gpu_l2_caches[i].cache_io_up_tasks = create_task(gpu_l2_cache_up_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "cache_io_down_task");
		gpu_l2_caches[i].cache_io_down_tasks = create_task(gpu_l2_cache_down_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

		/*link cache virtual functions*/
		if(cgm_gpu_cache_protocol == cgm_protocol_non_coherent)
		{
			gpu_l2_caches[i].gpu_l2_get = cgm_nc_gpu_l2_get;
			gpu_l2_caches[i].gpu_l2_write_block = cgm_nc_gpu_l2_write_block;
		}
		else if(cgm_gpu_cache_protocol == cgm_protocol_mesi)
		{
			gpu_l2_caches[i].gpu_l2_getx = cgm_mesi_gpu_l2_getx;
			gpu_l2_caches[i].gpu_l2_get = NULL;
			gpu_l2_caches[i].gpu_l2_write_block = cgm_mesi_gpu_l2_write_block;
		}
		else
		{
			fatal("invalid protocol at GPU L2 cache init\n");
		}

		//stats//stats
		gpu_l2_caches[i].fetches = 0;
		gpu_l2_caches[i].loads = 0;
		gpu_l2_caches[i].stores = 0;
		gpu_l2_caches[i].hits = 0;
		gpu_l2_caches[i].invalid_hits = 0;
		gpu_l2_caches[i].misses = 0;
		gpu_l2_caches[i].assoc_conflict = 0;
		gpu_l2_caches[i].upgrade_misses = 0;
		gpu_l2_caches[i].retries = 0;
		gpu_l2_caches[i].coalesces = 0;
		gpu_l2_caches[i].mshr_entries = 0;
		gpu_l2_caches[i].stalls = 0;

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
	}

	return 0;
}

int directory_read_config(void* user, const char* section, const char* name, const char* value){

	if(MATCH("Directory", "BlockSize"))
	{
		dir_block_size = atoi(value);
	}

	if(MATCH("Directory", "Mode"))
	{
		dir_mode = atoi(value);
	}

	/*if(MATCH("Directory", "MemSize"))
	{
		dir_mem_image_size = atoll(value);//4GB;
	}*/

	/*if(MATCH("Directory", "VectorSize"))
	{
		dir_vector_size = atoi(value);
	}*/

	return 0;
}

int directory_finish_create(void){

	/*int num_cores = x86_cpu_num_cores;
	char buff[100];
	int i = 0;*/

	// create the number of queues we need.
	// for our base system one entry per l3 cache slice.
	// this can also be set up as one entry per system agent.

	//do some checks
	if(dir_block_size != l3_caches->block_size)
	{
		fatal("directory_finish_create() block size needs to match l3 block size. Check the cgm_config.ini file");
	}

	//init the data vectors
	//m2s doesn't natively account for dir memory overhead IN the memory image
	//get the number of blocks in the system
	dir_num_blocks = (dir_mem_image_size/dir_block_size);

	//this accounts for directory overhead
	//directory->num_blocks = (directory->mem_image_size/(directory->block_size + directory->vector_size));

	//get the block mask
	dir_block_mask = dir_block_size - 1; // this is 0 - N


	/*if(directory->vector_size == 1)
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
	}*/


	//set up input queues
	/*if (directory->mode)
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
	}*/

	/*printf("directory->mem_image_size %llu\n", directory->mem_image_size);
	printf("directory->block_size %llu\n", directory->block_size);
	printf("num_blocks %llu\n", num_blocks);
	getchar();*/

	return 0;

}

int switch_read_config(void* user, const char* section, const char* name, const char* value){

	int num_cores = x86_cpu_num_cores;
	/*int num_cus = si_gpu_num_compute_units;*/
	int Ports = 0;
	int i = 0;
	int WireLatency = 0;
	int Latency = 0;
	int Bus_width = 0;
	int MSHR = 0;
	int maxcoal = 0;

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

	if(MATCH("Switch", "Latency"))
	{
		Latency = atoi(value);

		for (i = 0; i < (num_cores + extras); i++)
		{
			switches[i].latency = Latency;
		}
	}

	if(MATCH("Bus", "Switches"))
	{
		Bus_width = atoi(value);

		for (i = 0; i < (num_cores + extras); i++)
		{
			switches[i].bus_width = Bus_width;

			if(switches[i].bus_width == 0)
			{
				fatal("switch_read_config(): switch bus width is out of bounds\n");
			}
		}
	}


	/**************here*************/

	if(MATCH("Hub-IOMMU", "MSHR"))
	{
		hub_iommu->mshr_size = atoi(value);
	}

	if(MATCH("Hub-IOMMU", "MaxCoalesce"))
	{
		hub_iommu->max_coal = atoi(value);
	}

	if(MATCH("Hub-IOMMU", "Latency"))
	{
		hub_iommu->latency = atoi(value);
	}

	if(MATCH("Hub-IOMMU", "WireLatency"))
	{
		hub_iommu->wire_latency = atoi(value);
	}

	if(MATCH("Bus", "Switches"))
	{
		hub_iommu->bus_width = atoi(value);

		if(hub_iommu->bus_width == 0)
		{
			fatal("switch_read_config(): switch bus width is out of bounds\n");
		}
	}

	return 0;
}

int switch_finish_create(void){

	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);

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

			switches[i].switch_id = i;

			//configure cross bar
			switches[i].crossbar = switch_crossbar_create();

			//Rx queues
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

			//Tx queues
			switches[i].Tx_north_queue = list_create();
			switches[i].Tx_east_queue = list_create();
			switches[i].Tx_south_queue = list_create();
			switches[i].Tx_west_queue = list_create();

			memset (buff,'\0' , 100);
			snprintf(buff, 100, "switch[%d].Tx_north_queue", i);
			switches[i].Tx_north_queue->name = strdup(buff);

			memset (buff,'\0' , 100);
			snprintf(buff, 100, "switch[%d].Tx_east_queue", i);
			switches[i].Tx_east_queue->name = strdup(buff);

			memset (buff,'\0' , 100);
			snprintf(buff, 100, "switch[%d].Tx_south_queue", i);
			switches[i].Tx_south_queue->name = strdup(buff);

			memset (buff,'\0' , 100);
			snprintf(buff, 100, "switch[%d].Tx_west_queue", i);
			switches[i].Tx_west_queue->name = strdup(buff);


			//init the queue pointer
			switches[i].queue = west_queue;

			//init the switche's network node number
			//star todo fix this (we shouldn't need to specifiy this).
			//for now manually set the last switch to node 13
			//if you don't you can't change the number of cores from 4
			if(i == num_cores)
			{
				switches[i].switch_node_number = str_map_string(&node_strn_map, "switch[4]");
			}
			else
			{
				switches[i].switch_node_number = str_map_string(&node_strn_map, switches[i].name);
			}

			median ++;//= switches[i].switch_node_number;

			//io event counts
			switches[i].switches_north_io_ec = (void *) calloc((1), sizeof(eventcount));
			switches[i].switches_east_io_ec = (void *) calloc((1), sizeof(eventcount));
			switches[i].switches_south_io_ec = (void *) calloc((1), sizeof(eventcount));
			switches[i].switches_west_io_ec = (void *) calloc((1), sizeof(eventcount));

			memset(buff,'\0' , 100);
			snprintf(buff, 100, "switch_north_io_ec");
			switches[i].switches_north_io_ec = new_eventcount(strdup(buff));

			memset(buff,'\0' , 100);
			snprintf(buff, 100, "switch_east_io_ec");
			switches[i].switches_east_io_ec = new_eventcount(strdup(buff));

			memset(buff,'\0' , 100);
			snprintf(buff, 100, "switch_south_io_ec");
			switches[i].switches_south_io_ec = new_eventcount(strdup(buff));

			memset(buff,'\0' , 100);
			snprintf(buff, 100, "switch_west_io_ec");
			switches[i].switches_west_io_ec = new_eventcount(strdup(buff));

			//io tasks
			switches[i].switches_north_io_tasks = (void *) calloc((1), sizeof(task));
			switches[i].switches_east_io_tasks = (void *) calloc((1), sizeof(task));
			switches[i].switches_south_io_tasks = (void *) calloc((1), sizeof(task));
			switches[i].switches_west_io_tasks = (void *) calloc((1), sizeof(task));

			memset(buff,'\0' , 100);
			snprintf(buff, 100, "switch_north_io_task");
			switches[i].switches_north_io_tasks = create_task(switch_north_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

			memset(buff,'\0' , 100);
			snprintf(buff, 100, "switch_east_io_task");
			switches[i].switches_east_io_tasks = create_task(switch_east_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

			memset(buff,'\0' , 100);
			snprintf(buff, 100, "switch_south_io_task");
			switches[i].switches_south_io_tasks = create_task(switch_south_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

			memset(buff,'\0' , 100);
			snprintf(buff, 100, "switch_west_io_task");
			switches[i].switches_west_io_tasks = create_task(switch_west_io_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

		}

		//init the median node number on all switches
		for (i = 0; i < (num_cores + extras); i++)
		{
			//get the average
			switches[i].switch_median_node = median/2;
		}

		//configure the bi-directional ring.
		for (i = 0; i < (num_cores + extras); i++)
		{
			if(i == 0)
			{
				switches[i].next_west = switches[num_cores + extras - 1].east_queue;
				switches[i].next_west_id = num_cores + extras - 1;
				switches[i].next_east = switches[i+1].west_queue;
				switches[i].next_east_id = i+1;

			}
			else if( i > 0 && i < (num_cores + extras - 1))
			{
				switches[i].next_west = switches[i-1].east_queue;
				switches[i].next_west_id = i-1;
				switches[i].next_east = switches[i+1].west_queue;
				switches[i].next_east_id = i+1;
			}
			else if(i == (num_cores + extras - 1))
			{
				switches[i].next_west = switches[i-1].east_queue;
				switches[i].next_west_id = i-1;
				switches[i].next_east = switches[0].west_queue;
				switches[i].next_east_id = 0;
			}
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

	////////////
	//GPU hub-iommu
	////////////

	//configure the gpu hub-iommu here (its a multiplexer).
	memset (buff,'\0' , 100);
	snprintf(buff, 100, "hub_iommu");
	hub_iommu->name = strdup(buff);

	hub_iommu->gpu_l2_num = gpu_group_cache_num;

	//create one Rx and Tx queue for each gpu l2 caches
	hub_iommu->Rx_queue_top = (void *) calloc(gpu_group_cache_num, sizeof(struct list_t));
	hub_iommu->Tx_queue_top = (void *) calloc(gpu_group_cache_num, sizeof(struct list_t));

	for(i = 0; i < gpu_group_cache_num; i ++)
	{
		hub_iommu->Rx_queue_top[i] = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "hub_iommu.Rx_queue_top[%d]", i);
		hub_iommu->Rx_queue_top[i]->name = strdup(buff);

		hub_iommu->Tx_queue_top[i] = list_create();
		memset (buff,'\0' , 100);
		snprintf(buff, 100, "hub_iommu.Tx_queue_top[%d]", i);
		hub_iommu->Tx_queue_top[i]->name = strdup(buff);

	}

	//create the bottom Rx and Tx queues
	hub_iommu->Rx_queue_bottom = list_create();
	memset (buff,'\0' , 100);
	snprintf(buff, 100, "hub_iommu.Rx_queue_bottom");
	hub_iommu->Rx_queue_bottom->name = strdup(buff);

	hub_iommu->Tx_queue_bottom = list_create();
	memset (buff,'\0' , 100);
	snprintf(buff, 100, "hub_iommu.Tx_queue_bottom");
	hub_iommu->Tx_queue_bottom->name = strdup(buff);

	//create io ctrl tasks
	hub_iommu->hub_iommu_io_up_ec = (void *) calloc(gpu_group_cache_num, sizeof(eventcount));
	hub_iommu->hub_iommu_io_up_tasks = (void *) calloc(gpu_group_cache_num, sizeof(task));

	//top Tx ctrl
	for(i = 0; i < gpu_group_cache_num; i ++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "hub_iommu_io_up_ec_%d", i);
		hub_iommu->hub_iommu_io_up_ec[i] = new_eventcount(strdup(buff));

		memset(buff,'\0' , 100);
		snprintf(buff, 100, "hub_iommu_io_up_task_%d", i);
		hub_iommu->hub_iommu_io_up_tasks[i] = create_task(hub_iommu_io_up_ctrl, DEFAULT_STACK_SIZE, strdup(buff));
	}

	//bottom Tx ctrl
	hub_iommu->hub_iommu_io_down_ec = (void *) calloc((1), sizeof(eventcount));
	hub_iommu->hub_iommu_io_down_tasks = (void *) calloc((1), sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "hub_iommu_io_down_ec");
	hub_iommu->hub_iommu_io_down_ec = new_eventcount(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "hub_iommu_io_down_task");
	hub_iommu->hub_iommu_io_down_tasks = create_task(hub_iommu_io_down_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	//set a pointer to the next queue
	hub_iommu->next_queue = hub_iommu->Rx_queue_top[0];

	//connect to switch queue
	hub_iommu->switch_id = (num_cores + extras - 1);

	//hub_iommu->switch_queue = switches[(num_cores + extras - 1)].north_queue;
	hub_iommu->switch_queue = switches[hub_iommu->switch_id].north_queue;


	//set up translation table
	assert(gpu_l2_caches[0].mshr_size);
	int table_size = gpu_group_cache_num * gpu_l2_caches[0].mshr_size;

	//store table size
	hub_iommu->translation_table_size = table_size;

	//initialize 2D array pointer
	hub_iommu->translation_table = (unsigned int **) malloc(table_size * sizeof(unsigned int *));

	//initialize rows and columns
	for(i = 0; i < table_size; i++)
	{
		hub_iommu->translation_table[i] = (unsigned int *) malloc(3 * sizeof(unsigned int));
	}

	//initialize the ID and address columns
	for(i = 0; i < table_size; i++)
	{
		hub_iommu->translation_table[i][0] = i;
		hub_iommu->translation_table[i][1] = -1;
		hub_iommu->translation_table[i][2] = -1;
	}

	//configure hub-iommu virtual functions
	if(cgm_gpu_cache_protocol == cgm_protocol_non_coherent)
	{
		hub_iommu_ctrl = hub_iommu_noncoherent_ctrl;
	}
	else if(cgm_gpu_cache_protocol == cgm_protocol_mesi)
	{
		hub_iommu_ctrl = hub_iommu_coherent_ctrl;

		if(hub_iommu_connection_type != 1)
			fatal("switch_finish_create(): hub-iommu connection type invalid, must be L3 if coherent.\n");
	}

	hub_iommu_create_tasks(hub_iommu_ctrl);

	//configure correct routing function
	if(hub_iommu_connection_type == hub_to_mc)
	{
		hub_iommu_put_next_queue = hub_iommu_put_next_queue_MC;
		printf("---GPU connection mode is MC---\n");
	}
	else if(hub_iommu_connection_type == hub_to_l3)
	{
		hub_iommu_put_next_queue = hub_iommu_put_next_queue_L3;
		printf("---GPU connection mode is L3---\n");
	}
	else if(hub_iommu_connection_type == 2)
	{
		fatal("switch_finish_create(): HUB_IOMMU_CONNECTION_MODE invlid setting\n");
	}
	else
	{
		fatal("switch_finish_create(): HUB_IOMMU_CONNECTION_MODE invlid setting\n");
	}

	return 0;
}

int sys_agent_config(void* user, const char* section, const char* name, const char* value){

	int Ports = 0;
	int WireLatency = 0;
	int Latency = 0;
	int Down_Bus_width = 0;
	int Up_Bus_width = 0;


	if(MATCH("SysAgent", "Ports"))
	{
		Ports = atoi(value);
		system_agent->num_ports = Ports;

	}

	if(MATCH("SysAgent", "WireLatency"))
	{
		WireLatency = atoi(value);
		system_agent->wire_latency = WireLatency;
	}

	if(MATCH("SysAgent", "Latency"))
	{
		Latency = atoi(value);
		system_agent->latency = Latency;
	}

	if(MATCH("Bus", "MC-SA"))
	{
		Down_Bus_width = atoi(value);
		system_agent->down_bus_width = Down_Bus_width;

		if(system_agent->down_bus_width == 0)
		{
			fatal("mem_ctrl_config(): mem_ctrl down bus width is out of bounds\n");
		}
	}

	if(MATCH("Bus", "Switches"))
	{
		Up_Bus_width = atoi(value);
		system_agent->up_bus_width = Up_Bus_width;

		if(system_agent->up_bus_width == 0)
		{
			fatal("mem_ctrl_config(): mem_ctrl up bus width is out of bounds\n");
		}
	}

	return 0;
}

int sys_agent_finish_create(void){

	//star todo fix this
	int num_cores = x86_cpu_num_cores;
	//int extras = 1;
	//int num_cus = si_gpu_num_compute_units;

	char buff[100];

	//set cache name
	memset (buff,'\0' , 100);
	snprintf(buff, 100, "sys_agent");
	system_agent->name = strdup(buff);

	system_agent->switch_id = (num_cores);

	system_agent->Rx_queue_top = list_create();
	memset (buff,'\0' , 100);
	snprintf(buff, 100, "system_agent.Rx_queue_top");
	system_agent->Rx_queue_top->name = strdup(buff);

	system_agent->Rx_queue_bottom = list_create();
	memset (buff,'\0' , 100);
	snprintf(buff, 100, "system_agent.Rx_queue_bottom");
	system_agent->Rx_queue_bottom->name = strdup(buff);

	system_agent->Tx_queue_top = list_create();
	memset (buff,'\0' , 100);
	snprintf(buff, 100, "system_agent.Tx_queue_top");
	system_agent->Tx_queue_top->name = strdup(buff);

	system_agent->Tx_queue_bottom = list_create();
	memset (buff,'\0' , 100);
	snprintf(buff, 100, "system_agent.Tx_queue_bottom");
	system_agent->Tx_queue_bottom->name = strdup(buff);

	//point to switch queue.
	system_agent->switch_queue = switches[system_agent->switch_id].south_queue;

	//pointer to next queue.
	system_agent->next_queue = system_agent->Rx_queue_top;

	return 0;
}

int mem_ctrl_config(void* user, const char* section, const char* name, const char* value){

	//int Ports = 0;
	int WireLatency = 0;
	int DRAMLatency = 0;
	int Latency = 0;
	int Bus_width = 0;

	if(MATCH("MemCtrl", "Latency"))
	{
		Latency = atoi(value);
		mem_ctrl->latency = Latency;
	}

	if(MATCH("MemCtrl", "WireLatency"))
	{
		WireLatency = atoi(value);
		mem_ctrl->wire_latency = WireLatency;
	}

	if(MATCH("MemCtrl", "DRAMLatency"))
	{
		DRAMLatency = atoi(value);
		mem_ctrl->DRAM_latency = DRAMLatency;
	}

	if(MATCH("Bus", "MC-SA"))
	{
		Bus_width = atoi(value);
		mem_ctrl->bus_width = Bus_width;

		if(mem_ctrl->bus_width == 0)
		{
			fatal("mem_ctrl_config(): mem_ctrl bus width is out of bounds\n");
		}
	}

	/*dram config*/
	if(MATCH("DRAM", "DRAMSim"))
	{
		DRAMSim = atoi(value);

		if (DRAMSim < 0 || DRAMSim > 1)
		{
			fatal("mem_ctrl_config(): dramsim invalid setting as %d\n", DRAMSim);
		}
	}

	/*dram config*/
	if(MATCH("DRAM", "Size"))
	{
		mem_size = atoi(value);
	}

	if(MATCH("DRAM", "DDR_Module_Path"))
	{
		dramsim_ddr_config_path = strdup(value);
	}

	if(MATCH("DRAM", "DRAMSim_Config_Path"))
	{
		dramsim_system_config_path = strdup(value);
	}

	if(MATCH("DRAM", "DRAMSim_Trace_Path"))
	{
		dramsim_trace_config_path = strdup(value);
	}

	if(MATCH("DRAM", "DRAMSim_Vis_File_Name"))
	{
		dramsim_vis_config_path = strdup(value);
	}

	return 0;
}

int mem_ctrl_finish_create(struct mem_t *mem){

	char buff[100];

	//set cache name
	memset (buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl");
	mem_ctrl->name = strdup(buff);

	mem_ctrl->Rx_queue_top = list_create();

	memset (buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl.Rx_queue_top");
	mem_ctrl->Rx_queue_top->name = strdup(buff);

	mem_ctrl->Tx_queue = list_create();

	memset (buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl.Tx_queue");
	mem_ctrl->Tx_queue->name = strdup(buff);

	mem_ctrl->pending_accesses = list_create();

	memset (buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl.pending_accesses");
	mem_ctrl->pending_accesses->name = strdup(buff);

	mem_ctrl->system_agent_queue = system_agent->Rx_queue_bottom;

	//link the memory controller to the simulated memory image
	mem_ctrl->mem = mem;

	/*dramsim stuff*/
	if(DRAMSim == 1)
	{
		/*get CPU frequency*/
		cpu_freq = x86_cpu_frequency * MHZ;

		dramsim_create_mem_object();
		dramsim_register_call_backs();
		dramsim_set_cpu_freq();

		printf("---DRAMSim is connected---\n");
		printf("---Total main memory is %d---\n", mem_size);
	}
	else
	{
		printf("---DRAMSim is disconnected---\n");
	}

	/*stats*/
	mem_ctrl->active_cycles = 0;
	mem_ctrl->num_reads = 0;
	mem_ctrl->num_writes = 0;
	mem_ctrl->ave_dram_read_lat = 0;
	mem_ctrl->ave_dram_write_lat = 0;
	mem_ctrl->ave_dram_total_lat = 0;
	mem_ctrl->read_min = LLONG_MAX;
	mem_ctrl->read_max = 0;
	mem_ctrl->write_min = LLONG_MAX;
	mem_ctrl->write_max = 0;
	mem_ctrl->pedding_accesses_max = 0;
	mem_ctrl->rx_max = 0;
	mem_ctrl->tx_max = 0;
	mem_ctrl->bytes_read = 0;
	mem_ctrl->bytes_wrote = 0;

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
