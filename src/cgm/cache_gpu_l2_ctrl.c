/*
 * cache_ctrl.h
 *
 *  Created on: Apr 21, 2015
 *      Author: stardica
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <arch/si/timing/gpu.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>

#include <cgm/cgm.h>

#include <cgm/switch.h>



void gpu_l2_cache_ctrl(void){

	long long step = 1;


	while(1)
	{
		//wait here until there is a job to do.
		await(gpu_l2_cache, step);
		step++;

	}

	//should never get here
	fatal("gpu_l2_cache_ctrl task is broken\n");
	return;

}
