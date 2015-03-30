/*
 * directory.h
 *
 *  Created on: Mar 30, 2015
 *      Author: stardica
 */


#ifndef DIRECTORY_H_
#define DIRECTORY_H_


#include <lib/util/list.h>

//number of 64byte memory blocks in 4GB
#define MAX_SETS 1048576

enum directory_mode_t{

	soc = 0,
	node

};

struct directory_t{

	//directory queues
	struct list_t *Rx_queue_0;
	struct list_t *Rx_queue_1;
	struct list_t *Rx_queue_2;
	struct list_t *Rx_queue_3;

	//directory data
	unsigned char *bit_vector; // gives <0,0,0,0,0,0,0,0>

};


extern struct directory_t *directory;

void dir_init(void);


#endif /*DIRECTORY_H_*/
