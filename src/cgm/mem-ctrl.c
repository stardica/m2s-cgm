/*
 * mem-ctrl.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */

#include <cgm/mem-ctrl.h>


//structure declarations
struct mem_ctrl_t *mem_ctrl;
eventcount volatile *mem_ctrl_ec;
eventcount volatile *mem_ctrl_io_ec;
task *mem_ctrl_task;
task *mem_ctrl_io_task;
int mem_ctrl_pid = 0;
int mem_ctrl_io_pid = 0;

void memctrl_init(void){

	memctrl_create();
	memctrl_create_tasks();

	return;
}

void memctrl_create(void){

	//one mem ctrl per CPU
	mem_ctrl = (void *) malloc(sizeof(struct mem_ctrl_t));

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

	/*long long access_id = 0;
	enum cgm_access_kind_t access_type;
	unsigned int addr;*/

	/*****NOTE!!*****/
	/*the memory image is entirely based on the ELF's provided virtual addresses
	to access the memory image from the memory controller you first have to do a quick
	swap back to the virtual address. This is just a simulator-ism. In the real world
	the real physical address would be used at this point to gather data.*/

	/*unsigned char buffer[20];
	unsigned char *buffer_ptr;

	buffer_ptr = mem_get_buffer(mem_ctrl->mem, mmu_get_vtladdr(0, message_packet->address), 20, mem_access_read);

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
	}*/

	/*int i = 0;*/

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(mem_ctrl_ec, step);

		if(list_count(mem_ctrl->pending_accesses) >= 32)	//!sys_agent_can_access_bottom())
		{
			printf("MC stalling dram ctrl full cycle %llu\n", P_TIME);
			P_PAUSE(1);
		}
		else
		{
			step++;

			message_packet = list_get(mem_ctrl->Rx_queue_top, 0);
			assert(message_packet);

			if(message_packet->access_type == cgm_access_mc_store)
			{
				/*the message is a store message (Write Back) from a L3 cache
				for now charge the latency for the store, then, just destroy the packet*/

				if(DRAMSim == 1)
				{
					if(dramsim_add_transaction(message_packet->access_type, message_packet->address))
					{
						message_packet = list_remove(mem_ctrl->Rx_queue_top, message_packet);
						list_enqueue(mem_ctrl->pending_accesses, message_packet);
					}
					else
					{
						printf("MC stalling dram ctrl busy?? %llu\n", P_TIME);
						step--;
					}
				}
				else
				{
					P_PAUSE(mem_ctrl->DRAM_latency);
					message_packet = list_remove(mem_ctrl->Rx_queue_top, message_packet);
					free(message_packet);
				}
			}
			else if(message_packet->access_type == cgm_access_mc_load)
			{
				/*This is a L3 load request (cached memory system miss)
				charge the latency for the load, then, reply with data*/

				if(DRAMSim == 1)
				{
					//printf("C side reading from memory access id %llu addr 0x%08x cycle %llu\n", message_packet->access_id, message_packet->address, P_TIME);
					if(dramsim_add_transaction(message_packet->access_type, message_packet->address))
					{
						//printf("MC access addr 0x%08x cycle%llu\n", message_packet->address, P_TIME);
						message_packet = list_remove(mem_ctrl->Rx_queue_top, message_packet);
						list_enqueue(mem_ctrl->pending_accesses, message_packet);
					}
					else
					{
						//dram ctrl in queue is full, wait and try again.
						printf("MC stalling dram ctrl busy?? %llu\n", P_TIME);
						step--;
					}
				}
				else
				{
					P_PAUSE(mem_ctrl->DRAM_latency);

					//set the access type
					message_packet->access_type = cgm_access_mc_put;
					message_packet->size = l3_caches[0].block_size;

					//reply to L3
					message_packet = list_remove(mem_ctrl->Rx_queue_top, message_packet);
					list_enqueue(mem_ctrl->Tx_queue, message_packet);
					advance(mem_ctrl_io_ec);
				}
			}
		}
	}
	return;
}
