/*
 * IOMMU.c
 *
 *  Created on: May 2, 2015
 *      Author: stardica
 */



#include <cgm/hub-iommu.h>

/*
#include <stdio.h>
#include <stdlib.h>

#include <arch/si/timing/gpu.h>

#include <lib/util/list.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>

#include <cgm/cgm.h>
#include <cgm/tasking.h>
#include <cgm/hub-iommu.h>
#include <cgm/switch.h>
#include <cgm/packet.h>
*/


//max number of GPU hub queues 8:1
struct str_map_t Rx_queue_strn_map =
{ 	Rx_queue_num, {

		{ "hub_iommu.Rx_queue_top[0]", Rx_queue_top_0},
		{ "hub_iommu.Rx_queue_top[1]", Rx_queue_top_1},
		{ "hub_iommu.Rx_queue_top[2]", Rx_queue_top_2},
		{ "hub_iommu.Rx_queue_top[3]", Rx_queue_top_3},
		{ "hub_iommu.Rx_queue_top[4]", Rx_queue_top_4},
		{ "hub_iommu.Rx_queue_top[5]", Rx_queue_top_5},
		{ "hub_iommu.Rx_queue_top[6]", Rx_queue_top_6},
		{ "hub_iommu.Rx_queue_top[7]", Rx_queue_top_7},
		{ "hub_iommu.Rx_queue_bottom", Rx_queue_bottom},
		}
};

struct str_map_t Tx_queue_strn_map =
{ 	Rx_queue_num, {

		{ "hub_iommu.Tx_queue_top[0]", Tx_queue_top_0},
		{ "hub_iommu.Tx_queue_top[1]", Tx_queue_top_1},
		{ "hub_iommu.Tx_queue_top[2]", Tx_queue_top_2},
		{ "hub_iommu.Tx_queue_top[3]", Tx_queue_top_3},
		{ "hub_iommu.Tx_queue_top[4]", Tx_queue_top_4},
		{ "hub_iommu.Tx_queue_top[5]", Tx_queue_top_5},
		{ "hub_iommu.Tx_queue_top[6]", Tx_queue_top_6},
		{ "hub_iommu.Tx_queue_top[7]", Tx_queue_top_7},
		{ "hub_iommu.Tx_queue_bottom", Tx_queue_bottom},
		}
};


struct hub_iommu_t *hub_iommu;
eventcount volatile *hub_iommu_ec;
task *hub_iommu_tasks;
int hub_iommu_pid = 0;
int hub_iommu_io_up_pid = 0;
int hub_iommu_io_down_pid = 0;
int hub_iommu_connection_type = 0;


void hub_iommu_init(void){

	hub_iommu_create();

	//moved to configure to support ini configuration inputs.
	/*hub_iommu_create_tasks();*/

	return;
}

void hub_iommu_create(void){

	hub_iommu = (void *) calloc(1, sizeof(struct hub_iommu_t));

	return;
}

void hub_iommu_create_tasks(void (*func)(void)){

	char buff[100];
	//int i = 0;

	hub_iommu_ec = (void *) calloc(1, sizeof(eventcount));
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "hub_iommu");
	hub_iommu_ec = new_eventcount(strdup(buff));


	hub_iommu_tasks = (void *) calloc(1, sizeof(task));
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "hub_iommu_ctrl");
	hub_iommu_tasks = create_task(func, DEFAULT_STACK_SIZE, strdup(buff));

	return;
}

int hub_iommu_can_access(struct list_t *queue){

	//check if in queue is full
	if(QueueSize <= list_count(queue))
	{
		return 0;
	}

	return 1;
}

struct cgm_packet_t *hub_iommu_get_from_queue(void){

	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	int current_queue_num = -1;

	struct cgm_packet_t *new_message;

	//rotate the queues, of which we don't really know how many we have until runtime.
	//current_queue_num = str_map_string(&queue_strn_map, hub_iommu->last_queue->name);
	//at least one of the queues has a message packet.

