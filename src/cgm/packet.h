/*
 * packet.h
 *
 *  Created on: Jan 16, 2015
 *      Author: stardica
 */

#ifndef PACKET_H_
#define PACKET_H_

/*
#include <cgm/protocol.h>
#include <cgm/cache.h>
*/

/*#include <lib/util/linked-list.h>

struct cgm_packet_t{

	char *name;

	enum cgm_access_kind_t access_type;
	enum cgm_access_kind_t l1_access_type;
	enum cgm_access_kind_t cpu_access_type;
	enum cgm_access_kind_t gpu_access_type;

	int gpu_cache_id;

	char *l2_cache_name;
	int l2_cache_id;

	long long access_id;
	unsigned int address;
	int set;
	int tag;
	unsigned int offset;
	int coalesced;

	//for protocol messages
	enum cache_block_state_t cache_block_state;

	//for routing
	char *src_name;
	int src_id;
	char *dest_name;
	int dest_id;

	//for m2s CPU and GPU
	struct linked_list_t *event_queue;
	int *witness_ptr;
	void *data;
};

extern struct cgm_packet_status_t{

	//used for global memory list

	enum cgm_access_kind_t access_type;
	unsigned int address;
	long long access_id;
	int in_flight;

	//for reverse routing
};

struct cgm_packet_status_t;

//star todo create functions to load/access the packet as needed by the various memory system elements.
struct cgm_packet_t *packet_create(void);
struct cgm_packet_status_t *status_packet_create(void);*/

#endif /* PACKET_H_ */
