/*
 * packet.c
 *
 *  Created on: Jan 16, 2015
 *      Author: stardica
 */

#include <stdlib.h>
#include <cgm/packet.h>




//CPU will call create packet and load into correct queue.
//I am not sure yet what to put into the packet.
struct cgm_packet_t *packet_create(void){

	struct cgm_packet_t *new_packet;

	new_packet = (void *) calloc(1, sizeof(struct cgm_packet_t));

	return new_packet;
}

struct cgm_packet_status_t *status_packet_create(void){

	struct cgm_packet_status_t *new_packet;

	new_packet = (void *) calloc(1, sizeof(struct cgm_packet_status_t));


	return new_packet;
}