	do
	{
		//round robin
		new_message = list_get(hub_iommu->next_queue, 0);

		//keep pointer to last queue
		hub_iommu->last_queue = hub_iommu->next_queue;

		//rotate
		current_queue_num = str_map_string(&Rx_queue_strn_map, hub_iommu->next_queue->name);

		if(current_queue_num < (gpu_group_cache_num - 1))
		{
			//go to next queue
			current_queue_num++;
			hub_iommu->next_queue = hub_iommu->Rx_queue_top[current_queue_num];
		}
		else if(current_queue_num == (gpu_group_cache_num - 1))
		{
			//at the last top queue
			hub_iommu->next_queue = hub_iommu->Rx_queue_bottom;
		}
		else if(current_queue_num == gpu_group_cache_num || current_queue_num == (Rx_queue_num - 1))
		{
			//start at beginning.
			hub_iommu->next_queue = hub_iommu->Rx_queue_top[0];
		}
		else
		{
			fatal("hub_iommu_get_from_queue(): unexpected queuing behavior queue num %d \n", current_queue_num);
		}

	}while(new_message == NULL);

	CGM_DEBUG(hub_iommu_debug_file, "%s access_id %llu cycle %llu pulled from %s queue size %d\n",
			hub_iommu->name, new_message->access_id, P_TIME,hub_iommu->last_queue->name, list_count(hub_iommu->last_queue));

	//shouldn't be exiting without a message
	return new_message;
}

/*int hub_iommu_put_next_queue_L3(struct cgm_packet_t *message_packet){

	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	int last_queue_num = -1;
	int l2_src_id = -1;

	struct cache_t *l3_cache_ptr = NULL;

	//get the number of the last queue
	last_queue_num = str_map_string(&Rx_queue_strn_map, hub_iommu->last_queue->name);

	//if we are pointing to one of the top queues put the packet on the switch.
	if(last_queue_num >= 0 && last_queue_num <= (gpu_group_cache_num -1))
	{
		//switch queue has a slot

		//update routing headers for the packet
		l3_cache_ptr = cgm_l3_cache_map(message_packet->set);


		//save the gpu l2 cache id
		message_packet->gpu_cache_id = message_packet->l2_cache_id;
		message_packet->gpu_cache_name = message_packet->l2_cache_name;

		message_packet->l2_cache_id = str_map_string(&node_strn_map, hub_iommu->name);
		message_packet->l2_cache_name = hub_iommu->name;

		//change src name and id
		SETROUTE(message_packet, hub_iommu, l3_cache_ptr);

		message_packet->dest_name = l3_caches[l3_cache_ptr].name;
		message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_cache_ptr].name);

		message_packet->src_name = hub_iommu->name;
		message_packet->src_id = str_map_string(&node_strn_map, hub_iommu->name);

		message_packet = list_remove(hub_iommu->last_queue, message_packet);
		assert(message_packet);

		if(GPU_HUB_IOMMU == 1)
			printf("hub-iommu access id %llu as %s first address 0x%08x set %d heading to L3 id %d \n",
					message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->address, message_packet->set, l3_cache_ptr->id);
		getchar();

		list_enqueue(hub_iommu->Tx_queue_bottom, message_packet);
		advance(hub_iommu->hub_iommu_io_down_ec);

	}
	//if we are pointing to the bottom queue route to the correct GPU l2 cache
	else if(last_queue_num == Rx_queue_bottom)
	{
		//return trip for memory access
		l2_src_id = str_map_string(&gpu_l2_strn_map, message_packet->gpu_cache_name);

		if(message_packet->access_id == 1638361)
		{
			fatal("message %llu hub_iomm received gpu cache name %s\n", message_packet->access_id, message_packet->gpu_cache_name);
		}

		//star todo fix this
		while(!cache_can_access_bottom(&gpu_l2_caches[l2_src_id]))
		{
			fatal("hub_iommu_put_next_queue(): hub stalling on return\n");
			P_PAUSE(1);
		}

		message_packet = list_remove(hub_iommu->last_queue, message_packet);

		//list_enqueue(gpu_l2_caches[l2_src_id].Rx_queue_bottom, message_packet);
		//advance(&gpu_l2_cache[l2_src_id]);
		//future_advance(&gpu_l2_cache[l2_src_id], WIRE_DELAY(gpu_l2_caches[l2_src_id].wire_latency));

		list_enqueue(hub_iommu->Tx_queue_top[l2_src_id], message_packet);
		advance(hub_iommu->hub_iommu_io_up_ec[l2_src_id]);

	}
	else
	{
		fatal("hub_iommu_put_next_queue(): got a queue id that is out of range\n");

	}


	star look at this
	return 0;
}*/


