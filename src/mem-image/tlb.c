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


/* Return {tag, set, offset} for a given address */
//void cgm_tlb_probe_address(struct tlb_t *tlb, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr)
//{

	//fatal("val %d %d %d\n", (cache->log_block_size + cache->log_set_size), (cache->log_block_size), (cache->log_set_size));

	//*(tag_ptr) = (addr >> (cache->log_block_size + cache->log_set_size));//addr & ~(cache->block_mask);
	//*(set_ptr) =  ((addr >> cache->log_block_size) & (cache->set_mask));//(addr >> cache->log_block_size) % cache->num_sets;
	//*(offset_ptr) = addr & (cache->block_mask);

	//unsigned int tag_size = 0xFFFFFFFF;
	//tag_size = tag_size >> (cache->log_block_size + cache->log_set_size);

	//assert(*(tag_ptr) >= 0 && *(tag_ptr) < tag_size);
	//assert(*(set_ptr) >= 0 && *(set_ptr) < cache->num_sets);
	//assert(*(offset_ptr) >=0 && *(offset_ptr) <= cache->block_mask);
//}
