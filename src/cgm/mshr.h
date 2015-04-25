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
	unsigned int offset;
	int valid;
	int num_entries;
	struct list_t *entires;
};

struct cgm_packet_status_t *miss_status_packet_create(long long access_id, enum cgm_access_kind_t access_type, int set, int tag, unsigned int offset, int src_id);
int mshr_set(struct cache_t *cache, struct cgm_packet_status_t *miss_status_packet, struct cgm_packet_t *message_packet);
int mshr_get(struct cache_t *cache, int *set_ptr, int *tag_ptr);
void mshr_clear(struct mshr_t *mshrs);

#endif /* MSHR_H_ */


//struct cgm_packet_status_t *mshr_remove(struct cache_t *cache);



/*struct cgm_packet_status_t *mshr_remove(struct cache_t *cache){

	int i = 0;
	struct cgm_packet_status_t *miss_status_packet;
	struct cgm_packet_status_t *message_packet;


	list_enqueue(cache->retry_queue, message_packet);



	LIST_FOR_EACH(cache->mshr, i)
	{
		mshr_packet = list_get(cache->mshr, i);

		if (mshr_packet->access_id == access_id)
		{
			return list_remove_at(cache->mshr, i);
		}
	}

	return NULL;
}*/
