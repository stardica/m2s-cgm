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
	int queue_depth = 0;

	long long occ_start = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(mem_ctrl_io_ec, step);

		/*stats*/
		occ_start = P_TIME;

		if(list_count(mem_ctrl->system_agent_queue) >= QueueSize)
		{
			//warning("SA stalling tx_bottom %d tx_top %d cycle %llu\n", list_count(system_agent->Tx_queue_top), list_count(system_agent->Tx_queue_bottom), P_TIME);
			SYSTEM_PAUSE(1);
		}
		else
		{
			step++;

			/*stats*/
			if(mem_ctrl->tx_max < list_count(mem_ctrl->Tx_queue_top))
				mem_ctrl->tx_max = list_count(mem_ctrl->Tx_queue_top);

			message_packet = list_dequeue(mem_ctrl->Tx_queue_top);
			assert(message_packet);

			/*access_id = message_packet->access_id;*/
			transfer_time = (message_packet->size/mem_ctrl->bus_width);

			if(transfer_time == 0)
			{
				transfer_time = 1;
			}

			SYSTEM_PAUSE(transfer_time);

			mem_ctrl->io_busy_cycles += (transfer_time + 1);

			if(message_packet->access_type == cgm_access_mc_put)
				mem_ctrl->bytes_read += message_packet->size;

			if(list_count(mem_ctrl->system_agent_queue) > QueueSize)
				warning("%s size %d\n", mem_ctrl->system_agent_queue->name, list_count(mem_ctrl->system_agent_queue));

			list_enqueue(mem_ctrl->system_agent_queue, message_packet);
			advance(system_agent_ec);

			/*stats*/

			if(system_agent->max_south_rxqueue_depth < list_count(system_agent->Rx_queue_bottom))
					system_agent->max_south_rxqueue_depth = list_count(system_agent->Rx_queue_bottom);

			/*running ave = ((old count * old data) + next data) / next count*/
			system_agent->south_gets++;
			queue_depth = list_count(system_agent->Rx_queue_bottom);
			system_agent->ave_south_rxqueue_depth = ((((double) system_agent->south_gets - 1) * system_agent->ave_south_rxqueue_depth) + (double) queue_depth) / (double) system_agent->south_gets;
		}

		/*stats occupancy*/
		mem_ctrl->up_io_occupance += (P_TIME - occ_start);

	}

	fatal("memctrl_ctrl_io(): out of while loop\n");

	return;
}

long long flushes_rx = 0;

