
#include <cgm/sys-agent.h>

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/list.h>
#include <lib/util/string.h>

#include <cgm/cgm.h>
#include <cgm/sys-agent.h>
#include <cgm/mem-ctrl.h>
#include <cgm/tasking.h>
#include <cgm/cache.h>
#include <cgm/packet.h>
#include <cgm/switch.h>
#include <cgm/protocol.h>
*/


struct system_agent_t *system_agent;

int system_agent_pid = 0;
int system_agent_io_up_pid = 0;
int system_agent_io_down_pid = 0;

eventcount volatile *system_agent_ec;
task *system_agent_task;

eventcount volatile *system_agent_io_down_ec;
task *system_agent_io_down_task;

eventcount volatile *system_agent_io_up_ec;
task *system_agent_io_up_task;


void sys_agent_init(void){

	sys_agent_create();
	sys_agent_create_tasks();

	return;
}

void sys_agent_create(void){

	system_agent = (void*) malloc(sizeof(struct system_agent_t));

	return;
}

void sys_agent_create_tasks(void){

	char buff[100];

	//CTRL
	system_agent_ec = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_ec");
	system_agent_ec = new_eventcount(strdup(buff));

	system_agent_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_ctrl");
	system_agent_task = create_task(sys_agent_ctrl, DEFAULT_STACK_SIZE, strdup(buff));


	//IO
	system_agent_io_up_ec = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_io_up");
	system_agent_io_up_ec = new_eventcount(strdup(buff));

	system_agent_io_down_ec = (void *) calloc(1, sizeof(eventcount));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_io_down");
	system_agent_io_down_ec = new_eventcount(strdup(buff));

	system_agent_io_up_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_io_down_task");
	system_agent_io_up_task = create_task(sys_agent_ctrl_io_up, DEFAULT_STACK_SIZE, strdup(buff));

	system_agent_io_down_task = (void *) calloc(1, sizeof(task));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "system_agent_io_down_task");
	system_agent_io_down_task = create_task(sys_agent_ctrl_io_down, DEFAULT_STACK_SIZE, strdup(buff));

	return;
}


int sys_agent_can_access_top(void){

	//check if in queue is full
	if(QueueSize <= list_count(system_agent->Rx_queue_top))
	{
		return 0;
	}

	//we can access the system agent
	return 1;
}

int sys_agent_can_access_bottom(void){

	//check if in queue is full
	if(QueueSize <= list_count(system_agent->Rx_queue_bottom))
	{
		return 0;
	}

	//we can access the system agent
	return 1;
}


struct cgm_packet_t *sysagent_get_message(void){

	//star this is round robin
	struct cgm_packet_t *new_message;

	//star todo to give priority stay on a particular queue as long as it is not empty;

	new_message = list_get(system_agent->next_queue, 0);

	//keep pointer to last queue
	system_agent->last_queue = system_agent->next_queue;

	//rotate the queues
	if(system_agent->next_queue == system_agent->Rx_queue_top)
	{
		system_agent->next_queue = system_agent->Rx_queue_bottom;
	}
	else if(system_agent->next_queue == system_agent->Rx_queue_bottom)
	{
		system_agent->next_queue = system_agent->Rx_queue_top;
	}
	else
	{
		fatal("get_message() pointers arn't working");
	}

		//if we didn't get a message try again (now that the queues are rotated)
	if(new_message == NULL)
	{

		new_message = list_get(system_agent->next_queue, 0);

		//keep pointer to last queue
		system_agent->last_queue = system_agent->next_queue;
		//rotate the queues.
			if(system_agent->next_queue == system_agent->Rx_queue_top)
			{
				system_agent->next_queue = system_agent->Rx_queue_bottom;
			}
			else if(system_agent->next_queue == system_agent->Rx_queue_bottom)
			{
				system_agent->next_queue = system_agent->Rx_queue_top;
			}
			else
			{
				fatal("get_message() pointers arn't working");
			}
	}

	//shouldn't be exiting without a message
	assert(new_message != NULL);
	return new_message;
}

