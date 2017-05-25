/*
 * tlb.c
 *
 *  Created on: Jan 27, 2016
 *      Author: stardica
 */


#include <mem-image/tlb.h>


struct tlb_t *i_tlbs;
struct tlb_t *d_tlbs;

void tlb_init(void){

	tlb_create();

	return;
}


void tlb_create(void){


	//star todo make defaults so we don't always have to include cgm_config.ini
	int num_cores = x86_cpu_num_cores;
	//int num_cus = si_gpu_num_compute_units;
	//int gpu_group_cache_num = (num_cus/4);

	//note, use calloc because it initializes the contents of the caches

	////////////
	//CPU Caches
	////////////

	//initialize the CPU L1 I Caches
	i_tlbs = (void *) calloc(num_cores, sizeof(struct tlb_t));

	//initialize the CPU L1 D Caches
	d_tlbs = (void *) calloc(num_cores, sizeof(struct tlb_t));

	return;
}

void cgm_tlb_set_entry(struct tlb_t *tlb, int vtl_tag, int set, int way, unsigned int phy_tag){

	//clear tran state
	tlb->sets[set].blocks[way].transient_state = cgm_tlb_block_invalid;

	//set entry state, vtl tag and phy tag
	tlb->sets[set].blocks[way].state = cgm_tlb_block_valid;
	tlb->sets[set].blocks[way].vtl_tag = vtl_tag;
	tlb->sets[set].blocks[way].phy_page_num = (unsigned int) phy_tag;

	//Remember offset is always available in both phy and vtl addresses.

	return;
}

void cgm_tlb_set_tran_state(struct tlb_t *tlb, int set, int tag, int way, enum cgm_tlb_block_state_t tran_state){

	tlb->sets[set].blocks[way].transient_state = tran_state;
	tlb->sets[set].blocks[way].tran_tag = tag;
	return;
}

void cgm_tlb_invalidate(struct tlb_t *tlb, int set, int way){

	tlb->sets[set].blocks[way].state = cgm_tlb_block_invalid;

	return;
}


//this is terrible...sigh... same funcion as the cache version. need to consolidate somehow.
void cgm_tlb_update_waylist(struct tlb_set_t *set, struct tlb_block_t *blk, enum cache_waylist_enum where){

	if (!blk->way_prev && !blk->way_next)
	{
		//star note: for associativity of 1 i.e. no other block in this set.
		assert(set->way_head == blk && set->way_tail == blk);
		return;
	}
	else if (!blk->way_prev)
	{
		//star note: case block is already at the head
		assert(set->way_head == blk && set->way_tail != blk);
		if (where == cache_waylist_head)
			return;
		set->way_head = blk->way_next;
		blk->way_next->way_prev = NULL;

	}
	else if (!blk->way_next)
	{
		//star note: case block is already at the tail
		assert(set->way_head != blk && set->way_tail == blk);
		if (where == cache_waylist_tail)
			return;
		set->way_tail = blk->way_prev;
		blk->way_prev->way_next = NULL;
	}
	else
	{
		//star note: case block is somewhere in between head and tail
		assert(set->way_head != blk && set->way_tail != blk);
		blk->way_prev->way_next = blk->way_next;
		blk->way_next->way_prev = blk->way_prev;
	}
	if (where == cache_waylist_head)
	{
		//star note: put the block at the head of the list
		blk->way_next = set->way_head;
		blk->way_prev = NULL;
		set->way_head->way_prev = blk;
		set->way_head = blk;
	}
	else
	{
		//star note: put the block at the tail of the list
		blk->way_prev = set->way_tail;
		blk->way_next = NULL;
		set->way_tail->way_next = blk;
		set->way_tail = blk;
	}

	return;
}

int cgm_tlb_get_victim(struct tlb_t *tlb, int set, int vtl_tag){

	int i = 0;

	struct tlb_block_t *block;

	assert(set >= 0 && set < tlb->num_sets);

	if(tlb->policy == tlb_policy_lru)
	{
		//get the tail block.
		block = tlb->sets[set].way_tail;

		//the block should not be in the transient state.
		for(i = 0; i < tlb->assoc; i++)
		{
			if(block->transient_state == cgm_tlb_block_invalid)
			{
				block->transient_state = cgm_tlb_block_transient;
				block->tran_tag = vtl_tag;
				break;
			}

			if(block->way_prev != NULL)
			{
				block = block->way_prev;
			}
			else
			{
				//we didn't find a block
				//printf("i is %d $ assoc %d\n", i, cache->assoc);
				assert(tlb->assoc == (i + 1));
				block = NULL;
			}
		}

		//assert(block->way >= 0 && block->way < cache->assoc);

		//set this block the MRU
		if(block)
		{
			cgm_tlb_update_waylist(&tlb->sets[set], block, cache_waylist_head);
			return block->way;
		}
	}
	else
	{
		fatal("cgm_tlb_get_victim(): %s invalid cache eviction policy\n", tlb->name);
	}

	return -1;

}

