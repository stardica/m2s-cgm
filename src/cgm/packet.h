/*
 * packet.h
 *
 *  Created on: Jan 16, 2015
 *      Author: stardica
 */

#include <lib/util/linked-list.h>

#ifndef PACKET_H_
#define PACKET_H_

//data
extern struct cgm_packet_t{

	char * name;
	unsigned int address;
	long long access_id;
	int in_flight;
	void *data;
	struct linked_list_t *event_queue;
	int *witness_ptr;
	int c_load;
	int c_store;
	int nc_load;
	int nc_store;
};


//star todo create functions to load/access the packet as needed by the various memory system elements.
struct cgm_packet_t *packet_create(void);

#endif /* PACKET_H_ */