void system_agent_route(struct cgm_packet_t *message_packet){

	enum cgm_access_kind_t access_type;
	access_type = message_packet->access_type;

	int queue_depth = 0;

	SYSTEM_PAUSE(system_agent->latency);

	if(access_type == cgm_access_mc_load || access_type == cgm_access_mc_store
			|| access_type == cgm_access_cpu_flush_ack || access_type == cgm_access_gpu_flush_ack)
	{

		if(list_count(system_agent->Tx_queue_bottom) > QueueSize)
			warning("SA south TX queue size %d\n", list_count(system_agent->Tx_queue_bottom));

		message_packet = list_remove(system_agent->last_queue, message_packet);
		list_enqueue(system_agent->Tx_queue_bottom, message_packet);
		advance(system_agent_io_down_ec);

		DEBUGSYS(SYSTEM == 1, "block 0x%08x %s routing to MC ID %llu type %d cycle %llu\n",
				(message_packet->address & ~mem_ctrl->block_mask), system_agent->name, message_packet->access_id, message_packet->access_type, P_TIME);

		/*if((((message_packet->address & ~mem_ctrl->block_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
		{
			if(SYSTEM == 1)
			{
				printf("block 0x%08x %s routing to MC ID %llu type %d cycle %llu\n",
						(message_packet->address & ~mem_ctrl->block_mask), system_agent->name, message_packet->access_id, message_packet->access_type, P_TIME);
			}
		}*/

		/*stats*/
		if(access_type == cgm_access_mc_load)
			system_agent->mc_loads++;

		if(access_type == cgm_access_mc_store)
			system_agent->mc_stores++;

		/*running ave = ((old count * old data) + next data) / next count*/
		system_agent->south_puts++;
		queue_depth = list_count(system_agent->Tx_queue_bottom);
		system_agent->ave_south_txqueue_depth = ((((double) system_agent->south_puts - 1) * system_agent->ave_south_txqueue_depth) + (double) queue_depth) / (double) system_agent->south_puts;

		if(system_agent->max_south_txqueue_depth < list_count(system_agent->Tx_queue_bottom))
				system_agent->max_south_txqueue_depth = list_count(system_agent->Tx_queue_bottom);

	}
	else if(access_type == cgm_access_mc_put)
	{


		//set the dest and sources
		//message_packet->access_type = cgm_access_put;
		message_packet->dest_id = message_packet->src_id;
		message_packet->dest_name = message_packet->src_name;
		message_packet->src_name = system_agent->name;
		message_packet->src_id = str_map_string(node_strn_map, system_agent->name);


		if(list_count(system_agent->Tx_queue_top) > QueueSize)
			warning("SA north TX queue size %d\n", list_count(system_agent->Tx_queue_top));

		//success
		message_packet = list_remove(system_agent->last_queue, message_packet);
		list_enqueue(system_agent->Tx_queue_top, message_packet);
		advance(system_agent_io_up_ec);

		DEBUGSYS(SYSTEM == 1, "block 0x%08x %s routing from MC ID %llu type %d cycle %llu\n",
				(message_packet->address & ~mem_ctrl->block_mask), system_agent->name, message_packet->access_id, message_packet->access_type, P_TIME);

		/*if((((message_packet->address & ~mem_ctrl->block_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
		{
			if(SYSTEM == 1)
			{
				printf("block 0x%08x %s routing from MC ID %llu type %d cycle %llu\n",
						(message_packet->address & ~mem_ctrl->block_mask), system_agent->name, message_packet->access_id, message_packet->access_type, P_TIME);
			}
		}*/

		/*stats*/
		system_agent->mc_returns++;

		system_agent->north_puts++;
		queue_depth = list_count(system_agent->Tx_queue_top);
		system_agent->ave_north_txqueue_depth = ((((double) system_agent->north_puts - 1) * system_agent->ave_north_txqueue_depth) + (double) queue_depth) / (double) system_agent->north_puts;

		if(system_agent->max_north_txqueue_depth < list_count(system_agent->Tx_queue_top))
				system_agent->max_north_txqueue_depth = list_count(system_agent->Tx_queue_top);

	}
	else
	{
		fatal("system_agent_route(): access type %d \n", message_packet->access_type);

	}

	return;
}

void sys_dir_lookup(struct cgm_packet_t *message_packet){


	return;
}

void sys_mem_access(struct cgm_packet_t *message_packet){


	return;
}


