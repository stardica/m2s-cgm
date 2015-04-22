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

#include <lib/util/debug.h>

struct cgm_packet_status_t *miss_status_packet_create(long long access_id, enum cgm_access_kind_t access_type, int set, int tag, unsigned int offset){

	struct cgm_packet_status_t *new_packet = status_packet_create();

	new_packet->access_id = access_id;
	new_packet->access_type = access_type;
	new_packet->tag = tag;
	new_packet->set = set;
	new_packet->offset = offset;

	if (access_id == 1)
	{
		printf("miss_status_packet created() acess_id %llu\n", access_id);
		getchar();
	}


	return new_packet;
}

//returns 1 if accesses is stored or 0 if failed/full.
int mshr_set(struct cache_t *cache, struct cgm_packet_status_t *miss_status_packet, struct cgm_packet_t *message_packet){

	unsigned int mshr_size = cache->mshr_size;

	int tag = miss_status_packet->tag;
	int set = miss_status_packet->set;
	unsigned int offset = miss_status_packet->offset;
	int i = 0;
	int row = 0;
	int size = 0;

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
			//add to mshr and increment number of entries.
			message_packet->access_type = cgm_access_retry;
			miss_status_packet->coalesced = 1;
			miss_status_packet->coalesced_packet = message_packet;

			printf("miss_status_packet->coalesced_packet name %s\n", miss_status_packet->coalesced_packet->name);
			printf("getchar() here\n");
			getchar();

			list_remove(cache->Rx_queue_top, message_packet);
			list_enqueue(cache->mshrs[row].entires, miss_status_packet);
			cache->mshrs[row].num_entries++;

			//stats
			cache->coalesces++;
			return 2;
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
				size = cache->mshrs[row].num_entries;
				break;
			}
		}

		if(size == 0)
		{
			//found empty row add to mshr and increment number of entries.
			list_enqueue(cache->mshrs[row].entires, miss_status_packet);
			cache->mshrs[row].num_entries++;

			if (miss_status_packet->access_id == 1)
			{
				printf("added to mshrs row %d total entries %d\n", row, cache->mshrs[row].num_entries);
			}

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


int mshr_get(struct cache_t *cache, int *set_ptr, int *tag_ptr){

	unsigned int mshr_size = cache->mshr_size;

	int tag = *tag_ptr;
	int set = *set_ptr;
	int i = 0;
	int row = -1;

	//seek the miss in the mshr

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

	return row;
}


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

void mshr_clear(struct mshr_t *mshrs){

	mshrs->tag = -1;
	mshrs->set = -1;
	mshrs->offset = 0;
	mshrs->num_entries = 0;
	mshrs->valid = -1;
	list_clear(mshrs->entires);

	return;
}
