/*
 * packet.h
 *
 *  Created on: Jan 16, 2015
 *      Author: stardica
 */

#ifndef PACKET_H_
#define PACKET_H_

#include <stdio.h>
#include <stdlib.h>

//#include <cgm/cgm.h>
#include <cgm/protocol.h>

#include <lib/util/linked-list.h>
#include <lib/util/list.h>

extern struct cgm_packet_t{

	enum cgm_access_kind_t access_type;
	char *name;

	unsigned int address;
	long long access_id;

	int coalesced;

	//for routing
	char *src_name;
	int source_id;

	char *dest_name;
	int dest_id;

	//for m2s CPU and GPU
	struct linked_list_t *event_queue;
	int *witness_ptr;

};


/*extern struct cgm_packet_status_t{

	enum cgm_access_kind_t access_type;
	unsigned int address;
	long long access_id;
	int set;
	int tag;
	unsigned int offset;
	int in_flight;

	//for reverse routing
	int source_id;

	int coalesced;
	struct cgm_packet_t *coalesced_packet;

};*/


//star todo create functions to load/access the packet as needed by the various memory system elements.
struct cgm_packet_t *packet_create(void);
struct cgm_packet_status_t *status_packet_create(void);

void packet_copy()

#endif /* PACKET_H_ */