void sys_agent_ctrl_io_up(void){

	int my_pid = system_agent_io_up_pid;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;

	long long occ_start = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(system_agent_io_up_ec, step);

		/*stats*/
		occ_start = P_TIME;

		message_packet = list_get(system_agent->Tx_queue_top, 0);
		assert(message_packet);

		if(list_count(switches[system_agent->switch_id].south_rx_reply_queue) > QueueSize)
		{
			//warning("SA stalling tx_bottom %d tx_top %d cycle %llu\n", list_count(system_agent->Tx_queue_top), list_count(system_agent->Tx_queue_bottom), P_TIME);
			SYSTEM_PAUSE(1);
		}
		else
		{

			step++;

			/*access_id = message_packet->access_id;*/
			transfer_time = (message_packet->size/system_agent->up_bus_width);

			if(transfer_time == 0)
				transfer_time = 1;

			SYSTEM_PAUSE(transfer_time);

			if(list_count(switches[system_agent->switch_id].south_rx_reply_queue) > QueueSize)
				warning("%s size %d\n", switches[system_agent->switch_id].south_rx_reply_queue->name, list_count(mem_ctrl->Rx_queue_top));

			system_agent->north_io_busy_cycles += (transfer_time + 1);

			//list_enqueue(system_agent->switch_queue, message_packet);
			//advance(&switches_ec[system_agent->switch_id]);
			message_packet = list_remove(system_agent->Tx_queue_top, message_packet);
			list_enqueue(switches[system_agent->switch_id].south_rx_reply_queue, message_packet);
			advance(&switches_ec[system_agent->switch_id]);

			//printf("made it here sw id %d cycle %llu\n", system_agent->switch_id, P_TIME);
			//cache_dump_queue(switches[system_agent->switch_id].south_rx_reply_queue);
			//printf("\n\n");

		}

		/*stats occupancy*/
		system_agent->up_io_occupance += (P_TIME - occ_start);

	}

	fatal("sys_agent_ctrl_io_up(): out of while loop\n");

	return;
}

void sys_agent_ctrl_io_down(void){

	int my_pid = system_agent_io_down_pid;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;

	long long occ_start = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(system_agent_io_down_ec, step);

		/*stats*/
		occ_start = P_TIME;

		if(list_count(mem_ctrl->Rx_queue_top) >= QueueSize)
		{
			//warning("SA stalling tx_bottom %d tx_top %d cycle %llu\n", list_count(system_agent->Tx_queue_top), list_count(system_agent->Tx_queue_bottom), P_TIME);
			SYSTEM_PAUSE(1);
		}
		else
		{
			step++;

			message_packet = list_dequeue(system_agent->Tx_queue_bottom);
			assert(message_packet);

			transfer_time = (message_packet->size/system_agent->down_bus_width);

			if(transfer_time == 0)
			{
				transfer_time = 1;
			}

			SYSTEM_PAUSE(transfer_time);

			DEBUGSYS(SYSTEM == 1, "block 0x%08x %s IO to MC transfer finish time %d ID %llu type %d cycle %llu\n",
					(message_packet->address & ~mem_ctrl->block_mask), system_agent->name, transfer_time * SYSTEM_LATENCY_FACTOR, message_packet->access_id, message_packet->access_type, P_TIME);

			/*if((((message_packet->address & ~mem_ctrl->block_mask) == WATCHBLOCK) && WATCHLINE) || DUMP)
			{
				if(SYSTEM == 1)
				{
					printf("block 0x%08x %s IO to MC transfer finish time %d ID %llu type %d cycle %llu\n",
							(message_packet->address & ~mem_ctrl->block_mask), system_agent->name, transfer_time * SYSTEM_LATENCY_FACTOR, message_packet->access_id, message_packet->access_type, P_TIME);
				}
			}*/

			if(list_count(mem_ctrl->Rx_queue_top) > QueueSize)
				warning("mem ctrl rx queue size %d\n", list_count(mem_ctrl->Rx_queue_top));

			system_agent->south_io_busy_cycles += (transfer_time + 1);

			list_enqueue(mem_ctrl->Rx_queue_top, message_packet);
			advance(mem_ctrl_ec);
		}

		/*stats occupancy*/
		system_agent->down_io_occupance += (P_TIME - occ_start);

	}

	fatal("sys_agent_ctrl_io_down(): out of while loop\n");

	return;
}


