/*
 * cache.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */


#include <cgm/cache.h>


struct str_map_t cgm_cache_block_state_map =
{ 	7, 	{
		{ "I", cache_block_invalid},
		{ "N", cache_block_noncoherent },
		{ "M", cache_block_modified },
		{ "O", cache_block_owned },
		{ "E", cache_block_exclusive },
		{ "S", cache_block_shared },
		{ "!", cache_block_null },
		}
};


struct str_map_t cgm_cache_policy_map =
{
	3, 	{
		{ "LRU", cache_policy_lru },
		{ "FIFO", cache_policy_fifo },
		{ "Random", cache_policy_random }
		}
};


int QueueSize;
int l1_i_inf = 0;
int l1_d_inf = 0;
int l2_inf = 0;
int l3_inf = 0;
int l1_i_miss = 0;
int l1_d_miss = 0;
int l2_miss = 0;
int l3_miss = 0;
int gpu_l1_inf = 0;
int gpu_l2_inf = 0;


//CPU caches
struct cache_t *l1_i_caches;
struct cache_t *l1_d_caches;
struct cache_t *l2_caches;
struct cache_t *l3_caches;

//GPU caches
struct cache_t *gpu_v_caches;
struct cache_t *gpu_s_caches;
struct cache_t *gpu_l2_caches;
struct cache_t *gpu_lds_units;

//global tasking related
int l1_i_pid = 0;
int l1_d_pid = 0;
int l2_pid = 0;
int l3_pid = 0;
int gpu_v_pid = 0;
int gpu_s_pid = 0;
int gpu_l2_pid = 0;
int gpu_lds_pid = 0;

int l1_i_io_pid = 0;
int l1_d_io_pid = 0;
int l2_up_io_pid = 0;
int l2_down_io_pid = 0;
int l3_up_io_pid = 0;
int l3_down_io_pid = 0;
int gpu_v_io_pid = 0;
int gpu_s_io_pid = 0;
int gpu_l2_down_io_pid = 0;
int gpu_l2_up_io_pid = 0;
int gpu_lds_io_pid = 0;


//event counts
eventcount volatile *l1_i_cache;
eventcount volatile *l1_d_cache;
eventcount volatile *l2_cache;
eventcount volatile *l3_cache;
eventcount volatile *gpu_l2_cache;
eventcount volatile *gpu_v_cache;
eventcount volatile *gpu_s_cache;
eventcount volatile *gpu_lds_unit;

task *l1_i_cache_tasks;
task *l1_d_cache_tasks;
task *l2_cache_tasks;
task *l3_cache_tasks;
task *gpu_l2_cache_tasks;
task *gpu_v_cache_tasks;
task *gpu_s_cache_tasks;
task *gpu_lds_tasks;

void cache_init(void){

	cache_create();
	cache_create_tasks();

	return;
}

void cache_create(void){


	//star todo make defaults so we don't always have to include cgm_config.ini
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);


	////////////
	//CPU Caches
	////////////

	//initialize the CPU L1 I Caches
	l1_i_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	//initialize the CPU L1 D Caches
	l1_d_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	//initialize the CPU L2 caches
	l2_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	//initialize the L3 caches (1 slice per core).
	l3_caches = (void *) calloc(num_cores, sizeof(struct cache_t));

	////////////
	//GPU Caches
	////////////

	//initialize the GPU L1V caches
	gpu_v_caches = (void *) calloc(num_cus, sizeof(struct cache_t));

	//initialize the GPU L1S caches
	gpu_s_caches = (void *) calloc(num_cus, sizeof(struct cache_t));

	//initialize the GPU L2 caches.
	gpu_l2_caches = (void *) calloc(gpu_group_cache_num, sizeof(struct cache_t));

	//initialize the GPU LDS
	gpu_lds_units = (void *) calloc(num_cus, sizeof(struct cache_t));

	return ;
}

