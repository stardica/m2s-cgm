/*
 * cache_ctrl.h
 *
 *  Created on: Apr 21, 2015
 *      Author: stardica
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <arch/si/timing/gpu.h>
//#include <arch/x86/timing/cpu.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>
//#include <lib/util/linked-list.h>
//#include <lib/util/string.h>
//#include <lib/util/misc.h>

#include <cgm/cgm.h>
//#include <cgm/cache.h>
//#include <cgm/tasking.h>
//#include <cgm/packet.h>
#include <cgm/switch.h>
//#include <cgm/protocol.h>
//#include <cgm/mshr.h>


void gpu_v_cache_ctrl(void){

	int my_pid = gpu_v_pid++;
	long long step = 1;
	int num_cus = si_gpu_num_compute_units;

	struct cgm_packet_t *message_packet;
	struct cgm_packet_status_t *mshr_packet;

	enum cgm_access_kind_t access_type;
	unsigned int addr = 0;
	long long access_id = 0;
	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int state = 0;
	int cache_status;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;


	assert(my_pid <= num_cus);
	set_id((unsigned int)my_pid);


	while(1)
	{
		//wait here until there is a job to do.
		//In any given cycle I might have to service 1 to N number of caches
		await(&gpu_v_cache[my_pid], step);
		step++;

		//get the message out of the unit's queue
		message_packet = list_get(gpu_v_caches[my_pid].Rx_queue_top, 0);
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		addr = message_packet->address;

		//probe the address for set, tag, and offset.
		cgm_cache_decode_address(&(gpu_v_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);


		//star todo figure out what to do with this.
		if(access_type == cgm_access_load || access_type == cgm_access_store || access_type == cgm_access_nc_store)
		{//then the packet is from the L2 cache

			P_PAUSE(mem_miss);

			//clear the gpu uop witness_ptr
			(*message_packet->witness_ptr)++;

			list_remove(gpu_v_caches[my_pid].Rx_queue_top, message_packet);
			free(message_packet);

		}
		else
		{
			fatal("gpu_v_cache_ctrl(): unknown access type = %d\n", message_packet->access_type);
		}

	}
	//should never get here
	fatal("gpu_v_cache_ctrl task is broken\n");
	return;
}

void gpu_s_cache_ctrl(void){

	int my_pid = gpu_s_pid++;
	long long step = 1;
	int num_cus = si_gpu_num_compute_units;

	struct cgm_packet_t *message_packet;
	struct cgm_packet_status_t *mshr_packet;

	enum cgm_access_kind_t access_type;
	unsigned int addr = 0;
	long long access_id = 0;
	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	int state = 0;
	int cache_status;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;
	int *state_ptr = &state;

	assert(my_pid <= num_cus);
	set_id((unsigned int)my_pid);

	while(1)
	{
		//wait here until there is a job to do.
		//In any given cycle I might have to service 1 to N number of caches
		await(&gpu_s_cache[my_pid], step);
		step++;

		//get the message out of the unit's queue
		message_packet = list_get(gpu_s_caches[my_pid].Rx_queue_top, 0);
		assert(message_packet);

		access_type = message_packet->access_type;
		access_id = message_packet->access_id;
		addr = message_packet->address;

		//probe the address for set, tag, and offset.
		cgm_cache_decode_address(&(gpu_s_caches[my_pid]), addr, set_ptr, tag_ptr, offset_ptr);


		//star todo figure out what to do with this.
		if(access_type == cgm_access_load)
		{//then the packet is from the L2 cache

			P_PAUSE(mem_miss);

			//clear the gpu uop witness_ptr
			(*message_packet->witness_ptr)++;

			list_remove(gpu_s_caches[my_pid].Rx_queue_top, message_packet);
			free(message_packet);

		}
		else
		{
			fatal("gpu_s_cache_ctrl(): unknown access type = %d\n", message_packet->access_type);
		}

	}
	 //should never get here
	fatal("gpu_s_cache_ctrl task is broken\n");
	return;
}

void gpu_l2_cache_ctrl(void){

	long long step = 1;


	while(1)
	{
		//wait here until there is a job to do.
		await(gpu_l2_cache, step);
		step++;

	}

	//should never get here
	fatal("gpu_l2_cache_ctrl task is broken\n");
	return;

}