void hub_probe_address(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//re-probes the address if it has been changed...
	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	//int way = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	//int *way_ptr = &way;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	//store the decode in the packet for now.
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;
	//message_packet->way = way;

	return;
}

int hub_iommu_put_next_queue_func(struct cgm_packet_t *message_packet){

	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	int last_queue_num = -1;
	//int l2_src_id = -1;

	struct cache_t *l3_cache_ptr = NULL;

	//get the number of the last queue
	last_queue_num = str_map_string(&Rx_queue_strn_map, hub_iommu->last_queue->name);

	//if we are pointing to one of the top queues put the packet on the switch.
	if(last_queue_num >= 0 && last_queue_num <= (gpu_group_cache_num -1))
	{
		//switch queue has a slot

		if(list_count(hub_iommu->Tx_queue_bottom) <= QueueSize)
		{
			GPU_PAUSE(hub_iommu->latency);

			//save the gpu l2 cache id
			message_packet->gpu_cache_id = message_packet->l2_cache_id;
			message_packet->gpu_cache_name = message_packet->l2_cache_name;

			/*account for coherence and connection type*/
			if(cgm_gpu_cache_protocol == cgm_protocol_non_coherent)
			{
				iommu_nc_translate(message_packet);
			}
			else if (cgm_gpu_cache_protocol == cgm_protocol_mesi)
			{
				iommu_translate(message_packet);
			}
			else
			{
				fatal("hub_iommu_put_next_queue_func(): invalid hub-iommu coherence type\n");
			}

			if(hub_iommu_connection_type == hub_to_mc)
			{
				SETROUTE(message_packet, hub_iommu, system_agent);
			}
			else if (hub_iommu_connection_type == hub_to_l3)
			{
				//update routing headers for the packet

				if(message_packet->access_type != cgm_access_put_clnx && message_packet->access_type != cgm_access_putx)
				{

					hub_probe_address(&l2_caches[0], message_packet);

					l3_cache_ptr = cgm_l3_cache_map(message_packet->set);

					//message_packet->l2_cache_id = hub_iommu->switch_id;
					//message_packet->l2_cache_name = hub_iommu->name;

					//change src name and id
					SETROUTE(message_packet, hub_iommu, l3_cache_ptr);
				}
				else
				{
					//note dest already set in protocol function.
					message_packet->src_name = hub_iommu->name;
					message_packet->src_id = str_map_string(&node_strn_map, hub_iommu->name);
				}

			}
			else
			{
				fatal("hub_iommu_put_next_queue_MC(): invalid connection type\n");
			}

			message_packet = list_remove(hub_iommu->last_queue, message_packet);
			assert(message_packet);

			list_enqueue(hub_iommu->Tx_queue_bottom, message_packet);
			advance(hub_iommu->hub_iommu_io_down_ec);
		}
		else
		{
			return 0;
		}
	}
	//if we are pointing to the bottom queue route to the correct GPU l2 cache
	else if(last_queue_num == Rx_queue_bottom)
	{
		//return trip for memory access
		//l2_src_id = str_map_string(&gpu_l2_strn_map, message_packet->gpu_cache_name);

		if(list_count(hub_iommu->Tx_queue_top[cgm_gpu_cache_map(&gpu_v_caches[0], message_packet->address)]) <= QueueSize)
		{

			GPU_PAUSE(hub_iommu->latency);

			/*account for coherence and connection type*/
			if(cgm_gpu_cache_protocol == cgm_protocol_non_coherent)
			{
				iommu_nc_translate(message_packet);
			}
			else if (cgm_gpu_cache_protocol == cgm_protocol_mesi)
			{
				iommu_translate(message_packet);
			}
			else
			{
				fatal("hub_iommu_put_next_queue_func(): invalid hub-iommu coherence type\n");
			}

			message_packet = list_remove(hub_iommu->last_queue, message_packet);
			list_enqueue(hub_iommu->Tx_queue_top[cgm_gpu_cache_map(&gpu_v_caches[0], message_packet->address)], message_packet);
			advance(hub_iommu->hub_iommu_io_up_ec[cgm_gpu_cache_map(&gpu_v_caches[0], message_packet->address)]);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		fatal("hub_iommu_put_next_queue(): got a queue id that is out of range\n");

	}

	//failed to route the packet because the out box was full so stall.
	return 1;
}

/*void hub_iommu_noncoherent_ctrl(void){

	int my_pid = hub_iommu_pid++;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	struct cgm_packet_t *message_packet;
	long long step = 1;
	//int l3_map;

	set_id((unsigned int)my_pid);

	while(1)
	{
		//we have received a packet
		await(hub_iommu_ec, step);

		//star todo figure out a way to check the l2 in queue
		if(!switch_can_access(hub_iommu->switch_queue))// && !cache_can_access_bottom(&gpu_l2_caches[l2_src_id]))
		{
			P_PAUSE(1);
			fatal("hub_iommu_ctrl(): hub stalled\n");
		}
		else
		{
			step++;

			//if we made it here we should have a packet.
			message_packet = hub_iommu_get_from_queue();
			assert(message_packet);

			P_PAUSE(hub_iommu->latency);

			iommu_nc_translate(message_packet);

			//star the hub multiplexes GPU memory requests.
			hub_iommu_put_next_queue(message_packet);
		}
	}

	fatal("hub_iommu_ctrl(): reached end of func\n");
	return;
}*/

void hub_iommu_ctrl_func(void){

	int my_pid = hub_iommu_pid++;
	struct cgm_packet_t *message_packet;
	long long step = 1;

	set_id((unsigned int)my_pid);

	while(1)
	{
		//we have received a packet
		await(hub_iommu_ec, step);

		//if we made it here we should have a packet.
		message_packet = hub_iommu_get_from_queue();
		assert(message_packet);

		//star the hub multiplexes GPU memory requests.
		if(hub_iommu_put_next_queue(message_packet))
		{
			step++;
		}
		else
		{

			/*if(P_TIME > 106034981)
				printf("%s stalling\n", hub_iommu->name);*/


			GPU_PAUSE(1);
		}

	}

	fatal("hub_iommu_ctrl(): reached end of func\n");

	return;
}

int iommu_get_translation_table_size(void){

	int i = 0;
	int j = 0;

	//get the size of the ORT
	for (i = 0; i < hub_iommu->translation_table_size; i++)
	{
		if(hub_iommu->translation_table[i][1] != -1)
		{
			j++;
		}
	}

	return j;
}

int iommu_translation_table_insert_address(unsigned int address){

	int i = 0;
	int size = 0;

	//check that the table is not full
	size = iommu_get_translation_table_size();
	assert(size >= 0 && size < hub_iommu->translation_table_size);

	/*insert the record into the first available slot in the table
	Order doesn't matter here*/
	for(i = 0; i < hub_iommu->translation_table_size; i++)
	{
		if(hub_iommu->translation_table[i][1] == -1)
		{
			//empty slot found
			hub_iommu->translation_table[i][1] = address;
			break;
		}
	}

	assert(i != hub_iommu->translation_table_size);

	return i;
}

unsigned int iommu_translation_table_get_address(int id){

	unsigned int vtl_address = -1;

	/*pull the address from the look up table*/
	vtl_address = hub_iommu->translation_table[id][1];
	assert(vtl_address != -1);

	//clear the table entry
	hub_iommu->translation_table[id][1] = -1;

	return vtl_address;
}


void iommu_nc_translate(struct cgm_packet_t *message_packet){

	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	int last_queue_num = -1;

	//get the number of the last queue
	last_queue_num = str_map_string(&Rx_queue_strn_map, hub_iommu->last_queue->name);

	//if we are pointing to one of the top queues put the packet on the switch.
	if(last_queue_num >= 0 && last_queue_num <= (gpu_group_cache_num -1))
	{
		/*load and stores are heading to the system agent.*/
		if(GPU_HUB_IOMMU == 1)
			printf("hub-iommu NC ACCESS vtl address in 0x%08x access type %s id %llu\n",
					message_packet->address, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->access_id);
		message_packet->address = mmu_translate(1, message_packet->address, mmu_access_gpu);
		if(GPU_HUB_IOMMU == 1)
			printf("hub-iommu NC ACCESS phy address out 0x%08x\n", message_packet->address);
	}
	else if(last_queue_num == Rx_queue_bottom)
	{
		/*messages coming from system*/
		if(GPU_HUB_IOMMU == 1)
			printf("hub-iommu NC return phy address in 0x%08x access type %s id %llu\n",
					message_packet->address, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->access_id);
		message_packet->address = mmu_reverse_translate(1, message_packet->address, mmu_access_gpu);
		if(GPU_HUB_IOMMU == 1)
			printf("hub-iommu NC return vtl address out 0x%08x\n", message_packet->address);
	}
	else
	{
		fatal("iommu_nc_translate(): got a queue id that is out of range\n");
	}

	return;
}

void iommu_translate(struct cgm_packet_t *message_packet){

	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	int last_queue_num = -1;

	//get the number of the last queue
	last_queue_num = str_map_string(&Rx_queue_strn_map, hub_iommu->last_queue->name);

	//if we are pointing to one of the top queues put the packet on the switch.
	if(last_queue_num >= 0 && last_queue_num <= (gpu_group_cache_num -1))
	{
		if(GPU_HUB_IOMMU == 1)
			printf("hub-iommu C ACCESS vtl address in 0x%08x blk addr 0x%08x access type %s id %llu\n",
					message_packet->address, get_block_address(message_packet->address, ~0x3F),
					str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->access_id);

		message_packet->address = mmu_forward_translate_guest(0, si_emu->pid, message_packet->address);

		if(GPU_HUB_IOMMU == 1)
			printf("hub-iommu C ACCESS phy address out 0x%08x blk addr 0x%08x access type %s id %llu\n",
					message_packet->address, get_block_address(message_packet->address, ~0x3F),
					str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->access_id);

	}
	else if(last_queue_num == Rx_queue_bottom)
	{
		/*message coming from the system agent or L3.*/
		if(GPU_HUB_IOMMU == 1)
			printf("hub-iommu C RETURN phy address in 0x%08x blk addr 0x%08x access type %s id %llu\n",
					message_packet->address, get_block_address(message_packet->address, ~0x3F),
					str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->access_id);

		message_packet->address = mmu_reverse_translate_guest(0, si_emu->pid, message_packet->address);

		if(GPU_HUB_IOMMU == 1)
			printf("hub-iommu C RETURN vtl address out 0x%08x blk addr 0x%08x access type %s id %llu\n",
					message_packet->address, get_block_address(message_packet->address, ~0x3F),
					str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->access_id);
	}
	else
	{
		fatal("iommu_translate(): got a queue id that is out of range\n");
	}

	return;
}


void iommu_put_translation_table(unsigned int vtl_index, unsigned int phy_index){

	/*find an empty row in the table
	There should always be a spot in the table
	becasue table size is determined to be the number
	of GPU L2s x the ORT table size*/

	int i = 0;

	for(i = 0; i < hub_iommu->translation_table_size; i++)
	{
		if(hub_iommu->translation_table[i][0] == -1 && hub_iommu->translation_table[i][1] == -1)
		{
			/*found an empty row*/
			hub_iommu->translation_table[i][0] = vtl_index;
			hub_iommu->translation_table[i][1] = phy_index;
		}
	}

	assert(i < hub_iommu->translation_table_size);

	return;
}

int iommu_get_translation_table(unsigned int phy_index){

	/*get the stored vtl_index return it and clear the row*/

	int i = 0;
	int vtl_index = 0;

	for(i = 0; i < hub_iommu->translation_table_size; i++)
	{
		if(hub_iommu->translation_table[i][1] == phy_index)
		{
			/*found the entry*/
			vtl_index = hub_iommu->translation_table[i][0];
			break;
		}
	}

	assert(i < hub_iommu->translation_table_size);

	iommu_clear_translation_table(i);

	return vtl_index;
}

void iommu_clear_translation_table(int row){

	hub_iommu->translation_table[row][1] = -1;
	hub_iommu->translation_table[row][0] = -1;

	return;
}


void hub_iommu_io_up_ctrl(void){

	int my_pid = hub_iommu_io_up_pid++;
	/*int num_cus = si_gpu_num_compute_units;*/
	/*int gpu_group_cache_num = (num_cus/4);*/
	struct cgm_packet_t *message_packet;

	/*long long access_id = 0;*/
	int transfer_time = 0;
	long long step = 1;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(hub_iommu->hub_iommu_io_up_ec[my_pid], step);
		//step++;

		message_packet = list_get(hub_iommu->Tx_queue_top[my_pid], 0);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
		transfer_time = (message_packet->size/hub_iommu->bus_width);

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}


		if(message_packet->access_type == cgm_access_puts || message_packet->access_type == cgm_access_putx
					|| message_packet->access_type == cgm_access_put_clnx || message_packet->access_type == cgm_access_get_fwd
					|| message_packet->access_type == cgm_access_getx_fwd || message_packet->access_type == cgm_access_get_nack
					|| message_packet->access_type == cgm_access_getx_nack || message_packet->access_type == cgm_access_upgrade_getx_fwd
					|| message_packet->access_type == cgm_access_upgrade || message_packet->access_type == cgm_access_mc_put
					|| message_packet->access_type == cgm_access_gpu_flush)
		{

			if(list_count(gpu_l2_caches[my_pid].Rx_queue_bottom) >= QueueSize)
			{
				GPU_PAUSE(1);
			}
			else
			{
				step++;

				GPU_PAUSE(transfer_time);

				if(list_count(gpu_l2_caches[my_pid].Rx_queue_bottom) > QueueSize)
					warning("hub_iommu_io_up_ctrl(): %s %s size exceeded %d\n",
							gpu_l2_caches[my_pid].name, gpu_l2_caches[my_pid].Rx_queue_bottom->name, list_count(gpu_l2_caches[my_pid].Rx_queue_bottom));

				message_packet = list_remove(hub_iommu->Tx_queue_top[my_pid], message_packet);
				list_enqueue(gpu_l2_caches[my_pid].Rx_queue_bottom, message_packet);
				advance(&gpu_l2_cache[my_pid]);

			}
		}
		else if (message_packet->access_type == cgm_access_flush_block || message_packet->access_type == cgm_access_upgrade_ack
				|| message_packet->access_type == cgm_access_upgrade_nack || message_packet->access_type == cgm_access_upgrade_inval
				|| message_packet->access_type == cgm_access_upgrade_putx_n || message_packet->access_type == cgm_access_downgrade_nack
				|| message_packet->access_type == cgm_access_getx_fwd_nack )
		{

			if(list_count(gpu_l2_caches[my_pid].Coherance_Rx_queue) >= QueueSize)
			{
				GPU_PAUSE(1);
			}
			else
			{
				step++;

				GPU_PAUSE(transfer_time);

				if(list_count(gpu_l2_caches[my_pid].Coherance_Rx_queue) > QueueSize)
					warning("hub_iommu_io_up_ctrl(): %s %s size exceeded %d\n",
							gpu_l2_caches[my_pid].name, gpu_l2_caches[my_pid].Coherance_Rx_queue->name, list_count(gpu_l2_caches[my_pid].Coherance_Rx_queue));

				message_packet = list_remove(hub_iommu->Tx_queue_top[my_pid], message_packet);
				list_enqueue(gpu_l2_caches[my_pid].Coherance_Rx_queue, message_packet);
				advance(&gpu_l2_cache[my_pid]);

			}
		}
		else
		{
			fatal("hub_iommu_io_up_ctrl(): bad access type as %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
		}

		//===================================================



		/*list_enqueue(gpu_l2_caches[my_pid].Rx_queue_bottom, message_packet);
		advance(&gpu_l2_cache[my_pid]);*/
	}
	return;
}