void cache_create_tasks(void){

	//star todo make this dynamic
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	char buff[100];
	int i = 0;

	/////////////
	//eventcounts
	/////////////

	//l1_i_caches
	l1_i_cache = (void *) calloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_cache_%d", i);
		l1_i_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//l1 d caches
	l1_d_cache = (void *) calloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_cache_%d", i);
		l1_d_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//l2 caches
	l2_cache = (void *) calloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l2_cache_%d", i);
		l2_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//l3 caches
	l3_cache = (void *) calloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l3_cache_%d", i);
		l3_cache[i] = *(new_eventcount(strdup(buff)));
	}



	//GPU L2
	gpu_l2_cache = (void *) calloc(gpu_group_cache_num, sizeof(eventcount));
	for(i = 0; i < gpu_group_cache_num; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_cache_%d", i);
		gpu_l2_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//GPU vector
	gpu_v_cache = (void *) calloc(num_cus, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_cache_%d", i);
		gpu_v_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//GPU scalar
	gpu_s_cache = (void *) calloc(num_cus, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_cache_%d", i);
		gpu_s_cache[i] = *(new_eventcount(strdup(buff)));
	}

	//GPU lds
	gpu_lds_unit = (void *) calloc(num_cus, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_lds_unit_%d", i);
		gpu_lds_unit[i] = *(new_eventcount(strdup(buff)));
	}


	////////////////////
	//tasks
	////////////////////

	//l1 i caches
	l1_i_cache_tasks = (void *) calloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l1_i_cache_ctrl_%d", i);
		l1_i_cache_tasks[i] = *(create_task(l1_i_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
		//printf("l1_i_cache_tasks[i].id = %d name = %s\n", l1_i_cache_tasks[i].id, l1_i_cache_tasks[i].name);
	}

	//l1 d caches
	l1_d_cache_tasks = (void *) calloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l1_d_cache_ctrl_%d", i);
		l1_d_cache_tasks[i] = *(create_task(l1_d_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//l2 caches
	l2_cache_tasks = (void *) calloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l2_cache_ctrl_%d", i);
		l2_cache_tasks[i] = *(create_task(l2_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//l3 caches
	l3_cache_tasks = (void *) calloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "l3_cache_ctrl_%d", i);
		l3_cache_tasks[i] = *(create_task(l3_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//gpu l2 caches
	gpu_l2_cache_tasks = (void *) calloc(gpu_group_cache_num, sizeof(task));
	for(i = 0; i < gpu_group_cache_num; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_l2_cache_ctrl");
		gpu_l2_cache_tasks[i] = *(create_task(gpu_l2_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//gpu v caches
	gpu_v_cache_tasks = (void *) calloc(num_cus, sizeof(task));
	for(i = 0; i < num_cus; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_v_cache_ctrl");
		gpu_v_cache_tasks[i] = *(create_task(gpu_v_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//gpu s caches
	gpu_s_cache_tasks = (void *) calloc(num_cus, sizeof(task));
	for(i = 0; i < num_cus; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_s_cache_ctrl");
		gpu_s_cache_tasks[i] = *(create_task(gpu_s_cache_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//gpu lds unit
	gpu_lds_tasks = (void *) calloc(num_cus, sizeof(task));
	for(i = 0; i < num_cus; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "gpu_lds_unit_ctrl");
		gpu_lds_tasks[i] = *(create_task(gpu_lds_unit_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	return;
}

struct cgm_packet_t *cache_get_message(struct cache_t *cache){

	struct cgm_packet_t *new_message;

	//get various cache statuses
	int ort_status = get_ort_status(cache);
	int ort_coalesce_size = list_count(cache->ort_list);
	int retry_queue_size = list_count(cache->retry_queue);
	int bottom_queue_size = list_count(cache->Rx_queue_bottom);

	assert(ort_status <= cache->mshr_size);
	assert(ort_coalesce_size <= (cache->max_coal + 1));

	/*if the ort or the coalescer are full we can't process a CPU request
	because a miss will overrun the table.*/

	//pull from the retry queue if we have accesses waiting...
	if((ort_status == cache->mshr_size || ort_coalesce_size > cache->max_coal) && retry_queue_size > 0)
	{
		new_message = list_get(cache->retry_queue, 0);
		cache->last_queue = cache->retry_queue;

		assert(new_message);
	}
	//pull from the bottom queue if the retry queue is empty...
	else if ((ort_status == cache->mshr_size || ort_coalesce_size > cache->max_coal) && bottom_queue_size > 0)
	{
		new_message = list_get(cache->Rx_queue_bottom, 0);
		cache->last_queue = cache->Rx_queue_bottom;

		assert(new_message);
	}
	//ORT is not full, we can process CPU requests and lower level cache replies in a round robin fashion.
	else if(ort_status < cache->mshr_size && ort_coalesce_size <= cache->max_coal)
	{
		//try to pull from the retry queue first.
		if(retry_queue_size > 0)
		{
			new_message = list_get(cache->retry_queue, 0);
			cache->last_queue = cache->retry_queue;
			assert(new_message);
		}
		else
		{
			new_message = list_get(cache->next_queue, 0);

			//keep pointer to last queue
			cache->last_queue = cache->next_queue;

			//rotate the queues
			if(cache->next_queue == cache->Rx_queue_top)
			{
				cache->next_queue = cache->Rx_queue_bottom;
			}
			else if(cache->next_queue == cache->Rx_queue_bottom)
			{
				cache->next_queue = cache->Rx_queue_top;
			}
			else
			{
				fatal("get_message() pointers arn't working");
			}

			//if we didn't get a message try again (now that the queues are rotated)
			if(new_message == NULL)
			{

				new_message = list_get(cache->next_queue, 0);

				//keep pointer to last queue
				cache->last_queue = cache->next_queue;

				//rotate the queues.
				if(cache->next_queue == cache->Rx_queue_top)
				{
					cache->next_queue = cache->Rx_queue_bottom;
				}
				else if(cache->next_queue == cache->Rx_queue_bottom)
				{
					cache->next_queue = cache->Rx_queue_top;
				}
				else
				{
					fatal("get_message() pointers arn't working");
				}
			}
		}
	}
	else
	{
		//ORT is full and there are no retry or bottom queue messages so stall
		return NULL;
	}

	//if we made it here we better have a message.
	assert(new_message);

	//debugging
	if(cache->cache_type == l1_i_cache_t || cache->cache_type == l1_d_cache_t || cache->cache_type == l2_cache_t || cache->cache_type == l3_cache_t)
	{
		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu pulled from %s queue size %d\n",
				cache->name, new_message->access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
	}
	else if(cache->cache_type == gpu_s_cache_t || cache->cache_type == gpu_v_cache_t || cache->cache_type == gpu_l2_cache_t)
	{
		CGM_DEBUG(GPU_cache_debug_file, "%s access_id %llu cycle %llu pulled from %s queue size %d\n",
				cache->name, new_message->access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
	}
	else
	{
		fatal("cache_get_message(): bad cache type\n");
	}
	return new_message;
}

int get_ort_status(struct cache_t *cache){

	int i = 0;

	// checks the ort to find an empty row
	for (i = 0; i < cache->mshr_size; i++)
	{
		if(cache->ort[i][0] == -1 && cache->ort[i][1] == -1 && cache->ort[i][2] == -1)
		{
			//hit in the ORT table
			break;
		}
	}
	return i;
}

int get_ort_num_coalesced(struct cache_t *cache, int entry, int tag, int set){

	int i = 0;
	int size = 0;
	struct cgm_packet_t *ort_packet;

	LIST_FOR_EACH(cache->ort_list, i)
	{
		//get pointer to access in queue and check it's status.
		ort_packet = list_get(cache->ort_list, i);

		if(ort_packet->tag == tag && ort_packet->set == set)
		{
			ort_packet = list_remove_at(cache->ort_list, i);
			ort_packet->access_type = cgm_access_retry;
			list_enqueue(cache->retry_queue, ort_packet);
			advance(cache->ec_ptr);

			//this may cause problems the intent is to run one coalesced packet per iteration of the retry state.
			size ++;
		}
	}

	return size;
}

int ort_search(struct cache_t *cache, int tag, int set){

	int i = 0;

	for (i = 0; i < cache->mshr_size; i++)
	{
		if(cache->ort[i][0] == tag && cache->ort[i][1] == set && cache->ort[i][2] == 1)
		{
			//hit in the ORT table
			break;
		}
	}

	return i;
}

void ort_clear(struct cache_t *cache, int entry){

	cache->ort[entry][0] = -1;
	cache->ort[entry][1] = -1;
	cache->ort[entry][2] = -1;

	return;
}

void ort_set(struct cache_t *cache, int entry, int tag, int set){

	cache->ort[entry][0] = tag;
	cache->ort[entry][1] = set;
	cache->ort[entry][2] = 1;

	return;
}


void ort_dump(struct cache_t *cache){

	int i = 0;

	for (i = 0; i <  cache->mshr_size; i++)
	{
		printf("ort row %d tag %d set %d valid %d\n", i, cache->ort[i][0], cache->ort[i][1], cache->ort[i][2]);
	}
	return;
}


void cgm_cache_get_block(struct cache_t *cache, int set, int way, int *tag_ptr, int *state_ptr)
{
	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);
	PTR_ASSIGN(tag_ptr, cache->sets[set].blocks[way].tag);
	PTR_ASSIGN(state_ptr, cache->sets[set].blocks[way].state);
}



int cgm_l3_cache_map(int *set){

	int num_cores = x86_cpu_num_cores;
	int map = -1;

	//star todo fix this
	int map_type = l3_caches[0].slice_type;
	//int map_type = 1;

	//stripe or block
	if (map_type == 1)
	{
		//map = *(set) % num_cores;
		//star this is a faster way to do the look up.
		map = (unsigned int) *set & (unsigned int) (num_cores - 1);
	}
	else if (map_type == 0)
	{
		//get the address range
		//fatal("cgm_l3_cache_map(): invalid map_type set\n");
		map = 0;
	}
	else
	{
		fatal("cgm_l3_cache_map(): invalid map_type set\n");
	}

	assert(map >= 0 && map <= (num_cores - 1));
	return map;
}

int cgm_gpu_cache_map(int cache_id){

	int map;

	if(cache_id < 4)
	{
		map = 0;
	}
	else if(cache_id >= 4 && cache_id < 8)
	{
		map = 1;
	}
	else if(cache_id >= 8 && cache_id < 12)
	{
		map = 2;
	}
	else if(cache_id >= 12 && cache_id < 16)
	{
		map = 3;
	}
	else if(cache_id >= 16 && cache_id < 20)
	{
		map = 4;
	}
	else if(cache_id >= 20 && cache_id < 24)
	{
		map = 5;
	}
	else if(cache_id >= 24 && cache_id < 28)
	{
		map = 6;
	}
	else if(cache_id >= 28 && cache_id < 32)
	{
		map = 7;
	}
	else if (cache_id < 0 && cache_id > 31)
	{
		fatal("cgm_gpu_cache_map(): cache_id out of bounds\n");
	}

	return map;
}


int cache_can_access_top(struct cache_t *cache){

	int i = 0;
	int j = 0;

	//check if mshr/ort queue is full
	for (i = 0; i < cache->mshr_size; i++)
	{
		if(cache->ort[i][0] == -1 && cache->ort[i][1] == -1 && cache->ort[i][2] == -1)
		{
			//hit in the ORT table
			break;
		}
	}

	if(i >= cache->mshr_size -1)
	{
		return 0;
	}


	//check if in queue is full
	if(QueueSize <= list_count(cache->Rx_queue_top))
	{
		return 0;
	}

	//cache queue is accessible.
	return 1;
}


int cache_can_access_bottom(struct cache_t *cache){


	//check if in queue is full
	if(QueueSize <= list_count(cache->Rx_queue_bottom))
	{
		return 0;
	}

	//cache queue is accessible.
	return 1;
}


/* Return {tag, set, offset} for a given address */
void cgm_cache_probe_address(struct cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr, unsigned int *offset_ptr)
{
	//star i reworked this a little//addr = message_packet->address;
	*(tag_ptr) = (addr >> (cache->log_block_size + cache->log_set_size));//addr & ~(cache->block_mask);
	*(set_ptr) =  (addr >> (cache->log_block_size) & (cache->set_mask));//(addr >> cache->log_block_size) % cache->num_sets;
	*(offset_ptr) = addr & (cache->block_mask);
}


/* Look for a block in the cache. If it is found and its state is other than 0,
 * the function returns 1 and the state and way of the block are also returned.
 * The set where the address would belong is returned anyways. */
int cgm_cache_find_block(struct cache_t *cache, int *tag_ptr, int *set_ptr, unsigned int *offset_ptr, int *way_ptr, int *state_ptr){

	int set, tag, way;
	//unsigned int offset;

	/* Locate block */
	tag = *(tag_ptr);
	set = *(set_ptr);
	//offset = *(offset_ptr);

	*(state_ptr) = 0;

	for (way = 0; way < cache->assoc; way++)
	{
		if (cache->sets[set].blocks[way].tag == tag && cache->sets[set].blocks[way].state)
		//if (cache->sets[set].blocks[way].tag == tag)
		{
			/* Block found */
			*(way_ptr) = way;
			*(state_ptr) = cache->sets[set].blocks[way].state;
			return 1;
		}
	}

	/* Block not found */
	if (way == cache->assoc)
	{
		return 0;
	}
	else
	{
		fatal("cgm_cache_find_block() incorrect behavior\n");
	}

	fatal("cgm_cache_find_block() reached bottom\n");
}

/* Set the tag and state of a block.
 * If replacement policy is FIFO, update linked list in case a new
 * block is brought to cache, i.e., a new tag is set. */
void cgm_cache_set_block(struct cache_t *cache, int set, int way, int tag, int state)
{
	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);

	if (cache->policy == cache_policy_fifo && cache->sets[set].blocks[way].tag != tag)
	{
		cgm_cache_update_waylist(&cache->sets[set], &cache->sets[set].blocks[way], cache_waylist_head);
	}

	cache->sets[set].blocks[way].tag = tag;
	cache->sets[set].blocks[way].state = state;
}

/* Return the way of the block to be replaced in a specific set,
 * depending on the replacement policy */
int cgm_cache_replace_block(struct cache_t *cache, int set)
{
	struct cache_block_t *block;

	/* Try to find an invalid block. Do this in the LRU order, to avoid picking the
	 * MRU while its state has not changed to valid yet. */
	assert(set >= 0 && set < cache->num_sets);

	/*for (block = cache->sets[set].way_tail; block; block = block->way_prev)
		if (!block->state)
			return block->way;*/

	/* LRU and FIFO replacement: return block at the
	 * tail of the linked list */
	if (cache->policy == cache_policy_lru || cache->policy == cache_policy_fifo)
	{
		int way = cache->sets[set].way_tail->way;
		cgm_cache_update_waylist(&cache->sets[set], cache->sets[set].way_tail, cache_waylist_head);
		return way;
	}

	//star >> never enters here on LRU policy.
	/* Random replacement */
	assert(cache->policy == cache_policy_random);
	assert(0);
	return random() % cache->assoc;
}

void cgm_cache_access_block(struct cache_t *cache, int set, int way)
{
	int move_to_head;

	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);

	/* A block is moved to the head of the list for LRU policy.
	 * It will also be moved if it is its first access for FIFO policy, i.e., if the
	 * state of the block was invalid. */
	move_to_head = cache->policy == cache_policy_lru || (cache->policy == cache_policy_fifo && !cache->sets[set].blocks[way].state);
	if (move_to_head && cache->sets[set].blocks[way].way_prev)
		cgm_cache_update_waylist(&cache->sets[set], &cache->sets[set].blocks[way], cache_waylist_head);
}

void cgm_cache_update_waylist(struct cache_set_t *set, struct cache_block_t *blk, enum cache_waylist_enum where){

	if (!blk->way_prev && !blk->way_next)
	{
		assert(set->way_head == blk && set->way_tail == blk);
		return;

	}
	else if (!blk->way_prev)
	{
		assert(set->way_head == blk && set->way_tail != blk);
		if (where == cache_waylist_head)
			return;
		set->way_head = blk->way_next;
		blk->way_next->way_prev = NULL;

	}
	else if (!blk->way_next)
	{
		assert(set->way_head != blk && set->way_tail == blk);
		if (where == cache_waylist_tail)
			return;
		set->way_tail = blk->way_prev;
		blk->way_prev->way_next = NULL;

	}
	else
	{
		assert(set->way_head != blk && set->way_tail != blk);
		blk->way_prev->way_next = blk->way_next;
		blk->way_next->way_prev = blk->way_prev;
	}

	if (where == cache_waylist_head)
	{
		blk->way_next = set->way_head;
		blk->way_prev = NULL;
		set->way_head->way_prev = blk;
		set->way_head = blk;
	}
	else
	{
		blk->way_prev = set->way_tail;
		blk->way_next = NULL;
		set->way_tail->way_next = blk;
		set->way_tail = blk;
	}
}

void cache_dump_stats(void){

	int num_cores = x86_cpu_num_cores;
	int num_threads = x86_cpu_num_threads;
	int i = 0;

	CGM_STATS(cgm_stats_file, "[General]\n");
	CGM_STATS(cgm_stats_file, "NumCores = %d\n", num_cores);
	CGM_STATS(cgm_stats_file, "ThreadsPerCore = %d\n", num_threads);
	CGM_STATS(cgm_stats_file, "TotalCycles = %lld\n", P_TIME);
	CGM_STATS(cgm_stats_file, "\n");

	for(i = 0; i < num_cores; i++)
	{
		CGM_STATS(cgm_stats_file, "[L1_I_Cache_%d]\n", i);
		CGM_STATS(cgm_stats_file, "Sets = %d\n", l1_i_caches[i].num_sets);
		CGM_STATS(cgm_stats_file, "BlockSize = %d\n", l1_i_caches[i].block_size);
		CGM_STATS(cgm_stats_file, "Fetches = %lld\n", l1_i_caches[i].loads);
		CGM_STATS(cgm_stats_file, "Hits = %lld\n", l1_i_caches[i].hits);
		CGM_STATS(cgm_stats_file, "Misses = %lld\n", l1_i_caches[i].misses);
		CGM_STATS(cgm_stats_file, "\n");

		CGM_STATS(cgm_stats_file, "[L1_D_Cache_%d]\n", i);
		CGM_STATS(cgm_stats_file, "Sets = %d\n", l1_d_caches[i].num_sets);
		CGM_STATS(cgm_stats_file, "BlockSize = %d\n", l1_d_caches[i].block_size);
		CGM_STATS(cgm_stats_file, "Loads = %lld\n", l1_d_caches[i].loads);
		CGM_STATS(cgm_stats_file, "Stores = %lld\n", l1_d_caches[i].stores);
		CGM_STATS(cgm_stats_file, "Hits = %lld\n", l1_d_caches[i].hits);
		CGM_STATS(cgm_stats_file, "Misses = %lld\n", l1_d_caches[i].misses);
		CGM_STATS(cgm_stats_file, "\n");

		CGM_STATS(cgm_stats_file, "[L2_Cache_%d]\n", i);
		CGM_STATS(cgm_stats_file, "Sets = %d\n", l2_caches[i].num_sets);
		CGM_STATS(cgm_stats_file, "BlockSize = %d\n", l2_caches[i].block_size);
		CGM_STATS(cgm_stats_file, "Accesses = %lld\n", (l2_caches[i].fetches + l2_caches[i].loads + l2_caches[i].stores));
		CGM_STATS(cgm_stats_file, "Hits = %lld\n", l2_caches[i].hits);
		CGM_STATS(cgm_stats_file, "Misses = %lld\n", l2_caches[i].misses);
		CGM_STATS(cgm_stats_file, "\n");
	}

	return;
}

void l1_i_cache_ctrl(void){

	//my_pid increments for the number of CPU cores. i.e. 0 - 4 for a quad core
	int my_pid = l1_i_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{

		//wait here until there is a job to do
		await(&l1_i_cache[my_pid], step);

		//try to pull a message from one of the input queues.
		message_packet = cache_get_message(&(l1_i_caches[my_pid]));

		if (message_packet == NULL || !cache_can_access_top(&l2_caches[my_pid]))
		{
			//the cache state is preventing the cache from working this cycle stall
			//PRINT("l1_i_cache null packet bottom queue size %d cycle %llu\n", list_count(l1_i_caches[my_pid].Rx_queue_bottom), P_TIME);
			P_PAUSE(1);
		}
		else
		{
			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if (access_type == cgm_access_fetch)
			{
				cpu_l1_cache_access_load(&(l1_i_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_puts)
			{
				//message from L2
				cpu_cache_access_put(&(l1_i_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_retry)
			{
				//organic retry
				cpu_cache_access_retry(&(l1_i_caches[my_pid]), message_packet);
			}
			else
			{
				fatal("l1_i_cache_ctrl(): %s access_id %llu bad access type %s at cycle %llu\n",
						l1_i_caches[my_pid].name, access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}
		//could potentially do some work here...
	}
	/* should never get here*/
	fatal("l1_i_cache_ctrl task is broken\n");
	return;
}

void l1_d_cache_ctrl(void){

	int my_pid = l1_d_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{
		//wait here until there is a job to do.
		await(&l1_d_cache[my_pid], step);

		//get the message out of the queue
		message_packet = cache_get_message(&(l1_d_caches[my_pid]));

		if (message_packet == NULL || !cache_can_access_top(&l2_caches[my_pid]))
		{
			//the cache state is preventing the cache from working this cycle stall.
			P_PAUSE(1);
			//future_advance(&l1_d_cache[my_pid], etime.count + 2);
		}
		else
		{
			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if (access_type == cgm_access_load)
			{
				cpu_l1_cache_access_load(&(l1_d_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_store)
			{
				cpu_l1_cache_access_store(&(l1_d_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_puts)
			{
				cpu_cache_access_put(&(l1_d_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_retry)
			{
				cpu_cache_access_retry(&(l1_d_caches[my_pid]), message_packet);
			}
			else
			{
				fatal("l1_d_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
						access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}
	}

	//should never get here
	fatal("l1_d_cache_ctrl task is broken\n");
	return;
}

void l2_cache_ctrl(void){

	int my_pid = l2_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{
		/*wait here until there is a job to do.*/
		await(&l2_cache[my_pid], step);

		//check the top or bottom rx queues for messages.
		message_packet = cache_get_message(&(l2_caches[my_pid]));

		if (message_packet == NULL || !switch_can_access(switches[my_pid].north_queue))
		{
			//the cache state is preventing the cache from working this cycle stall.
			//PRINT("l2_cache null packet cycle %llu\n", P_TIME);
			P_PAUSE(1);
			//future_advance(&l2_cache[my_pid], etime.count + 2);
		}
		else
		{
			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if(access_type == cgm_access_gets_i || access_type == cgm_access_gets_d)
			{
				cpu_cache_access_get(&l2_caches[my_pid], message_packet);
			}
			else if (access_type == cgm_access_getx)
			{

			}
			else if(access_type == cgm_access_puts)
			{
				cpu_cache_access_put(&l2_caches[my_pid], message_packet);
			}
			else if (access_type == cgm_access_retry)
			{
				cpu_cache_access_retry(&l2_caches[my_pid], message_packet);
			}
			else
			{
				fatal("l2_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
						access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}

		//could potentially do some work here.

	}

	/* should never get here*/
	fatal("l2_cache_ctrl task is broken\n");
	return;
}

void l3_cache_ctrl(void){

	int my_pid = l3_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;
	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{
		/*wait here until there is a job to do.*/
		await(&l3_cache[my_pid], step);

		//get the message out of the queue
		message_packet = cache_get_message(&(l3_caches[my_pid]));

		if (message_packet == NULL || !switch_can_access(switches[my_pid].south_queue))
		{
			//the cache state is preventing the cache from working this cycle stall.
			P_PAUSE(1);
		}
		else
		{
			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if (access_type == cgm_access_gets)
			{
				cpu_cache_access_get(&l3_caches[my_pid], message_packet);
			}
			else if (access_type == cgm_access_puts)
			{
				cpu_cache_access_put(&l3_caches[my_pid], message_packet);
			}
			else if (access_type == cgm_access_retry)
			{
				cpu_cache_access_retry(&l3_caches[my_pid], message_packet);
			}
			else
			{
				fatal("l3_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
						access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}
	}
	/* should never get here*/
	fatal("l3_cache_ctrl task is broken\n");
	return;
}

void gpu_s_cache_ctrl(void){

	int my_pid = gpu_s_pid++;
	int num_cus = si_gpu_num_compute_units;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

	assert(my_pid <= num_cus);
	set_id((unsigned int)my_pid);

	while(1)
	{
		//wait here until there is a job to do
		await(&gpu_s_cache[my_pid], step);

		//get a message from the top or bottom queues.
		message_packet = cache_get_message(&(gpu_s_caches[my_pid]));

		if (message_packet == NULL || !cache_can_access_top(&gpu_l2_caches[cgm_gpu_cache_map(my_pid)]))
		{
			P_PAUSE(1);
			//future_advance(&gpu_s_cache[my_pid], etime.count + 2);
		}
		else
		{
			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if (access_type == cgm_access_load_s)
			{
				gpu_l1_cache_access_load(&(gpu_s_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_puts)
			{
				gpu_cache_access_put(&(gpu_s_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_retry)
			{
				gpu_cache_access_retry(&(gpu_s_caches[my_pid]), message_packet);
			}
			else
			{
				fatal("gpu_s_cache_ctrl(): access_id %llu bad access type %s at cycle %llu\n",
						access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}
	}

	/////////testing
	//(*message_packet->witness_ptr)++;
	//list_remove(gpu_s_caches[my_pid].Rx_queue_top, message_packet);
	//continue;
	/////////testing

	/* should never get here*/
	fatal("gpu_s_cache_ctrl task is broken\n");
	return;
}

void gpu_v_cache_ctrl(void){

	int my_pid = gpu_v_pid++;
	long long step = 1;
	int num_cus = si_gpu_num_compute_units;

	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;


	assert(my_pid <= num_cus);
	set_id((unsigned int)my_pid);


	while(1)
	{
		//wait here until there is a job to do.
		//In any given cycle I might have to service 1 to N number of caches
		await(&gpu_v_cache[my_pid], step);


		//get the message out of the unit's queue
		message_packet = cache_get_message(&(gpu_v_caches[my_pid]));

		if (message_packet == NULL || !cache_can_access_top(&gpu_l2_caches[cgm_gpu_cache_map(my_pid)]))
		{
			P_PAUSE(1);
			//printf("%s stalling cycle %llu\n", gpu_v_caches[my_pid].name, P_TIME);
			//future_advance(&gpu_v_cache[my_pid], etime.count + 2);
		}
		else
		{
			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if(access_type == cgm_access_load_v)
			{
				gpu_l1_cache_access_load(&(gpu_v_caches[my_pid]), message_packet);
				//gpu_cache_access_load(&(gpu_v_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_store_v || access_type == cgm_access_nc_store)
			{
				gpu_l1_cache_access_store(&(gpu_v_caches[my_pid]), message_packet);
				//gpu_cache_access_store(&(gpu_v_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_retry)
			{
				gpu_cache_access_retry(&(gpu_v_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_puts)
			{
				gpu_cache_access_put(&(gpu_v_caches[my_pid]), message_packet);
			}
			else
			{
				fatal("gpu_v_cache_ctrl(): access_id %llu bad access type %s at cycle %llu\n",
						access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}


		}
		/////////testing
		//(*message_packet->witness_ptr)++;
		//list_remove(gpu_v_caches[my_pid].Rx_queue_top, message_packet);
		//continue;
		/////////testing

	}
	//should never get here
	fatal("gpu_v_cache_ctrl task is broken\n");
	return;
}

void gpu_l2_cache_ctrl(void){

	int my_pid = gpu_l2_pid++;
	long long step = 1;

	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);


	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;


	assert(my_pid <= gpu_group_cache_num);
	set_id((unsigned int)my_pid);

	while(1)
	{

		/*wait here until there is a job to do.*/
		await(&gpu_l2_cache[my_pid], step);

		//check the top or bottom rx queues for messages.
		message_packet = cache_get_message(&(gpu_l2_caches[my_pid]));

		if (message_packet == NULL || !hub_iommu_can_access(hub_iommu->Rx_queue_top[my_pid]))
		{
			//the cache state is preventing the cache from working this cycle stall.
			/*printf("%s stalling cycle %llu\n", gpu_l2_caches[my_pid].name, P_TIME);
			printf("stalling\n");
			future_advance(&gpu_l2_cache[my_pid], etime.count + 2);*/
			P_PAUSE(1);
		}
		else
		{
			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if(access_type == cgm_access_gets_s || access_type == cgm_access_gets_v)
			{
				gpu_cache_access_get(&gpu_l2_caches[my_pid], message_packet);
				//gpu_l2_cache_access_gets(&gpu_l2_caches[my_pid], message_packet);
			}
			else if (access_type == cgm_access_retry)
			{
				gpu_cache_access_retry(&gpu_l2_caches[my_pid], message_packet);
				//gpu_l2_cache_access_retry(&gpu_l2_caches[my_pid], message_packet);
			}
			else if(access_type == cgm_access_puts)
			{
				gpu_cache_access_put(&gpu_l2_caches[my_pid], message_packet);
				//gpu_l2_cache_access_puts(&gpu_l2_caches[my_pid], message_packet);
			}
			else
			{
				fatal("gpu_l2_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
					access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}
	}
	/* should never get here*/
	fatal("gpu_l2_cache_ctrl task is broken\n");
	return;
}

void l1_i_cache_down_io_ctrl(void){

	int my_pid = l1_i_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l1_i_caches[my_pid].cache_io_down_ec, step);
		step++;

		//printf("here\n");

		message_packet = list_dequeue(l1_i_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		access_id = message_packet->access_id;
		transfer_time = (message_packet->size/l1_i_caches[my_pid].bus_width);

		P_PAUSE(transfer_time);

		//drop into next east queue.
		list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);
		advance(&l2_cache[my_pid]);
	}

	return;
}

void l1_d_cache_down_io_ctrl(void){

	int my_pid = l1_d_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l1_d_caches[my_pid].cache_io_down_ec, step);
		step++;

		message_packet = list_dequeue(l1_d_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		access_id = message_packet->access_id;

		transfer_time = (message_packet->size/l1_d_caches[my_pid].bus_width);

		P_PAUSE(transfer_time);

		//drop into next east queue.
		list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);
		advance(&l2_cache[my_pid]);
	}

	return;
}

void l2_cache_up_io_ctrl(void){

	int my_pid = l2_up_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l2_caches[my_pid].cache_io_up_ec, step);
		step++;

		message_packet = list_dequeue(l2_caches[my_pid].Tx_queue_top);
		assert(message_packet);

		access_id = message_packet->access_id;
		//star todo fix this we need a top and bottom bus_width
		transfer_time = (message_packet->size/l1_i_caches[my_pid].bus_width);

		P_PAUSE(transfer_time);

		//drop into the correct l1 cache queue.
		if (message_packet->cpu_access_type == cgm_access_fetch)
		{
			list_enqueue(l1_i_caches[my_pid].Rx_queue_bottom, message_packet);
			advance(&l1_i_cache[my_pid]);
		}
		else if (message_packet->cpu_access_type == cgm_access_load || message_packet->cpu_access_type == cgm_access_store)
		{
			list_enqueue(l1_d_caches[my_pid].Rx_queue_bottom, message_packet);
			advance(&l1_d_cache[my_pid]);
		}
		else
		{
			fatal("l2_cache_up_io_ctrl(): bad cpu access type\n");// str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
		}
	}
	return;
}

void l2_cache_down_io_ctrl(void){


	int my_pid = l2_down_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l2_caches[my_pid].cache_io_down_ec, step);
		step++;

		message_packet = list_dequeue(l2_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		access_id = message_packet->access_id;

		//star todo fix this we need a top and bottom bus_width
		transfer_time = (message_packet->size/l2_caches[my_pid].bus_width);

		P_PAUSE(transfer_time);

		//drop in to the switch queue
		list_enqueue(switches[my_pid].north_queue, message_packet);
		advance(&switches_ec[my_pid]);

	}
	return;
}

void l3_cache_up_io_ctrl(void){

	int my_pid = l3_up_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l3_caches[my_pid].cache_io_up_ec, step);
		step++;

		message_packet = list_dequeue(l3_caches[my_pid].Tx_queue_top);
		assert(message_packet);

		access_id = message_packet->access_id;

		//star todo fix this we need a top and bottom bus_width
		transfer_time = (message_packet->size/l3_caches[my_pid].bus_width);

		P_PAUSE(transfer_time);

		//drop in to the switch queue
		list_enqueue(switches[my_pid].south_queue, message_packet);
		advance(&switches_ec[my_pid]);

		/*printf("l3 -> l2\n");*/
	}
	return;

}

void l3_cache_down_io_ctrl(void){

	int my_pid = l3_down_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l3_caches[my_pid].cache_io_down_ec, step);
		step++;

		message_packet = list_dequeue(l3_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		access_id = message_packet->access_id;

		//star todo fix this we need a top and bottom bus_width
		transfer_time = (message_packet->size/l3_caches[my_pid].bus_width);

		P_PAUSE(transfer_time);

		//drop in to the switch queue
		list_enqueue(switches[my_pid].south_queue, message_packet);
		advance(&switches_ec[my_pid]);

		/*printf("l3 -> SA\n");*/
	}
	return;
}


void gpu_s_cache_down_io_ctrl(void){

	int my_pid = gpu_s_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(gpu_s_caches[my_pid].cache_io_down_ec, step);
		step++;

		//printf("here\n");

		message_packet = list_dequeue(gpu_s_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		access_id = message_packet->access_id;
		transfer_time = (message_packet->size/gpu_s_caches[my_pid].bus_width);

		P_PAUSE(transfer_time);

		//drop into next east queue.
		list_enqueue(gpu_l2_caches[cgm_gpu_cache_map(my_pid)].Rx_queue_top, message_packet);
		advance(&gpu_l2_cache[cgm_gpu_cache_map(my_pid)]);
	}

	return;
}

void gpu_v_cache_down_io_ctrl(void){

	int my_pid = gpu_v_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(gpu_v_caches[my_pid].cache_io_down_ec, step);
		step++;

		/*printf("here\n");*/

		message_packet = list_dequeue(gpu_v_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		access_id = message_packet->access_id;
		transfer_time = (message_packet->size/gpu_v_caches[my_pid].bus_width);

		P_PAUSE(transfer_time);

		//drop into next east queue.
		list_enqueue(gpu_l2_caches[cgm_gpu_cache_map(my_pid)].Rx_queue_top, message_packet);
		advance(&gpu_l2_cache[cgm_gpu_cache_map(my_pid)]);
	}

	return;

}
void gpu_l2_cache_up_io_ctrl(void){

	int my_pid = gpu_l2_up_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(gpu_l2_caches[my_pid].cache_io_up_ec, step);
		step++;

		message_packet = list_dequeue(gpu_l2_caches[my_pid].Tx_queue_top);
		assert(message_packet);

		access_id = message_packet->access_id;
		//star todo fix this we need a top and bottom bus_width
		transfer_time = (message_packet->size/gpu_v_caches[my_pid].bus_width);

		P_PAUSE(transfer_time);

		//printf("L2 -> L1 access_id %llu cycle %llu\n", access_id, P_TIME);

		//drop into the correct l1 cache queue.
		if(message_packet->gpu_access_type == cgm_access_load_s)
		{
			list_enqueue(gpu_s_caches[message_packet->gpu_cache_id].Rx_queue_bottom, message_packet);
			advance(&gpu_s_cache[message_packet->gpu_cache_id]);
		}
		else if(message_packet->gpu_access_type == cgm_access_load_v ||
						message_packet->gpu_access_type == cgm_access_store_v ||
						message_packet->gpu_access_type == cgm_access_nc_store)
		{
			list_enqueue(gpu_v_caches[message_packet->gpu_cache_id].Rx_queue_bottom, message_packet);
			advance(&gpu_v_cache[message_packet->gpu_cache_id]);
		}
		else
		{
			fatal("gpu_l2_cache_up_io_ctrl(): bad gpu access type\n");// str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
		}

	}

	return;


/*	list_enqueue(gpu_s_caches[message_packet->gpu_cache_id].Rx_queue_bottom, message_packet);
	advance(&gpu_s_cache[message_packet->gpu_cache_id]);*/

}

void gpu_l2_cache_down_io_ctrl(void){

	int my_pid = gpu_l2_down_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(gpu_l2_caches[my_pid].cache_io_down_ec, step);
		step++;

		message_packet = list_dequeue(gpu_l2_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		access_id = message_packet->access_id;
		//star todo fix this we need a top and bottom bus_width
		transfer_time = (message_packet->size/gpu_l2_caches[my_pid].bus_width);

		P_PAUSE(transfer_time);

		/*printf("l2 -> SA\n");*/

		list_enqueue(hub_iommu->Rx_queue_top[my_pid], message_packet);
		advance(hub_iommu_ec);

		//drop into the correct l1 cache queue.
		/*if(message_packet->gpu_access_type == cgm_access_load_s)
		{
			list_enqueue(gpu_s_caches[message_packet->gpu_cache_id].Rx_queue_bottom, message_packet);
			advance(&gpu_s_cache[message_packet->gpu_cache_id]);
		}
		else if(message_packet->gpu_access_type == cgm_access_load_v ||
						message_packet->gpu_access_type == cgm_access_store_v ||
						message_packet->gpu_access_type == cgm_access_nc_store)
		{
			list_enqueue(gpu_v_caches[message_packet->gpu_cache_id].Rx_queue_bottom, message_packet);
			advance(&gpu_v_cache[message_packet->gpu_cache_id]);
		}
		else
		{
			fatal("gpu_l2_cache_up_io_ctrl(): bad gpu access type\n");// str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
		}*/

	}
	return;
}
