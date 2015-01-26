/*
 * packet.h
 *
 *  Created on: Jan 16, 2015
 *      Author: stardica
 */


#ifndef PACKET_H_
#define PACKET_H_

//data
extern struct cgm_packet_t{

	long long access_id;
	int in_flight;
	unsigned int address;
	void *data;
};


//star todo create functions to load/access the packet as needed by the various memory system elements.
struct cgm_packet_t *packet_create(void);

#endif /* PACKET_H_ */
