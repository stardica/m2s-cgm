/*
 * directory.c
 *
 *  Created on: Mar 30, 2015
 *      Author: stardica
 */

#include <stdio.h>
#include <stdlib.h>


#include <cgm/directory.h>

//globals
struct directory_t *directory;



void dir_init(void){

	//init the directory struct
	directory = (void *) calloc(1, sizeof(struct directory_t));

	//init the data vectors

	directory->bit_vector = (void *) calloc(MAX_SETS, sizeof(unsigned char));

	return;
}
