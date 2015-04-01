/*
 * directory.h
 *
 *  Created on: Mar 30, 2015
 *      Author: stardica
 */

#ifndef DIRECTORY_H_
#define DIRECTORY_H_

#include <math.h>

#include <cgm/tasking.h>

#include <lib/util/list.h>


struct directory_t{

	//directory mode
	unsigned int mode; //1 = soc mode 0 equals system agent mode
	unsigned long long mem_image_size;
	unsigned int block_size;
	unsigned long long num_blocks;
	unsigned int block_mask;
	unsigned int vector_size;

	//directory queue(s) ass needed for cache access.
	struct list_t **Rx_queue;	//l3_slice_0 or sysagent_0 queue


	//directory data. We can vary the size.
	unsigned char *bit_vector_8; 		// 1 byte
	unsigned short *bit_vector_16; 		// 2 bytes
	unsigned long *bit_vector_24;		// 4 bytes
	unsigned long long *bit_vector_64;	// 8 bytes

};

extern struct directory_t *directory;

extern int *directory_data;
extern eventcount volatile *dir;

void directory_init(void);
void directory_create(void);
void directory_create_tasks(void);

void directory_ctrl(void);

/* 	- usage -
	unsigned int addr = 256;

	unsigned long long block_number = dir_map_block_number(addr);

	printf("block_number addr 0x%08X block is %llu\n", addr, block_number);
	getchar();
 */
unsigned long long directory_map_block_number(unsigned int addr);

#endif /*DIRECTORY_H_*/