//do some work.
void memctrl_ctrl(void){

	int my_pid = mem_ctrl_pid;
	struct cgm_packet_t *message_packet;
	long long step = 1;

	int queue_depth = 0;
	long long num_inserts = 0;
	long long occ_start = 0;

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

	int num_accesses = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(mem_ctrl_ec, step);

		occ_start = P_TIME;

		num_accesses = list_count(mem_ctrl->pending_accesses) + list_count(mem_ctrl->Tx_queue_top);

		if(num_accesses >= 32)	//!sys_agent_can_access_bottom())
		{
			//warning("MC stalling dram ctrl full cycle size %d %llu\n", list_count(mem_ctrl->pending_accesses), P_TIME);
			SYSTEM_PAUSE(1);
		}
		else
		{
			step++;

			SYSTEM_PAUSE(mem_ctrl->latency);

			message_packet = list_get(mem_ctrl->Rx_queue_top, 0);
			assert(message_packet);

			/*stats*/
			if(mem_ctrl->rx_max < list_count(mem_ctrl->Rx_queue_top))
				mem_ctrl->rx_max = list_count(mem_ctrl->Rx_queue_top);

			/*if (message_packet->access_type == cgm_access_cpu_flush_ack || message_packet->access_type == cgm_access_gpu_flush_ack)
					printf("flushes rx %llu\n", ++flushes_rx);*/

			if ((message_packet->access_type == cgm_access_cpu_flush_ack || message_packet->access_type == cgm_access_gpu_flush_ack)
					&& message_packet->cache_block_state == cgm_cache_block_invalid)
			{
				//Decrement the cores flush counter
				//l1_d_caches[message_packet->flush_core].flush_rx_counter++;
				l1_d_caches[message_packet->flush_core].flush_tx_counter--;

				/*if(message_packet->flush_core != 0)
					fatal("here 1\n");*/

				//warning("MC: flushes tx %llu flushes rx %llu core %d cycle %llu\n",
					//	l1_d_caches[message_packet->flush_core].flush_tx_counter, l1_d_caches[message_packet->flush_core].flush_rx_counter, message_packet->flush_core, P_TIME);

				message_packet = list_remove(mem_ctrl->Rx_queue_top, message_packet);
				packet_destroy(message_packet);
				continue;
			}

			if(message_packet->access_type == cgm_access_mc_store || message_packet->access_type == cgm_access_cpu_flush_ack
					|| message_packet->access_type == cgm_access_gpu_flush_ack)
			{
				/*the message is a store message (Write Back) from a L3 cache
				for now charge the latency for the store, then, just destroy the packet*/

				if(message_packet->size <= 8)
					fatal("here id %llu type %d blk 0x%08x cycle %llu\n",
							message_packet->access_id, message_packet->access_type, message_packet->address & l3_caches[0].block_address_mask, P_TIME);

				assert(message_packet->size >= 8);

				if(DRAMSim == 1)
				{
					if(dramsim_add_transaction(message_packet->access_type, GET_BLOCK(message_packet->address)))
					{
						/*stats*/
						message_packet->dram_start_cycle = P_TIME;
						mem_ctrl->bytes_wrote += message_packet->size;
						num_inserts++;

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
					SYSTEM_PAUSE(mem_ctrl->DRAM_latency);
					message_packet = list_remove(mem_ctrl->Rx_queue_top, message_packet);
					free(message_packet);
				}
			}
			else if(message_packet->access_type == cgm_access_mc_load)
			{
				/*This is a L3 load request (cached memory system miss)
				charge the latency for the load, then, reply with data*/

				assert(message_packet->size == 1);

				if(DRAMSim == 1)
				{
					//printf("C side reading from memory access id %llu addr 0x%08x cycle %llu\n", message_packet->access_id, message_packet->address, P_TIME);
					if(dramsim_add_transaction(message_packet->access_type, GET_BLOCK(message_packet->address)))
					{
						/*stats*/
						message_packet->dram_start_cycle = P_TIME;
						num_inserts++;

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
					SYSTEM_PAUSE(mem_ctrl->DRAM_latency);

					//set the access type
					message_packet->access_type = cgm_access_mc_put;
					message_packet->size = l3_caches[0].block_size;

					//reply to L3
					message_packet = list_remove(mem_ctrl->Rx_queue_top, message_packet);
					list_enqueue(mem_ctrl->Tx_queue_top, message_packet);
					advance(mem_ctrl_io_ec);
				}
			}
			else
			{
				fatal("memctrl_ctrl(): access type problem\n");
			}

			DEBUGSYS(SYSTEM == 1, "block 0x%08x %s DRAM access start ID %llu queue depth %d type %d cycle %llu\n",
					(message_packet->address & ~mem_ctrl->block_mask), mem_ctrl->name, message_packet->access_id,
					list_count(mem_ctrl->pending_accesses), message_packet->access_type, P_TIME);

			/*stats*/
			mem_ctrl->busy_cycles += (mem_ctrl->DRAM_latency + 1);

			if(mem_ctrl->dram_max_queue_depth < list_count(mem_ctrl->pending_accesses))
				mem_ctrl->dram_max_queue_depth = list_count(mem_ctrl->pending_accesses);

			/*running ave = ((old count * old data) + next data) / next count*/
			queue_depth = list_count(mem_ctrl->pending_accesses);
			mem_ctrl->dram_ave_queue_depth = ((((double) num_inserts - 1) * mem_ctrl->dram_ave_queue_depth) + (double) queue_depth) / (double) num_inserts;

		}

		/*stats occupancy*/
		mem_ctrl->occupance += (P_TIME - occ_start);

	}

	return;
}

void memctrl_store_stats(struct cgm_stats_t *cgm_stat_container){


	//Memory controller and DRAMSim
	cgm_stat_container->mem_ctrl_occupance = mem_ctrl->occupance;

	cgm_stat_container->mem_ctrl_busy_cycles = mem_ctrl->busy_cycles;
	cgm_stat_container->mem_ctrl_num_reads = mem_ctrl->num_reads;
	cgm_stat_container->mem_ctrl_num_writes = mem_ctrl->num_writes;
	cgm_stat_container->mem_ctrl_ave_dram_read_lat = mem_ctrl->ave_dram_read_lat;
	cgm_stat_container->mem_ctrl_ave_dram_write_lat = mem_ctrl->ave_dram_write_lat;
	cgm_stat_container->mem_ctrl_ave_dram_total_lat = mem_ctrl->ave_dram_total_lat;
	cgm_stat_container->mem_ctrl_read_min = mem_ctrl->read_min;
	cgm_stat_container->mem_ctrl_read_max = mem_ctrl->read_max;
	cgm_stat_container->mem_ctrl_write_min = mem_ctrl->write_min;
	cgm_stat_container->mem_ctrl_write_max = mem_ctrl->write_max;
	cgm_stat_container->mem_ctrl_dram_max_queue_depth = mem_ctrl->dram_max_queue_depth;
	cgm_stat_container->mem_ctrl_dram_ave_queue_depth = mem_ctrl->dram_ave_queue_depth;
	cgm_stat_container->mem_ctrl_dram_busy_cycles = mem_ctrl->dram_busy_cycles;
	cgm_stat_container->mem_ctrl_rx_max = mem_ctrl->rx_max;
	cgm_stat_container->mem_ctrl_tx_max = mem_ctrl->tx_max;
	cgm_stat_container->mem_ctrl_bytes_read = mem_ctrl->bytes_read;
	cgm_stat_container->mem_ctrl_bytes_wrote = mem_ctrl->bytes_wrote;
	cgm_stat_container->mem_ctrl_io_busy_cycles = mem_ctrl->io_busy_cycles;

	//IO Ctrl
	cgm_stat_container->mem_ctrl_up_io_occupance = mem_ctrl->up_io_occupance;

	return;
}

void memctrl_reset_stats(void){

	//Memory controller and DRAMSim
	mem_ctrl->occupance = 0;

	mem_ctrl->busy_cycles = 0;
	mem_ctrl->num_reads = 0;
	mem_ctrl->num_writes = 0;
	mem_ctrl->ave_dram_read_lat = 0;
	mem_ctrl->ave_dram_write_lat = 0;
	mem_ctrl->ave_dram_total_lat = 0;
	mem_ctrl->read_min = 0;
	mem_ctrl->read_max = 0;
	mem_ctrl->write_min = 0;
	mem_ctrl->write_max = 0;
	mem_ctrl->dram_max_queue_depth = 0;
	mem_ctrl->dram_ave_queue_depth = 0;
	mem_ctrl->dram_busy_cycles = 0;
	mem_ctrl->rx_max = 0;
	mem_ctrl->tx_max = 0;
	mem_ctrl->bytes_read = 0;
	mem_ctrl->bytes_wrote = 0;
	mem_ctrl->io_busy_cycles = 0;

	mem_ctrl->up_io_occupance = 0;

	return;
}

void memctrl_dump_stats(struct cgm_stats_t *cgm_stat_container){

	//cycles per nano second
	double ave_lat_ns = 0;

	/*if(DRAMSim == 0)
	{
		CGM_STATS(cgm_stats_file, "; Note DRAMsim is not connected! values are output as 0\n");
	}*/

	/*CGM_STATS(cgm_stats_file, "[MemCtrl]\n");*/
	CGM_STATS(cgm_stats_file, "mc_Occupancy = %llu\n", cgm_stat_container->mem_ctrl_occupance);
	CGM_STATS(cgm_stats_file, "mc_IOUpOccupancy = %llu\n", cgm_stat_container->mem_ctrl_up_io_occupance);
	if(cgm_stat_container->stats_type == systemStats)
	{
		CGM_STATS(cgm_stats_file, "mc_OccupancyPct = %0.6f\n", ((double) cgm_stat_container->mem_ctrl_occupance/(double) P_TIME));
	}
	else if (cgm_stat_container->stats_type == parallelSection)
	{
		CGM_STATS(cgm_stats_file, "mc_OccupancyPct = %0.6f\n", (((double) cgm_stat_container->mem_ctrl_occupance)/((double) cgm_stat_container->total_parallel_section_cycles)));
		CGM_STATS(cgm_stats_file, "mc_IOUpOccupancyPct = %0.6f\n", (((double) cgm_stat_container->mem_ctrl_up_io_occupance)/((double) cgm_stat_container->total_parallel_section_cycles)));
	}
	else
	{
		fatal("sys_agent_dump_stats(): bad container type\n");
	}



	CGM_STATS(cgm_stats_file, "mc_MemCtrlBusyCycles = %llu\n", cgm_stat_container->mem_ctrl_busy_cycles);
	CGM_STATS(cgm_stats_file, "mc_DramBusyCycles = %llu\n", cgm_stat_container->mem_ctrl_dram_busy_cycles);
	CGM_STATS(cgm_stats_file, "mc_TotalReads = %llu\n", cgm_stat_container->mem_ctrl_num_reads);
	CGM_STATS(cgm_stats_file, "mc_TotalWrites = %llu\n", cgm_stat_container->mem_ctrl_num_writes);

	if(DRAMSim == 1)
	{
		CGM_STATS(cgm_stats_file, "mc_AveDramReadLat = %.02f\n", cgm_stat_container->mem_ctrl_ave_dram_read_lat);
		CGM_STATS(cgm_stats_file, "mc_AveDramWriteLat = %.02f\n", cgm_stat_container->mem_ctrl_ave_dram_write_lat);
		CGM_STATS(cgm_stats_file, "mc_AveDramTotalLat(cycles) = %.02f\n", (cgm_stat_container->mem_ctrl_ave_dram_write_lat + cgm_stat_container->mem_ctrl_ave_dram_read_lat)/2);
		ave_lat_ns = (((cgm_stat_container->mem_ctrl_ave_dram_write_lat + cgm_stat_container->mem_ctrl_ave_dram_read_lat)/2) *GHZ) / cpu_freq;
		CGM_STATS(cgm_stats_file, "mc_AveDramTotalLat(ns) = %.02f\n", ave_lat_ns);
		CGM_STATS(cgm_stats_file, "mc_ReadMinLat = %llu\n", cgm_stat_container->mem_ctrl_read_min);
		CGM_STATS(cgm_stats_file, "mc_ReadMaxLat = %llu\n", cgm_stat_container->mem_ctrl_read_max);
		CGM_STATS(cgm_stats_file, "mc_WriteMinLat = %llu\n", cgm_stat_container->mem_ctrl_write_min);
		CGM_STATS(cgm_stats_file, "mc_WriteMaxLat = %llu\n", cgm_stat_container->mem_ctrl_write_max);
		CGM_STATS(cgm_stats_file, "mc_DramMaxQueueDepth = %llu\n", cgm_stat_container->mem_ctrl_dram_max_queue_depth);
		CGM_STATS(cgm_stats_file, "mc_DramAveQueueDepth = %.2f\n", cgm_stat_container->mem_ctrl_dram_ave_queue_depth);
		CGM_STATS(cgm_stats_file, "mc_RxMax = %llu\n", cgm_stat_container->mem_ctrl_rx_max);
		CGM_STATS(cgm_stats_file, "mc_TxMax = %llu\n", cgm_stat_container->mem_ctrl_tx_max);
		CGM_STATS(cgm_stats_file, "mc_ByteRead = %llu\n", cgm_stat_container->mem_ctrl_bytes_read);
		CGM_STATS(cgm_stats_file, "mc_BytesWrote = %llu\n", cgm_stat_container->mem_ctrl_bytes_wrote);
		CGM_STATS(cgm_stats_file, "mc_IOBusyCycles = %llu\n", cgm_stat_container->mem_ctrl_io_busy_cycles);
		/*CGM_STATS(cgm_stats_file, "\n");*/
	}
	else
	{
		CGM_STATS(cgm_stats_file, "mc_AveDramReadLat = %.02f\n", (float) 0);
		CGM_STATS(cgm_stats_file, "mc_AveDramWriteLat = %.02f\n", (float) 0);
		CGM_STATS(cgm_stats_file, "mc_AveDramTotalLat(cycles) = %.02f\n", (float) 0);
		CGM_STATS(cgm_stats_file, "mc_AveDramTotalLat(nanoseconds) = %.02f\n", (float) 0);
		CGM_STATS(cgm_stats_file, "mc_ReadMinLat = %llu\n", (long long) 0);
		CGM_STATS(cgm_stats_file, "mc_ReadMaxLat = %llu\n", (long long) 0);
		CGM_STATS(cgm_stats_file, "mc_WriteMinLat = %llu\n", (long long) 0);
		CGM_STATS(cgm_stats_file, "mc_WriteMaxLat = %llu\n", (long long) 0);
		CGM_STATS(cgm_stats_file, "mc_DramMaxQueueDepth = %llu\n", (long long) 0);
		CGM_STATS(cgm_stats_file, "mc_DramAveQueueDepth = %.2f\n", (float) 0);
		CGM_STATS(cgm_stats_file, "mc_RxMax = %llu\n", (long long) 0);
		CGM_STATS(cgm_stats_file, "mc_TxMax = %llu\n", (long long) 0);
		CGM_STATS(cgm_stats_file, "mc_ByteRead = %llu\n", (long long) 0);
		CGM_STATS(cgm_stats_file, "mc_BytesWrote = %llu\n", (long long) 0);
		CGM_STATS(cgm_stats_file, "mc_IOBusyCycles = %llu\n", (long long) 0);
		/*CGM_STATS(cgm_stats_file, "\n");*/
	}

	return;
}
