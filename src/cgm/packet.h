/*
 * packet.h
 *
 *  Created on: Jan 16, 2015
 *      Author: stardica
 */

#ifndef PACKET_H_
#define PACKET_H_

#include <cgm/cgm.h>
#include <lib/util/linked-list.h>

//data
extern struct cgm_packet_t{

	enum cgm_access_kind_t access_type;
	char *name;
	unsigned int address;
	long long access_id;
	int in_flight;
	int c_load;
	int c_store;
	int nc_load;
	int nc_store;
	void * data;

	struct linked_list_t *event_queue;
	int *witness_ptr;

};


//star todo create functions to load/access the packet as needed by the various memory system elements.
struct cgm_packet_t *packet_create(void);

#endif /* PACKET_H_ */
