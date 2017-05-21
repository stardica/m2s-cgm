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



void cgm_tlb_probe_address(struct tlb_t *tlb, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr);
int cgm_tlb_find_block(struct tlb_t *tlb, int *tag_ptr, int *set_ptr, unsigned int *offset_ptr, int *way_ptr, int *state_ptr);
unsigned int cgm_tlb_get_ppn(struct tlb_t *tlb, int tag, int set, int way);
int cgm_tlb_get_victim(struct tlb_t *tlb, int set, int phy_tag);
void cgm_tlb_update_waylist(struct tlb_set_t *set, struct tlb_block_t *blk, enum cache_waylist_enum where);
void cgm_tlb_set_tran_state(struct tlb_t *tlb, int set, int tag, int way, enum cgm_tlb_block_state_t tran_state);
void cgm_tlb_invalidate(struct tlb_t *tlb, int set, int way);



#endif /* TLB_H_ */
