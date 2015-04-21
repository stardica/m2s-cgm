/*
 * mshr.c
 *
 *  Created on: Nov 26, 2014
 *      Author: stardica
 */


#include <cgm/packet.h>
#include <cgm/protocol.h>
#include <cgm/cache.h>

struct cgm_packet_status_t *mshr_packet_create(unsigned int access_id, enum cgm_access_kind_t access_type, int set, int tag, unsigned int offset){

	struct cgm_packet_status_t *new_packet = status_packet_create();

	new_packet->access_id = access_id;
	new_packet->access_type = access_type;
	new_packet->tag = tag;
	new_packet->set = set;
	new_packet->offset = offset;

	return new_packet;
}

//returns 1 if accesses are coalesced else returns 0
int mshr_set(struct cache_t *cache, struct cgm_packet_status_t *mshr_packet){


	unsigned int size = cache->num_sets;
	unsigned int mshr_size = cache->mshr_size;
	int tag = mshr_packet->tag;
	int set = mshr_packet->set;
	unsigned int offset = mshr_packet->offset;
	int i, list_iter = 0;

	struct cgm_packet_status_t *temp;

	//store the miss in the mshr


	//check for duplicates
	for(i = 0; i < mshr_size; i ++)
	{
		//check if coalescable
		//compare the tag and set at the head of each element for a hit
		cache->mshr_2[i]->


	}


	//if coalesced return 1;




	return 0;
}


int mshr_get(struct cache_t *cache, struct cgm_packet_status_t *mshr_packet){


	return 1;
}