void sys_agent_ctrl(void){

	int my_pid = system_agent_pid++;
	struct cgm_packet_t *message_packet;
	long long step = 1;
	//int queue_depth = 0;

	long long occ_start = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(system_agent_ec, step);

		occ_start = P_TIME;

		if(list_count(system_agent->Tx_queue_bottom) >= QueueSize || list_count(system_agent->Tx_queue_top) >= QueueSize)
		{
			//warning("SA stalling tx_bottom %d tx_top %d cycle %llu\n", list_count(system_agent->Tx_queue_top), list_count(system_agent->Tx_queue_bottom), P_TIME);

			SYSTEM_PAUSE(1);
		}
		else
		{
			step++;

			//if we are here there should be a message in the queue
			message_packet = sysagent_get_message();
			assert(message_packet);

			if(simple_mem == 1)
				fatal("sys_agent_ctrl(): sa running, but simple_mem set 1 cycle %llu\n", P_TIME);

			system_agent_route(message_packet);

			/*stats*/
			system_agent->busy_cycles += (system_agent->latency + 1);
		}

		/*stats occupancy*/
		system_agent->occupance += (P_TIME - occ_start);

	}

	fatal("sys_agent_ctrl task is broken\n");
	return;
}

void sys_agent_store_stats(struct cgm_stats_t *cgm_stat_container){

	//system agent
	cgm_stat_container->system_agent_occupance = system_agent->occupance;

	cgm_stat_container->system_agent_busy_cycles = system_agent->busy_cycles;
	cgm_stat_container->system_agent_north_io_busy_cycles = system_agent->north_io_busy_cycles;
	cgm_stat_container->system_agent_south_io_busy_cycles = system_agent->south_io_busy_cycles;
	cgm_stat_container->system_agent_mc_loads = system_agent->mc_loads;
	cgm_stat_container->system_agent_mc_stores = system_agent->mc_stores;
	cgm_stat_container->system_agent_mc_returns = system_agent->mc_returns;
	cgm_stat_container->system_agent_max_north_rxqueue_depth = system_agent->max_north_rxqueue_depth;
	cgm_stat_container->system_agent_ave_north_rxqueue_depth = system_agent->ave_north_rxqueue_depth;
	cgm_stat_container->system_agent_max_south_rxqueue_depth = system_agent->max_south_rxqueue_depth;
	cgm_stat_container->system_agent_ave_south_rxqueue_depth = system_agent->ave_south_rxqueue_depth;
	cgm_stat_container->system_agent_max_north_txqueue_depth = system_agent->max_north_txqueue_depth;
	cgm_stat_container->system_agent_ave_north_txqueue_depth = system_agent->ave_north_txqueue_depth;
	cgm_stat_container->system_agent_max_south_txqueue_depth = system_agent->max_south_txqueue_depth;
	cgm_stat_container->system_agent_ave_south_txqueue_depth = system_agent->ave_south_txqueue_depth;
	/*cgm_stat_container->system_agent_north_gets = system_agent->north_gets;
	cgm_stat_container->system_agent_south_gets = system_agent->south_gets;
	cgm_stat_container->system_agent_north_puts = system_agent->north_puts;
	cgm_stat_container->system_agent_south_puts = system_agent->south_puts;*/

	//IO ctrl
	cgm_stat_container->system_agent_up_io_occupance = system_agent->up_io_occupance;
	cgm_stat_container->system_agent_down_io_occupance = system_agent->down_io_occupance;

	return;
}

void sys_agent_reset_stats(void){

	//system agent
	system_agent->occupance = 0;

	system_agent->busy_cycles = 0;
	system_agent->north_io_busy_cycles = 0;
	system_agent->south_io_busy_cycles = 0;
	system_agent->mc_loads = 0;
	system_agent->mc_stores = 0;
	system_agent->mc_returns = 0;
	system_agent->max_north_rxqueue_depth = 0;
	system_agent->ave_north_rxqueue_depth = 0;
	system_agent->max_south_rxqueue_depth = 0;
	system_agent->ave_south_rxqueue_depth = 0;
	system_agent->max_north_txqueue_depth = 0;
	system_agent->ave_north_txqueue_depth = 0;
	system_agent->max_south_txqueue_depth = 0;
	system_agent->ave_south_txqueue_depth = 0;
	system_agent->north_gets = 0;
	system_agent->south_gets = 0;
	system_agent->north_puts = 0;
	system_agent->south_puts = 0;

	system_agent->up_io_occupance = 0;
	system_agent->down_io_occupance = 0;

	return;
}