void hub_iommu_io_down_ctrl(void){

	int my_pid = hub_iommu_io_down_pid++;
	struct cgm_packet_t *message_packet;

	int transfer_time = 0;
	long long step = 1;

	set_id((unsigned int)my_pid);


	while(1)
	{

		await(hub_iommu->hub_iommu_io_down_ec, step);

		message_packet = list_get(hub_iommu->Tx_queue_bottom, 0);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
		transfer_time = (message_packet->size/hub_iommu->bus_width);

		if(transfer_time == 0)
			transfer_time = 1;


		//SYSTEM_PAUSE(transfer_time);


		//drop into the next correct virtual lane/queue.
		if(message_packet->access_type == cgm_access_get || message_packet->access_type == cgm_access_getx
				|| message_packet->access_type == cgm_access_mc_load || message_packet->access_type == cgm_access_mc_store)
		{

			//star fixme, don't know why but sometimes queue size will be overrun by 1. "QueueSize - 1" fixes the problem...
			if(list_count(switches[hub_iommu->switch_id].north_rx_request_queue) >= (QueueSize - 1))
			{
				GPU_PAUSE(1);
			}
			else
			{
				step++;

				GPU_PAUSE(transfer_time);

				//drop in to the switch queue
				message_packet = list_remove(hub_iommu->Tx_queue_bottom, message_packet);
				list_enqueue(switches[hub_iommu->switch_id].north_rx_request_queue, message_packet);
				advance(&switches_ec[hub_iommu->switch_id]);
			}
		}
		else if(message_packet->access_type == cgm_access_gpu_flush_ack || message_packet->access_type == cgm_access_putx)
		{

			if(list_count(switches[hub_iommu->switch_id].north_rx_reply_queue) >= QueueSize)
			{
				GPU_PAUSE(1);
			}
			else
			{
				step++;

				GPU_PAUSE(transfer_time);

				message_packet = list_remove(hub_iommu->Tx_queue_bottom, message_packet);
				list_enqueue(switches[hub_iommu->switch_id].north_rx_reply_queue, message_packet);
				advance(&switches_ec[hub_iommu->switch_id]);

			}
		}
		else if(message_packet->access_type == cgm_access_flush_block_ack || message_packet->access_type == cgm_access_downgrade_ack
				|| message_packet->access_type == cgm_access_getx_fwd_inval_ack || message_packet->access_type == cgm_access_write_back
				|| message_packet->access_type == cgm_access_getx_fwd_ack)
		{

			if(list_count(switches[hub_iommu->switch_id].north_rx_coherence_queue) >= QueueSize)
			{
				GPU_PAUSE(1);
			}
			else
			{
				step++;

				GPU_PAUSE(transfer_time);

				message_packet = list_remove(hub_iommu->Tx_queue_bottom, message_packet);
				list_enqueue(switches[hub_iommu->switch_id].north_rx_coherence_queue, message_packet);
				advance(&switches_ec[hub_iommu->switch_id]);

			}
		}
		else
		{
			fatal("hub_iommu_io_down_ctrl(): invalid access type as %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
		}

			//list_enqueue(hub_iommu->switch_queue, message_packet);
			//advance(&switches_ec[hub_iommu->switch_id]);
		//	list_enqueue(switches[hub_iommu->switch_id].north_rx_request_queue, message_packet);
		//	advance(&switches_ec[hub_iommu->switch_id]);

	}

	return;
}

