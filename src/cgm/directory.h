/*
 * directory.h
 *
 *  Created on: Mar 30, 2015
 *      Author: stardica
 */

#ifndef DIRECTORY_H_
#define DIRECTORY_H_

struct directory_entry_t{
	unsigned int p0 : 1;
	unsigned int p1 : 1;
	unsigned int p2 : 1;
	unsigned int p3 : 1;
	unsigned int p4 : 1; //GPU or etc
	unsigned int dirty : 1;
	unsigned int pending : 1;
	unsigned int upgrade : 1;
};

union directory_t{

	unsigned char entry;
	struct directory_entry_t entry_bits;
};

extern unsigned int dir_mode; //1 = scoc mode 0 equals system agent mode
extern unsigned long long dir_mem_image_size;
extern unsigned int dir_block_size;
extern unsigned long long dir_num_blocks;
extern unsigned int dir_block_mask;
extern unsigned int dir_vector_size;


#endif /*DIRECTORY_H_*/
