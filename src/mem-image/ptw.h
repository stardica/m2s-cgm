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
#include <cgm/cgm.h>
#include <mem-image/tlb.h>


extern eventcount volatile *ptw_ec;
extern task *ptw_task;


extern long long ptw_pid;
extern long long pages_created;
extern long long ptw_num_processed;

void ptw_ctrl(void);

void ptw_init(void);
//void ptw_create(void);

void ptw_clear_mmu_fault_bits(struct mmu_t *mmu);


#endif /* PTW_H_ */