void sys_agent_dump_stats(struct cgm_stats_t *cgm_stat_container){

	/*CGM_STATS(cgm_stats_file, "[SystemAgent]\n");*/

	CGM_STATS(cgm_stats_file, "sa_Occupancy = %llu\n", cgm_stat_container->system_agent_occupance);
	CGM_STATS(cgm_stats_file, "sa_IOUpOccupancy = %llu\n", cgm_stat_container->system_agent_up_io_occupance);
	CGM_STATS(cgm_stats_file, "sa_IODownOccupancy = %llu\n", cgm_stat_container->system_agent_down_io_occupance);

	if(cgm_stat_container->stats_type == systemStats)
	{
		CGM_STATS(cgm_stats_file, "sa_OccupancyPct = %0.6f\n", ((double) cgm_stat_container->system_agent_occupance/(double) P_TIME));
	}
	else if (cgm_stat_container->stats_type == parallelSection)
	{
		CGM_STATS(cgm_stats_file, "sa_OccupancyPct = %0.6f\n", (((double) cgm_stat_container->system_agent_occupance)/((double) cgm_stat_container->total_parallel_section_cycles)));
		CGM_STATS(cgm_stats_file, "sa_IOUpOccupancyPct = %0.6f\n", (((double) cgm_stat_container->system_agent_up_io_occupance)/((double) cgm_stat_container->total_parallel_section_cycles)));
		CGM_STATS(cgm_stats_file, "sa_IODownOccupancyPct = %0.6f\n", (((double) cgm_stat_container->system_agent_down_io_occupance)/((double) cgm_stat_container->total_parallel_section_cycles)));
	}
	else
	{
		fatal("sys_agent_dump_stats(): bad container type\n");
	}

	CGM_STATS(cgm_stats_file, "sa_TotalCtrlLoops = %llu\n", cgm_stat_container->system_agent_busy_cycles);
	CGM_STATS(cgm_stats_file, "sa_MCLoads = %llu\n", cgm_stat_container->system_agent_mc_loads);
	CGM_STATS(cgm_stats_file, "sa_MCStores = %llu\n", cgm_stat_container->system_agent_mc_stores);
	CGM_STATS(cgm_stats_file, "sa_MCReturns = %llu\n", cgm_stat_container->system_agent_mc_returns);
	CGM_STATS(cgm_stats_file, "sa_NorthIOBusyCycles = %llu\n", cgm_stat_container->system_agent_north_io_busy_cycles);
	CGM_STATS(cgm_stats_file, "sa_SouthIOBusyCycles = %llu\n", cgm_stat_container->system_agent_south_io_busy_cycles);
	CGM_STATS(cgm_stats_file, "sa_MaxNorthRxQueueDepth = %d\n", cgm_stat_container->system_agent_max_north_rxqueue_depth);
	CGM_STATS(cgm_stats_file, "sa_AveNorthRxQueueDepth = %0.2f\n", cgm_stat_container->system_agent_ave_north_rxqueue_depth);
	CGM_STATS(cgm_stats_file, "sa_MaxSouthRxQueueDepth = %d\n", cgm_stat_container->system_agent_max_south_rxqueue_depth);
	CGM_STATS(cgm_stats_file, "sa_AveSouthRxQueueDepth = %0.2f\n", cgm_stat_container->system_agent_ave_south_rxqueue_depth);
	CGM_STATS(cgm_stats_file, "sa_MaxNorthTxQueueDepth = %d\n", cgm_stat_container->system_agent_max_north_txqueue_depth);
	CGM_STATS(cgm_stats_file, "sa_AveNorthTxQueueDepth = %0.2f\n", cgm_stat_container->system_agent_ave_north_txqueue_depth);
	CGM_STATS(cgm_stats_file, "sa_MaxSouthTxQueueDepth = %d\n", cgm_stat_container->system_agent_max_south_txqueue_depth);
	CGM_STATS(cgm_stats_file, "sa_AveSouthTxQueueDepth = %0.2f\n", cgm_stat_container->system_agent_ave_south_txqueue_depth);
	/*CGM_STATS(cgm_stats_file, "\n");*/

	return;
}
