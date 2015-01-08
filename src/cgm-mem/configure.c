/*
 * configure.c
 *
 *  Created on: Nov 26, 2014
 *      Author: stardica
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cgm-mem/configure.h>
#include <cgm-mem/ini-parse.h>
#include <cgm-mem/queue.h>
#include <cgm-mem/cache.h>
#include <cgm-mem/cgm-mem.h>


int cgmmem_check_config = 0;
/*struct queue_config_t *q_config;
struct cache_config_t *c_config;*/

int cgmmem_configure(void){

	int error = 0;

	//star todo add parsing for mem-ctrl and memory image

	//star >> we dont need to pass a struct to the ini_parse function for it to work. Passing NULL is fine.
	/*q_config = (void *) malloc(sizeof(struct queue_config_t));
	c_config = (void *) malloc(sizeof(struct cache_config_t));

	if(!q_config)
	{

		printf("Unable to create struct q_config\n");
		return 1;

	}else if (!c_config)
	{
		printf("Unable to create struct cache_config\n");
		return 1;
	}*/

	//get some host sim configuration stats
	error = ini_parse(HOSTSIMCONFIGPATH, cpu_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for cpu configuration.\n");
		return 1;
	}

	//get check value
	error = ini_parse(CGMMEMCONFIGPATH, check_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for check configuration.\n");
		return 1;
	}

	//get queue configuration
	error = ini_parse(CGMMEMCONFIGPATH, queue_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for queue configuration.\n");
		return 1;
	}

	//get cache configuration
	error = ini_parse(CGMMEMCONFIGPATH, cache_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for cache configuration.\n");
		return 1;
	}

	//star >> uncomment these when these functions are ready.
	/*error = ini_parse(CONFIGPATH, sysagent_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for sysagent configuration.\n");
		return 1;
	}

	error = ini_parse(CONFIGPATH, memctrl_config, NULL);
	if (error < 0)
	{
		printf("Unable to open Config.ini for memctrl configuration.\n");
		return 1;
	}*/

	return 0;

}

int cpu_config(void* user, const char* section, const char* name, const char* value){


	//star >> for M2S config.ini files..
	if(MATCH("General", "Cores"))
	{
		//temp->size = atoi(value);
		host_sim_cpu_core_num = atoi(value);

	}

	if(MATCH("General", "Threads"))
	{
		//temp->size = atoi(value);
		host_sim_cpu_thread_num = atoi(value);

	}

	if(MATCH("Queues", "FetchQueueSize"))
	{
		//temp->size = atoi(value);
		host_sim_cpu_fetch_queue_size = atoi(value);

	}

	if(MATCH("Queues", "LsqSize"))
	{
		//temp->size = atoi(value);
		host_sim_cpu_lsq_queue_size = atoi(value);

	}

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


	return 0;
}

void print_config(void){

	//print config before runtime
	printf("CPU:\n");
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
	return;
}
