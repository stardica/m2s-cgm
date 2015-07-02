/*
 * lds.c
 *
 *  Created on: Apr 24, 2015
 *      Author: stardica
 */


#include <cgm/cache.h>


/*


#include <arch/si/timing/gpu.h>
#include <lib/util/debug.h>
*/

void gpu_lds_unit_ctrl(void){

	int my_pid = gpu_lds_pid++;
	long long step = 1;

	int num_cus = si_gpu_num_compute_units;
	struct cgm_packet_t *message_packet;

	enum cgm_access_kind_t access_type;

	assert(my_pid <= num_cus);
	set_id((unsigned int)my_pid);

	while(1)
	{
		/*wait here until there is a job to do.
		In any given cycle I might have to service 1 to N number of units*/
		await(&gpu_lds_unit[my_pid], step);
		step++;

		//get the message out of the unit's queue
		message_packet = list_get(gpu_lds_units[my_pid].Rx_queue_top, 0);
		assert(message_packet);

		access_type = message_packet->access_type;

		//for now charge a small amount of cycles for it's access.
		if(access_type == cgm_access_load || access_type == cgm_access_store)
		{//then the packet is from the L2 cache

			//LDS is close to the CU so delay a couple cycles for now
			//P_PAUSE(etime.count + gpu_lds_units[my_pid].latency);
			P_PAUSE(2);

			//clear the gpu uop witness_ptr
			(*message_packet->witness_ptr)++;
			message_packet = list_remove(gpu_lds_units[my_pid].Rx_queue_top, message_packet);
			packet_destroy(message_packet);
		}
		else
		{
			fatal("gpu_lds_unit_ctrl(): unknown L2 message type = %d\n", message_packet->access_type);
		}
	}
	/* should never get here*/
	fatal("gpu_lds_unit_ctrl task is broken\n");
	return;
}
