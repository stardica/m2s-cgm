/*
 * tlb.h
 *
 *  Created on: Jan 27, 2016
 *      Author: stardica
 */

#ifndef TLB_H_
#define TLB_H_

#include <lib/util/debug.h>
#include <cgm/cgm-struct.h>


extern struct tlb_t *i_tlbs;
extern struct tlb_t *d_tlbs;



void tlb_init(void);
void tlb_create(void);



void cgm_tlb_probe_address(struct cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr);



#endif /* TLB_H_ */
