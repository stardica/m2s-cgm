/*
 * mem-ctrl.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */


#include <cgm/cgm.h>
#include <cgm/mem-ctrl.h>

/*
#include <mem-image/memory.h>
#include <mem-image/mmu.h>
*/

/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/util/list.h>
#include <cgm/cgm.h>
#include <cgm/mem-ctrl.h>
#include <cgm/sys-agent.h>
#include <cgm/tasking.h>
#include <cgm/packet.h>
#include <cgm/cache.h>
#include <DRAMSim/DRAMSim.h>
#include <dramsim/DRAMSim.h>*/


//structure declarations
struct mem_ctrl_t *mem_ctrl;
eventcount volatile *mem_ctrl_ec;
eventcount volatile *mem_ctrl_io_ec;
task *mem_ctrl_task;
task *mem_ctrl_io_task;
int mem_ctrl_pid = 0;
int mem_ctrl_io_pid = 0;
eventcount volatile *dramsim;
task *dramsim_cpu_clock;

extern int DRAMSim = 1;
void *DRAM_object_ptr; /*pointers to DRAMSim memory objects*/
//star todo move this to INI file.
/*4GB models...
 * DDR3_micron_8M_8B_x16_sg15.ini
 * DDR3_micron_32M_8B_x8_sg25E.ini
 * DDR3_micron_32M_8B_x8_sg15.ini
 * DDR3_micron_32M_8B_x4_sg15.ini
 * DDR3_micron_32M_8B_x4_sg125.ini
 * DDR3_micron_16M_8B_x8_sg15.ini*/
char dramsim_ddr_config_path[250] = "/home/stardica/Desktop/DRAMSim2/ini/DDR3_micron_32M_8B_x8_sg25E.ini";
char dramsim_system_config_path[250] = "/home/stardica/Desktop/DRAMSim2/system.ini";
char dramsim_trace_config_path[250] = "/home/stardica/Desktop/DRAMSim2/traces";
char dramsim_vis_config_path[250] = "vis.out";
unsigned int mem_size = 4096;
unsigned int cpu_freq = 4000000000;



void memctrl_init(void){

	memctrl_create();
	memctrl_create_tasks();
	dram_init();

	return;
}

void memctrl_create(void){

	//one mem ctrl per CPU
	mem_ctrl = (void *) malloc(sizeof(struct mem_ctrl_t));

	return;
}

void dram_init(void){

	//print_dramsim();
	dramsim_start();
	dramsim_register_call_backs();
	dramsim_set_cpu_freq();


	return;
}

void print_dramsim(void){

	call_print_me();

	return;

}

int dramsim_add_transaction(bool read_write, unsigned int addr){

	int return_val = call_add_transaction(DRAM_object_ptr, read_write, addr);

	return return_val;
}


void dramsim_start(void){

	/*call requires -> char *dev, char *sys, char *pwd,  char *trc, unsigned int megsOfMemory, char *visfilename*/
	DRAM_object_ptr = (void *) call_get_memory_system_instance(dramsim_ddr_config_path, dramsim_system_config_path, dramsim_trace_config_path, "m2s-cgm", mem_size, dramsim_vis_config_path);

	printf("C side DRAM_object_ptr 0x%08x\n", DRAM_object_ptr);

	return;
}

void dramsim_set_cpu_freq(void){

	call_set_CPU_clock_speed(DRAM_object_ptr, cpu_freq);

	printf("C side freq set %d\n", cpu_freq);

	return;
}

void dramsim_register_call_backs(void){

	call_register_call_backs(DRAM_object_ptr, dramsim_read_complete, dramsim_write_complete, dramsim_power_callback);

	return;
}

/* callback functors */
void dramsim_read_complete(unsigned id, long long address, long long clock_cycle)
{
	fatal("[Callback on M2S] read complete: addr 0x%08x cycle %llu\n", (unsigned int) address, P_TIME);

	return;
}

