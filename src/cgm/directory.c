/*
 * directory.c
 *
 *  Created on: Mar 30, 2015
 *      Author: stardica
 */

#include <stdio.h>
#include <stdlib.h>

#include <cgm/directory.h>
#include <lib/util/list.h>


//globals
struct directory_t *directory;

void dir_init(void){

	//init the directory struct
	directory = (void *) calloc(1, sizeof(struct directory_t));

	return;
}

unsigned long long dir_map_block_number(unsigned int addr){

	unsigned long long block_number = (addr & ~(directory->block_mask))/(directory->block_size);

	return block_number;
}
