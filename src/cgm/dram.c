/*
 * dram.c
 *
 *  Created on: Jan 26, 2016
 *      Author: stardica
 */

#include <cgm/dram.h>
#include <arch/x86/timing/cpu.h>


int DRAMSim = 0;
eventcount volatile *dramsim;
task *dramsim_cpu_clock;
void *DRAM_object_ptr; /*pointer to DRAMSim memory object*/

/* 4GB models...
 * DDR3_micron_8M_8B_x16_sg15.ini
 * DDR3_micron_32M_8B_x8_sg25E.ini
 * DDR3_micron_32M_8B_x8_sg15.ini
 * DDR3_micron_32M_8B_x4_sg15.ini
 * DDR3_micron_32M_8B_x4_sg125.ini
 * DDR3_micron_16M_8B_x8_sg15.ini*/

char *dramsim_ddr_config_path = "";
char *dramsim_system_config_path = "";
char *dramsim_trace_config_path = "";
char *dramsim_vis_config_path = "";

unsigned int mem_size = 0;
unsigned int cpu_freq = 0;

void dramsim_init(void){

	cpu_freq = (unsigned int) x86_cpu_frequency * MHZ;

	dramsim_create_tasks();

	return;
}

void dramsim_create_tasks(void){

	char buff[100];

	//dramsim tasks
	dramsim = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "dramsim");
	dramsim = new_eventcount(strdup(buff));

	dramsim_cpu_clock = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "dramsim_cpu_clock");
	dramsim_cpu_clock = create_task(dramsim_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	return;
}

void dramsim_print(void){

	call_print_me();

	return;

}

int dramsim_add_transaction(enum cgm_access_kind_t access_type, unsigned int addr){

	bool write;
	(access_type == cgm_access_mc_store) ? (write = true) : (write = false);

	int return_val = call_add_transaction(DRAM_object_ptr, write, addr);

	return return_val;
}


void dramsim_create_mem_object(void){

	/*call requires -> char *dev, char *sys, char *pwd,  char *trc, unsigned int megsOfMemory, char *visfilename*/
	DRAM_object_ptr = (void *) call_get_memory_system_instance(dramsim_ddr_config_path, dramsim_system_config_path, dramsim_trace_config_path, "m2s-cgm", mem_size, dramsim_vis_config_path);

	//printf("C side DRAM_object_ptr 0x%08x\n", DRAM_object_ptr);

	return;
}

void dramsim_set_cpu_freq(void){

	call_set_CPU_clock_speed(DRAM_object_ptr, cpu_freq);

	//printf("C side freq set %d\n", cpu_freq);

	return;
}

void dramsim_register_call_backs(void){

	call_register_call_backs(DRAM_object_ptr, dramsim_read_complete, dramsim_write_complete, dramsim_power_callback);

	return;
}

