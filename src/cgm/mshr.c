/*
 * mshr.c
 *
 *  Created on: Nov 26, 2014
 *      Author: stardica
 */


#include <cgm/mshr.h>
#include <cgm/packet.h>
#include <cgm/protocol.h>
#include <cgm/cache.h>

struct cgm_packet_status_t *miss_status_packet_create(long long access_id, enum cgm_access_kind_t access_type, int set, int tag, unsigned int offset){

	struct cgm_packet_status_t *new_packet = status_packet_create();

	new_packet->access_id = access_id;
	new_packet->access_type = access_type;
	new_packet->tag = tag;
	new_packet->set = set;
	new_packet->offset = offset;

	return new_packet;
}

//returns 1 if accesses is stored or 0 if failed/full.
int mshr_set(struct cache_t *cache, struct cgm_packet_status_t *miss_status_packet){

	unsigned int size = cache->num_sets;
	unsigned int mshr_size = cache->mshr_size;

	int tag = miss_status_packet->tag;
	int set = miss_status_packet->set;
	unsigned int offset = miss_status_packet->offset;
	int i, row, size_mshr = 0;

	//store the miss in the mshr

	//check for existing memory accesses to same set and tag
	for(i = 0; i < mshr_size; i ++)
	{
		//compare if the mshr has entries compare the tag and set
		if(cache->mshrs[i].num_entries > 0 && cache->mshrs[i].tag == tag && cache->mshrs[i].set == set)
		{
			row = i;
			break;
		}

	}

	if(row)
	{
		//duplicate tag and set detected try to coalesce up to the maximum
		if(cache->mshrs[row].num_entries > cache->max_coal)
		{
			//row is present but full
			return 0;
		}
		else
		{
			//add to mshr and increment number of entires.
			list_enqueue(cache->mshrs->entires, miss_status_packet);
			cache->mshrs[row].num_entries++;

			//stats
			cache->coalesces++;
			return 1;
		}

	}
	else
	{
		//access insn't in mshr so find the first empty row
		for(i = 0; i < mshr_size; i ++)
		{
			//compare if the mshr has entries compare the tag and set
			if(cache->mshrs[i].num_entries == 0)
			{
				row = i;
				size_mshr = cache->mshrs[i].num_entries;
				break;
			}
		}

		if(size == 0)
		{
			//found empty row
			//add to mshr and increment number of entries.
			list_enqueue(cache->mshrs->entires, miss_status_packet);
			cache->mshrs[row].num_entries++;

			//stats
			cache->mshr_entires++;
			return 1;
		}
		else
		{
			//no open rows mshr is full
			return 0;
		}

	}

	fatal("mshr_set() reached bottom\n");
}


int mshr_get(struct cache_t *cache, struct cgm_packet_status_t *mshr_packet){


	return 1;
}


struct cgm_packet_status_t *mshr_remove(struct cache_t *cache, long long access_id){

	int i = 0;
	struct cgm_packet_status_t *mshr_packet;

	LIST_FOR_EACH(cache->mshr, i)
	{
		mshr_packet = list_get(cache->mshr, i);

		if (mshr_packet->access_id == access_id)
		{
			return list_remove_at(cache->mshr, i);
		}
	}

	return NULL;
}
