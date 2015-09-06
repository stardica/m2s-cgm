/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MEM_SYSTEM_CACHE_H
#define MEM_SYSTEM_CACHE_H


extern struct str_map_t cache_policy_map;
extern struct str_map_t cache_block_state_map;

enum cache_policy_t
{
	cache_policy_invalid = 0,
	cache_policy_lru,
	cache_policy_fifo,
	cache_policy_random
};

enum cache_block_state_t
{
	cache_block_invalid = 0,
	cache_block_noncoherent,
	cache_block_modified,
	cache_block_owned,
	cache_block_exclusive,
	cache_block_shared
};

struct m2s_cache_block_t
{
	struct m2s_cache_block_t *way_next;
	struct m2s_cache_block_t *way_prev;

	int tag;
	int transient_tag;
	int way;
	int prefetched;

	enum cache_block_state_t state;
};

struct m2s_cache_set_t
{
	struct m2s_cache_block_t *way_head;
	struct m2s_cache_block_t *way_tail;
	struct m2s_cache_block_t *blocks;
};


struct m2s_cache_t
{
	char *name;

	unsigned int num_sets;
	unsigned int block_size;
	unsigned int assoc;
	enum cache_policy_t policy;

	struct m2s_cache_set_t *sets;
	unsigned int block_mask;
	int log_block_size;

	struct prefetcher_t *prefetcher;
};


struct m2s_cache_t *m2s_cache_create(char *name, unsigned int num_sets, unsigned int block_size, unsigned int assoc, enum cache_policy_t policy);
void cache_free(struct m2s_cache_t *cache);

void cache_decode_address(struct m2s_cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr);
int cache_find_block(struct m2s_cache_t *cache, unsigned int addr, int *set_ptr, int *pway, int *state_ptr);
void cache_set_block(struct m2s_cache_t *cache, int set, int way, int tag, int state);
void cache_get_block(struct m2s_cache_t *cache, int set, int way, int *tag_ptr, int *state_ptr);

void cache_access_block(struct m2s_cache_t *cache, int set, int way);
int cache_replace_block(struct m2s_cache_t *cache, int set);
void cache_set_transient_tag(struct m2s_cache_t *cache, int set, int way, int tag);


#endif
