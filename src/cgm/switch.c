/*

 * switch.c
 *
 *  Created on: Feb 9, 2015
 *      Author: stardica
 */


#include <cgm/switch.h>



struct switch_t *switches;
eventcount volatile *switches_ec;
task *switches_tasks;
int switch_pid = 0;

int switch_io_delay_factor = 4;

int switch_north_io_pid = 0;
int switch_east_io_pid = 0;
int switch_south_io_pid = 0;
int switch_west_io_pid = 0;

//supports quad core with ring bus
struct str_map_t l1_strn_map =
{ 	l1_number, {
		{ "l1_i_caches[0]", l1_i_cache_0},
		{ "l1_d_caches[0]", l1_d_cache_0},
		{ "l1_i_caches[1]", l1_i_cache_1},
		{ "l1_d_caches[1]", l1_d_cache_1},
		{ "l1_i_caches[2]", l1_i_cache_2},
		{ "l1_d_caches[2]", l1_d_cache_2},
		{ "l1_i_caches[3]", l1_i_cache_3},
		{ "l1_d_caches[3]", l1_d_cache_3},
		}
};

struct str_map_t l2_strn_map =
{ l2_number, {
		{ "l2_caches[0]", l2_caches_0},
		{ "l2_caches[1]", l2_caches_1},
		{ "l2_caches[2]", l2_caches_2},
		{ "l2_caches[3]", l2_caches_3},
		{ "hub-iommu", hub_iommu_0},
		}
};

struct str_map_t gpu_l1_strn_map =
{ gpu_l1_number, {
		{ "gpu_s_caches[0]", gpu_s_cache_0},
		{ "gpu_v_caches[0]", gpu_v_cache_0},
		{ "gpu_s_caches[1]", gpu_s_cache_1},
		{ "gpu_v_caches[1]", gpu_v_cache_1},
		{ "gpu_s_caches[2]", gpu_s_cache_2},
		{ "gpu_v_caches[2]", gpu_v_cache_2},
		{ "gpu_s_caches[3]", gpu_s_cache_3},
		{ "gpu_v_caches[3]", gpu_v_cache_3},
		{ "gpu_s_caches[4]", gpu_s_cache_4},
		{ "gpu_v_caches[4]", gpu_v_cache_4},
		{ "gpu_s_caches[5]", gpu_s_cache_5},
		{ "gpu_v_caches[5]", gpu_v_cache_5},
		{ "gpu_s_caches[6]", gpu_s_cache_6},
		{ "gpu_v_caches[6]", gpu_v_cache_6},
		{ "gpu_s_caches[7]", gpu_s_cache_7},
		{ "gpu_v_caches[7]", gpu_v_cache_7},
		{ "gpu_s_caches[8]", gpu_s_cache_8},
		{ "gpu_v_caches[8]", gpu_v_cache_8},
		{ "gpu_s_caches[9]", gpu_s_cache_9},
		{ "gpu_v_caches[9]", gpu_v_cache_9},
		{ "gpu_s_caches[10]", gpu_s_cache_10},
		{ "gpu_v_caches[10]", gpu_v_cache_10},
		{ "gpu_s_caches[11]", gpu_s_cache_11},
		{ "gpu_v_caches[11]", gpu_v_cache_11},
		{ "gpu_s_caches[12]", gpu_s_cache_12},
		{ "gpu_v_caches[12]", gpu_v_cache_12},
		{ "gpu_s_caches[13]", gpu_s_cache_13},
		{ "gpu_v_caches[13]", gpu_v_cache_13},
		{ "gpu_s_caches[14]", gpu_s_cache_14},
		{ "gpu_v_caches[14]", gpu_v_cache_14},
		{ "gpu_s_caches[15]", gpu_s_cache_15},
		{ "gpu_v_caches[15]", gpu_v_cache_15},
		{ "gpu_s_caches[16]", gpu_s_cache_16},
		{ "gpu_v_caches[16]", gpu_v_cache_16},
		{ "gpu_s_caches[17]", gpu_s_cache_17},
		{ "gpu_v_caches[17]", gpu_v_cache_17},
		{ "gpu_s_caches[18]", gpu_s_cache_18},
		{ "gpu_v_caches[18]", gpu_v_cache_18},
		{ "gpu_s_caches[19]", gpu_s_cache_19},
		{ "gpu_v_caches[19]", gpu_v_cache_19},
		{ "gpu_s_caches[20]", gpu_s_cache_20},
		{ "gpu_v_caches[20]", gpu_v_cache_20},
		{ "gpu_s_caches[21]", gpu_s_cache_21},
		{ "gpu_v_caches[21]", gpu_v_cache_21},
		{ "gpu_s_caches[22]", gpu_s_cache_22},
		{ "gpu_v_caches[22]", gpu_v_cache_22},
		{ "gpu_s_caches[23]", gpu_s_cache_23},
		{ "gpu_v_caches[23]", gpu_v_cache_23},
		{ "gpu_s_caches[24]", gpu_s_cache_24},
		{ "gpu_v_caches[24]", gpu_v_cache_24},
		{ "gpu_s_caches[25]", gpu_s_cache_25},
		{ "gpu_v_caches[25]", gpu_v_cache_25},
		{ "gpu_s_caches[26]", gpu_s_cache_26},
		{ "gpu_v_caches[26]", gpu_v_cache_26},
		{ "gpu_s_caches[27]", gpu_s_cache_27},
		{ "gpu_v_caches[27]", gpu_v_cache_27},
		{ "gpu_s_caches[28]", gpu_s_cache_28},
		{ "gpu_v_caches[28]", gpu_v_cache_28},
		{ "gpu_s_caches[29]", gpu_s_cache_29},
		{ "gpu_v_caches[29]", gpu_v_cache_29},
		{ "gpu_s_caches[30]", gpu_s_cache_30},
		{ "gpu_v_caches[30]", gpu_v_cache_30},
		{ "gpu_s_caches[31]", gpu_s_cache_31},
		{ "gpu_v_caches[31]", gpu_v_cache_31},
		}
};


