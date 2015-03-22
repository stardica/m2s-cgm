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
#include <lib/util/list.h>


extern struct cgm_packet_t{

	enum cgm_access_kind_t access_type;
	char *name;
	struct list_t source_id;
	unsigned int address;
	long long access_id;
	int tag;
	int in_flight;

	int c_load;
	int c_store;
	int nc_load;
	int nc_store;
	void * data;

	//for m2s CPU and GPU
	struct linked_list_t *event_queue;
	int *witness_ptr;

	//for routing
	char *src_name;
	char *dest_name;

};


extern struct cgm_packet_status_t{

	enum cgm_access_kind_t access_type;
	unsigned int address;
	long long access_id;
	int in_flight;

};


//star todo create functions to load/access the packet as needed by the various memory system elements.
struct cgm_packet_t *packet_create(void);
struct cgm_packet_status_t *status_packet_create(void);

#endif /* PACKET_H_ */
