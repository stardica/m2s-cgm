/*
 * mshr.h
 *
 *  Created on: Nov 26, 2014
 *      Author: stardica
 */


#ifndef MSHR_H_
#define MSHR_H_

#include <lib/util/list.h>

#include <cgm/cache.h>
#include <cgm/packet.h>


//star todo convert the cache based mshr stuff to its own struct.

//int mshr_can_access()


struct cgm_packet_status_t *mshr_packet_create(unsigned int access_id, enum cgm_access_kind_t access_type, int set, int tag, unsigned int offset);


int mshr_set(struct cache_t *cache, struct cgm_packet_status_t *mshr_packet);


int mshr_get(struct cache_t *cache, struct cgm_packet_status_t *mshr_packet);


#endif /* MSHR_H_ */