struct str_map_t gpu_l2_strn_map =
{ gpu_l2_number, {
		{ "gpu_l2_caches[0]", gpu_l2_caches_0},
		{ "gpu_l2_caches[1]", gpu_l2_caches_1},
		{ "gpu_l2_caches[2]", gpu_l2_caches_2},
		{ "gpu_l2_caches[3]", gpu_l2_caches_3},
		{ "gpu_l2_caches[4]", gpu_l2_caches_4},
		{ "gpu_l2_caches[5]", gpu_l2_caches_5},
		{ "gpu_l2_caches[6]", gpu_l2_caches_6},
		{ "gpu_l2_caches[7]", gpu_l2_caches_7},
		}
};


struct str_map_t node_strn_map =
{ node_number, {
		{ "l2_caches[0]", l2_cache_0},
		{ "switch[0]", switch_0},
		{ "l3_caches[0]", l3_cache_0},
		{ "l2_caches[1]", l2_cache_1},
		{ "switch[1]", switch_1},
		{ "l3_caches[1]", l3_cache_1},
		{ "l2_caches[2]", l2_cache_2},
		{ "switch[2]", switch_2},
		{ "l3_caches[2]", l3_cache_2},
		{ "l2_caches[3]", l2_cache_3},
		{ "switch[3]", switch_3},
		{ "l3_caches[3]", l3_cache_3},
		{ "hub_iommu", hub_iommu_4},
		{ "switch[4]", switch_4},
		{ "sys_agent", sys_agent_4},
		}
};

struct str_map_t port_name_map =
{ 	port_num, {
		{ "north_queue", north_queue},
		{ "east_queue", east_queue},
		{ "south_queue", south_queue},
		{ "west_queue", west_queue},
		}
};


void switch_init(void){

	switch_create();
	//route_create();
	switch_create_tasks();
	return;
}


void switch_create(void){

	int num_cores = x86_cpu_num_cores;
	/*int num_cus = si_gpu_num_compute_units;*/

	//for now the number of GPU connected switches is hard coded
	//this one switch for all of the GPU.
	//star todo fix this
	int extras = 1;

	//for now model a ring bus on each CPU
	switches = (void *) calloc((num_cores + extras), sizeof(struct switch_t));

	return;
}

