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

	unsigned int address;

	char *queue_name;
	char *task;
	char *data;
};

struct cgm_packet_t *packet_create(void);


//star todo create functions to load/access the packet as needed by the various memory system elements.


#endif /* PACKET_H_ */
