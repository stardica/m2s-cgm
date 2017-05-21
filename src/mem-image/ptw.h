/*
 * ptw.h
 *
 *  Created on: May 16, 2017
 *      Author: stardica
 */

#ifndef PTW_H_
#define PTW_H_

#include <lib/util/debug.h>
#include <cgm/cgm-struct.h>
#include <cgm/tasking.h>


extern eventcount volatile *ptw_ec;
extern task *ptw_task;


extern long long ptw_pid;

void ptw_ctrl(void);

void ptw_init(void);
void ptw_create(void);


#endif /* PTW_H_ */