void dramsim_write_complete(unsigned id, long long address, long long clock_cycle)
{
	fatal("[Callback] write complete: %d 0x%016x cycle=%lu\n", id, address, clock_cycle);

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


//do some work.
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

void memctrl_create_tasks(void){

	char buff[100];

	mem_ctrl_ec = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl_ec");
	mem_ctrl_ec = new_eventcount(strdup(buff));

	mem_ctrl_io_ec = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl_io_ec");
	mem_ctrl_io_ec = new_eventcount(strdup(buff));

	mem_ctrl_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl_task");
	mem_ctrl_task = create_task(memctrl_ctrl, DEFAULT_STACK_SIZE, strdup(buff));

	mem_ctrl_io_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "mem_ctrl_io");
	mem_ctrl_io_task = create_task(memctrl_ctrl_io, DEFAULT_STACK_SIZE, strdup(buff));


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

int memctrl_can_access(void){

	//check if in queue is full
	if(QueueSize <= list_count(mem_ctrl->Rx_queue_top))
	{
		return 0;
	}

	//we can access the system agent
	return 1;
}

void memctrl_ctrl_io(void){

	int my_pid = mem_ctrl_io_pid;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(mem_ctrl_io_ec, step);
		step++;

		message_packet = list_dequeue(mem_ctrl->Tx_queue);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
		transfer_time = (message_packet->size/mem_ctrl->bus_width);

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		P_PAUSE(transfer_time);

		/*while(transfer_time > 0)
		{
			P_PAUSE(1);
			transfer_time--;
			//printf("Access_is %llu cycle %llu transfer %d\n", access_id, P_TIME, transfer_time);
		}*/

		list_enqueue(mem_ctrl->system_agent_queue, message_packet);
		advance(system_agent_ec);
	}
	return;
}

//do some work.
void memctrl_ctrl(void){

	int my_pid = mem_ctrl_pid;
	struct cgm_packet_t *message_packet;
	long long step = 1;

	long long access_id = 0;
	enum cgm_access_kind_t access_type;
	unsigned int addr;

	//for accessing the memory image.
	unsigned char buffer[20];
	unsigned char *buffer_ptr;

	int i = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		//printf("mem_ctrl\n");
		await(mem_ctrl_ec, step);

		if(!sys_agent_can_access_bottom())
		{
			printf("MC stalling up\n");
			P_PAUSE(1);
		}
		else
		{
			step++;

			//star todo connect up DRAMsim here.
			message_packet = list_dequeue(mem_ctrl->Rx_queue_top);
			assert(message_packet);

			//access_type = message_packet->access_type;
			access_id = message_packet->access_id;
			addr = message_packet->address;
			access_type = message_packet->access_type;

			CGM_DEBUG(memctrl_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u\n",
					mem_ctrl->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr);

			P_PAUSE(mem_ctrl->DRAM_latency);

			/*****NOTE!!*****/
			/*the memory image is entirely based on the ELF's provided virtual addresses
			to access the memory image from the memory controller you first have to do a quick
			swap back to the virtual address. This is just a simulator-ism. In the real world
			the real physical address would be used at this point to gather data.*/

			if(message_packet->access_type == cgm_access_mc_store)
			{
				/*the message is a store message (Write Back) from a L3 cache
				for now charge the latency for the store, then, just destroy the packet*/

				dramsim_add_transaction(true, message_packet->address);

				message_packet = list_remove(mem_ctrl->Rx_queue_top, message_packet);
				free(message_packet);
			}
			else if(message_packet->access_type == cgm_access_mc_load)
			{
				/*This is a L3 load request (cached memory system miss)
				charge the latency for the load, then, reply with data*/

				/*buffer_ptr = mem_get_buffer(mem_ctrl->mem, mmu_get_vtladdr(0, message_packet->address), 20, mem_access_read);

				if (!buffer_ptr)
				{
					 Disable safe mode. If a part of the 20 read bytes does not belong to the
					 actual instruction, and they lie on a page with no permissions, this would
					 generate an undesired protection fault.
					mem_ctrl->mem->safe = 0;
					buffer_ptr = buffer;
					mem_access(mem_ctrl->mem, mmu_get_vtladdr(0, message_packet->address), 20, buffer_ptr, mem_access_read);
				}

				mem_ctrl->mem->safe = mem_safe_mode;

				for(i = 0; i < 20; i++)
				{
					printf("buffer 0x%02x\n", *buffer_ptr);
					buffer_ptr++;
				}
				printf(" blah blah blah!!! address 0x%08x\n", mmu_get_vtladdr(0, message_packet->address)),
				getchar();*/


				printf("reading from memory cycle %llu\n", P_TIME);
				dramsim_add_transaction(false, message_packet->address);


				//set the access type
				message_packet->access_type = cgm_access_mc_put;
				message_packet->size = l3_caches[0].block_size;

				//reply to L3
				list_enqueue(mem_ctrl->Tx_queue, message_packet);
				advance(mem_ctrl_io_ec);
			}
		}
	}
	return;
}
