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
#include <lib/util/list.h>

struct cgm_packet_t *miss_status_packet_copy(struct cgm_packet_t *message_packet_old, int set, int tag, unsigned int offset, int src_id){

	struct cgm_packet_t *new_packet = packet_create();

	new_packet->access_type = message_packet_old->access_type;
	new_packet->l1_access_type = message_packet_old->access_type;
	new_packet->access_id = message_packet_old->access_id;
	new_packet->address = message_packet_old->address;
	new_packet->set = set;
	new_packet->tag = tag;
	new_packet->offset = offset;
	new_packet->src_id = src_id;

	if(message_packet_old->event_queue && message_packet_old->data)
	{
		new_packet->event_queue = message_packet_old->event_queue;
		new_packet->data = message_packet_old->data;
	}

	if(message_packet_old->cpu_access_type)
	{
		new_packet->cpu_access_type = message_packet_old->cpu_access_type;
	}

	if(message_packet_old->gpu_access_type)
	{
		new_packet->gpu_access_type = message_packet_old->gpu_access_type;
	}

	assert(new_packet->address != 0 || new_packet->access_id != 0);
	//assert(new_packet->set != 0 || new_packet->tag != 0);

	return new_packet;
}

//returns 1 if accesses is stored or 0 if failed/full.
int mshr_set(struct cache_t *cache, struct cgm_packet_t *miss_status_packet){

	//unsigned int mshr_size = cache->mshr_size;

	int tag = miss_status_packet->tag;
	int set = miss_status_packet->set;
	//unsigned int offset = miss_status_packet->offset;
	int i = 0;
	int row = 0;
	int size = 0;


	//CGM_DEBUG(mshr_debug_file, "cycle %llu access_id %llu tag %d set %d\n", P_TIME, miss_status_packet->access_id, miss_status_packet->tag, miss_status_packet->set);

	//store the miss in the mshr
	//check for existing memory accesses to same set and tag
	for(i = 0; i < cache->mshr_size; i ++)
	{
		//compare if the mshr has entries compare the tag and set
		if(cache->mshrs[i].tag == tag && cache->mshrs[i].set == set)
		{
			row = i;
			break;
		}

	}


	if(row)
	{
		//duplicate tag and set found, but row is full.
		if(cache->mshrs[row].num_entries > cache->max_coal)
		{
			CGM_DEBUG(mshr_debug_file, "%s mshr access_id %llu cycle %llu row full\n", cache->name, miss_status_packet->access_id, P_TIME);
			return 0;
		}
		else
		{

			CGM_DEBUG(mshr_debug_file, "mshr[%d] access_id %llu cycle %llu tag %d set %d coalesced in row %d and size %d\n",
					cache->id, miss_status_packet->access_id, P_TIME, miss_status_packet->tag, miss_status_packet->set, row, list_count(cache->mshrs[row].entires));

			//add to mshr and increment number of entries.
			miss_status_packet->access_type = cgm_access_retry;
			miss_status_packet->coalesced = 1;
			cache->mshrs[row].num_entries++;

			//fprintf(cgm_debug, "access_id %llu at %llu\n", miss_status_packet->access_id, P_TIME);
			//fprintf(cgm_debug, "coalesced in mshr row %d with size %d\n\n", row, cache->mshrs[row].num_entries);

			//list_remove(cache->Rx_queue_top, message_packet);
			list_enqueue(cache->mshrs[row].entires, miss_status_packet);

			//stats
			cache->coalesces++;
			return 2;
		}

	}
	else
	{
		//access insn't in mshr so find the first empty row
		for(i = 0; i < cache->mshr_size; i ++)
		{
			//compare if the mshr has entries compare the tag and set
			if(cache->mshrs[i].num_entries == 0)
			{
				row = i;
				break;
			}
		}

		if(cache->mshrs[row].num_entries == 0)
		{
			//found empty row add to mshr and increment number of entries.
			cache->mshrs[row].tag = tag;
			cache->mshrs[row].set = set;
			cache->mshrs[row].num_entries++;

			miss_status_packet->access_type = cgm_access_retry;
			miss_status_packet->coalesced = 0;

			list_enqueue(cache->mshrs[row].entires, miss_status_packet);

			//stats
			cache->mshr_entires++;
			return 1;

		}
		else
		{

			fatal("mshr_set(): non empty row selected cycle %llu access_id %llu\n", P_TIME, miss_status_packet->access_id);
			//return 0;
		}

	}

	fatal("mshr_set() reached bottom\n");
}


int mshr_get(struct cache_t *cache, int *set_ptr, int *tag_ptr, long long access_id){

	//unsigned int mshr_size = cache->mshr_size;
	int tag = *tag_ptr;
	int set = *set_ptr;
	long long id = access_id;
	int i = 0, j = 0;
	int row = -1;

	struct cgm_packet_t *temp;
	//seek the miss in the mshr

	//check for existing memory accesses to same set and tag
	for(i = 0; i < cache->mshr_size; i ++)
	{
		//compare if the mshr entries match the tag and set
		if(cache->mshrs[i].num_entries > 0 && cache->mshrs[i].tag == tag && cache->mshrs[i].set == set)
		{
			row = i;

			//star todo fix this there is a temporal issue with the MSHR
			temp = list_get(cache->mshrs[i].entires, 0);
			if(temp->access_id == access_id)
			{
				break;
			}
			/*else if (temp->access_id != access_id)
			{
				row = -1;
			}*/
		}
	}

	return row;
}

void mshr_dump(struct cache_t *cache){

	int i = 0, j = 0;
	struct cgm_packet_t *temp;

	for(i = 0; i < cache->mshr_size; i ++)
	{
		printf("mshr[%d] size %d\n", i, cache->mshrs[i].num_entries);

		for(j = 0; j < cache->mshrs[i].num_entries; j ++)
		{
			temp = list_get(cache->mshrs[i].entires, 0);

			if(temp)
			{
				printf("entry %d access_id %llu tag %d set %d\n", j, temp->access_id, temp->tag, temp->set);
			}
			else
			{
				printf("entry pulled but list not cleared yet\n");
			}

		}
	}

	return;
}

void mshr_clear(struct mshr_t *mshrs){

	mshrs->tag = -1;
	mshrs->set = -1;
	mshrs->offset = -1;
	mshrs->num_entries = 0;
	mshrs->valid = -1;
	list_clear(mshrs->entires);

	return;
}
