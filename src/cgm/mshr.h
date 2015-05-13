/*
 * mshr.h
 *
 *  Created on: Nov 26, 2014
 *      Author: stardica
 */


#ifndef MSHR_H_
#define MSHR_H_

#include <cgm/cache.h>
#include <cgm/packet.h>

#include <lib/util/list.h>

struct mshr_t{
	char *name;
	int set;
	int tag;
	long long access_id;
	int core;
	int offset;
	int valid;
	int num_entries;
	struct list_t *entires;
};

struct cgm_packet_t *miss_status_packet_copy(struct cgm_packet_t *message_packet_old, int set, int tag, unsigned int offset, int src_id);
int mshr_get_status(struct cache_t *cache, int *tag_ptr, int *set_ptr, long long access_id);
int mshr_get_empty_row(struct cache_t *cache, int *tag_ptr, int *set_ptr, long long access_id);
int l1_mshr_set(struct cache_t *cache, int mshr_row, int *tag_ptr, int *set_ptr, long long access_id);
int l1_mshr_coalesce(struct cache_t *cache, struct cgm_packet_t *miss_status_packet, int mshr_row);

int mshr_set(struct cache_t *cache, struct cgm_packet_t *miss_status_packet);
//int mshr_get(struct cache_t *cache, int *set_ptr, int *tag_ptr, long long access_id);
void mshr_dump(struct cache_t *cache);
void mshr_clear(struct mshr_t *mshrs);

#endif /* MSHR_H_ */
