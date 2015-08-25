/*
 * cache.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */


#include <cgm/cache.h>


struct str_map_t cgm_cache_block_state_map =
{ 	cgm_cache_block_state_num, 	{
		{ "I", cgm_cache_block_invalid},
		{ "N", cgm_cache_block_noncoherent},
		{ "M", cgm_cache_block_modified },
		{ "O", cgm_cache_block_owned },
		{ "E", cgm_cache_block_exclusive },
		{ "S", cgm_cache_block_shared },
		{ "T", cgm_cache_block_transient },
		{ "!", cgm_cache_block_null },
		}
};


struct str_map_t cgm_cache_policy_map =
{ cache_policy_num, 	{
		{ "INV", cache_policy_invalid },
		{ "LRU", cache_policy_lru },
		{ "FIFO", cache_policy_fifo },
		{ "Random", cache_policy_random },
		}
};


int QueueSize = 0;
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

	/*//gpu l2 caches
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
	}*/

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
	int write_back_queue_size = list_count(cache->write_back_buffer);
	int coherence_queue_size = list_count(cache->Coherance_Rx_queue);

	//queues shouldn't exceed their sizes.
	assert(ort_status <= cache->mshr_size);
	assert(ort_coalesce_size <= (cache->max_coal + 1));
	assert(write_back_queue_size <= (QueueSize +1));

	/*if the ort or the coalescer are full we can't process a CPU request because a miss will overrun the table.*/

	//pull from the retry queue if we have accesses waiting...
	//pull from the coherence queue if there is a message waiting.
	if(coherence_queue_size > 0)
	{
		new_message = list_get(cache->Coherance_Rx_queue, 0);
		cache->last_queue = cache->Coherance_Rx_queue;
		assert(new_message);
	}

	else if((ort_status == cache->mshr_size || ort_coalesce_size > cache->max_coal) && retry_queue_size > 0)
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
	//schedule write back if the retry queue is empty and the bottom queue is empty and the cache has nothing else to do.
	else if((ort_status == cache->mshr_size || ort_coalesce_size > cache->max_coal) && write_back_queue_size > 0)
	{
		new_message = list_get(cache->write_back_buffer, 0);
		cache->last_queue = cache->write_back_buffer;
		assert(new_message);
	}
	//schedule write back if the wb queue is full.
	else if(write_back_queue_size >= QueueSize)
	{
		new_message = list_get(cache->write_back_buffer, 0);
		cache->last_queue = cache->write_back_buffer;
		assert(new_message);
	}

	//ORT is not full, we can process CPU requests and lower level cache replies in a round robin fashion.
	else if(ort_status < cache->mshr_size && ort_coalesce_size <= cache->max_coal)
	{
		//pull from the retry queue first.
		if(retry_queue_size > 0)
		{
			new_message = list_get(cache->retry_queue, 0);
			cache->last_queue = cache->retry_queue;
			assert(new_message);
		}
		else
		{
			//star todo give priority to the CPU here
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

void ort_clear(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int row = 0;

	/*star todo fix this, the set, tag offset
	is overridden by lower level caches and breaks this func on puts.*/
	int set = 0;
	int tag = 0;
	unsigned int offset = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;

	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	//store the decode
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	row = ort_search(cache, message_packet->tag, message_packet->set);

	assert(row < cache->mshr_size);
	cache->ort[row][0] = -1;
	cache->ort[row][1] = -1;
	cache->ort[row][2] = -1;
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


int cgm_l3_cache_map(int set){

	int num_cores = x86_cpu_num_cores;
	int map = -1;

	/*assert(l3_caches[0].slice_type == 1);*/

	int map_type = l3_caches[0].slice_type;

	//stripe
	if(map_type == 0)
	{
		//get the address range
		map = 0;
	}
	//block (one big shared L3 cache
	else if (map_type == 1)
	{
		//map = *(set) % num_cores;
		//star this is a faster way to do the look up.
		map = (unsigned int) set & (unsigned int) (num_cores - 1);
	}
	// individual non shared L3 caches
	else if (map_type < 0 || map_type >= 2)
	{
		fatal("cgm_l3_cache_map(): invalid map_type set, simulator doesn't support directory at system agent yet.\n");
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

	//check if in queue is full
	if(QueueSize <= list_count(cache->Rx_queue_top))
	{
		return 0;
	}

	//cache queue is accessible.
	return 1;
}

int cache_can_access_Tx_bottom(struct cache_t *cache){

	//check if Tx queue is full
	if(QueueSize <= list_count(cache->Tx_queue_bottom))
	{
		return 0;
	}

	//cache queue is accessible.
	return 1;
}

int cache_can_access_Tx_top(struct cache_t *cache){

	//check if Tx queue is full
	if(QueueSize <= list_count(cache->Tx_queue_top))
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
	//star i reworked this a little
	*(tag_ptr) = (addr >> (cache->log_block_size + cache->log_set_size));//addr & ~(cache->block_mask);
	*(set_ptr) =  (addr >> (cache->log_block_size) & (cache->set_mask));//(addr >> cache->log_block_size) % cache->num_sets;
	*(offset_ptr) = addr & (cache->block_mask);
}

unsigned int cgm_cache_build_address(struct cache_t *cache, int set, int tag){

	unsigned int addr = 0;
	unsigned int merge_tag = 0;
	unsigned int merge_set = 0;

	/*examples of cache sizes

	L1 block size 64, number of sets 64
	log_block_size = 6
	log_set_size = 6

	L2 block size 64, number of sets 256
	log_block_size = 6
	log_set_size = 8

	L3 block size 64, number of sets 512
	log_block_size = 6
	log_set_size = 9
	*/

	//shift the tag and set to correct position
	merge_tag = (tag << (cache->log_block_size + cache->log_set_size));
	merge_set = (set << (cache->log_block_size));

	//or them together into an address note offset is missing
	addr = merge_tag | merge_set;

	return addr;
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

int cgm_cache_get_way(struct cache_t *cache, int tag, int set){

	int way;

	for (way = 0; way < cache->assoc; way++)
	{
		if (cache->sets[set].blocks[way].tag == tag && cache->sets[set].blocks[way].state)
		{
			return way;
		}
	}

	fatal("gm_cache_get_way(): reached bottom\n");
	return way;
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
	return;
}

void cgm_cache_inval_block(struct cache_t *cache, int set, int way){

	enum cgm_cache_block_state_t victim_state;

	//get the victim's state
	victim_state = cgm_cache_get_block_state(cache, set, way);

	//if dirty data is found
	if (victim_state == cgm_cache_block_modified)
	{
		//move the block to the WB buffer
		struct cgm_packet_t *write_back_packet = packet_create();

		init_write_back_packet(cache, write_back_packet, set, way);

		list_enqueue(cache->write_back_buffer, write_back_packet);
	}

	//set the block state to invalid
	cgm_cache_set_block_state(cache, set, way, cgm_cache_block_invalid);

	return;
}


void cgm_L3_cache_evict_block(struct cache_t *cache, int set, int way, int sharers){

	enum cgm_cache_block_state_t victim_state;
	int i = 0, core_id = 0;
	unsigned char bit_vector;
	int num_cores = x86_cpu_num_cores;
	assert(sharers >= 0 && sharers <= num_cores);
	assert(cache->cache_type == l3_cache_t);
	assert(cache->share_mask > 0);

	//get the victim's state
	victim_state = cgm_cache_get_block_state(cache, set, way);

	//if dirty data is found
	if (victim_state == cgm_cache_block_modified)
	{
		//move the block to the WB buffer
		struct cgm_packet_t *write_back_packet = packet_create();

		init_write_back_packet(cache, write_back_packet, set, way);

		list_enqueue(cache->write_back_buffer, write_back_packet);
	}

	//set the block state to invalid
	cgm_cache_set_block_state(cache, set, way, cgm_cache_block_invalid);

	//send eviction notices
	/*star todo account for block sizes
	for exmaple, if the L3 cache is 64 bytes and L2 is 128 L2 should send two invals*/
	for(i = 0; i < sharers; i++)
	{
		//get the presence bits from the directory
		bit_vector = cache->sets[set].blocks[way].directory_entry.entry;
		bit_vector = bit_vector & cache->share_mask;

		//for each core that has a copy of the cache block send the inval
		if((bit_vector & 1) == 1)
		{
			struct cgm_packet_t *inval_packet = packet_create();

			init_inval_packet(cache, inval_packet, set, way);

			inval_packet->cpu_access_type = cgm_access_store;

			//update routing
			inval_packet->dest_id = str_map_string(&node_strn_map, l2_caches[i].name);
			inval_packet->dest_name = str_map_value(&l2_strn_map, inval_packet->dest_id);
			inval_packet->src_name = cache->name;
			inval_packet->src_id = str_map_string(&node_strn_map, cache->name);

			list_enqueue(cache->Tx_queue_top, inval_packet);
			advance(cache->cache_io_up_ec);
		}

		//shift the vector to the next position and continue
		bit_vector = bit_vector >> 1;
	}

	return;
}

void cgm_L2_cache_evict_block(struct cache_t *cache, int set, int way){

	enum cgm_cache_block_state_t victim_state;
	assert(cache->cache_type == l2_cache_t);

	//get the victim's state
	victim_state = cgm_cache_get_block_state(cache, set, way);

	//if dirty data is found
	if (victim_state == cgm_cache_block_modified)
	{
		//move the block to the WB buffer
		struct cgm_packet_t *write_back_packet = packet_create();

		init_write_back_packet(cache, write_back_packet, set, way);

		list_enqueue(cache->write_back_buffer, write_back_packet);
	}

	//set the block state to invalid
	cgm_cache_set_block_state(cache, set, way, cgm_cache_block_invalid);

	//send eviction notices
	//star todo account for block sizes if the L1 cache is 64 bytes and L2 is 128 L2 should send two invals
	struct cgm_packet_t *inval_packet = packet_create();

	init_inval_packet(cache, inval_packet, set, way);

	/*star todo fix this, this is only needed for correct routing from L2 to L1 D
	figure out another way to do this*/
	inval_packet->cpu_access_type = cgm_access_load;

	list_enqueue(cache->Tx_queue_top, inval_packet);
	advance(cache->cache_io_up_ec);

	return;
}

void cgm_cache_set_block_type(struct cache_t *cache, int type, int set, int way){

	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);

	assert(type == 0 || type == 1);

	cache->sets[set].blocks[way].data_type = type;

	return;
}

int cgm_cache_get_block_type(struct cache_t *cache, int set, int way, int tag){

	int type;

	type = cache->sets[set].blocks[way].data_type;

	assert(type == 0 || type == 1);
	return type;
}

/* Return the way of the block to be replaced in a specific set,
 * depending on the replacement policy */
int cgm_cache_replace_block(struct cache_t *cache, int set)
{
	/*struct cache_block_t *block;*/

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

		assert(way >= 0 && way <= cache->num_sets);
		return way;

		//star changed this to give last way if block isn't present
		/*if(way >= cache->assoc)
		{
			printf("here t %d \n", way);
			return (cache->assoc - 1);
		}
		else if (way >= 0 && way < cache->assoc)
		{
			printf("here b %d \n", way);
			return way;
		}*/
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

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{

		//wait here until there is a job to do
		await(&l1_i_cache[my_pid], step);

		//peak at a message from the input queues.
		message_packet = cache_get_message(&(l1_i_caches[my_pid]));

		/*star todo fix this, these should be related to the message type.
		This can be done more efficiently than a caret blanch stall */
		if (message_packet == NULL || !cache_can_access_top(&l2_caches[my_pid]) || !cache_can_access_Tx_bottom(&(l1_i_caches[my_pid])))
		{
			//the cache state is preventing the cache from working this cycle stall
			l1_i_caches[my_pid].stalls++;

			P_PAUSE(1);
		}
		else
		{
			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			///////////protocol v2
			if (access_type == cgm_access_fetch || access_type == cgm_access_fetch_retry)
			{
				//get the status of the cache block
				//star todo remove the cache_block_hit_ptr this isn't needed.
				cache_get_block_status(&(l1_i_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				switch(*cache_block_state_ptr)
				{
					case cgm_cache_block_noncoherent:
					case cgm_cache_block_modified:
					case cgm_cache_block_owned:
						fatal("l1_i_cache_ctrl(): Invalid block state on hit as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
						break;

					//miss or invalid cache block states
					case cgm_cache_block_invalid:

						/*printf("I$ miss fetch\n");*/

						//stats
						l1_i_caches[my_pid].misses++;

						//check ORT for coalesce
						cache_check_ORT(&(l1_i_caches[my_pid]), message_packet);

						if(message_packet->coalesced == 1)
							continue;

						//add some routing/status data to the packet
						//message_packet->cpu_access_type = cgm_access_fetch;
						message_packet->access_type = cgm_access_gets;
						message_packet->l1_access_type = cgm_access_gets;

						//find victim
						message_packet->l1_victim_way = cgm_cache_replace_block(&(l1_i_caches[my_pid]), message_packet->set);

						//charge delay
						P_PAUSE(l1_i_caches[my_pid].latency);

						//transmit to L2
						cache_put_io_down_queue(&(l1_i_caches[my_pid]), message_packet);
						break;

					//hit states
					case cgm_cache_block_exclusive:
					case cgm_cache_block_shared:

						//stats
						l1_i_caches[my_pid].hits++;

						//set retry state and delay
						if(access_type == cgm_access_fetch_retry || message_packet->coalesced == 1)
						{
							P_PAUSE(l1_i_caches[my_pid].latency);

							//enter retry state.
							cache_coalesed_retry(&(l1_i_caches[my_pid]), message_packet->tag, message_packet->set);
						}

						/*printf("fetch id %llu set %d tag %d complete cycle %llu\n", message_packet->access_id, message_packet->tag, message_packet->set, P_TIME);*/
						message_packet->end_cycle = P_TIME;
						cache_l1_i_return(&(l1_i_caches[my_pid]),message_packet);
						break;
				}
			}
			else if (access_type == cgm_access_puts)
			{
				//find the access in the ORT table and clear it.
				ort_clear(&(l1_i_caches[my_pid]), message_packet);

				//set the block and retry the access in the cache.
				cache_put_block(&(l1_i_caches[my_pid]), message_packet);
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


int array[10000];

void run_array(void){


	int i = 0;

	printf("running access array\n");

	for(i = 0; i < 10000; i ++)
	{
		if(array[i] == 1)
		{
			printf("access_id %d failed\n", i);
		}

	}

}

void l1_d_cache_ctrl(void){

	int my_pid = l1_d_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	enum cgm_access_kind_t access_type;
	long long access_id = 0;

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{
		//wait here until there is a job to do.
		await(&l1_d_cache[my_pid], step);

		//get the message out of the queue
		message_packet = cache_get_message(&(l1_d_caches[my_pid]));

		//star todo this can be refined a lot.
		if (message_packet == NULL || !cache_can_access_top(&l2_caches[my_pid]) || !cache_can_access_Tx_bottom(&(l1_d_caches[my_pid])))
		{
			//the cache state is preventing the cache from working this cycle stall.
			l1_d_caches[my_pid].stalls++;

			P_PAUSE(1);
		}
		else
		{
			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			/*printf("l1 D access id %llu\n", access_id);*/

			///////////protocol v2
			if (access_type == cgm_access_load || access_type == cgm_access_load_retry)
			{
				//get the status of the cache block
				cache_get_block_status(&(l1_d_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				switch(*cache_block_state_ptr)
				{
					case cgm_cache_block_transient:
					case cgm_cache_block_owned:
					case cgm_cache_block_noncoherent:
						fatal("l1_d_cache_ctrl(): Invalid block state on load hit %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
						break;

					//miss or invalid cache block states
					case cgm_cache_block_invalid:

						/*printf("D$ load miss\n");*/

						//stats
						l1_d_caches[my_pid].misses++;

						//check ORT for coalesce
						cache_check_ORT(&(l1_d_caches[my_pid]), message_packet);

						if(message_packet->coalesced == 1)
							continue;

						/*printf("load access_id %llu set %d tag %d as %s coalesced cycle %llu\n",
									message_packet->access_id, message_packet->set, message_packet->tag, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);*/

						//add some routing/status data to the packet
						message_packet->access_type = cgm_access_get;
						message_packet->l1_access_type = cgm_access_get;

						//find victim
						message_packet->l1_victim_way = cgm_cache_replace_block(&(l1_d_caches[my_pid]), message_packet->set);

						//charge delay
						P_PAUSE(l1_d_caches[my_pid].latency);

						//transmit to L2
						cache_put_io_down_queue(&(l1_d_caches[my_pid]), message_packet);
						break;

					//hit states
					case cgm_cache_block_modified:
					case cgm_cache_block_exclusive:
					case cgm_cache_block_shared:

						/*printf("D$ load hit\n");*/

						//stats
						l1_d_caches[my_pid].hits++;

						/*printf("d cache load block state %s\n", str_map_value(&cache_block_state_map, *cache_block_state_ptr));*/

						//set the retry state and charge latency
						if(access_type == cgm_access_load_retry || message_packet->coalesced == 1)
						{
							P_PAUSE(l1_d_caches[my_pid].latency);

							//enter retry state.
							cache_coalesed_retry(&(l1_d_caches[my_pid]), message_packet->tag, message_packet->set);
						}

						/*printf("load id %llu set %d tag %d complete cycle %llu\n", message_packet->access_id, message_packet->set, message_packet->tag, P_TIME);*/
						/*printf("access_id %llu L1 hit cycle %llu \n", message_packet->access_id, P_TIME);
						array[(int)message_packet->access_id] = 0;*/

						message_packet->end_cycle = P_TIME;
						cache_l1_d_return(&(l1_d_caches[my_pid]),message_packet);
						break;
				}
			}
			else if(access_type == cgm_access_store || access_type == cgm_access_store_retry)
			{
				/*printf("D$ store\n");*/

				//get the status of the cache block
				cache_get_block_status(&(l1_d_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				switch(*cache_block_state_ptr)
				{
					case cgm_cache_block_owned:
					case cgm_cache_block_noncoherent:
						fatal("l1_d_cache_ctrl(): Invalid block state on store hit %s \n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
						break;

					//miss or invalid cache block state
					case cgm_cache_block_invalid:

						//stats
						l1_d_caches[my_pid].misses++;

						//check ORT for coalesce
						cache_check_ORT(&(l1_d_caches[my_pid]), message_packet);

						if(message_packet->coalesced == 1)
							continue;

						/*printf("store (inval) access_id %llu set %d tag %d as %s coalesced cycle %llu\n",
							message_packet->access_id, message_packet->set, message_packet->tag, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);*/

						//add some routing/status data to the packet
						message_packet->access_type = cgm_access_getx;
						message_packet->l1_access_type = cgm_access_getx;

						//find victim
						message_packet->l1_victim_way = cgm_cache_replace_block(&(l1_d_caches[my_pid]), message_packet->set);

						//charge delay
						P_PAUSE(l1_d_caches[my_pid].latency);

						//transmit to L2
						cache_put_io_down_queue(&(l1_d_caches[my_pid]), message_packet);
						break;


					case cgm_cache_block_shared:

						//stats
						l1_d_caches[my_pid].upgrade_misses++;

						//should not be here in the retry state.
						//assert(message_packet->access_type != cgm_access_store_retry);

						/*star todo find a better way to do this.
						this is for a special case where a coalesced store
						can be pulled from the ORT and is an upgrade miss here
						at this point we want the access to be treated as a new miss
						so set coalesced to 0. packets in the ORT will stay in the ort
						preserving order until the missing access returns with the upgrade.*/
						if(message_packet->coalesced == 1)
						{
							message_packet->coalesced = 0;
						}

						//check ORT for coalesce
						cache_check_ORT(&(l1_d_caches[my_pid]), message_packet);

						if(message_packet->coalesced == 1)
							continue;

						/*printf("store (share) access_id %llu set %d tag %d as %s coalesced cycle %llu\n",
							message_packet->access_id, message_packet->set, message_packet->tag, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);*/

						//set block transient state
						cgm_cache_set_block_transient_state(&(l1_d_caches[my_pid]), message_packet->set, message_packet->way, message_packet->access_id, cgm_cache_block_transient);

						message_packet->access_type = cgm_access_upgrade;

						//charge delay
						P_PAUSE(l1_d_caches[my_pid].latency);

						//transmit upgrade request to L2
						//printf("access_id %llu forwarded set %d tag %d cycle %llu\n", message_packet->access_id, message_packet->set, message_packet->tag, P_TIME);
						cache_put_io_down_queue(&(l1_d_caches[my_pid]), message_packet);
						break;

					case cgm_cache_block_exclusive:
					case cgm_cache_block_modified:

						l1_d_caches[my_pid].hits++;

						//set modified if current block state is exclusive
						if(*cache_block_state_ptr == cgm_cache_block_exclusive)
						{
							cgm_cache_set_block_state(&(l1_d_caches[my_pid]), message_packet->set, message_packet->way, cgm_cache_block_modified);
						}

						//check for retry state
						if(access_type == cgm_access_store_retry || message_packet->coalesced == 1)
						{
							P_PAUSE(l1_d_caches[my_pid].latency);

							//enter retry state.
							cache_coalesed_retry(&(l1_d_caches[my_pid]), message_packet->tag, message_packet->set);
						}

						message_packet->end_cycle = P_TIME;
						cache_l1_d_return(&(l1_d_caches[my_pid]),message_packet);
						break;
				}
			}
			else if (access_type == cgm_access_puts || access_type == cgm_access_putx || access_type == cgm_access_put_clnx)
			{
				enum cgm_cache_block_state_t victim_trainsient_state;
				long long t_state_id;

				//check the transient state of the victim
				//if the state is set, an earlier access is bringing the block
				//if it is not set the victim is clear to evict
				cache_get_block_status(&(l1_d_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				victim_trainsient_state = cgm_cache_get_block_transient_state(&(l1_d_caches[my_pid]), message_packet->set, message_packet->l1_victim_way);

				//if the block is in the transient state there are stores waiting to write to the block.
				if(victim_trainsient_state == cgm_cache_block_transient)
				{

					//the victim is locked, either wait, choose another victim, or schedule something else.
					t_state_id = cgm_cache_get_block_transient_state_id(&(l1_d_caches[my_pid]), message_packet->set, message_packet->l1_victim_way);

					//check ordering
					assert(message_packet->access_id >= t_state_id);

					fatal("l1 D cache stalled on transient\n");
					/*printf("l1 D cache stalled on transient\n");*/

					//try again we will pull the coherence message eventually.
					step--;
					P_PAUSE(1);
				}
				else
				{
					//find the access in the ORT table and clear it.
					ort_clear(&(l1_d_caches[my_pid]), message_packet);

					//set the block and retry the access in the cache.
					cache_put_block(&(l1_d_caches[my_pid]), message_packet);
				}
			}
			else if (access_type == cgm_access_write_back)
			{
				/*printf("l1 D WB\n");*/

				P_PAUSE(l1_d_caches[my_pid].latency);

				//send the write back to the L2 cache.
				cache_put_io_down_queue(&(l1_d_caches[my_pid]), message_packet);

				/*write backs are internally scheduled so decrement the counter
				figure out a way to do this better..
				perhaps have the cache advance itself when the evict results in a write back buffer entry.*/
				step--;
			}
			else if (access_type == cgm_access_inv)
			{
				//Invalidation request from lower cache

				/*printf("l1 d inval\n");*/

				//get the block status
				cache_get_block_status(&(l1_d_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				//find and invalidate the block
				if(*cache_block_hit_ptr == 1)
				{
					cgm_cache_inval_block(&(l1_d_caches[my_pid]), message_packet->set, message_packet->way);
				}

				//free the message packet
				message_packet = list_remove(l1_d_caches[my_pid].last_queue, message_packet);
				packet_destroy(message_packet);

			}
			else if (access_type == cgm_access_upgrade_ack)
			{
				//we have permission to upgrade our set block state and retry access

				//get the status of the cache block
				cache_get_block_status(&(l1_d_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				//find the access in the ORT table and clear it.
				ort_clear(&(l1_d_caches[my_pid]), message_packet);

				/*if the block is no longer here on upgrade_ack,
				the block was evicted and maybe in the WB we can treat this as a miss.
				This means the trasient state is broken.*/
				if(*cache_block_hit_ptr != 1 || *cache_block_state_ptr != cgm_cache_block_shared)
				{
					//printf("evicted\n");
					//printf("access_id %llu as %s cycle %llu\n", message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
					fatal("access_id %llu evicted or not shared as %s cycle %llu\n", message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
				}

				//set the state to exclusive and clear the transient state
				cgm_cache_set_block_state(&(l1_d_caches[my_pid]), message_packet->set, message_packet->way, cgm_cache_block_exclusive);

				cgm_cache_set_block_transient_state(&(l1_d_caches[my_pid]), message_packet->set, message_packet->way, NULL, cgm_cache_block_null);

				//enter the retry state
				message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);
				assert(message_packet->access_type == cgm_access_store_retry);
				assert(message_packet->coalesced != 1);

				message_packet = list_remove(l1_d_caches[my_pid].last_queue, message_packet);
				list_enqueue(l1_d_caches[my_pid].retry_queue, message_packet);

				//run again
				step--;
			}
			else
			{
				fatal("l1_d_cache_ctrl(): %s access_id %llu bad access type %s at cycle %llu\n",
						l1_d_caches[my_pid].name, access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
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

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	int l3_map;

	int i = 0;
	for(i = 0; i <10000; i++)
	{
		array[i] = 0;
	}

	while(1)
	{
		/*wait here until there is a job to do.*/
		await(&l2_cache[my_pid], step);

		//check the top or bottom rx queues for messages.
		message_packet = cache_get_message(&(l2_caches[my_pid]));

		if (message_packet == NULL || !switch_can_access(switches[my_pid].north_queue)
				|| !cache_can_access_Tx_bottom(&(l2_caches[my_pid])) || !cache_can_access_Tx_top(&(l2_caches[my_pid])))
		{
			//the cache state is preventing the cache from working this cycle stall.
			l2_caches[my_pid].stalls++;

			P_PAUSE(1);
		}
		else
		{
			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			//printf("%s request access type %s cycle %llu\n", l2_caches[my_pid].name, str_map_value(&cgm_mem_access_strn_map, access_type), P_TIME);

			if(access_type == cgm_access_gets || access_type == cgm_access_fetch_retry)
			{
				//get the status of the cache block
				cache_get_block_status(&(l2_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				switch(*cache_block_state_ptr)
				{

					case cgm_cache_block_noncoherent:
					case cgm_cache_block_modified:
					case cgm_cache_block_owned:
						fatal("l2_cache_ctrl(): Invalid block state on fetch hit as %s cycle %llu\n",
								str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), P_TIME);
						break;

					case cgm_cache_block_invalid:

						//stats
						l2_caches[my_pid].misses++;

						//check ORT for coalesce
						cache_check_ORT(&(l2_caches[my_pid]), message_packet);

						if(message_packet->coalesced == 1)
							continue;

						//add some routing/status data to the packet
						message_packet->access_type = cgm_access_gets;

						/*message_packet->cache_block_state = *cache_block_hit_ptr;*/

						l3_map = cgm_l3_cache_map(message_packet->set);
						message_packet->l2_cache_id = l2_caches[my_pid].id;
						message_packet->l2_cache_name = str_map_value(&l2_strn_map, l2_caches[my_pid].id);

						message_packet->src_name = l2_caches[my_pid].name;
						message_packet->src_id = str_map_string(&node_strn_map, l2_caches[my_pid].name);
						message_packet->dest_name = l3_caches[l3_map].name;
						message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

						//find victim
						message_packet->l2_victim_way = cgm_cache_replace_block(&(l2_caches[my_pid]), message_packet->set);

						//charge delay
						P_PAUSE(l2_caches[my_pid].latency);

						//transmit to L3
						cache_put_io_down_queue(&(l2_caches[my_pid]), message_packet);
						break;


					case cgm_cache_block_exclusive:
					case cgm_cache_block_shared:

						//stats
						l2_caches[my_pid].hits++;

						/*if this fails the I$ has tried to accesses a block in the D$ swim lane.*/
						/*assert(*cache_block_state_ptr != cgm_cache_block_exclusive);*/

						/*star todo this is broken, try to fix this.
						the I$ is accessing memory in the D$'s swim lane*/
						if(*cache_block_state_ptr == cgm_cache_block_exclusive)
						{
							message_packet->cache_block_state = cgm_cache_block_shared;
							//printf("I$ block found exclusive in L2 cycle %llu\n", P_TIME);
						}
						else
						{
							message_packet->cache_block_state = *cache_block_state_ptr;
						}
						//star todo end fix it code here. Delete the code up above if the error goes away

						P_PAUSE(l2_caches[my_pid].latency);

						//set message size
						message_packet->size = l1_i_caches[my_pid].block_size; //this can be either L1 I or L1 D cache block size.

						//update message status
						message_packet->access_type = cgm_access_puts;
						message_packet->cache_block_state = *cache_block_state_ptr;

						cache_put_io_up_queue(&(l2_caches[my_pid]), message_packet);

						//check if the packet has coalesced accesses.
						if(access_type == cgm_access_fetch_retry || message_packet->coalesced == 1)
						{
							//enter retry state.
							cache_coalesed_retry(&(l2_caches[my_pid]), message_packet->tag, message_packet->set);
						}

						break;
				}
			}
			else if(access_type == cgm_access_get || access_type == cgm_access_load_retry)
			{
				//get the status of the cache block
				cache_get_block_status(&(l2_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				switch(*cache_block_state_ptr)
				{
					case cgm_cache_block_noncoherent:
					case cgm_cache_block_owned:
						fatal("l2_cache_ctrl(): Invalid block state on load hit as %s cycle %llu\n",
								str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr), P_TIME);
						break;

					case cgm_cache_block_invalid:

						/*printf("l2 get miss\n");*/

						//stats
						l2_caches[my_pid].misses++;

						//check ORT for coalesce
						cache_check_ORT(&(l2_caches[my_pid]), message_packet);

						if(message_packet->coalesced == 1)
							continue;

						//error checking check this in L3 cache
						//message_packet->cache_block_state = *cache_block_hit_ptr;

						//add some routing/status data to the packet
						message_packet->access_type = cgm_access_get;

						l3_map = cgm_l3_cache_map(message_packet->set);
						message_packet->l2_cache_id = l2_caches[my_pid].id;
						message_packet->l2_cache_name = str_map_value(&l2_strn_map, l2_caches[my_pid].id);

						message_packet->src_name = l2_caches[my_pid].name;
						message_packet->src_id = str_map_string(&node_strn_map, l2_caches[my_pid].name);
						message_packet->dest_name = l3_caches[l3_map].name;
						message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

						// we are bringing a new block so evict the victim and invalidate the L1 copies
						//find victim
						message_packet->l2_victim_way = cgm_cache_replace_block(&(l2_caches[my_pid]), message_packet->set);
						cgm_L2_cache_evict_block(&(l2_caches[my_pid]), message_packet->set, message_packet->l2_victim_way);

						//charge delay
						P_PAUSE(l2_caches[my_pid].latency);

						//transmit to L3
						cache_put_io_down_queue(&(l2_caches[my_pid]), message_packet);
						break;

					case cgm_cache_block_modified:
					case cgm_cache_block_exclusive:
					case cgm_cache_block_shared:

						P_PAUSE(l2_caches[my_pid].latency);

						//set message size
						message_packet->size = l1_d_caches[my_pid].block_size; //this can be either L1 I or L1 D cache block size.

						//update message status
						if(*cache_block_state_ptr == cgm_cache_block_modified)
						{
							message_packet->access_type = cgm_access_putx;
						}
						else if(*cache_block_state_ptr == cgm_cache_block_exclusive)
						{
							message_packet->access_type = cgm_access_put_clnx;
						}
						else if(*cache_block_state_ptr == cgm_cache_block_shared)
						{
							message_packet->access_type = cgm_access_puts;
						}

						/*this will send the block and block state up to the high level cache.*/
						message_packet->cache_block_state = *cache_block_state_ptr;
						/*assert(*cache_block_state_ptr == cgm_cache_block_shared);*/

						cache_put_io_up_queue(&(l2_caches[my_pid]), message_packet);

						if(access_type == cgm_access_load_retry || message_packet->coalesced == 1)
						{
							//enter retry state.
							cache_coalesed_retry(&(l2_caches[my_pid]), message_packet->tag, message_packet->set);
						}

						break;
				}
			}
			else if(access_type == cgm_access_getx || access_type == cgm_access_store_retry)
			{
				//get the status of the cache block
				cache_get_block_status(&(l2_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				//printf("l2 GetX access\n");
				switch(*cache_block_state_ptr)
				{
					case cgm_cache_block_noncoherent:
					case cgm_cache_block_owned:
						fatal("l2_cache_ctrl(): Invalid block state on store hit assss %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
						break;

					case cgm_cache_block_invalid:

						//stats
						l2_caches[my_pid].misses++;

						//check L1 block state
						assert(message_packet->cache_block_state == cgm_cache_block_invalid);

						//check ORT for coalesce
						cache_check_ORT(&(l2_caches[my_pid]), message_packet);

						if(message_packet->coalesced == 1)
							continue;

						//find victim
						message_packet->l2_victim_way = cgm_cache_replace_block(&(l2_caches[my_pid]), message_packet->set);

						//set the data type bit in the block
						int type;
						type = message_packet->cpu_access_type == cgm_access_fetch ? 1 : 0;
						cgm_cache_set_block_type(&(l2_caches[my_pid]), type, message_packet->set, message_packet->l2_victim_way);

						//set access type
						message_packet->access_type = cgm_access_getx;

						//set L2 block state
						message_packet->cache_block_state = *cache_block_state_ptr;

						//set routing to the packet

						l3_map = cgm_l3_cache_map(message_packet->set);
						message_packet->l2_cache_id = l2_caches[my_pid].id;
						message_packet->l2_cache_name = str_map_value(&l2_strn_map, l2_caches[my_pid].id);

						message_packet->src_name = l2_caches[my_pid].name;
						message_packet->src_id = str_map_string(&node_strn_map, l2_caches[my_pid].name);
						message_packet->dest_name = l3_caches[l3_map].name;
						message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

						//charge delay
						P_PAUSE(l2_caches[my_pid].latency);

						//transmit to L3
						cache_put_io_down_queue(&(l2_caches[my_pid]), message_packet);

						//printf("L2 transmitting GetX as %s cycle %llu\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
						break;

					case cgm_cache_block_modified:
					case cgm_cache_block_exclusive:

						//stats;
						l2_caches[my_pid].hits++;

						P_PAUSE(l2_caches[my_pid].latency);

						//set message status and size
						message_packet->size = l1_d_caches[my_pid].block_size; //this can be either L1 I or L1 D cache block size.

						//message must be in exclusive or modified state for a hit on GetX
						if(*cache_block_state_ptr == cgm_cache_block_modified)
						{
							message_packet->access_type = cgm_access_putx;
						}
						else if(*cache_block_state_ptr == cgm_cache_block_exclusive)
						{
							message_packet->access_type = cgm_access_put_clnx;
						}

						message_packet->cache_block_state = *cache_block_state_ptr;

						cache_put_io_up_queue(&(l2_caches[my_pid]), message_packet);

						if(access_type == cgm_access_store_retry || message_packet->coalesced == 1)
						{
							//enter retry state.
							cache_coalesed_retry(&(l2_caches[my_pid]), message_packet->tag, message_packet->set);
						}
						break;

					case cgm_cache_block_shared:

						//stats
						l2_caches[my_pid].upgrade_misses++;

						assert(message_packet->access_type != cgm_access_store_retry);

						//for potential coalesced load-store condition
						if(message_packet->coalesced == 1)
						{
							message_packet->coalesced = 0;
						}

						//check ORT for coalesce
						cache_check_ORT(&(l2_caches[my_pid]), message_packet);

						if(message_packet->coalesced == 1)
							continue;

						//set block transient state
						cgm_cache_set_block_transient_state(&(l2_caches[my_pid]), message_packet->set, message_packet->way, message_packet->access_id, cgm_cache_block_transient);

						//set type and route message
						message_packet->access_type = cgm_access_upgrade;

						l3_map = cgm_l3_cache_map(message_packet->set);
						message_packet->l2_cache_id = l2_caches[my_pid].id;
						message_packet->l2_cache_name = str_map_value(&l2_strn_map, l2_caches[my_pid].id);

						message_packet->src_name = l2_caches[my_pid].name;
						message_packet->src_id = str_map_string(&node_strn_map, l2_caches[my_pid].name);
						message_packet->dest_name = l3_caches[l3_map].name;
						message_packet->dest_id = str_map_string(&node_strn_map, l3_caches[l3_map].name);

						//charge delay
						P_PAUSE(l2_caches[my_pid].latency);

						//transmit upgrade request to L3
						cache_put_io_down_queue(&(l2_caches[my_pid]), message_packet);
						break;
				}
			}
			else if(access_type == cgm_access_get_fwd)
			{
				printf("L2 get fwd cycle %llu\n", P_TIME);
				STOP;

			}
			else if(access_type == cgm_access_upgrade)
			{

				//printf("L2 upgrade\n");
				//received upgrade request from L1

				//check my own cache to see if I have the block and if it is in the exclusive state.
				//if I have the block and it is in the exclusive state grant the upgrade
				//if I have the block and it is in the shared state forward upgrade to L3
				//if I don't have the block inclusion is broken...

				//get the status of the cache block
				cache_get_block_status(&(l2_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				//block should be in this cache
				assert(*cache_block_hit_ptr != 0);

				P_PAUSE(l2_caches[my_pid].latency);

				//star todo this needs to change, currently it just sends the block back up to the l1 d cache
				if(*cache_block_state_ptr == cgm_cache_block_exclusive || *cache_block_state_ptr == cgm_cache_block_modified)
				{
					/*printf("here\n");*/

					//grant the upgrade...
					message_packet->access_type = cgm_access_upgrade_ack;
					cache_put_io_up_queue(&(l2_caches[my_pid]), message_packet);
				}
				else if(*cache_block_hit_ptr == cgm_cache_block_shared)
				{
					/*printf("here\n");*/

					//I have the block but need it upgraded as well.

					/*star todo this needs to tie into the L3 upgrade request
					for now set the block to exclusive in our cache and send back so the assert doesn't fail*/

					cgm_cache_set_block_state(&(l2_caches[my_pid]), message_packet->set, message_packet->way, cgm_cache_block_exclusive);

					message_packet->access_type = cgm_access_upgrade_ack;
					cache_put_io_up_queue(&(l2_caches[my_pid]), message_packet);
				}
				else
				{
					fatal("L2 upgrade invalid cache block state as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_hit_ptr));
				}

				//run again
				//step--;

				/*P_PAUSE(l2_caches[my_pid].latency);

				//grant the upgrade...
				message_packet->access_type = cgm_access_upgrade_ack;
				cache_put_io_up_queue(&(l2_caches[my_pid]), message_packet);*/

			}
			else if (access_type == cgm_access_upgrade_ack)
			{
				//ack from L3
				P_PAUSE(l2_caches[my_pid].latency);

				//we have permission to upgrade our set block state and retry access

				//get the status of the cache block
				cache_get_block_status(&(l2_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				//find the access in the ORT table and clear it.
				ort_clear(&(l2_caches[my_pid]), message_packet);

				/*if the block is no longer here on upgrade_ack,
				the block was evicted and maybe in the WB we can treat this as a miss.
				This means the trasient state is broken.*/
				if(*cache_block_hit_ptr != 1 || *cache_block_state_ptr != cgm_cache_block_shared)
				{
					//printf("evicted\n");
					//printf("access_id %llu as %s cycle %llu\n", message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
					fatal("access_id %llu evicted or not shared as %s cycle %llu\n", message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
				}

				//set the state to exclusive and clear the transient state
				cgm_cache_set_block_state(&(l2_caches[my_pid]), message_packet->set, message_packet->way, cgm_cache_block_exclusive);
				cgm_cache_set_block_transient_state(&(l2_caches[my_pid]), message_packet->set, message_packet->way, (int) NULL, cgm_cache_block_null);

				//enter the retry state
				message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);
				assert(message_packet->access_type == cgm_access_store_retry);
				assert(message_packet->coalesced != 1);

				message_packet = list_remove(l2_caches[my_pid].last_queue, message_packet);
				list_enqueue(l2_caches[my_pid].retry_queue, message_packet);

				//run again
				step--;
			}
			else if (access_type == cgm_access_inv)
			{
				//Invalidation request from lower cache

				/*printf("l2 inval\n");*/

				//get the block status
				cache_get_block_status(&(l2_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				/*message_packet = list_remove(l2_caches[my_pid].last_queue, message_packet);
				packet_destroy(message_packet);*/

				//find and invalidate the block
				/*if the block is missing the it has previously been removed
				from the L1 D cache as well, so we can ignore*/
				if(*cache_block_hit_ptr == 0)
				{
					//free the message packet
					message_packet = list_remove(l2_caches[my_pid].last_queue, message_packet);
					packet_destroy(message_packet);
				}
				/*if the block is found in the L2 it may or may not be in the L1 cache
				we must invalidate here and send an invalidation to the L1 D cache*/
				else if(*cache_block_hit_ptr == 1)
				{
					//get the way of the block
					message_packet->l2_victim_way = cgm_cache_replace_block(&(l2_caches[my_pid]), message_packet->set);
					cgm_L2_cache_evict_block(&(l2_caches[my_pid]), message_packet->set, message_packet->l2_victim_way);

					message_packet = list_remove(l2_caches[my_pid].last_queue, message_packet);
					packet_destroy(message_packet);
				}

			}
			else if(access_type == cgm_access_puts || access_type == cgm_access_putx || access_type == cgm_access_put_clnx)
			{

				/*if(access_type == cgm_access_put_clnx)
				{
					printf("L2 put clnx\n");
					STOP;
				}*/


				enum cgm_cache_block_state_t victim_trainsient_state;
				long long t_state_id;

				cache_get_block_status(&(l2_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				victim_trainsient_state = cgm_cache_get_block_transient_state(&(l2_caches[my_pid]), message_packet->set, message_packet->l2_victim_way);

				if(victim_trainsient_state == cgm_cache_block_transient)
				{
					//the victim is locked, either wait or choose another victim.
					t_state_id = cgm_cache_get_block_transient_state_id(&(l2_caches[my_pid]), message_packet->set, message_packet->l2_victim_way);

					//check for write before read condition.
					if(message_packet->access_id >= t_state_id)
					{
						//star todo i don't know if this is an actually problem or not
						printf("t_state_id %llu message_packet id %llu as %s\n", t_state_id, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
						assert(message_packet->access_id >= t_state_id);
					}

					//try again we will pull the coherence message eventually.
					printf("looping cache block %d\n", victim_trainsient_state);
					//printf("access_id %llu as %s cycle %llu\n", message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
					step--;
					P_PAUSE(1);
				}
				else
				{
					//find the access in the ORT table and clear it.
					ort_clear(&(l2_caches[my_pid]), message_packet);

					//set the block and retry the access in the cache.
					cache_put_block(&(l2_caches[my_pid]), message_packet);
				}

			}
			else if(access_type == cgm_access_write_back)
			{
				//on a write back with inclusive caches L2 Merges the line
				//if the write back is a surprise the block will be exclusive in the L2 cache, but the data is old.

				//WB from L1 D cache
				if(l2_caches[my_pid].last_queue == l2_caches[my_pid].Rx_queue_top)
				{

					//get the state of the cache block
					cache_get_block_status(&(l2_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

					switch(*cache_block_state_ptr)
					{
						case cgm_cache_block_invalid:

						/*star todo fix this, right now our test set up just randomly
						writes to blocks each time we access l2, this is one reason why the WB could miss
						come back to this once we have finished the rest of the protocol and don't need to set l2 inf*/

						/*printf("l2 wb miss cycle %llu\n", P_TIME);*/
						P_PAUSE(l2_caches[my_pid].latency);

						message_packet = list_remove(l2_caches[my_pid].last_queue, message_packet);
						assert(message_packet->write_back == 1);
						packet_destroy(message_packet);
						break;

						case cgm_cache_block_noncoherent:
						case cgm_cache_block_owned:
						case cgm_cache_block_shared:
						fatal("l2_cache_ctrl(): Invalid block state on writeback as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
							break;

						//star todo missing L2 cache code to upgrade its own block from shared to exclusive
						case cgm_cache_block_modified:
						case cgm_cache_block_exclusive:

							//set modified if the line was exclusive
							if(*cache_block_hit_ptr == cgm_cache_block_exclusive)
							{
								cgm_cache_set_block_state(&(l1_d_caches[my_pid]), message_packet->set, message_packet->way, cgm_cache_block_modified);
							}

							//in the real world we would merge changes with L2 block here.

							//move on
							P_PAUSE(l2_caches[my_pid].latency);

							message_packet = list_remove(l2_caches[my_pid].last_queue, message_packet);
							packet_destroy(message_packet);
							break;
					}
				}
				//if here the L2 generated it's own write back.
				else if(l2_caches[my_pid].last_queue == l2_caches[my_pid].write_back_buffer)
				{
					//this shouldn't be working yet.
					fatal("l2 WB buffer access\n");

					P_PAUSE(l2_caches[my_pid].latency);

					//send the write back to the L2 cache.
					cache_put_io_down_queue(&(l2_caches[my_pid]), message_packet);

					/*write backs are internally scheduled so decrement the counter
					figure out a way to do this better..
					perhaps have the cache advance itself when the evict results in a write back buffer entry.*/
					step--;

				}
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

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	int dirty;
	int sharers;
	int owning_core;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{
		/*wait here until there is a job to do.*/
		await(&l3_cache[my_pid], step);

		//get the message out of the queue
		message_packet = cache_get_message(&(l3_caches[my_pid]));

		if (message_packet == NULL || !switch_can_access(switches[my_pid].south_queue)
				|| !cache_can_access_Tx_bottom(&(l3_caches[my_pid])) || !cache_can_access_Tx_top(&(l3_caches[my_pid])))
		{
			//the cache state is preventing the cache from working this cycle stall.

			l3_caches[my_pid].stalls++;

			P_PAUSE(1);
		}
		else
		{
			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if(access_type == cgm_access_gets || access_type == cgm_access_fetch_retry)
			{
				//get the status of the cache block
				cache_get_block_status(&(l3_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				//check the directory dirty bit status for sanity check
				dirty = cgm_cache_get_dir_dirty_bit(&(l3_caches[my_pid]), message_packet->set, message_packet->way);

				switch(*cache_block_state_ptr)
				{
					case cgm_cache_block_noncoherent:
					case cgm_cache_block_modified:
					case cgm_cache_block_owned:
						/*printf("l3_cache_ctrl(): GetS invalid block state on hit as %d\n", *cache_block_state_ptr);*/
						fatal("l3_cache_ctrl(): GetS invalid block state on hit as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
						break;

					case cgm_cache_block_invalid:

						//stats;
						l3_caches[my_pid].misses++;

						//check ORT for coalesce
						cache_check_ORT(&(l3_caches[my_pid]), message_packet);

						if(message_packet->coalesced == 1)
							continue;

						//find victim again because LRU has been updated on hits.
						message_packet->l3_victim_way = cgm_cache_replace_block(&(l3_caches[my_pid]), message_packet->set);

						//add some routing/status data to the packet
						message_packet->access_type = cgm_access_mc_get;

						//set return cache block state
						//star todo look into this, this should at least works for I$ requests
						message_packet->cache_block_state = cgm_cache_block_shared;

						assert(message_packet->cpu_access_type == cgm_access_fetch);

						message_packet->src_name = l3_caches[my_pid].name;
						message_packet->src_id = str_map_string(&node_strn_map, l3_caches[my_pid].name);
						message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
						message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

						//charge delay
						P_PAUSE(l3_caches[my_pid].latency);

						//transmit to SA/MC
						cache_put_io_down_queue(&(l3_caches[my_pid]), message_packet);
						break;

					case cgm_cache_block_exclusive:
					case cgm_cache_block_shared:

						//stats;
						l3_caches[my_pid].hits++;

						//GetS should never be dirty in directory
						assert(dirty == 0);

						//set the presence bit in the directory for the requesting core.
						cgm_cache_set_dir(&(l3_caches[my_pid]), message_packet->set, message_packet->way, message_packet->l2_cache_id);

						//update message packet status
						message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;
						message_packet->access_type = cgm_access_puts;

						/*star todo this is broken, try to fix this.
						the I$ is accessing memory in the D$'s swim lane*/
						if(*cache_block_state_ptr == cgm_cache_block_exclusive)
						{
							message_packet->cache_block_state = cgm_cache_block_shared;
						}
						else
						{
							message_packet->cache_block_state = *cache_block_state_ptr;
						}

						//message_packet->cache_block_state = cache_block_shared;
						//assert(message_packet->cache_block_state == cache_block_shared);
						/*printf("l3 block type %s\n", str_map_value(&cache_block_state_map, *cache_block_state_ptr));*/

						message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
						message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
						message_packet->src_name = l3_caches[my_pid].name;
						message_packet->src_id = str_map_string(&node_strn_map, l3_caches[my_pid].name);

						P_PAUSE(l3_caches[my_pid].latency);

						//printf("Sending %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

						cache_put_io_up_queue(&(l3_caches[my_pid]), message_packet);

						//check if the packet has coalesced accesses.
						if(access_type == cgm_access_fetch_retry || message_packet->coalesced == 1)
						{
							//enter retry state.
							cache_coalesed_retry(&(l3_caches[my_pid]), message_packet->tag, message_packet->set);
						}
						break;
				}
			}
			else if(access_type == cgm_access_get || access_type == cgm_access_load_retry)
			{

				//get the status of the cache block
				cache_get_block_status(&(l3_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				//confer with the directory
				//check the directory dirty bit status
				dirty = cgm_cache_get_dir_dirty_bit(&(l3_caches[my_pid]), message_packet->set, message_packet->way);
				//get number of sharers
				sharers = cgm_cache_get_num_shares(&(l3_caches[my_pid]), message_packet->set, message_packet->way);
				//check to see is access is from an already owning core
				owning_core = cgm_cache_is_owning_core(&(l3_caches[my_pid]), message_packet->set, message_packet->way, message_packet->l2_cache_id);

				switch(*cache_block_state_ptr)
				{
					case cgm_cache_block_noncoherent:
					case cgm_cache_block_owned:
					fatal("l3_cache_ctrl(): Get invalid block state on hit as %s\n", str_map_value(&cgm_cache_block_state_map, *cache_block_state_ptr));
						break;

					case cgm_cache_block_invalid:

						/*printf("l3 load miss\n");*/

						//stats;
						l3_caches[my_pid].misses++;
						assert(message_packet->cpu_access_type == cgm_access_load);

						//check ORT for coalesce
						cache_check_ORT(&(l3_caches[my_pid]), message_packet);

						if(message_packet->coalesced == 1)
							continue;

						//find victim again because LRU has been updated on hits.
						message_packet->l3_victim_way = cgm_cache_replace_block(&(l3_caches[my_pid]), message_packet->set);

						//changes start here
						cgm_L3_cache_evict_block(&(l3_caches[my_pid]), message_packet->set, message_packet->l3_victim_way, sharers);

						//add some routing/status data to the packet
						message_packet->access_type = cgm_access_mc_get;

						//star todo this should be exclusive when Get is fully working
						/*message_packet->cache_block_state = cgm_cache_block_exclusive;*/
						message_packet->cache_block_state = cgm_cache_block_shared;

						message_packet->src_name = l3_caches[my_pid].name;
						message_packet->src_id = str_map_string(&node_strn_map, l3_caches[my_pid].name);
						message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
						message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

						//charge delay
						P_PAUSE(l3_caches[my_pid].latency);

						//transmit to SA/MC
						cache_put_io_down_queue(&(l3_caches[my_pid]), message_packet);
						break;

					case cgm_cache_block_shared:

						//stats;
						l3_caches[my_pid].hits++;

						assert(dirty == 0);

						//update message status
						message_packet->access_type = cgm_access_puts;

						//get the cache block state
						message_packet->cache_block_state = *cache_block_state_ptr;

						//testing
						/*uncomment this when exclusive/modified is working.
						currently the sim by passes the exclusive stage.*/
						assert(*cache_block_state_ptr == cgm_cache_block_shared);
						//there should be at least 1 sharer (after a downgrade)
						/*assert(sharers >= 1);*/

						//set the presence bit in the directory for the requesting core.
						cgm_cache_set_dir(&(l3_caches[my_pid]), message_packet->set, message_packet->way, message_packet->l2_cache_id);

						//set message package size
						message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

						//update routing
						message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
						message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
						message_packet->src_name = l3_caches[my_pid].name;
						message_packet->src_id = str_map_string(&node_strn_map, l3_caches[my_pid].name);

						P_PAUSE(l3_caches[my_pid].latency);

						cache_put_io_up_queue(&(l3_caches[my_pid]), message_packet);

						//check if the packet has coalesced accesses.
						if(access_type == cgm_access_load_retry || message_packet->coalesced == 1)
						{
							//enter retry state.
							cache_coalesed_retry(&(l3_caches[my_pid]), message_packet->tag, message_packet->set);
						}

						break;

					case cgm_cache_block_exclusive:

						//stats;
						l3_caches[my_pid].hits++;

						//star todo update this message when working in GETX
						/*on the first GET the block should have been brought in as exclusive.
						Then it will be a hit on retry with no presence bits set (exclusive).
						On a subsequent access (by either the requesting core or a different core) the block will be here as exclusive,
						if the request comes from the original core the block can be sent as exclusive
						if the request comes from a different core the block will need to be downgraded to shared before sending to requesting core.
						Once the block is downgraded to shared it will be in both cores and L3 as shared*/

						assert(sharers >= 0 && sharers <= num_cores);
						assert(owning_core >= 0 && owning_core <= 1);

						//if it is a new access or a repeat access from an already owning core.
						if(sharers == 0 || owning_core == 1)
						{
							//update message status
							message_packet->access_type = cgm_access_put_clnx;

							//get the cache block state
							message_packet->cache_block_state = *cache_block_state_ptr;

							//testing
							assert(dirty == 0);
							assert(*cache_block_state_ptr == cgm_cache_block_exclusive || *cache_block_state_ptr == cgm_cache_block_modified);

							//set the presence bit in the directory for the requesting core.
							cgm_cache_set_dir(&(l3_caches[my_pid]), message_packet->set, message_packet->way, message_packet->l2_cache_id);

							//set message package size
							message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;

							//update routing headers
							message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
							message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
							message_packet->src_name = l3_caches[my_pid].name;
							message_packet->src_id = str_map_string(&node_strn_map, l3_caches[my_pid].name);

							P_PAUSE(l3_caches[my_pid].latency);

							//send it out
							cache_put_io_up_queue(&(l3_caches[my_pid]), message_packet);

							//check if the packet has coalesced accesses.
							if(access_type == cgm_access_load_retry || message_packet->coalesced == 1)
							{
								//enter retry state.
								cache_coalesed_retry(&(l3_caches[my_pid]), message_packet->tag, message_packet->set);
							}
						}
						//if it is a new access from another core(s). downgrade the owning core.
						else if (sharers >= 1)
						{
							// in the exclusive state there should only be one core with the cache block
							assert(sharers == 1);

							/*its possible that the data is dirty in the owning core
							so forward the GET to the owning core*/

							//change the access type
							message_packet->access_type = cgm_access_get_fwd;

							//don't set the block state (yet)

							//don't set the presence bit in the directory for the requesting core (yet).

							//don't change the message package size

							//increment the directory pending bit by the number of sharers. Its exclusive so should only be 1.
							cgm_cache_set_dir_pending_bit(&(l3_caches[my_pid]), message_packet->set, message_packet->way, sharers);

							/*update the routing headers.
							set src as requesting cache and dest as owning cache.
							We can derive the home (directory) later from the access address.*/

							//owning node L2
							//get the id of the owning core
							owning_core = cgm_cache_get_xown_core(&(l3_caches[my_pid]), message_packet->set, message_packet->way);
							assert(owning_core >= 0 && owning_core <num_cores);
							message_packet->dest_name = str_map_value(&l2_strn_map, owning_core);
							message_packet->dest_id = str_map_string(&node_strn_map, message_packet->dest_name);

							//requesting node L2
							message_packet->src_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
							message_packet->src_name = str_map_value(&l2_strn_map, message_packet->dest_id);

							P_PAUSE(l3_caches[my_pid].latency);

							//send the message out for the total number of sharers todo
							cache_put_io_up_queue(&(l3_caches[my_pid]), message_packet);

							//check if the packet has coalesced accesses.
							if(access_type == cgm_access_load_retry || message_packet->coalesced == 1)
							{
								//enter retry state.
								cache_coalesed_retry(&(l3_caches[my_pid]), message_packet->tag, message_packet->set);
							}

						}

						break;

					case cgm_cache_block_modified:


						if(*cache_block_state_ptr == cgm_cache_block_modified)
						{
							message_packet->access_type = cgm_access_putx;
						}

						fatal("L3 modified cache block without GetX\n");
						break;

				}
			}
			else if(access_type == cgm_access_getx || access_type == cgm_access_store_retry)
			{

				//get the status of the cache block
				cache_get_block_status(&(l3_caches[my_pid]), message_packet, cache_block_hit_ptr, cache_block_state_ptr);

				//check the directory dirty bit status
				dirty = cgm_cache_get_dir_dirty_bit(&(l3_caches[my_pid]), message_packet->set, message_packet->l3_victim_way);

				switch(*cache_block_state_ptr)
				{

					case cgm_cache_block_noncoherent:
					case cgm_cache_block_owned:
						fatal("l3_cache_ctrl(): Invalid block state on hit\n");
						break;

					case cgm_cache_block_invalid:

						//stats
						l3_caches[my_pid].misses++;

						//check ORT for coalesce
						cache_check_ORT(&(l3_caches[my_pid]), message_packet);

						if(message_packet->coalesced == 1)
							continue;

						//find victim again because LRU has been updated on hits.
						message_packet->l3_victim_way = cgm_cache_replace_block(&(l3_caches[my_pid]), message_packet->set);

						//add some routing/status data to the packet
						message_packet->access_type = cgm_access_mc_get;
						message_packet->cache_block_state = cgm_cache_block_exclusive;

						message_packet->src_name = l3_caches[my_pid].name;
						message_packet->src_id = str_map_string(&node_strn_map, l3_caches[my_pid].name);
						message_packet->dest_id = str_map_string(&node_strn_map, "sys_agent");
						message_packet->dest_name = str_map_value(&node_strn_map, message_packet->dest_id);

						//charge delay
						P_PAUSE(l3_caches[my_pid].latency);

						//transmit to L3
						cache_put_io_down_queue(&(l3_caches[my_pid]), message_packet);
						break;

					case cgm_cache_block_modified:
					case cgm_cache_block_shared:
					case cgm_cache_block_exclusive:

						//stats;
						l3_caches[my_pid].hits++;

						//star todo block may be in dirty state, need to manipulate as required.

						//L2 error checking
						//assert(message_packet->cache_block_state == cache_block_invalid || message_packet->cache_block_state == cache_block_shared);

						//update directory
						cgm_cache_set_dir(&(l3_caches[my_pid]), message_packet->set, message_packet->l3_victim_way, message_packet->l2_cache_id);

						// update message packet status
						message_packet->size = l2_caches[str_map_string(&node_strn_map, message_packet->l2_cache_name)].block_size;
						message_packet->access_type = cgm_access_putx;
						message_packet->cache_block_state = *cache_block_state_ptr;

						//message_packet->cache_block_state = *cache_block_state_ptr;
						/*printf("l3 block type %s\n", str_map_value(&cache_block_state_map, *cache_block_state_ptr));*/

						message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
						message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
						message_packet->src_name = l3_caches[my_pid].name;
						message_packet->src_id = str_map_string(&node_strn_map, l3_caches[my_pid].name);

						P_PAUSE(l3_caches[my_pid].latency);

						//printf("Sending %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));

						cache_put_io_up_queue(&(l3_caches[my_pid]), message_packet);

						//check if the packet has coalesced accesses.
						if(access_type == cgm_access_store_retry || message_packet->coalesced == 1)
						{
							//enter retry state.
							cache_coalesed_retry(&(l3_caches[my_pid]), message_packet->tag, message_packet->set);
						}

						break;
				}
			}
			else if(access_type == cgm_access_upgrade)
			{
				//star todo this needs to check state of directory and respond accordingly.
				/*printf("L3 upgrade request\n");*/

				P_PAUSE(l3_caches[my_pid].latency);

				message_packet->access_type = cgm_access_upgrade_ack;

				//set route etc
				message_packet->dest_id = str_map_string(&node_strn_map, message_packet->l2_cache_name);
				message_packet->dest_name = str_map_value(&l2_strn_map, message_packet->dest_id);
				message_packet->src_name = l3_caches[my_pid].name;
				message_packet->src_id = str_map_string(&node_strn_map, l3_caches[my_pid].name);

				cache_put_io_up_queue(&(l3_caches[my_pid]), message_packet);

			}
			else if(access_type == cgm_access_write_back)
			{
				//star todo finish this up.
				P_PAUSE(l3_caches[my_pid].latency);

				//printf("l3 wb\n");

				message_packet = list_remove(l3_caches[my_pid].last_queue, message_packet);
				assert(message_packet->write_back == 1);
				packet_destroy(message_packet);
			}
			else if (access_type == cgm_access_mc_put)
			{
				//find the access in the ORT table and clear it.
				ort_clear(&(l3_caches[my_pid]), message_packet);

				//set the block and retry the access in the cache.
				cache_put_block(&(l3_caches[my_pid]), message_packet);
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

/*void gpu_s_cache_ctrl(void){

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
				//gpu_l1_cache_access_load(&(gpu_s_caches[my_pid]), message_packet);
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

	 should never get here
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

		wait here until there is a job to do.
		await(&gpu_l2_cache[my_pid], step);

		//check the top or bottom rx queues for messages.
		message_packet = cache_get_message(&(gpu_l2_caches[my_pid]));

		if (message_packet == NULL || !hub_iommu_can_access(hub_iommu->Rx_queue_top[my_pid]))
		{
			//the cache state is preventing the cache from working this cycle stall.
			printf("%s stalling cycle %llu\n", gpu_l2_caches[my_pid].name, P_TIME);
			printf("stalling\n");
			future_advance(&gpu_l2_cache[my_pid], etime.count + 2);
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
	 should never get here
	fatal("gpu_l2_cache_ctrl task is broken\n");
	return;
}*/

void l1_i_cache_down_io_ctrl(void){

	int my_pid = l1_i_io_pid++;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l1_i_caches[my_pid].cache_io_down_ec, step);
		step++;

		message_packet = list_dequeue(l1_i_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		access_id = message_packet->access_id;
		transfer_time = (message_packet->size/l1_i_caches[my_pid].bus_width);

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		P_PAUSE(transfer_time);

		//drop into the next virtual lane correct queue.
		list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);
		advance(&l2_cache[my_pid]);
	}

	return;
}

void l1_d_cache_down_io_ctrl(void){

	int my_pid = l1_d_io_pid++;
	long long step = 1;

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

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		P_PAUSE(transfer_time);

		//drop into the next correct virtual lane/queue.
		if(message_packet->access_type == cgm_access_get || message_packet->access_type == cgm_access_getx
				|| message_packet->access_type == cgm_access_write_back)
		{
			list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);
			advance(&l2_cache[my_pid]);
		}
		else if(message_packet->access_type == cgm_access_upgrade)
		{
			list_enqueue(l2_caches[my_pid].Coherance_Rx_queue, message_packet);
			advance(&l2_cache[my_pid]);
		}
	}

	return;
}

void l2_cache_up_io_ctrl(void){

	int my_pid = l2_up_io_pid++;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	long long access_id = 0;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l2_caches[my_pid].cache_io_up_ec, step);
		step++;

		message_packet = list_dequeue(l2_caches[my_pid].Tx_queue_top);

		//printf("cycle %llu\n");
		//fflush(stdout);

		assert(message_packet);

		access_id = message_packet->access_id;

		//star todo fix this we need a top and bottom bus_width
		transfer_time = (message_packet->size/l1_i_caches[my_pid].bus_width);

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		P_PAUSE(transfer_time);

		//drop into the correct l1 cache queue and lane.
		if (message_packet->cpu_access_type == cgm_access_fetch)
		{
			list_enqueue(l1_i_caches[my_pid].Rx_queue_bottom, message_packet);
			advance(&l1_i_cache[my_pid]);
		}
		else if (message_packet->cpu_access_type == cgm_access_load || message_packet->cpu_access_type == cgm_access_store)
		{
			if(message_packet->access_type == cgm_access_puts || message_packet->access_type == cgm_access_putx
					|| message_packet->access_type == cgm_access_put_clnx)
			{
				list_enqueue(l1_d_caches[my_pid].Rx_queue_bottom, message_packet);
				advance(&l1_d_cache[my_pid]);
			}
			else if(message_packet->access_type == cgm_access_upgrade_ack || message_packet->access_type == cgm_access_inv)
			{
				list_enqueue(l1_d_caches[my_pid].Coherance_Rx_queue, message_packet);
				advance(&l1_d_cache[my_pid]);
			}
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

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

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

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

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

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		P_PAUSE(transfer_time);

		//drop in to the switch queue
		list_enqueue(switches[my_pid].south_queue, message_packet);
		advance(&switches_ec[my_pid]);
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

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

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

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

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

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

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

		if(transfer_time == 0)
		{
			transfer_time = 1;
		}

		P_PAUSE(transfer_time);

		list_enqueue(hub_iommu->Rx_queue_top[my_pid], message_packet);
		advance(hub_iommu_ec);

	}
	return;
}

void cache_get_block_status(struct cache_t *cache, struct cgm_packet_t *message_packet, int *cache_block_hit_ptr, int *cache_block_state_ptr){

	enum cgm_access_kind_t access_type;
	long long access_id = 0;
	unsigned int addr = 0;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;
	/*int state = 0;*/

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;

	//stats

	//access information
	access_type = message_packet->access_type;
	access_id = message_packet->access_id;
	addr = message_packet->address;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, addr, set_ptr, tag_ptr, offset_ptr);

	//store the decode
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;

	CGM_DEBUG(CPU_cache_debug_file,"%s access_id %llu cycle %llu as %s addr 0x%08u, tag %d, set %d, offset %u\n",
			cache->name, access_id, P_TIME, (char *)str_map_value(&cgm_mem_access_strn_map, access_type), addr, *tag_ptr, *set_ptr, *offset_ptr);

	//////testing
	if(l1_i_inf && cache->cache_type == l1_i_cache_t)
	{
		cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, cache_block_state_ptr);
		assert(way >= 0 && way <cache->num_sets);
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_shared);
	}

	if(l1_i_miss && cache->cache_type == l1_i_cache_t)
	{
		cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, cache_block_state_ptr);
		assert(way >= 0 && way <cache->num_sets);
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_invalid);
	}


	if(l1_d_inf && cache->cache_type == l1_d_cache_t)
	{
		cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, cache_block_state_ptr);
		assert(way >= 0 && way <cache->num_sets);

		if(message_packet->cpu_access_type == cgm_access_load)
		{	//cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cache_block_exclusive);
			cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_shared);
		}
		if(message_packet->cpu_access_type == cgm_access_store)
		{
			cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_exclusive);
		}
	}

	if(l1_d_miss && cache->cache_type == l1_d_cache_t)
	{
		cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, cache_block_state_ptr);
		assert(way >= 0 && way <cache->num_sets);
		cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_invalid);
	}


	if(l2_inf && cache->cache_type == l2_cache_t)
	{
		cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, cache_block_state_ptr);
		assert(way >= 0 && way <cache->num_sets);

		if(message_packet->cpu_access_type == cgm_access_fetch )
		{
			cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_shared);
		}
		else if(message_packet->cpu_access_type == cgm_access_store || message_packet->cpu_access_type == cgm_access_load)
		{
			cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_exclusive);
		}
	}

	if(l3_inf && cache->cache_type == l3_cache_t)
	{
		if(message_packet->cpu_access_type == cgm_access_fetch)
		{
			cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_shared);
		}
		else if(message_packet->cpu_access_type == cgm_access_store || message_packet->cpu_access_type == cgm_access_load)
		{
			cgm_cache_set_block(cache, *set_ptr, *way_ptr, *tag_ptr, cgm_cache_block_exclusive);
		}
	}

	if(l2_miss || l3_miss)
	{
		fatal("l2 and l3 caches set to miss");
	}
	//////testing

	//get the block and the state of the block
	*(cache_block_hit_ptr) = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, cache_block_state_ptr);

	//store the way
	message_packet->way = way;

	//update way list for LRU if block is present.
	if(*(cache_block_hit_ptr) == 1)
	{
		cgm_cache_access_block(cache, set, way);
	}

	return;
}

void cache_l1_i_return(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//debug
	CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu cleared from mem system\n", cache->name, message_packet->access_id, P_TIME);

	//remove packet from cache queue, global queue, and simulator memory
	message_packet = list_remove(cache->last_queue, message_packet);
	remove_from_global(message_packet->access_id);

	//stats
	CGM_STATS(mem_trace_file, "l1_i_cache_%d total cycles %llu access_id %llu\n", cache->id, (message_packet->end_cycle - message_packet->start_cycle), message_packet->access_id);

	packet_destroy(message_packet);
	return;
}

void cache_l1_d_return(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//debug
	CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu cleared from mem system\n", cache->name, message_packet->access_id, P_TIME);

	//remove packet from cache queue, global queue, and simulator memory
	message_packet = list_remove(cache->last_queue, message_packet);
	linked_list_add(message_packet->event_queue, message_packet->data);

	/*printf("access_id %llu finished cycles %llu \n", message_packet->access_id, (message_packet->end_cycle - message_packet->start_cycle));*/

	packet_destroy(message_packet);
	return;
}

void cache_check_ORT(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int i, row;

	i = ort_search(cache, message_packet->tag, message_packet->set);

	//unique memory accesses
	if(i == cache->mshr_size)
	{
		//find an empty row and add it
		row = get_ort_status(cache);
		assert(row < cache->mshr_size);
		ort_set(cache, row, message_packet->tag, message_packet->set);
	}
	//can be coalesced
	else if(i >= 0 && i < cache->mshr_size)
	{
		//entry found in ORT so coalesce the packet
		assert(cache->ort[i][0] == message_packet->tag && cache->ort[i][1] == message_packet->set && cache->ort[i][2] == 1);

		CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu coalesced\n", cache->name, message_packet->access_id, P_TIME);

		message_packet->coalesced = 1;

		list_remove(cache->last_queue, message_packet);
		list_enqueue(cache->ort_list, message_packet);
	}
	else
	{
		fatal("cache_check_ORT(): %s i outside of bounds\n", cache->name);
	}

	return;
}

void cache_put_io_up_queue(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu L1 bottom queue free size %d\n",
			cache->name, access_id, P_TIME, list_count(l1_i_caches[cache->id].Rx_queue_bottom));*/

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->Tx_queue_top, message_packet);
	advance(cache->cache_io_up_ec);
	return;
}

void cache_put_io_down_queue(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu l2 queue free size %d\n",
			cache->name, message_packet->access_id, P_TIME, list_count(l2_caches[cache->id].Rx_queue_top));
	CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu removed from %s size %d\n",
			cache->name, message_packet->access_id, P_TIME, cache->last_queue->name, list_count(cache->last_queue));
	CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu %s as %s\n",
			cache->name, message_packet->access_id, P_TIME, l2_caches[cache->id].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
	CGM_DEBUG(protocol_debug_file, "%s Access_id %llu cycle %llu %s miss SEND %s %s\n",
			cache->name, message_packet->access_id, P_TIME, cache->name, l2_caches[cache->id].name, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));*/

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->Tx_queue_bottom, message_packet);
	advance(cache->cache_io_down_ec);
	return;
}

void cache_put_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	enum cgm_cache_block_state_t victim_state;

	//star todo figure out how to handle the write back here.
	if(cache->cache_type == l3_cache_t)
	{
		//check if directory entry is dirty or clean for victim
		//dirty = cgm_cache_get_dir_dirty_bit(cache, message_packet->set, message_packet->l3_victim_way);

		victim_state = cgm_cache_get_block_state(cache, message_packet->set, message_packet->l3_victim_way);

		/*first if the block is modified it is dirty and needs to be written back
		move a copy of the block to the write back buffer*/
		if (victim_state == cgm_cache_block_modified)
		{
			//move the block to the WB buffer
			struct cgm_packet_t *write_back_packet = packet_create();

			//star todo remember to set l2 cache id in WB packet
			init_write_back_packet(cache, write_back_packet, message_packet->set, message_packet->l3_victim_way);

			list_enqueue(cache->write_back_buffer, write_back_packet);
		}

		//clear the directory entry
		cgm_cache_clear_dir(cache, message_packet->set, message_packet->l3_victim_way);

		//set the block data
		cgm_cache_set_block(cache, message_packet->set, message_packet->l3_victim_way, message_packet->tag, message_packet->cache_block_state);

		//set the sharer in the directory
		/*cgm_cache_set_dir(cache, message_packet->set, message_packet->l3_victim_way, message_packet->l2_cache_id);*/

		/*assert(message_packet->cache_block_state);*/

		//set retry state
		message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);
	}
	else if(cache->cache_type == l2_cache_t)
	{

		/*printf("access_id %llu L2 putting cycle %llu \n", message_packet->access_id, P_TIME);
		printf("size %d \n", list_count(cache->last_queue));

		//star todo add the wb back in.

		//evict the victim
		//the block can be dropped if it is not modified.

		//victim_state = cgm_cache_get_block_state(cache, message_packet->set, message_packet->l2_victim_way);

		first if the block is modified it is dirty and needs to be written back
		move a copy of the block to the write back buffer
		if (victim_state == cgm_cache_block_modified)
		{
			//move the block to the WB buffer
			struct cgm_packet_t *write_back_packet = packet_create();

			//star todo remember to set l2 cache id in WB packet
			init_write_back_packet(cache, write_back_packet, message_packet->set, message_packet->l2_victim_way);

			list_enqueue(cache->write_back_buffer, write_back_packet);
		}*/

		//error checking
		if(message_packet->access_type == cgm_access_puts)
		{
			assert(message_packet->cache_block_state == cgm_cache_block_shared);
		}
		else if(message_packet->access_type == cgm_access_putx)
		{
			assert(message_packet->cache_block_state == cgm_cache_block_modified);
		}
		else if(message_packet->access_type == cgm_access_put_clnx)
		{
			assert(message_packet->cache_block_state == cgm_cache_block_exclusive);
		}

		cgm_cache_set_block(cache, message_packet->set, message_packet->l2_victim_way, message_packet->tag, message_packet->cache_block_state);

		//set retry state
		message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);

	}
	else if(cache->cache_type == l1_d_cache_t)
	{
		victim_state = cgm_cache_get_block_state(cache, message_packet->set, message_packet->l1_victim_way);

		/*first if the block is modified it is dirty and needs to be written back
		move a copy of the block to the write back buffer*/
		if (victim_state == cgm_cache_block_modified)
		{
			//move the block to the WB buffer
			struct cgm_packet_t *write_back_packet = packet_create();

			//star todo remember to set l2 cache id in WB packet
			init_write_back_packet(cache, write_back_packet, message_packet->set, message_packet->l1_victim_way);

			list_enqueue(cache->write_back_buffer, write_back_packet);
		}

		//checks
		if(message_packet->access_type == cgm_access_put_clnx ||message_packet->access_type == cgm_access_putx)
		{
			assert(message_packet->cache_block_state == cgm_cache_block_exclusive || message_packet->cache_block_state == cgm_cache_block_modified);
		}
		else if(message_packet->access_type == cgm_access_puts)
		{
			assert(message_packet->cache_block_state == cgm_cache_block_shared);
		}

		//the block can be dropped if it is not modified.
		cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

		//set retry state
		message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);

	}
	else if(cache->cache_type == l1_i_cache_t)
	{
		//error checking
		/*assert(message_packet->access_type == cgm_access_puts);
		assert(message_packet->cache_block_state == cache_block_shared);*/

		//write the block
		cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);

		//set retry state
		message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);
	}
	else
	{
		fatal("cache_put_block(): bad cache type cycle %llu\n", P_TIME);
	}

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);
	advance(cache->ec_ptr);

	return;
}

void cache_coalesed_retry(struct cache_t *cache, int tag, int set){

	struct cgm_packet_t *ort_packet;
	int i = 0;

	LIST_FOR_EACH(cache->ort_list, i)
	{
		//get pointer to access in queue and check it's status.
		ort_packet = list_get(cache->ort_list, i);

		if(ort_packet->tag == tag && ort_packet->set == set)
		{
			ort_packet = list_remove_at(cache->ort_list, i);

			//set the correct retry type
			//star todo retry types could be a potential problem.
			/*if(cache->cache_type == l1_i_cache_t || cache->cache_type == l1_d_cache_t)
			{
				ort_packet->access_type = cgm_cache_get_retry_state(ort_packet->cpu_access_type);
				//ort_packet->access_type = cgm_cache_get_retry_state(ort_packet->cpu_access_type);
			}
			else if(cache->cache_type == l2_cache_t || cache->cache_type == l3_cache_t)
			{
				ort_packet->access_type = cgm_cache_get_retry_state(ort_packet->cpu_access_type);
			}*/

			ort_packet->access_type = cgm_cache_get_retry_state(ort_packet->cpu_access_type);

			list_enqueue(cache->retry_queue, ort_packet);
			advance(cache->ec_ptr);

			/*this may cause problems the intent is to run one coalesced
			packet per iteration of the retry state so the timing is correctly charged*/
			return;
		}
	}

	//no coalesced packets remaining.
	return;
}

void cgm_cache_clear_dir(struct cache_t *cache, int set, int way){

	cache->sets[set].blocks[way].directory_entry.entry = 0;

	return;
}

void cgm_cache_set_dir(struct cache_t *cache, int set, int way, int l2_cache_id){

	int num_cores = x86_cpu_num_cores;

	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);
	assert(l2_cache_id > (-1) && l2_cache_id < num_cores);

	if(l2_cache_id == 0)
	{
		cache->sets[set].blocks[way].directory_entry.entry_bits.p0 = 1;
	}
	else if(l2_cache_id == 1)
	{
		cache->sets[set].blocks[way].directory_entry.entry_bits.p1 = 1;
	}
	else if(l2_cache_id == 2)
	{
		cache->sets[set].blocks[way].directory_entry.entry_bits.p2 = 1;
	}
	else if(l2_cache_id == 3)
	{
		cache->sets[set].blocks[way].directory_entry.entry_bits.p3 = 1;
	}
	else
	{
		fatal("cgm_cache_set_dir(): current dir implementation supports up to 4 cores.\n");
	}

	return;
}

int cgm_cache_get_dir_dirty_bit(struct cache_t *cache, int set, int way){

	int dirty = 0;

	dirty = cache->sets[set].blocks[way].directory_entry.entry_bits.dirty;

	assert(dirty == 1 || dirty == 0);
	return dirty;
}

void cgm_cache_set_dir_pending_bit(struct cache_t *cache, int set, int way, int sharers){

	cache->sets[set].blocks[way].directory_entry.entry_bits.pending = sharers;

	return;

}

int cgm_cache_is_owning_core(struct cache_t *cache, int set, int way, int l2_cache_id){

	int core_match = 0;
	int num_cores = x86_cpu_num_cores;

	if(l2_cache_id == 0 && cache->sets[set].blocks[way].directory_entry.entry_bits.p0 == 1)
	{
		core_match++;
	}
	else if(l2_cache_id == 1 && cache->sets[set].blocks[way].directory_entry.entry_bits.p1 == 1)
	{
		core_match++;
	}
	else if(l2_cache_id == 2 && cache->sets[set].blocks[way].directory_entry.entry_bits.p2 == 1)
	{
		core_match++;
	}
	else if(l2_cache_id == 3 && cache->sets[set].blocks[way].directory_entry.entry_bits.p3 == 1)
	{
		core_match++;
	}


	if(l2_cache_id < 0 && l2_cache_id >= num_cores)
	{
		fatal("cgm_cache_is_owning_core(): current dir implementation supports up to 4 cores.\n");
	}

	assert(core_match == 0 || core_match == 1);

	return core_match;
}

int cgm_cache_get_num_shares(struct cache_t *cache, int set, int way){

	int sharers = 0;
	int num_cores = x86_cpu_num_cores;
	int i = 0;
	unsigned char bit_vector;

	/*get the number of shares, mask away everything but the the share bit field
	and take the log of the vale to get the number of sharers*/

	//testing
	/*cache->sets[set].blocks[way].directory_entry.entry_bits.p0 = 0;
	cache->sets[set].blocks[way].directory_entry.entry_bits.p1 = 1;
	cache->sets[set].blocks[way].directory_entry.entry_bits.p2 = 1;
	cache->sets[set].blocks[way].directory_entry.entry_bits.p3 = 1;*/
	//testing

	//star todo this is dynamic, but the simulator only supports up to 4 cores for now.
	bit_vector = cache->sets[set].blocks[way].directory_entry.entry;
	bit_vector = bit_vector & cache->share_mask;

	for(i = 0; i < num_cores; i ++)
	{
		if((bit_vector & 1) == 1)
			sharers++;

		bit_vector = bit_vector >> 1;
	}

	return sharers;
}

int cgm_cache_get_xown_core(struct cache_t *cache, int set, int way){

	int xowner = 0;
	int i = 0, j = 0;
	int num_cores = x86_cpu_num_cores;

	//cycles through the core and try to match the core id with the share

	for(i = 0; i < num_cores; i++)
	{
		if(cgm_cache_is_owning_core(cache, set, way, i ))
		{
			xowner = i;
			j++;
		}
	}

	//if j is greater than 1 the block is in more than one core; BAD!!!
	assert(j == 1);

	return xowner;

}

void cgm_cache_set_block_state(struct cache_t *cache, int set, int way, enum cgm_cache_block_state_t state){

	cache->sets[set].blocks[way].state = state;

	return;
}

enum cgm_cache_block_state_t cgm_cache_get_block_state(struct cache_t *cache, int set, int way){

	enum cgm_cache_block_state_t victim_state;

	victim_state = cache->sets[set].blocks[way].state;

	return victim_state;
}


void cgm_cache_set_block_transient_state(struct cache_t *cache, int set, int way, long long id, enum cgm_cache_block_state_t t_state){

	cache->sets[set].blocks[way].transient_state = t_state;

	if(id)
	{
		cache->sets[set].blocks[way].transient_access_id = id;
	}

	return;
}

enum cgm_cache_block_state_t cgm_cache_get_block_transient_state(struct cache_t *cache, int set, int way){

	enum cgm_cache_block_state_t t_state;

	t_state = cache->sets[set].blocks[way].transient_state;

	return t_state;
}

long long cgm_cache_get_block_transient_state_id(struct cache_t *cache, int set, int way){

	long long id;

	id = cache->sets[set].blocks[way].transient_access_id;

	assert(id > 0);

	return id;
}

enum cgm_access_kind_t cgm_cache_get_retry_state(enum cgm_access_kind_t r_state){

	enum cgm_access_kind_t retry_state;

	if(r_state == cgm_access_fetch)
	{
		retry_state = cgm_access_fetch_retry;
	}
	else if(r_state == cgm_access_load)
	{
		retry_state = cgm_access_load_retry;
	}
	else if(r_state == cgm_access_store)
	{
		retry_state = cgm_access_store_retry;
	}
	else
	{
		fatal("cgm_cache_get_retry_state(): unrecognized state\n");
	}

	return retry_state;
}

/*void cgm_cache_set_way(enum cache_type_enum cache_type, struct cgm_packet_t *message_packet){

		return;
}*/

