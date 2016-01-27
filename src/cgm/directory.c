/*
 * directory.c
 *
 *  Created on: Mar 30, 2015
 *      Author: stardica
 */


#include <cgm/directory.h>

//globals
unsigned int dir_mode = 0; //1 = soc mode 0 equals system agent mode
unsigned long long dir_mem_image_size = 0;
unsigned int dir_block_size = 0;
unsigned long long dir_num_blocks = 0;
unsigned int dir_block_mask = 0;
unsigned int dir_vector_size = 0;