/* callback functions */
void dramsim_read_complete(unsigned id, long long address, long long clock_cycle)
{
	struct cgm_packet_t *message_packet;

	/*printf("[Callback on M2S] read complete: addr 0x%08x cycle %llu\n", (unsigned int) address, P_TIME);*/

	int i = 0;
	int hit = 0;

	LIST_FOR_EACH(mem_ctrl->pending_accesses, i)
	{
		//get pointer to access in queue and check it's status.
		message_packet = list_get(mem_ctrl->pending_accesses, i);

		//found block in write back buffer
		if(GET_BLOCK(message_packet->address) == (unsigned int)address)
		{
			hit = 1;
			break;
		}
	}

	assert(hit == 1);
	assert(GET_BLOCK(message_packet->address) == (unsigned int)address);
	assert(message_packet->access_type == cgm_access_mc_load);

	//printf("load msaddr 0x%08x\n dsaddr 0x%08x\n", message_packet->address, (unsigned int)address);

	//set the access type
	message_packet->access_type = cgm_access_mc_put;
	message_packet->size = l3_caches[0].block_size;

	//stats//////
	mem_ctrl->num_reads++;
	long long elapsed_cycles = P_TIME - message_packet->dram_start_cycle;

	/*running ave = ((old count * old data) + next data) / next count*/
	mem_ctrl->ave_dram_read_lat = (((mem_ctrl->num_writes - 1) * mem_ctrl->ave_dram_write_lat) + elapsed_cycles) / mem_ctrl->num_writes;

	if(mem_ctrl->read_min > elapsed_cycles)
		mem_ctrl->read_min = elapsed_cycles;

	if(mem_ctrl->read_max < elapsed_cycles)
		mem_ctrl->read_max = elapsed_cycles;

	mem_ctrl->dram_busy_cycles += elapsed_cycles;
	//stats end//////

	if(list_count(mem_ctrl->Tx_queue_top) > QueueSize)
		warning("mem ctrl tx queue top size %d\n", list_count(mem_ctrl->Tx_queue_top));

	//reply to L3
	message_packet = list_remove(mem_ctrl->pending_accesses, message_packet);
	list_enqueue(mem_ctrl->Tx_queue_top, message_packet);
	advance(mem_ctrl_io_ec);

	if((((message_packet->address & ~mem_ctrl->block_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(SYSTEM == 1)
		{
			printf("block 0x%08x %s DRAM access complete (Read) ID %llu type %d cycle %llu\n",
					(message_packet->address & ~mem_ctrl->block_mask), mem_ctrl->name, message_packet->access_id, message_packet->access_type, P_TIME);
		}
	}

	return;
}

void dramsim_write_complete(unsigned id, long long address, long long clock_cycle)
{
	struct cgm_packet_t *message_packet;

	int i = 0;
	int hit = 0;

	LIST_FOR_EACH(mem_ctrl->pending_accesses, i)
	{
		//get pointer to access in queue and check it's status.
		message_packet = list_get(mem_ctrl->pending_accesses, i);

		if(GET_BLOCK(message_packet->address) == (unsigned int)address)
		{
			hit = 1;
			break;
		}
	}

	assert(hit == 1);
	assert(GET_BLOCK(message_packet->address) == (unsigned int)address);
	assert(message_packet->access_type == cgm_access_mc_store);

	//stats/////////
	mem_ctrl->num_writes++;
	long long elapsed_cycles = P_TIME - message_packet->dram_start_cycle;

	/*running ave = ((old count * old data) + next data) / next count*/
	mem_ctrl->ave_dram_write_lat = (((mem_ctrl->num_writes - 1) * mem_ctrl->ave_dram_write_lat) + elapsed_cycles) / mem_ctrl->num_writes;

	if(mem_ctrl->write_min > elapsed_cycles)
		mem_ctrl->write_min = elapsed_cycles;

	if(mem_ctrl->write_max < elapsed_cycles)
		mem_ctrl->write_max = elapsed_cycles;

	mem_ctrl->dram_busy_cycles += elapsed_cycles;
	//stats/////////

	message_packet = list_remove(mem_ctrl->pending_accesses, message_packet);
	free(message_packet);

	if((((message_packet->address & ~mem_ctrl->block_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
	{
		if(SYSTEM == 1)
		{
			printf("block 0x%08x %s DRAM access complete (Write) ID %llu type %d cycle %llu\n",
					(message_packet->address & ~mem_ctrl->block_mask), mem_ctrl->name, message_packet->access_id, message_packet->access_type, P_TIME);
		}
	}

	return;
}

void dramsim_power_callback(double a, double b, double c, double d)
{
	return;
}

void dramsim_update_cpu_clock(void){

	call_update(DRAM_object_ptr);

	return;
}


void dramsim_ctrl(void){

	long long step = 1;
	set_id(0);

	while(1)
	{
		//printf("mem_ctrl\n");
		await(dramsim, step);
		step++;

		//printf("dramsim tick cycle %llu\n", P_TIME);

		dramsim_update_cpu_clock();
	}

	return;
}