void switch_create_tasks(void){

	int num_cores = x86_cpu_num_cores;
	/*int num_cus = si_gpu_num_compute_units;*/

	//star todo fix this
	int extras = 1;

	char buff[100];
	int i = 0;

	switches_ec = (void *) calloc((num_cores + extras), sizeof(eventcount));
	for(i = 0; i < (num_cores + extras); i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "switch_%d", i);
		switches_ec[i] = *(new_eventcount(strdup(buff)));
	}

	switches_tasks = (void *) calloc((num_cores + extras), sizeof(task));
	for(i = 0; i < (num_cores + extras); i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "switch_ctrl_%d", i);
		switches_tasks[i] = *(create_task(switch_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//event counts and tasks for io ctrl are over in configure.

	return;
}

struct crossbar_t *switch_crossbar_create(){

	struct crossbar_t *crossbar;

	crossbar = (void *) malloc(sizeof(struct crossbar_t));

	//set up the cross bar variables
	crossbar->num_ports = 4;
	crossbar->num_pairs = 0;

	crossbar->north_in_out_linked_queue = invalid_queue;
	crossbar->east_in_out_linked_queue = invalid_queue;
	crossbar->south_in_out_linked_queue = invalid_queue;
	crossbar->west_in_out_linked_queue = invalid_queue;

	return crossbar;
}

void switch_crossbar_clear_state(struct switch_t *switches){

	//clear the cross bar state
	switches->crossbar->num_pairs = 0;

	switches->crossbar->north_in_out_linked_queue = invalid_queue;
	switches->crossbar->east_in_out_linked_queue = invalid_queue;
	switches->crossbar->south_in_out_linked_queue = invalid_queue;
	switches->crossbar->west_in_out_linked_queue = invalid_queue;

	return;
}

void switch_set_link(struct switch_t *switches, enum port_name tx_queue){

	/*we have the in queue with switches->queue and the tx_queue
	try to link them...*/

	//check if the out on the cross bar is busy. If not assign the link.
	if(tx_queue == north_queue)
	{
		/*if(switches->queue == north_queue)
			printf("error here cycle %llu\n", P_TIME);*/

		assert(switches->queue != north_queue);
		if(switches->crossbar->north_in_out_linked_queue == invalid_queue)
		{
			switches->crossbar->north_in_out_linked_queue = switches->queue;
			switches->crossbar->num_pairs++;
		}
	}
	else if(tx_queue == east_queue)
	{
		assert(switches->queue != east_queue);
		if(switches->crossbar->east_in_out_linked_queue == invalid_queue)
		{
			switches->crossbar->east_in_out_linked_queue = switches->queue;
			switches->crossbar->num_pairs++;
		}
	}
	else if(tx_queue == south_queue)
	{
		assert(switches->queue != south_queue);
		if(switches->crossbar->south_in_out_linked_queue == invalid_queue)
		{
			switches->crossbar->south_in_out_linked_queue = switches->queue;
			switches->crossbar->num_pairs++;
		}
	}
	else if(tx_queue == west_queue)
	{
		assert(switches->queue != west_queue);
		if(switches->crossbar->west_in_out_linked_queue == invalid_queue)
		{
			switches->crossbar->west_in_out_linked_queue = switches->queue;
			switches->crossbar->num_pairs++;
		}
	}
	else
	{
		fatal("switch_set_link(): invalid switch link\n");
	}

	return;
}

enum port_name switch_get_route(struct switch_t *switches, struct cgm_packet_t *message_packet){

	enum port_name tx_port = invalid_queue;

	int dest_node;
	int src_node;
	int switch_node = switches->switch_node_number;
	float distance;

	//send the packet to it's destination OR on to the next hop
	//look up the node number of the destination
	dest_node = message_packet->dest_id;
	src_node = message_packet->src_id;

/*
	if(message_packet->access_id == 4446145 || message_packet->access_id == 4446155)
	{
		printf("\tswtich routing id %llu src %d dest %d\n", message_packet->access_id, src_node, dest_node);
		getchar();
	}*/

	//if dest is an L2/L3/HUB-IOMMU/SA connected to this switch.
	if(dest_node == (switch_node - 1) || dest_node == (switch_node + 1))
	{
		//if the node number is lower this means it is an L2 cache or HUB IOMMU
		if(dest_node < switch_node)
		{
			tx_port = north_queue; //switches[switches->switch_id].Tx_north_queue;
		}
		//if the node number is high this means it is an L3 cache or the SYS Agent
		else if(dest_node > switch_node)
		{
			tx_port = south_queue; //switches[switches->switch_id].Tx_south_queue;
		}
	}
	else
	{
		//send packet to adjacent switch
		//there is no transfer direction established.
		if(switches->queue == north_queue || switches->queue == south_queue)
		{
			//new packets from connected L2 or L3 cache.
			if(dest_node > src_node)
			{
				distance = switch_get_distance(dest_node, src_node);

				//go in the direction with the shortest number of hops.
				if(distance <= switches->switch_median_node)
				{//go east

					tx_port = east_queue; //switches[switches->switch_id].Tx_east_queue;
				}
				else
				{//go west
					tx_port = west_queue; //switches[switches->switch_id].Tx_west_queue;
				}
			}
			else if(dest_node < src_node)
			{
				distance = switch_get_distance(dest_node, src_node);

				//go in the direction with the shortest number of hops.
				if(distance <= switches->switch_median_node)
				{//go west
					tx_port = west_queue; //switches[switches->switch_id].Tx_west_queue;
				}
				else
				{//go east
					tx_port = east_queue; //switches[switches->switch_id].Tx_east_queue;
				}
			}
		}
		else if(switches->queue == east_queue || switches->queue == west_queue)
		{
			//packet came from another switch, but needs to continue on.
			if(switches->queue == east_queue)
			{//continue going west
				tx_port = west_queue; //switches[switches->switch_id].Tx_west_queue;
			}
			else if(switches->queue == west_queue)
			{//continue going east
				tx_port = east_queue; //switches[switches->switch_id].Tx_east_queue;
			}
		}
		else
		{
			fatal("switch_get_route(): directional queue error as %d.\n", switches->queue);
		}
	}

	assert(tx_port != invalid_queue);
	return tx_port;
}


void switch_crossbar_link(struct switch_t *switches){

	struct cgm_packet_t *packet = NULL;
	enum port_name tx_queue = invalid_queue;
	int i = 0;

	if(switches->arb_style == round_robin)
	{
		/*printf("start_2 queue %d\n", switches->queue);*/

		for(i = 0; i < switches->crossbar->num_ports; i++)
		{
			/*printf("\tChecking queue %d\n", switches->queue);*/

			//try to form link pairs
			if(switches->queue == north_queue)
			{
				//see if there is a packet waiting...
				packet = list_get(switches->north_queue, 0);

				//found a new packet, try to link it...
				if(packet)
				{
					/*printf("\tnorth hit\n");*/

					tx_queue = switch_get_route(switches, packet);

					//try to assign the link
					/*printf("packet id %llu dest %d access type %d\n", packet->evict_id, packet->dest_id, packet->access_type);*/
					switch_set_link(switches, tx_queue);
				}
			}

			if(switches->queue == east_queue)
			{
				//see if there is a packet waiting...
				packet = list_get(switches->east_queue, 0);

				//found a new packet, try to link it...
				if(packet)
				{
					/*printf("\teast hit\n");*/
					tx_queue = switch_get_route(switches, packet);

					//try to assign the link
					switch_set_link(switches, tx_queue);
				}
			}

			if(switches->queue == south_queue)
			{
				//see if there is a packet waiting...
				packet = list_get(switches->south_queue, 0);

				//found a new packet, try to link it...
				if(packet)
				{
					/*printf("\tsouth hit\n");*/
					tx_queue = switch_get_route(switches, packet);

					//try to assign the link
					switch_set_link(switches, tx_queue);

					/*if(packet->access_id == 4446145 || packet->access_id == 4446155)
					{

						printf("\%s cross bar linking to %d id %llu to switch north tx queue cross bar\n", switches->name, tx_queue, packet->access_id);
						cache_dump_request_queue(switches->south_queue);
					}*/
				}
			}

			if(switches->queue == west_queue)
			{
				//see if there is a packet waiting...
				packet = list_get(switches->west_queue, 0);

				//found a new packet, try to link it...
				if(packet)
				{
					/*printf("\twest hit\n");*/

					tx_queue = switch_get_route(switches, packet);

					//try to assign the link
					switch_set_link(switches, tx_queue);
				}
			}
			/*else
			{
				fatal("switch_crossbar_link(): Invalid queue\n");
			}*/

			assert(switches->queue >= 1 && switches->queue <= switches->crossbar->num_ports);

			//advance the queue pointer.
			switches->queue = get_next_queue_rb(switches->queue);
		}
	}
	else
	{
		fatal("switch_crossbar_link(): Invalid ARB set\n");
	}

	/*if(switches->crossbar->num_pairs > 1)
		printf("switches->crossbar->num_pairs %d\n", switches->crossbar->num_pairs);*/

	//advance the queue ptr to prevent starvation and or deadlocks.

	/*printf("next_1 queue %d\n", switches->queue);*/

	switches->queue = get_next_queue_rb(switches->queue);

	/*printf("next_2 queue %d\n", switches->queue);*/

	//we should have at least one pair, but no more than the number of ports on the switch.
	assert(switches->crossbar->num_pairs > 0 && switches->crossbar->num_pairs <= switches->crossbar->num_ports);
	return;
}


int switch_can_access(struct list_t *queue){

	//check if target queue is full
	if(QueueSize <= list_count(queue))
	{
		return 0;
	}

	return 1;
}


void switch_ctrl(void){

	int my_pid = switch_pid++;
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	struct cgm_packet_t *message_packet = NULL;
	long long step = 1;

	enum port_name next_queue = west_queue;

	long long queue_depth;

	/*long long access_id = 0;*/

	assert(my_pid <= (num_cores + num_cus));

	set_id((unsigned int)my_pid);

	while(1)
	{
		/*we have been advanced. Note that it's possible for a
		switch to be advanced more than once per cycle*/
		await(&switches_ec[my_pid], step);

		SYSTEM_PAUSE(switches[my_pid].latency);

		assert(next_queue == switches[my_pid].queue);

		/*switches models a cross bar.
		link as many inputs to outputs as possible*/
		switch_crossbar_link(&switches[my_pid]);

		/*crossbar state is set. now run through and move each packet as required.*/
		if(switches[my_pid].crossbar->north_in_out_linked_queue != invalid_queue)
		{
			/*the north out queue is linked to an input queue
			move the packet from the input queue to the correct output queue*/
			message_packet = list_remove_at(switch_get_in_queue(&switches[my_pid], switches[my_pid].crossbar->north_in_out_linked_queue), 0);
			assert(message_packet);


			/*if(message_packet->access_id == 4446145 || message_packet->access_id == 4446155)
			{
				printf("\tswtich putting id %llu in switch io north tx queue\n", message_packet->access_id);
				cache_dump_request_queue(switches[my_pid].south_queue);
				cache_dump_request_queue(switches[my_pid].Tx_north_queue);
			}*/

			list_enqueue(switches[my_pid].Tx_north_queue, message_packet);
			advance(switches[my_pid].switches_north_io_ec);

			/*stats*/
			switches[my_pid].north_tx_inserts++;
			queue_depth = list_count(switches[my_pid].Tx_north_queue);
			/*max depth*/
			if(queue_depth > switches[my_pid].north_txqueue_max_depth)
					switches[my_pid].north_txqueue_max_depth = queue_depth;

			/*ave depth = ((old count * old data) + next data) / next count*/
			switches[my_pid].north_txqueue_ave_depth =
					((((double) switches[my_pid].north_tx_inserts - 1) * switches[my_pid].north_txqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].north_tx_inserts;

			message_packet = NULL;
		}

		if(switches[my_pid].crossbar->east_in_out_linked_queue != invalid_queue)
		{
			message_packet = list_remove_at(switch_get_in_queue(&switches[my_pid], switches[my_pid].crossbar->east_in_out_linked_queue), 0);
			assert(message_packet);

			list_enqueue(switches[my_pid].Tx_east_queue, message_packet);
			advance(switches[my_pid].switches_east_io_ec);

			/*stats*/
			switches[my_pid].east_tx_inserts++;
			queue_depth = list_count(switches[my_pid].Tx_east_queue);
			/*max depth*/
			if(queue_depth > switches[my_pid].east_txqueue_max_depth)
					switches[my_pid].east_txqueue_max_depth = queue_depth;

			/*ave depth = ((old count * old data) + next data) / next count*/
			switches[my_pid].east_txqueue_ave_depth =
					((((double) switches[my_pid].east_tx_inserts - 1) * switches[my_pid].east_txqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].east_tx_inserts;

			message_packet = NULL;
		}

		if(switches[my_pid].crossbar->south_in_out_linked_queue != invalid_queue)
		{
			message_packet = list_remove_at(switch_get_in_queue(&switches[my_pid], switches[my_pid].crossbar->south_in_out_linked_queue), 0);
			assert(message_packet);

			list_enqueue(switches[my_pid].Tx_south_queue, message_packet);
			advance(switches[my_pid].switches_south_io_ec);

			/*stats*/
			switches[my_pid].south_tx_inserts++;
			queue_depth = list_count(switches[my_pid].Tx_south_queue);
			/*max depth*/
			if(queue_depth > switches[my_pid].south_txqueue_max_depth)
					switches[my_pid].south_txqueue_max_depth = queue_depth;

			/*ave depth = ((old count * old data) + next data) / next count*/
			switches[my_pid].south_txqueue_ave_depth =
					((((double) switches[my_pid].south_tx_inserts - 1) * switches[my_pid].south_txqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].south_tx_inserts;

			message_packet = NULL;
		}

		if(switches[my_pid].crossbar->west_in_out_linked_queue != invalid_queue)
		{
			message_packet = list_remove_at(switch_get_in_queue(&switches[my_pid], switches[my_pid].crossbar->west_in_out_linked_queue), 0);
			assert(message_packet);

			list_enqueue(switches[my_pid].Tx_west_queue, message_packet);
			advance(switches[my_pid].switches_west_io_ec);

			/*stats*/
			switches[my_pid].west_tx_inserts++;
			queue_depth = list_count(switches[my_pid].Tx_west_queue);
			/*max depth*/
			if(queue_depth > switches[my_pid].west_txqueue_max_depth)
					switches[my_pid].west_txqueue_max_depth = queue_depth;

			/*ave depth = ((old count * old data) + next data) / next count*/
			switches[my_pid].west_txqueue_ave_depth =
					((((double) switches[my_pid].west_tx_inserts - 1) * switches[my_pid].west_txqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].west_tx_inserts;

			message_packet = NULL;
		}

		/*stats*/
		if(switches[my_pid].switch_max_links < switches[my_pid].crossbar->num_pairs)
			switches[my_pid].switch_max_links = switches[my_pid].crossbar->num_pairs;

		switches[my_pid].switch_total_links += switches[my_pid].crossbar->num_pairs;
		switches[my_pid].switch_total_wakes++;

		//increase step by number of pairs formed
		step += switches[my_pid].crossbar->num_pairs;

		//clear the current cross bar state
		switch_crossbar_clear_state(&switches[my_pid]);

		//set message_paket null
		message_packet = NULL;

		next_queue = switches[my_pid].queue;
	}

	fatal("switch_ctrl() quit\n");
	return;
}


float switch_get_distance(int dest_node, int src_node){

	float distance = 0;

	if(dest_node > src_node)
	{
		//get the distance from this switch to the destination (left to right)
		if(dest_node % 3 == 0 && src_node % 3 == 0)
		{
			//L2 to L2
			distance = (dest_node - src_node)/3;
		}
		else if(dest_node % 3 != 0 && src_node % 3 == 0)
		{
			//L2 to L3/SA || L3 to L2 (works for both ways)
			distance = (dest_node - (src_node + 2))/3;
		}
		else
		{
			//L3 to L3/SA
			distance = (dest_node - src_node)/3;
		}
	}
	else
	{
		//get the distance from this switch to the destination (right to left)
		if(src_node % 3 == 0 && dest_node % 3 == 0)
		{
			//L2 to L2
			distance = (src_node - dest_node)/3;
		}
		else if(src_node % 3 == 0 && dest_node % 3 != 0)
		{
			//L2 to L3/SA || L3 to L2 (works for both ways)
			distance = ((src_node + 2) - dest_node)/3;
		}
		else
		{
			//L3 to L3/SA
			distance = (src_node - dest_node)/3;
		}
	}
	return distance;
}


struct cgm_packet_t *get_from_queue(struct switch_t *switches){

	struct cgm_packet_t *new_packet = NULL;
	int i = 0;

	//choose a port this cycle to work from
	if(switches->arb_style == round_robin)
	{

		//get a packet
		for(i = 0; i < switches->port_num; i++)
		{
			//set switches->queue to the next queue.
			switches->queue = get_next_queue_rb(switches->queue);

			//if we don't have a message go on to the next.
			if(switches->queue == north_queue)
			{
				new_packet = list_get(switches->north_queue, 0);
				switches->current_queue = switches->north_queue;
			}
			else if(switches->queue == east_queue)
			{
				new_packet = list_get(switches->east_queue, 0);
				switches->current_queue = switches->east_queue;
			}
			else if(switches->queue == south_queue)
			{
				new_packet = list_get(switches->south_queue, 0);
				switches->current_queue = switches->south_queue;
			}
			else if(switches->queue == west_queue)
			{
				new_packet = list_get(switches->west_queue, 0);
				switches->current_queue = switches->west_queue;
			}

			//when we have a packet break out.
			//next advance start with the next queue
			if(new_packet)
			{
				i = 0;
				break;
			}
		}
	}
	else
	{
		fatal("get_from_queue() invalid arbitration set switch %s\n", switches->name);
	}

	/*CGM_DEBUG(switch_debug_file, "%s access_id %llu cycle %llu get from %s with size %d\n",
			switches->name, new_packet->access_id, P_TIME, (char *)str_map_value(&port_name_map, switches->queue), list_count(switches->current_queue));*/

	assert(new_packet != NULL);
	return new_packet;
}


struct list_t *switch_get_in_queue(struct switch_t *switches, enum port_name queue){

	struct list_t *in_queue = NULL;

	if(queue == north_queue)
	{
		in_queue = switches->north_queue;
	}
	else if(queue == east_queue)
	{
		in_queue = switches->east_queue;
	}
	else if(queue == south_queue)
	{
		in_queue = switches->south_queue;
	}
	else if(queue == west_queue)
	{
		in_queue = switches->west_queue;
	}
	else
	{
		fatal("switche_get_out_queue() invalid port name\n");
	}

	assert(in_queue != NULL);
	return in_queue;
}

void remove_from_queue(struct switch_t *switches, struct cgm_packet_t *message_packet){


	if(switches->queue == north_queue)
	{
		list_remove(switches->north_queue, message_packet);
	}
	else if(switches->queue == east_queue)
	{
		list_remove(switches->east_queue, message_packet);
	}
	else if(switches->queue == south_queue)
	{
		list_remove(switches->south_queue, message_packet);
	}
	else if(switches->queue == west_queue)
	{
		list_remove(switches->west_queue, message_packet);
	}
	else
	{
		fatal("remove_from_queue() invalid port name\n");
	}

	CGM_DEBUG(switch_debug_file, "%s access_id %llu cycle %llu removed from %s with size %d\n",
			switches->name, message_packet->access_id, P_TIME, (char *)str_map_value(&port_name_map, switches->queue), list_count(switches->current_queue));

	return;
}

enum port_name get_next_queue_rb(enum port_name queue){

	enum port_name next_queue = invalid_queue;

	if(queue == west_queue)
	{
		next_queue = north_queue;
	}
	else if(queue == north_queue)
	{
		next_queue = east_queue;
	}
	else if(queue == east_queue)
	{
		next_queue = south_queue;
	}
	else if(queue == south_queue)
	{
		next_queue = west_queue;
	}
	else
	{
		fatal("get_next_queue() Invalid port name\n");
	}

	assert(next_queue != invalid_queue);
	return next_queue;
}

void switch_north_io_ctrl(void){

	int my_pid = switch_north_io_pid++;
	long long step = 1;
	int num_cores = x86_cpu_num_cores;
	struct cgm_packet_t *message_packet;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(switches[my_pid].switches_north_io_ec, step);
		step++;

		message_packet = list_dequeue(switches[my_pid].Tx_north_queue);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
		transfer_time = (message_packet->size/switches[my_pid].bus_width);

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		SYSTEM_PAUSE(transfer_time);

		//L2 switches
		if(my_pid < num_cores)
		{

			if(message_packet->access_type == cgm_access_puts || message_packet->access_type == cgm_access_putx
					|| message_packet->access_type == cgm_access_put_clnx || message_packet->access_type == cgm_access_get_fwd
					|| message_packet->access_type == cgm_access_getx_fwd || message_packet->access_type == cgm_access_get_nack
					|| message_packet->access_type == cgm_access_getx_nack || message_packet->access_type == cgm_access_upgrade_getx_fwd
					|| message_packet->access_type == cgm_access_upgrade)
			{

				if(list_count(l2_caches[my_pid].Rx_queue_bottom) >= QueueSize)
					warning("switch_north_io_ctrl(): %s %s size exceeded %d\n", l2_caches[my_pid].name, l2_caches[my_pid].Rx_queue_bottom->name, list_count(l2_caches[my_pid].Rx_queue_bottom));

				list_enqueue(l2_caches[my_pid].Rx_queue_bottom, message_packet);
				advance(&l2_cache[my_pid]);
			}
			else if (message_packet->access_type == cgm_access_flush_block || message_packet->access_type == cgm_access_upgrade_ack
					|| message_packet->access_type == cgm_access_upgrade_nack || message_packet->access_type == cgm_access_upgrade_inval
					|| message_packet->access_type == cgm_access_upgrade_putx_n || message_packet->access_type == cgm_access_downgrade_nack)
			{

				if(list_count(l2_caches[my_pid].Coherance_Rx_queue) >= QueueSize)
					warning("switch_north_io_ctrl(): %s %s size exceeded %d\n", l2_caches[my_pid].name, l2_caches[my_pid].Coherance_Rx_queue->name, list_count(l2_caches[my_pid].Coherance_Rx_queue));

				list_enqueue(l2_caches[my_pid].Coherance_Rx_queue, message_packet);
				advance(&l2_cache[my_pid]);
			}
			else
			{
				fatal("switch_north_io_ctrl(): bad access type as %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
			}
		}
		//hub-iommu
		else if(my_pid >= num_cores)
		{
			list_enqueue(hub_iommu->Rx_queue_bottom, message_packet);
			advance(hub_iommu_ec);
		}
		else
		{
			fatal("switch_north_io_ctrl(): my_pid is out of bounds %d\n", my_pid);
		}

		/*stats*/
		switches[my_pid].switch_north_io_transfers++;
		switches[my_pid].switch_north_io_transfer_cycles += transfer_time;
		switches[my_pid].switch_north_io_bytes_transfered += message_packet->size;

	}

	return;
}

void switch_east_io_ctrl(void){

	int my_pid = switch_east_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;
	long long queue_depth = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(switches[my_pid].switches_east_io_ec, step);
		step++;

		message_packet = list_dequeue(switches[my_pid].Tx_east_queue);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
		transfer_time = (message_packet->size/switches[my_pid].bus_width);

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		SYSTEM_PAUSE(transfer_time);

		if(list_count(switches[my_pid].next_east) >= QueueSize)
			warning("switch_east_io_ctrl(): %s %s size exceeded %d\n", switches[my_pid].name, switches[my_pid].next_east->name, list_count(switches[my_pid].next_east));

		//drop into next east queue.
		list_enqueue(switches[my_pid].next_east, message_packet);
		advance(&switches_ec[switches[my_pid].next_east_id]);

		/*stats*/
		switches[my_pid].switch_east_io_transfers++;
		switches[my_pid].switch_east_io_transfer_cycles += transfer_time;
		switches[my_pid].switch_east_io_bytes_transfered += message_packet->size;

		//note these stats are for the adjacent switch to the east which puts packets in the west rx_queue
		switches[switches[my_pid].next_east_id].west_rx_inserts++;
		queue_depth = list_count(switches[my_pid].next_east); //tricky
		/*max depth*/
		if(queue_depth > switches[switches[my_pid].next_east_id].west_rxqueue_max_depth)
			switches[switches[my_pid].next_east_id].west_rxqueue_max_depth = queue_depth;

		/*ave depth = ((old count * old data) + next data) / next count*/
		switches[switches[my_pid].next_east_id].west_rxqueue_ave_depth =
			((((double) switches[switches[my_pid].next_east_id].west_rx_inserts - 1) * switches[switches[my_pid].next_east_id].west_rxqueue_ave_depth)
					+ (double) queue_depth) / (double) switches[switches[my_pid].next_east_id].west_rx_inserts;
	}

	return;
}

void switch_west_io_ctrl(void){

	int my_pid = switch_west_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;
	long long queue_depth = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(switches[my_pid].switches_west_io_ec, step);
		step++;

		message_packet = list_dequeue(switches[my_pid].Tx_west_queue);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
		transfer_time = (message_packet->size/switches[my_pid].bus_width);

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		SYSTEM_PAUSE(transfer_time);

		if(list_count(switches[my_pid].next_west) >= QueueSize)
			warning("switch_west_io_ctrl(): %s %s size exceeded %d\n", switches[my_pid].name, switches[my_pid].next_west->name, list_count(switches[my_pid].next_west));

		//drop into next east queue.
		list_enqueue(switches[my_pid].next_west, message_packet);
		advance(&switches_ec[switches[my_pid].next_west_id]);

		/*stats*/
		switches[my_pid].switch_west_io_transfers++;
		switches[my_pid].switch_west_io_transfer_cycles += transfer_time;
		switches[my_pid].switch_west_io_bytes_transfered += message_packet->size;

		//note these stats are for the adjacent switch
		switches[switches[my_pid].next_west_id].east_rx_inserts++;
		queue_depth = list_count(switches[my_pid].next_west); //tricky
		/*max depth*/
		if(queue_depth > switches[switches[my_pid].next_west_id].east_rxqueue_max_depth)
			switches[switches[my_pid].next_west_id].east_rxqueue_max_depth = queue_depth;

		/*ave depth = ((old count * old data) + next data) / next count*/
		switches[switches[my_pid].next_west_id].east_rxqueue_ave_depth =
			((((double) switches[switches[my_pid].next_west_id].east_rx_inserts - 1) * switches[switches[my_pid].next_west_id].east_rxqueue_ave_depth)
					+ (double) queue_depth) / (double) switches[switches[my_pid].next_west_id].east_rx_inserts;
	}

	return;

}

void switch_south_io_ctrl(void){

	int my_pid = switch_south_io_pid++;
	long long step = 1;

	int num_cores = x86_cpu_num_cores;
	/*int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;
	int queue_depth = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(switches[my_pid].switches_south_io_ec, step);
		step++;

		message_packet = list_dequeue(switches[my_pid].Tx_south_queue);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
		transfer_time = (message_packet->size/switches[my_pid].bus_width);

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		SYSTEM_PAUSE(transfer_time);

		//L3 caches
		if(my_pid < num_cores)
		{
			//put the message in the right queue.
			if(message_packet->access_type == cgm_access_gets || message_packet->access_type == cgm_access_getx
					|| message_packet->access_type == cgm_access_get || message_packet->access_type == cgm_access_upgrade)
			{
				if(list_count(l3_caches[my_pid].Rx_queue_top) >= QueueSize)
					warning("switch_north_io_ctrl(): %s %s size exceeded %d\n", l3_caches[my_pid].name, l3_caches[my_pid].Rx_queue_top->name, list_count(l3_caches[my_pid].Rx_queue_top));

				list_enqueue(l3_caches[my_pid].Rx_queue_top, message_packet);
			}
			else if(message_packet->access_type == cgm_access_mc_put)
			{
				if(list_count(l3_caches[my_pid].Rx_queue_bottom) >= QueueSize)
					warning("switch_north_io_ctrl(): %s %s size exceeded %d\n", l3_caches[my_pid].name, l3_caches[my_pid].Rx_queue_bottom->name, list_count(l3_caches[my_pid].Rx_queue_bottom));

				list_enqueue(l3_caches[my_pid].Rx_queue_bottom, message_packet);
			}
			else if (message_packet->access_type == cgm_access_downgrade_ack || message_packet->access_type == cgm_access_downgrade_nack
					|| message_packet->access_type == cgm_access_getx_fwd_ack || message_packet->access_type == cgm_access_getx_fwd_nack
					|| message_packet->access_type == cgm_access_getx_fwd_upgrade_nack || message_packet->access_type == cgm_access_get_fwd_upgrade_nack
					|| message_packet->access_type == cgm_access_flush_block_ack || message_packet->access_type == cgm_access_write_back
					|| message_packet->access_type == cgm_access_upgrade_ack)
			{

				if(list_count(l3_caches[my_pid].Coherance_Rx_queue) >= QueueSize)
					warning("switch_north_io_ctrl(): %s %s size exceeded %d\n", l3_caches[my_pid].name, l3_caches[my_pid].Coherance_Rx_queue->name, list_count(l3_caches[my_pid].Coherance_Rx_queue));

				list_enqueue(l3_caches[my_pid].Coherance_Rx_queue, message_packet);
			}
			else
			{
					fatal("switch_south_io_ctrl(): bad access_type as %s access_id %llu cycle %llu\n",
						str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->access_id, P_TIME);
			}

			advance(&l3_cache[my_pid]);
		}
		//Sys Agent
		else if(my_pid >= num_cores)
		{
			list_enqueue(system_agent->Rx_queue_top, message_packet);
			advance(system_agent_ec);

			/*stats*/
			/*running ave = ((old count * old data) + next data) / next count*/

			/*stats*/
			if(system_agent->max_north_rxqueue_depth < list_count(system_agent->Rx_queue_top))
				system_agent->max_north_rxqueue_depth = list_count(system_agent->Rx_queue_top);

			system_agent->north_gets++;
			queue_depth = list_count(system_agent->Rx_queue_top);
			system_agent->ave_north_rxqueue_depth = ((((double) system_agent->north_gets - 1) * system_agent->ave_north_rxqueue_depth) + (double) queue_depth) / (double) system_agent->north_gets;
		}
		else
		{
			fatal("switch_south_io_ctrl(): my_pid is out of bounds %d\n", my_pid);
		}

		/*stats*/
		switches[my_pid].switch_south_io_transfers++;
		switches[my_pid].switch_south_io_transfer_cycles += transfer_time;
		switches[my_pid].switch_south_io_bytes_transfered += message_packet->size;

	}
	return;
}

void switch_dump_stats(struct cgm_stats_t *cgm_stat_container){

	int num_cores = x86_cpu_num_cores;
	int i = 0;

	/*there is a switch for the GPU this for loop will pick it up*/
	for(i = 0; i <= num_cores; i++)
	{
		/*CGM_STATS(cgm_stats_file, "[Switch_%d]\n", i);*/
		CGM_STATS(cgm_stats_file, "s_%d_TotalSwitchCtrlLoops = %llu\n", i, cgm_stat_container->switch_total_wakes[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SwitchOccupance = %0.2f\n", i, (double) cgm_stat_container->switch_total_wakes[i]/ (double) P_TIME);
		CGM_STATS(cgm_stats_file, "s_%d_NumberLinks = %llu\n", i, cgm_stat_container->switch_total_links[i]);
		CGM_STATS(cgm_stats_file, "s_%d_MaxNumberLinks = %d\n", i, cgm_stat_container->switch_max_links[i]);
		CGM_STATS(cgm_stats_file, "s_%d_AveNumberLinksPerAccess = %.02f\n", i, (double)cgm_stat_container->switch_total_links[i]/(double)cgm_stat_container->switch_total_wakes[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthIOTransfers = %llu\n", i, cgm_stat_container->switch_north_io_transfers[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthIOCycles = %llu\n", i, cgm_stat_container->switch_north_io_transfer_cycles[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthIOBytesTransfered = %llu\n", i, cgm_stat_container->switch_north_io_bytes_transfered[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthRxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_north_rxqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthRxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_north_rxqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthTxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_north_txqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthTxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_north_txqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastIOTransfers = %llu\n", i, cgm_stat_container->switch_east_io_transfers[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastIOCycles = %llu\n", i, cgm_stat_container->switch_east_io_transfer_cycles[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastIOBytesTransfered = %llu\n", i, cgm_stat_container->switch_east_io_bytes_transfered[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastRxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_east_rxqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastRxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_east_rxqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastTxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_east_txqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastTxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_east_txqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthIOTransfers = %llu\n", i, cgm_stat_container->switch_south_io_transfers[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthIOCycles = %llu\n", i, cgm_stat_container->switch_south_io_transfer_cycles[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthIOBytesTransfered = %llu\n", i, cgm_stat_container->switch_south_io_bytes_transfered[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthRxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_south_rxqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthRxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_south_rxqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthTxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_south_txqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthTxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_south_txqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestIOTransfers = %llu\n", i, cgm_stat_container->switch_west_io_transfers[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestIOCycles = %llu\n", i, cgm_stat_container->switch_west_io_transfer_cycles[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestIOBytesTransfered = %llu\n", i, cgm_stat_container->switch_west_io_bytes_transfered[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestRxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_west_rxqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestRxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_west_rxqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestTxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_west_txqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestTxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_west_txqueue_ave_depth[i]);
		/*CGM_STATS(cgm_stats_file, "\n");*/
	}

	/*CGM_STATS(cgm_stats_file, "[Switch_SA]\n");
	CGM_STATS(cgm_stats_file, "NumberSwitchCtrlLoops = %llu\n", cgm_stat_container->switch_total_wakes[num_cores]);
	CGM_STATS(cgm_stats_file, "SwitchOccupance = %0.2f\n", (double) cgm_stat_container->switch_total_wakes[num_cores]/ (double) P_TIME);
	CGM_STATS(cgm_stats_file, "NumberLinks = %llu\n", cgm_stat_container->switch_total_links[num_cores]);
	CGM_STATS(cgm_stats_file, "MaxNumberLinks = %d\n", cgm_stat_container->switch_max_links[num_cores]);
	CGM_STATS(cgm_stats_file, "AveNumberLinksPerCtrlLoop = %.02f\n", (double)cgm_stat_container->switch_total_links[num_cores]/(double)cgm_stat_container->switch_total_wakes[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthIOTransfers = %llu\n", cgm_stat_container->switch_north_io_transfers[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthIOCycles = %llu\n", cgm_stat_container->switch_north_io_transfer_cycles[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthIOBytesTransfered = %llu\n", cgm_stat_container->switch_north_io_bytes_transfered[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthRxQueueMaxDepth = %llu\n", cgm_stat_container->switch_north_rxqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthRxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_north_rxqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthTxQueueMaxDepth = %llu\n", cgm_stat_container->switch_north_txqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthTxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_north_txqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "EastIOTransfers = %llu\n", cgm_stat_container->switch_east_io_transfers[num_cores]);
	CGM_STATS(cgm_stats_file, "EastIOCycles = %llu\n", cgm_stat_container->switch_east_io_transfer_cycles[num_cores]);
	CGM_STATS(cgm_stats_file, "EastIOBytesTransfered = %llu\n", cgm_stat_container->switch_east_io_bytes_transfered[num_cores]);
	CGM_STATS(cgm_stats_file, "EastRxQueueMaxDepth = %llu\n", cgm_stat_container->switch_east_rxqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "EastRxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_east_rxqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "EastTxQueueMaxDepth = %llu\n", cgm_stat_container->switch_east_txqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "EastTxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_east_txqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthIOTransfers = %llu\n", cgm_stat_container->switch_south_io_transfers[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthIOCycles = %llu\n", cgm_stat_container->switch_south_io_transfer_cycles[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthIOBytesTransfered = %llu\n", cgm_stat_container->switch_south_io_bytes_transfered[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthRxQueueMaxDepth = %llu\n", cgm_stat_container->switch_south_rxqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthRxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_south_rxqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthTxQueueMaxDepth = %llu\n", cgm_stat_container->switch_south_txqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthTxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_south_txqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "WestIOTransfers = %llu\n", cgm_stat_container->switch_west_io_transfers[num_cores]);
	CGM_STATS(cgm_stats_file, "WestIOCycles = %llu\n", cgm_stat_container->switch_west_io_transfer_cycles[num_cores]);
	CGM_STATS(cgm_stats_file, "WestIOBytesTransfered = %llu\n", cgm_stat_container->switch_west_io_bytes_transfered[num_cores]);
	CGM_STATS(cgm_stats_file, "WestRxQueueMaxDepth = %llu\n", cgm_stat_container->switch_west_rxqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "WestRxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_west_rxqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "WestTxQueueMaxDepth = %llu\n", cgm_stat_container->switch_west_txqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "WestTxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_west_txqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "\n");*/

	return;
}