unsigned int cgm_tlb_get_ppn(struct tlb_t *tlb, int tag, int set, int way){

	assert(tlb->sets[set].blocks[way].vtl_tag == tag);

	return (unsigned int) tlb->sets[set].blocks[way].phy_page_num;
}

int cgm_tlb_find_entry(struct tlb_t *tlb, int *tag_ptr, int *set_ptr, unsigned int *offset_ptr, int *way_ptr, int *state_ptr){

	int set, tag, way;
	//unsigned int offset;

	/* Locate block */
	tag = *(tag_ptr);
	set = *(set_ptr);

	//offset = *(offset_ptr);

	*(state_ptr) = 0;

	for (way = 0; way < tlb->assoc; way++)
	{
		if (tlb->sets[set].blocks[way].vtl_tag == tag && tlb->sets[set].blocks[way].state)
		//if (cache->sets[set].blocks[way].tag == tag)
		{
			/* Block found */
			*(way_ptr) = way;
			*(state_ptr) = tlb->sets[set].blocks[way].state;
			return 1;
		}
	}

	assert(way == tlb->assoc);

	/* Block not found */
	return 0;
}

int cgm_tlb_get_tag(struct tlb_t *tlb, unsigned int addr){

	return (addr >> (tlb->page_set_log + tlb->page_offset_log));
}

int cgm_tlb_get_ppn_tag(struct tlb_t *tlb, unsigned int addr){

	return addr >> tlb->page_offset_log;
}

void tlb_dump_set(struct tlb_t *tlb, int set){

	int i = 0;

	for(i=0; i<tlb->assoc; i++)
		printf("tlb id %d set %d vtl tag %d phy tag %d state %d tran tag %d tran state %d \n",
				tlb->id, tlb->sets[set].id, tlb->sets[set].blocks[i].vtl_tag, tlb->sets[set].blocks[i].phy_page_num, tlb->sets[set].blocks[i].state,
				tlb->sets[set].blocks[i].tran_tag, tlb->sets[set].blocks[i].transient_state);

	return;
}


int cgm_tlb_find_transient_entry(struct tlb_t *tlb, int *tag_ptr, int *set_ptr, int *way_ptr){

	int set, tag, way;

	/* Locate block */
	tag = *(tag_ptr);
	set = *(set_ptr);

	for (way = 0; way < tlb->assoc; way++)
	{

		//printf("looking for %d found tag %d state %d\n", tag, tlb->sets[set].blocks[way].tran_tag, tlb->sets[set].blocks[way].transient_state);

		if (tlb->sets[set].blocks[way].tran_tag == tag && tlb->sets[set].blocks[way].transient_state == cgm_tlb_block_transient)
		{
			/* Block found */
			*(way_ptr) = way;
			return 1;
		}
	}


	assert(way == tlb->assoc);
	return 0;
}


/* Return {tag, set, offset} for a given address */
void cgm_tlb_probe_address(struct tlb_t *tlb, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr)
{
	*(offset_ptr) = addr & tlb->page_offset_mask;
	*(set_ptr) =  ((addr & tlb->page_set_mask) >> (tlb->page_offset_log));
	*(tag_ptr) = (addr >> (tlb->page_set_log + tlb->page_offset_log)); //addr & ~(cache->block_mask);

	unsigned int tag_size = 0xFFFFFFFF;
	tag_size = tag_size >> (tlb->page_set_log + tlb->page_offset_log);

	//warning("tag value %u tag size %u\n", *tag_ptr, tag_size);
	assert((unsigned int)*(tag_ptr) >= 0 && (unsigned int)*(tag_ptr) < tag_size);

	//warning("addr 0x%08x set value %u set size %u tlb page set mask 0x%08x page offset log %d\n", addr, *set_ptr, tlb->num_sets, tlb->page_set_mask, tlb->page_offset_log);

	assert(*(set_ptr) >= 0 && *(set_ptr) < tlb->num_sets);
	assert(*(offset_ptr) >=0 && *(offset_ptr) <= tlb->page_offset_mask);

	//fatal("done\n");
}
