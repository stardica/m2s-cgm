/*
 * cache.c
 *
 *  Created on: Nov 25, 2014
 *      Author: stardica
 */


#include <cgm/cache.h>


struct str_map_t cgm_cache_block_state_map =
{ 	cgm_cache_block_state_num, 	{
		{ "Invalid", cgm_cache_block_invalid},
		{ "Non-Coherent", cgm_cache_block_noncoherent},
		{ "Modified", cgm_cache_block_modified },
		{ "Owned", cgm_cache_block_owned },
		{ "Exclusive", cgm_cache_block_exclusive },
		{ "Shared", cgm_cache_block_shared },
		{ "Transient", cgm_cache_block_transient },
		{ "Null", cgm_cache_block_null },
		}
};


struct str_map_t cgm_cache_policy_map =
{ cache_policy_num, 	{
		{ "INV", cache_policy_invalid },
		{ "LRU", cache_policy_lru },
		{ "FIFO", cache_policy_fifo },
		{ "Random", cache_policy_random },
		{ "FA" , cache_policy_first_available},
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

long long temp_id = 0;


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

	//note, use calloc because it initializes the contents of the caches

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

	int wb_q = 0;

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
	assert(write_back_queue_size <= QueueSize);


	/*if(write_back_queue_size >= (QueueSize + 1))
	{
		printf("%s check size %d cycle %llu\n", cache->name, write_back_queue_size, P_TIME);
	}*/

	/*if the ort or the coalescer are full we can't process a CPU request because a miss will overrun the table.*/

	//schedule write back if the wb queue is full.
	if(write_back_queue_size >= QueueSize)
	{
		new_message = list_get(cache->write_back_buffer, 0);

		//star todo check this
		if(new_message->flush_pending == 1)
			return NULL;

		cache->last_queue = cache->write_back_buffer;
		assert(new_message);
	}
	//schedule write back if the retry queue is empty and the bottom queue is empty and the cache has nothing else to do AND the wb isn't pending a flush.
	else if((ort_status == cache->mshr_size || ort_coalesce_size > cache->max_coal) && write_back_queue_size > 0)
	{
		new_message = list_get(cache->write_back_buffer, 0);

		//star todo check this
		if(new_message->flush_pending == 1)
			return NULL;

		cache->last_queue = cache->write_back_buffer;
		assert(new_message);
	}
	//pull from the retry queue if we have retry accesses waiting.
	else if((ort_status == cache->mshr_size || ort_coalesce_size > cache->max_coal) && retry_queue_size > 0)
	{
		new_message = list_get(cache->retry_queue, 0);
		cache->last_queue = cache->retry_queue;
		assert(new_message);
	}
	//pull from the coherence queue if there is a coherence message waiting.
	else if(coherence_queue_size > 0)
	{
		new_message = list_get(cache->Coherance_Rx_queue, 0);
		cache->last_queue = cache->Coherance_Rx_queue;
		assert(new_message);
	}
	//last pull from the bottom queue if no of the others have messages.
	else if ((ort_status == cache->mshr_size || ort_coalesce_size > cache->max_coal) && bottom_queue_size > 0)
	{
		new_message = list_get(cache->Rx_queue_bottom, 0);
		cache->last_queue = cache->Rx_queue_bottom;
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

struct cgm_packet_t *cache_search_wb(struct cache_t *cache, int tag, int set){

	int i = 0;
	int j = 0;
	struct cgm_packet_t *wb_packet = NULL;

	//error checking make sure there is only one matching wb in the buffer
	LIST_FOR_EACH(cache->write_back_buffer, i)
	{
		//get pointer to access in queue and check it's status.
		wb_packet = list_get(cache->write_back_buffer, i);

		//found block in write back buffer
		if(wb_packet->tag == tag && wb_packet->set == set)
		{
			j++;
		}
	}

	assert(j == 0 || j == 1);

	LIST_FOR_EACH(cache->write_back_buffer, i)
	{
		//get pointer to access in queue and check it's status.
		wb_packet = list_get(cache->write_back_buffer, i);

		//found block in write back buffer
		if(wb_packet->tag == tag && wb_packet->set == set)
		{
			assert(j == 1 && wb_packet->tag == tag && wb_packet->set == set);

			return wb_packet;
		}
	}

	return NULL;
}

void cgm_cache_insert_pending_request_buffer(struct cache_t *cache, struct cgm_packet_t *message_packet){

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->pending_request_buffer, message_packet);

	return;
}

struct cgm_packet_t *cache_search_pending_request_buffer(struct cache_t *cache, unsigned int address){

	int i = 0;
	struct cgm_packet_t *pending_request;

	LIST_FOR_EACH(cache->pending_request_buffer, i)
	{
		//get pointer to access in queue and check it's status.
		pending_request = list_get(cache->pending_request_buffer, i);

		if(pending_request->address == address)
		{
			return pending_request;
		}
	}

	return NULL;
}


/*int get_ort_num_coalesced(struct cache_t *cache, int entry, int tag, int set){

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
			size ++;
		}
	}

	return size;
}*/

void ort_set_row(struct cache_t *cache, int tag, int set){

	int i = 0;

	//find an empty row
	for (i = 0; i < cache->mshr_size; i++)
	{
		if(cache->ort[i][0] == -1 && cache->ort[i][1] == -1 && cache->ort[i][2] == -1)
		{
			//hit in the ORT table
			break;
		}
	}

	assert(i >= 0 && i <cache->mshr_size);

	//set the row
	ort_set(cache, i, tag, set);

	return;
}


void ort_get_row_sets_size(struct cache_t *cache, int tag, int set, int *hit_row_ptr, int *num_sets_ptr, int *ort_size_ptr){

	int i = 0;
	int j = 0;
	int k = 0;

	//first look for a matching tag and set
	for (i = 0; i < cache->mshr_size; i++)
	{
		if(cache->ort[i][0] == tag && cache->ort[i][1] == set && cache->ort[i][2] == 1)
		{
			//hit in the ORT table
			break;
		}
	}
	*(hit_row_ptr) = i;


	//next look for the number of outstanding set accesses for a given set
	for (i = 0; i < cache->mshr_size; i++)
	{
		if(cache->ort[i][1] == set)
		{
			//hit in the ORT table
			j ++;
		}
	}
	*(num_sets_ptr) = j;

	//get the size of the ORT
	for (i = 0; i < cache->mshr_size; i++)
	{
		if(cache->ort[i][0] != -1)
		{
			assert(cache->ort[i][0] != -1 && cache->ort[i][1] != -1 && cache->ort[i][2] != -1);
			k++;
		}
	}

	*ort_size_ptr = k;

	return;
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

	int map = -1;

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
	else
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
	*(tag_ptr) = (addr >> (cache->log_block_size + cache->log_set_size));//addr & ~(cache->block_mask);
	*(set_ptr) =  ((addr >> cache->log_block_size) & (cache->set_mask));//(addr >> cache->log_block_size) % cache->num_sets;
	*(offset_ptr) = addr & (cache->block_mask);

	unsigned int tag_size = 0xFFFFFFFF;
	tag_size = tag_size >> (cache->log_block_size + cache->log_set_size);

	assert(*(tag_ptr) >= 0 && *(tag_ptr) < tag_size);
	assert(*(set_ptr) >= 0 && *(set_ptr) < cache->num_sets);
	assert(*(offset_ptr) >=0 && *(offset_ptr) <= cache->block_mask);
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

	assert(way == cache->assoc);

	/* Block not found */
	return 0;
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

	/*if (cache->policy == cache_policy_fifo && cache->sets[set].blocks[way].tag != tag)
	{
		cgm_cache_update_waylist(&cache->sets[set], &cache->sets[set].blocks[way], cache_waylist_head);
	}*/

	cache->sets[set].blocks[way].tag = tag;
	cache->sets[set].blocks[way].state = state;
	cache->sets[set].blocks[way].transient_state = cgm_cache_block_invalid;
	return;
}

/*void cgm_cache_inval_block(struct cache_t *cache, int set, int way){

	enum cgm_cache_block_state_t victim_state;

	//get the victim's state
	victim_state = cgm_cache_get_block_state(cache, set, way);

	//if dirty data is found
	if (victim_state == cgm_cache_block_modified)
	{
		//move the block to the WB buffer
		struct cgm_packet_t *write_back_packet = packet_create();

		init_write_back_packet(cache, write_back_packet, set, way);
		write_back_packet->cache_block_state = victim_state;

		list_enqueue(cache->write_back_buffer, write_back_packet);
	}

	//set the block state to invalid
	cgm_cache_set_block_state(cache, set, way, cgm_cache_block_invalid);

	return;
}*/


void cgm_L1_cache_evict_block(struct cache_t *cache, int set, int way){

	assert(cache->cache_type == l1_d_cache_t);
	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);

	enum cgm_cache_block_state_t victim_state;

	//get the victim's state
	victim_state = cgm_cache_get_block_state(cache, set, way);

	//put the block in the write back buffer if in E/M states
	if (victim_state == cgm_cache_block_exclusive || victim_state == cgm_cache_block_modified)
	{
		//move the block to the WB buffer
		struct cgm_packet_t *write_back_packet = packet_create();

		init_write_back_packet(cache, write_back_packet, set, way, 0, victim_state);

		list_enqueue(cache->write_back_buffer, write_back_packet);
	}

	//set the local block state to invalid
	cgm_cache_set_block_state(cache, set, way, cgm_cache_block_invalid);

	return;
}


void cgm_L2_cache_evict_block(struct cache_t *cache, int set, int way){

	enum cgm_cache_block_state_t victim_state;
	assert(cache->cache_type == l2_cache_t);

	//get the victim's state
	victim_state = cgm_cache_get_block_state(cache, set, way);

	//if dirty data is found
	if (victim_state == cgm_cache_block_modified || victim_state == cgm_cache_block_exclusive)
	{
		//move the block to the WB buffer
		struct cgm_packet_t *write_back_packet = packet_create();

		init_write_back_packet(cache, write_back_packet, set, way, 0, victim_state);

		list_enqueue(cache->write_back_buffer, write_back_packet);
	}

	//send eviction notices and flush the L1 cache

	/*get the block flush_pending_bit
	if the pending bit is set a flush was previously sent*/
	/*assert(cgm_cache_get_block_flush_pending_bit(cache, set, way) == 0);*/
	if(cgm_cache_get_block_flush_pending_bit(cache, set, way) == 0)
	{
		//star todo account for block sizes if the L1 cache is 64 bytes and L2 is 128 L2 should send two invals
		struct cgm_packet_t *flush_packet = packet_create();

		init_flush_packet(cache, flush_packet, set, way);

		/*needed for correct routing from L2 to L1 D
		figure out another way to do this*/
		flush_packet->cpu_access_type = cgm_access_load;

		list_enqueue(cache->Tx_queue_top, flush_packet);
		advance(cache->cache_io_up_ec);
	}

	//set the block state to invalid
	cgm_cache_set_block_state(cache, set, way, cgm_cache_block_invalid);

	return;
}

void cgm_L3_cache_evict_block(struct cache_t *cache, int set, int way, int sharers){

	enum cgm_cache_block_state_t victim_state;
	int i = 0;
	unsigned char bit_vector;
	int num_cores = x86_cpu_num_cores;

	struct cgm_packet_t *write_back_packet = NULL;
	struct cgm_packet_t *flush_packet = NULL;

	assert(sharers >= 0 && sharers <= num_cores);
	assert(cache->cache_type == l3_cache_t);
	assert(cache->share_mask > 0);

	//get the victim's state
	victim_state = cgm_cache_get_block_state(cache, set, way);

	//if block is in the E/M state dirty data is found
	if (victim_state == cgm_cache_block_modified || victim_state == cgm_cache_block_exclusive)
	{
		//move the block to the WB buffer
		 write_back_packet = packet_create();

		init_write_back_packet(cache, write_back_packet, set, way, 0, victim_state);

		list_enqueue(cache->write_back_buffer, write_back_packet);
	}

	//send eviction notices
	/*star todo account for block sizes
	for example, if the L3 cache is 64 bytes and L2 is 128 L2 should send two evictions*/
	for(i = 0; i < num_cores; i++)
	{

		//get the presence bits from the directory
		bit_vector = cache->sets[set].blocks[way].directory_entry.entry;
		bit_vector = bit_vector & cache->share_mask;

		//for each core that has a copy of the cache block send the eviction
		if((bit_vector & 1) == 1)
		{
			flush_packet = packet_create();

			init_flush_packet(cache, flush_packet, set, way);

			flush_packet->cpu_access_type = cgm_access_store;

			//update routing
			flush_packet->dest_id = str_map_string(&node_strn_map, l2_caches[i].name);
			flush_packet->dest_name = str_map_value(&l2_strn_map, flush_packet->dest_id);
			flush_packet->src_name = cache->name;
			flush_packet->src_id = str_map_string(&node_strn_map, cache->name);

			list_enqueue(cache->Tx_queue_top, flush_packet);
			advance(cache->cache_io_up_ec);
		}

		//shift the vector to the next position and continue
		bit_vector = bit_vector >> 1;
	}

	//set the block state to invalid
	cgm_cache_set_block_state(cache, set, way, cgm_cache_block_invalid);

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

void cgm_cache_dump_set(struct cache_t *cache, int set){

	int i = 0;

	for(i=0; i < cache->assoc; i++)
	{
		printf("cache %s set %d way %d tag %d state %s t_state %s cycle %llu\n",
				cache->name, set, i, cache->sets[set].blocks[i].tag,
				str_map_value(&cgm_cache_block_state_map, cache->sets[set].blocks[i].state),
				str_map_value(&cgm_cache_block_state_map, cache->sets[set].blocks[i].transient_state), P_TIME);
	}

	return;
}

int cgm_cache_get_victim_for_wb(struct cache_t *cache, int set){

	int way = -1;
	int i = 0;

	assert(set >= 0 && set < cache->num_sets);

	for(i = 0; i < cache->assoc; i++)
	{
		assert(cache->sets[set].blocks[i].transient_state == cgm_cache_block_invalid || cache->sets[set].blocks[i].transient_state == cgm_cache_block_transient);

		if(cache->sets[set].blocks[i].transient_state == cgm_cache_block_invalid && (cgm_cache_get_dir_pending_bit(cache, set, i) == 0))
		{
			way = i;
			break;
		}
	}

	assert(way >= 0 && way <= cache->assoc);
	return way;
}


int cgm_cache_get_victim(struct cache_t *cache, int set){

	int way = -1;
	int i = 0;

	struct cache_block_t *block;

	assert(set >= 0 && set < cache->num_sets);

	if(cache->policy == cache_policy_first_available)
	{
		for(i = 0; i < cache->assoc; i++)
		{
			assert(cache->sets[set].blocks[i].transient_state == cgm_cache_block_invalid || cache->sets[set].blocks[i].transient_state == cgm_cache_block_transient);

			if(cache->sets[set].blocks[i].transient_state == cgm_cache_block_invalid && (cgm_cache_get_dir_pending_bit(cache, set, i) == 0))
			{
				way = i;
				cache->sets[set].blocks[i].transient_state = cgm_cache_block_transient;
				break;
			}
		}

		assert(way >= 0 && way < cache->assoc);
		return way;
	}
	else if(cache->policy == cache_policy_lru)
	{
		//get the tail block.
		block = cache->sets[set].way_tail;

		//the block should not be in the transient state.

		/*if(P_TIME == 13687603)
			cgm_cache_dump_set(cache, set);*/

		for(i = 0; i < cache->assoc; i++)
		{
			assert(block->transient_state == cgm_cache_block_invalid || block->transient_state == cgm_cache_block_transient);

			if(block->transient_state == cgm_cache_block_invalid && block->directory_entry.entry_bits.pending == 0)
			{
				block->transient_state = cgm_cache_block_transient;
				break;
			}

			if(block->way_prev == NULL)
				cgm_cache_dump_set(cache, set);

			assert(block->way_prev != NULL);
			block = block->way_prev;
		}


		assert(block->way >= 0 && block->way < cache->assoc);

		//set this block the MRU
		cgm_cache_update_waylist(&cache->sets[set], block, cache_waylist_head);


		return block->way;
	}
	else
	{
		fatal("cgm_cache_get_victim(): %s invalid cache eviction policy\n", cache->name);
	}

	return -1;
}

/* Return the way of the block to be replaced in a specific set,
 * depending on the replacement policy */
int cgm_cache_replace_block(struct cache_t *cache, int set)
{
	assert(set >= 0 && set < cache->num_sets);

	/* LRU and FIFO replacement: return block at the
	 * tail of the linked list */
	if (cache->policy == cache_policy_lru || cache->policy == cache_policy_fifo)
	{
		int way = cache->sets[set].way_tail->way;
		cgm_cache_update_waylist(&cache->sets[set], cache->sets[set].way_tail, cache_waylist_head);

		assert(way >= 0 && way <= cache->num_sets);
		return way;
	}

	fatal("cgm_cache_replace_block(): should not reach here\n");

	//star >> never enters here on LRU policy.
	/* Random replacement */
	assert(cache->policy == cache_policy_random);
	assert(0);
	return random() % cache->assoc;
}


void cgm_cache_access_block(struct cache_t *cache, int set, int way)
{
	/*int move_to_head;*/

	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);

	/* A block is moved to the head of the list for LRU policy.
	 * It will also be moved if it is its first access for FIFO policy, i.e., if the
	 * state of the block was invalid. */
	/*move_to_head = cache->policy == cache_policy_lru || (cache->policy == cache_policy_fifo && !cache->sets[set].blocks[way].state);*/
	if (/*move_to_head && */cache->sets[set].blocks[way].way_prev)
		cgm_cache_update_waylist(&cache->sets[set], &cache->sets[set].blocks[way], cache_waylist_head);
}

void cgm_cache_update_waylist(struct cache_set_t *set, struct cache_block_t *blk, enum cache_waylist_enum where){

	if (!blk->way_prev && !blk->way_next)
	{
		//star note: for associativity of 1 i.e. no other block in this set.
		assert(set->way_head == blk && set->way_tail == blk);
		return;
	}
	else if (!blk->way_prev)
	{
		//star note: case block is already at the head
		assert(set->way_head == blk && set->way_tail != blk);
		if (where == cache_waylist_head)
			return;
		set->way_head = blk->way_next;
		blk->way_next->way_prev = NULL;

	}
	else if (!blk->way_next)
	{
		//star note: case block is already at the tail
		assert(set->way_head != blk && set->way_tail == blk);
		if (where == cache_waylist_tail)
			return;
		set->way_tail = blk->way_prev;
		blk->way_prev->way_next = NULL;
	}
	else
	{
		//star note: case block is somewhere in between head and tail
		assert(set->way_head != blk && set->way_tail != blk);
		blk->way_prev->way_next = blk->way_next;
		blk->way_next->way_prev = blk->way_prev;
	}
	if (where == cache_waylist_head)
	{
		//star note: put the block at the head of the list
		blk->way_next = set->way_head;
		blk->way_prev = NULL;
		set->way_head->way_prev = blk;
		set->way_head = blk;
	}
	else
	{
		//star note: put the block at the tail of the list
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

	/*int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;*/

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

			/*printf("L1 I %d stalling\n", l1_i_caches[my_pid].id);*/

			P_PAUSE(1);
		}
		else
		{
			step++;

			/*printf("%s running\n", l1_i_caches[my_pid].name);*/

			/*printf("cpu fetch running cycle %llu\n", P_TIME);
			STOP;*/

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			///////////protocol v2
			if (access_type == cgm_access_fetch || access_type == cgm_access_fetch_retry)
			{
				//Call back function (cgm_mesi_fetch)
				l1_i_caches[my_pid].l1_i_fetch(&(l1_i_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_puts)
			{
				//Call back function (cgm_mesi_l1_i_write_block)
				l1_i_caches[my_pid].l1_i_write_block(&(l1_i_caches[my_pid]), message_packet);

				//entered retry state run again.
				step--;
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
	int num_cus = si_gpu_num_compute_units;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	/*struct cgm_packet_t *wb_packet;*/
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

		//star todo this can be refined a lot.
		if (message_packet == NULL || !cache_can_access_top(&l2_caches[my_pid]) || !cache_can_access_Tx_bottom(&(l1_d_caches[my_pid])))
		{
			//the cache state is preventing the cache from working this cycle stall.
			l1_d_caches[my_pid].stalls++;

			/*printf("%s stalling cycle %llu\n", l1_d_caches[my_pid].name, P_TIME);*/
			/*printf("%s stalling: l2 rx_t %d, rx_b %d, cx_b %d tx_b %d, ort size %d ORT coal size %d cycle %llu\n",
					l1_d_caches[my_pid].name, list_count(l2_caches[my_pid].Rx_queue_top),
					list_count(l1_d_caches[my_pid].Rx_queue_bottom),
					list_count(l1_d_caches[my_pid].Coherance_Rx_queue),
					list_count(l1_d_caches[my_pid].Tx_queue_bottom),
					cache_get_ORT_size(&(l1_d_caches[my_pid])),
					list_count(l1_d_caches[my_pid].ort_list),
					P_TIME);*/

			/*if(P_TIME > 326207)
			{
				STOP;
			}*/

			P_PAUSE(1);
		}
		else
		{
			step++;

			/*if(P_TIME > 1457968)
			{
				printf("%s id %llu type %d cycle %llu\n", l1_d_caches[my_pid].name, message_packet->access_id, message_packet->access_type, P_TIME);

				int i = 0;
				for(i=0; i<num_cus; i++)
				{
					printf("%s ort %d $id %d cycle %llu\n", gpu_s_caches[i].name, list_count(gpu_s_caches[i].ort_list), gpu_s_caches[i].id, P_TIME);
				}
			}*/

			/*printf("%s running\n", l1_d_caches[my_pid].name);*/

			/*if(message_packet->access_id == 87630)
			{
				printf("%s id %llu type %d cycle %llu\n", l1_d_caches[my_pid].name, message_packet->access_id, message_packet->access_type, P_TIME);

				printf("%s queue size %d, Tx bottom queue size %d, ort size %d  ORT coal size %d cycle %llu\n",
					l1_d_caches[my_pid].name, list_count(l1_d_caches[my_pid].Rx_queue_top),
					list_count(l1_d_caches[my_pid].Tx_queue_bottom),
					cache_get_ORT_size(&(l1_d_caches[my_pid])),
					list_count(l1_d_caches[my_pid].ort_list),
					P_TIME);

				getchar();
			}*/

			/*if(message_packet->access_id == 87630)
			{
				printf("%s id %llu type %d cycle %llu\n", l1_d_caches[my_pid].name, message_packet->access_id, message_packet->access_type, P_TIME);
				getchar();
			}*/

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if (access_type == cgm_access_load || access_type == cgm_access_load_retry)
			{
				//Call back function (cgm_mesi_load)
				l1_d_caches[my_pid].l1_d_load(&(l1_d_caches[my_pid]), message_packet);

			}
			else if(access_type == cgm_access_store || access_type == cgm_access_store_retry)
			{
				//Call back function (cgm_mesi_store)
				l1_d_caches[my_pid].l1_d_store(&(l1_d_caches[my_pid]), message_packet);

			}
			else if (access_type == cgm_access_puts || access_type == cgm_access_putx || access_type == cgm_access_put_clnx
					|| access_type == cgm_access_upgrade_putx)
			{
				//Call back function (cgm_mesi_l1_d_put)
				l1_d_caches[my_pid].l1_d_write_block(&(l1_d_caches[my_pid]), message_packet);

				//entered retry state run again.
				step--;

			}
			else if (access_type == cgm_access_write_back)
			{
				//Call back function (cgm_mesi_l1_d_write_back)
				l1_d_caches[my_pid].l1_d_write_back(&(l1_d_caches[my_pid]), message_packet);

				/*write backs are internally scheduled so decrement the counter*/
				step--;
			}
			else if (access_type == cgm_access_getx_fwd_inval)
			{
				//Call back function (cgm_mesi_l1_d_getx_fwd_inval)
				l1_d_caches[my_pid].l1_d_getx_fwd_inval(&(l1_d_caches[my_pid]), message_packet);


			}
			else if (access_type == cgm_access_upgrade_inval)
			{
				//Call back function (cgm_mesi_l1_d_upgrade_inval)
				l1_d_caches[my_pid].l1_d_upgrade_inval(&(l1_d_caches[my_pid]), message_packet);


			}
			else if (access_type == cgm_access_inv)
			{
				//Call back function (cgm_mesi_l1_d_inval)
				l1_d_caches[my_pid].l1_d_inval(&(l1_d_caches[my_pid]), message_packet);


			}
			else if (access_type == cgm_access_upgrade_ack)
			{
				//Call back function (cgm_mesi_l1_d_upgrade_ack)
				l1_d_caches[my_pid].l1_d_upgrade_ack(&(l1_d_caches[my_pid]), message_packet);

				//run again
				step--;
			}
			else if (access_type == cgm_access_downgrade)
			{
				//Call back function (cgm_mesi_l1_d_downgrade)
				l1_d_caches[my_pid].l1_d_downgrade(&(l1_d_caches[my_pid]), message_packet);
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

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

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
			/*printf("%s stalling\n", l2_caches[my_pid].name);*/
			l2_caches[my_pid].stalls++;
			/*printf("%s stalling: l2 in queue size %d, Tx bottom queue size %d, ORT size %d\n",
					l2_caches[my_pid].name, list_count(l2_caches[my_pid].Rx_queue_top), list_count(l2_caches[my_pid].Tx_queue_bottom), list_count(l2_caches[my_pid].ort_list));*/

			P_PAUSE(1);
		}
		else
		{
			step++;

			/*printf("%s running access id %llu type %d cycle %llu\n", l2_caches[my_pid].name, message_packet->access_id, message_packet->access_type, P_TIME);*/

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			/*printf("%s running id %llu type %s cycle %llu\n",
					l2_caches[my_pid].name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);*/

			if(access_type == cgm_access_gets || access_type == cgm_access_fetch_retry)
			{
				//Call back function (cgm_mesi_l2_gets)
				l2_caches[my_pid].l2_gets(&(l2_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_get || access_type == cgm_access_load_retry)
			{
				//Call back function (cgm_mesi_l2_get)
				l2_caches[my_pid].l2_get(&(l2_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_getx || access_type == cgm_access_store_retry)
			{
				//Call back function (cgm_mesi_l2_getx)

				//will run again if getx results in upgrade request at L2 level.
				if(!l2_caches[my_pid].l2_getx(&(l2_caches[my_pid]), message_packet))
					step--;
			}
			else if(access_type == cgm_access_write_back)
			{
				//Call back function (cgm_mesi_l2_write_back)

				//if the write back was internally scheduled decrement the counter.
				if(!l2_caches[my_pid].l2_write_back(&(l2_caches[my_pid]), message_packet))
					step--;
			}
			else if(access_type == cgm_access_puts || access_type == cgm_access_putx || access_type == cgm_access_put_clnx)
			{
				//Call back function (cgm_mesi_l2_put)
				l2_caches[my_pid].l2_write_block(&(l2_caches[my_pid]), message_packet);

				//run again
				step--;
			}
			else if(access_type == cgm_access_downgrade_ack)
			{
				//Call back function (cgm_mesi_l2_downgrade_ack)
				l2_caches[my_pid].l2_downgrade_ack(&(l2_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_get_nack)
			{
				//Call back function (cgm_mesi_l2_downgrade_ack)
				l2_caches[my_pid].l2_get_nack(&(l2_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_getx_nack)
			{
				//Call back function (cgm_mesi_l2_downgrade_ack)
				l2_caches[my_pid].l2_getx_nack(&(l2_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_get_fwd)
			{
				//Call back function (cgm_mesi_l2_get_fwd)
				l2_caches[my_pid].l2_get_fwd(&(l2_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_getx_fwd || message_packet->access_type == cgm_access_upgrade_getx_fwd)
			{
				//Call back function (cgm_mesi_l2_getx_fwd)
				l2_caches[my_pid].l2_getx_fwd(&(l2_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_getx_fwd_inval_ack)
			{
				//Call back function (cgm_mesi_l2_getx_fwd_inval_ack)
				l2_caches[my_pid].l2_getx_fwd_inval_ack(&(l2_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_upgrade)
			{
				//Call back function (cgm_mesi_l2_getx_fwd_inval_ack)
				if(!l2_caches[my_pid].l2_upgrade(&(l2_caches[my_pid]), message_packet))
					step--;
			}
			else if (access_type == cgm_access_upgrade_ack)
			{
				//Call back function (cgm_mesi_l2_upgrade_nack)
				l2_caches[my_pid].l2_upgrade_ack(&(l2_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_upgrade_nack)
			{
				//Call back function (cgm_mesi_l2_getx_fwd_inval_ack)
				l2_caches[my_pid].l2_upgrade_nack(&(l2_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_upgrade_putx_n)
			{
				//Call back function (cgm_mesi_l2_upgrade_putx_n)
				l2_caches[my_pid].l2_upgrade_putx_n(&(l2_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_upgrade_inval)
			{
				//Call back function (cgm_mesi_l2_upgrade_inval)
				l2_caches[my_pid].l2_upgrade_inval(&(l2_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_inv)
			{
						/*if(P_TIME == 224930)
						{
							printf("STOP\n");
							STOP;
						}*/

				//Call back function (cgm_mesi_l2_inval)
				l2_caches[my_pid].l2_inval(&(l2_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_inv_ack)
			{


				//Call back function (cgm_mesi_l2_inval_ack)
				l2_caches[my_pid].l2_inval_ack(&(l2_caches[my_pid]), message_packet);
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

	/*int cache_block_hit;
	int cache_block_state;*/
	/*int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;*/

	/*int dirty;
	int sharers;
	int owning_core;*/
	int i = 0;
	/*int flag = 0;*/

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	/*int temp;*/

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

			/*printf("%s stalling cycle %llu\n", l3_caches[my_pid].name, P_TIME);*/
			l3_caches[my_pid].stalls++;
			/*printf("L3 %d stalling\n", l3_caches[my_pid].id);*/

			P_PAUSE(1);
		}
		else
		{
			step++;

			/*if(P_TIME > 1457968)
			{
				printf("l3 running\n");
			}*/

			/*if(message_packet->access_id == 1215957)
			{
				fatal("%s id %llu type %d cycle %llu\n", l3_caches[my_pid].name, message_packet->access_id, message_packet->access_type, P_TIME);
			}*/

			/*printf("%s running\n", l3_caches[my_pid].name);*/

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			/*printf("%s running id %llu type %s cycle %llu\n",
					l3_caches[my_pid].name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);*/

			if(access_type == cgm_access_gets || access_type == cgm_access_fetch_retry)
			{
				//via call back function (cgm_mesi_l3_gets)
				l3_caches[my_pid].l3_gets(&(l3_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_get || access_type == cgm_access_load_retry)
			{
				//via call back function (cgm_mesi_l3_get)
				l3_caches[my_pid].l3_get(&(l3_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_getx || access_type == cgm_access_store_retry)
			{
				//via call back function (cgm_mesi_l3_getx)
				l3_caches[my_pid].l3_getx(&(l3_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_write_back)
			{
				//via call back function (cgm_mesi_l3_write_back)
				if(!l3_caches[my_pid].l3_write_back(&(l3_caches[my_pid]), message_packet))
					step--;
			}
			else if(access_type == cgm_access_downgrade_ack)
			{
				//via call back function (cgm_mesi_l3_downgrade_ack)
				l3_caches[my_pid].l3_downgrade_ack(&(l3_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_downgrade_nack)
			{
				//via call back function (cgm_mesi_l3_downgrade_nack)
				l3_caches[my_pid].l3_downgrade_nack(&(l3_caches[my_pid]), message_packet);

				//run again and pull the message_packet as a new access
				step--;
			}
			else if(access_type == cgm_access_getx_fwd_ack)
			{
				//via call back function (cgm_mesi_l3_getx_fwd_ack)
				l3_caches[my_pid].l3_getx_fwd_ack(&(l3_caches[my_pid]), message_packet);
			}
			else if(access_type == cgm_access_getx_fwd_nack)
			{
				//via call back function (cgm_mesi_l3_get_fwd_nack)
				l3_caches[my_pid].l3_getx_fwd_nack(&(l3_caches[my_pid]), message_packet);

				//run again and pull the message_packet as a new access
				step--;
			}
			else if(access_type == cgm_access_getx_fwd_upgrade_nack)
			{
				//via call back function (cgm_mesi_l3_get_fwd_nack)
				l3_caches[my_pid].l3_getx_fwd_upgrade_nack(&(l3_caches[my_pid]), message_packet);
				//run again and pull the message_packet as a new access
			}
			else if(access_type == cgm_access_get_fwd_upgrade_nack)
			{
				//via call back function (cgm_mesi_l3_get_fwd_nack)
				l3_caches[my_pid].l3_get_fwd_upgrade_nack(&(l3_caches[my_pid]), message_packet);
				//run again and pull the message_packet as a new access
			}
			else if(access_type == cgm_access_upgrade)
			{
				//via call back function (cgm_mesi_l3_upgrade)
				if(!l3_caches[my_pid].l3_upgrade(&(l3_caches[my_pid]), message_packet))
					step--;
			}

			else if (access_type == cgm_access_mc_put)
			{
				//via call back function (cgm_mesi_l3_write_block)
				l3_caches[my_pid].l3_write_block(&(l3_caches[my_pid]), message_packet);

				//retry state set so run again
				step--;
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
			//printf("%s stalling\n", gpu_s_caches[my_pid].name);

			P_PAUSE(1);
		}
		else
		{
			step++;

			/////////testing
			/*(*message_packet->witness_ptr)++;
			list_remove(gpu_s_caches[my_pid].last_queue, message_packet);
			packet_destroy(message_packet);
			continue;*/
			/////////testing

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if (access_type == cgm_access_load_s || access_type == cgm_access_load_retry)
			{
				//Call back function (cgm_nc_gpu_s_load)
				gpu_s_caches[my_pid].gpu_s_load(&(gpu_s_caches[my_pid]), message_packet);
			}
			//star todo this is wrong change this to put NOT puts
			else if (access_type == cgm_access_puts)
			{
				//Call back function (gpu_cache_access_put)
				gpu_s_caches[my_pid].gpu_s_write_block(&(gpu_s_caches[my_pid]), message_packet);

				//entered retry state run again.
				step--;
			}
			else
			{
				fatal("gpu_s_cache_ctrl(): access_id %llu bad access type %s at cycle %llu\n",
						access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}
	}

	//should never get here
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
			/*printf("%s stalling\n", gpu_v_caches[my_pid].name);*/
			P_PAUSE(1);
			//printf("%s stalling cycle %llu\n", gpu_v_caches[my_pid].name, P_TIME);
			//future_advance(&gpu_v_cache[my_pid], etime.count + 2);
		}
		else
		{
			step++;

			/////////testing
			/*(*message_packet->witness_ptr)++;
			list_remove(gpu_v_caches[my_pid].Rx_queue_top, message_packet);
			continue;*/
			/////////testing

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if(access_type == cgm_access_load_v || access_type == cgm_access_load_retry)
			{
				//Call back function (gpu_cache_access_load)
				gpu_v_caches[my_pid].gpu_v_load(&(gpu_v_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_store_v || access_type == cgm_access_nc_store || access_type == cgm_access_store_retry)
			{
				//Call back function (gpu_l1_cache_access_store)
				gpu_v_caches[my_pid].gpu_v_store(&(gpu_v_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_puts)
			{
				//Call back function (gpu_cache_access_put)
				gpu_v_caches[my_pid].gpu_v_write_block(&(gpu_v_caches[my_pid]), message_packet);

				//entered retry state run again.
				step--;
			}
			else
			{
				fatal("gpu_v_cache_ctrl(): access_id %llu bad access type %s at cycle %llu\n",
						access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}
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

		/*wait here until there is a job to do. */
		await(&gpu_l2_cache[my_pid], step);

		//check the top or bottom rx queues for messages.
		message_packet = cache_get_message(&(gpu_l2_caches[my_pid]));

		if (message_packet == NULL || !hub_iommu_can_access(hub_iommu->Rx_queue_top[my_pid]))
		{
			//the cache state is preventing the cache from working this cycle stall.
			//printf("%s stalling cycle %llu\n", gpu_l2_caches[my_pid].name, P_TIME);
			/*printf("stalling\n");*/
			/*future_advance(&gpu_l2_cache[my_pid], etime.count + 2);*/
			P_PAUSE(1);
		}
		else
		{

			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			/*if(P_TIME == 7530357)
			{
				printf("%s running id %llu type %s assoc %d cycle %llu\n",
						gpu_l2_caches[my_pid].name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), message_packet->assoc_conflict, P_TIME);

				cgm_cache_dump_set(&(gpu_l2_caches[my_pid]), 0);
				ort_dump(&(gpu_l2_caches[my_pid]));

			}*/

			if(access_type == cgm_access_gets_s || access_type == cgm_access_gets_v
					|| access_type == cgm_access_load_retry || access_type == cgm_access_store_retry)
			{
				//Call back function (gpu_cache_access_get)
				gpu_l2_caches[my_pid].gpu_l2_get(&gpu_l2_caches[my_pid], message_packet);
			}
			else if(access_type == cgm_access_mc_put)
			{
				//Call back function (gpu_cache_access_put)
				gpu_l2_caches[my_pid].gpu_l2_write_block(&gpu_l2_caches[my_pid], message_packet);

				//entered retry state run again.
				step--;
			}
			else
			{
				fatal("gpu_l2_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
					access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}
	}

	/*should never get here*/
	fatal("gpu_l2_cache_ctrl task is broken\n");
	return;
}

void l1_i_cache_down_io_ctrl(void){

	int my_pid = l1_i_io_pid++;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l1_i_caches[my_pid].cache_io_down_ec, step);
		step++;

		message_packet = list_dequeue(l1_i_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
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
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l1_d_caches[my_pid].cache_io_down_ec, step);
		step++;

		message_packet = list_dequeue(l1_d_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/

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
		else if(message_packet->access_type == cgm_access_upgrade || message_packet->access_type == cgm_access_inv_ack
				|| message_packet->access_type == cgm_access_downgrade_ack || message_packet->access_type == cgm_access_getx_fwd_inval_ack)
		{
			list_enqueue(l2_caches[my_pid].Coherance_Rx_queue, message_packet);
			advance(&l2_cache[my_pid]);
		}
		else
		{
			fatal("l1_d_cache_down_io_ctrl(): invalid access type\n");
		}
	}

	return;
}

void l2_cache_up_io_ctrl(void){

	int my_pid = l2_up_io_pid++;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
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

		/*access_id = message_packet->access_id;*/

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
			else if(message_packet->access_type == cgm_access_upgrade_ack || message_packet->access_type == cgm_access_inv
					|| message_packet->access_type == cgm_access_downgrade || message_packet->access_type == cgm_access_getx_fwd_inval
					|| message_packet->access_type == cgm_access_upgrade_inval)
			{
				list_enqueue(l1_d_caches[my_pid].Coherance_Rx_queue, message_packet);
				advance(&l1_d_cache[my_pid]);
			}
			else
			{
				fatal("l2_cache_up_io_ctrl(): bad access type\n");
			}
		}
		else
		{
			fatal("l2_cache_up_io_ctrl(): bad cpu access type\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
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
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l2_caches[my_pid].cache_io_down_ec, step);
		step++;

		message_packet = list_dequeue(l2_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/

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
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l3_caches[my_pid].cache_io_up_ec, step);
		step++;

		message_packet = list_dequeue(l3_caches[my_pid].Tx_queue_top);
		assert(message_packet);

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

void l3_cache_down_io_ctrl(void){

	int my_pid = l3_down_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l3_caches[my_pid].cache_io_down_ec, step);
		step++;

		message_packet = list_dequeue(l3_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/

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
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(gpu_s_caches[my_pid].cache_io_down_ec, step);
		step++;

		//printf("here\n");

		message_packet = list_dequeue(gpu_s_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
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
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(gpu_v_caches[my_pid].cache_io_down_ec, step);
		step++;

		/*printf("here\n");*/

		message_packet = list_dequeue(gpu_v_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
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
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(gpu_l2_caches[my_pid].cache_io_up_ec, step);
		step++;

		message_packet = list_dequeue(gpu_l2_caches[my_pid].Tx_queue_top);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
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
		else if(message_packet->gpu_access_type == cgm_access_load_v || message_packet->gpu_access_type == cgm_access_store_v
				|| message_packet->gpu_access_type == cgm_access_nc_store)
		{
			list_enqueue(gpu_v_caches[message_packet->gpu_cache_id].Rx_queue_bottom, message_packet);
			advance(&gpu_v_cache[message_packet->gpu_cache_id]);
		}
		else
		{
			fatal("gpu_l2_cache_up_io_ctrl(): bad gpu access type as %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->gpu_access_type));
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
	/*long long access_id = 0;*/
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(gpu_l2_caches[my_pid].cache_io_down_ec, step);
		step++;

		message_packet = list_dequeue(gpu_l2_caches[my_pid].Tx_queue_bottom);
		assert(message_packet);

		/*access_id = message_packet->access_id;*/
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

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(cache, message_packet->address, set_ptr, tag_ptr, offset_ptr);

	//lock for the block in the cache
	*(cache_block_hit_ptr) = cgm_cache_find_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, cache_block_state_ptr);

	//store the decode in the packet for now.
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;
	message_packet->way = way;

	return;
}

void cache_gpu_lds_return(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//clear the gpu uop witness_ptr
	(*message_packet->witness_ptr)++;
	message_packet = list_remove(cache->Rx_queue_top, message_packet);
	packet_destroy(message_packet);

}


void cache_gpu_v_return(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//remove packet from cache queue, global queue, and simulator memory
	(*message_packet->witness_ptr)++;
	message_packet = list_remove(cache->last_queue, message_packet);
	packet_destroy(message_packet);

	return;
}

void cache_gpu_s_return(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//remove packet from cache queue, global queue, and simulator memory

	/*if(cache->id == 0)
	{
		printf("%s access_id %llu finished cycle %llu \n", cache->name, message_packet->access_id, P_TIME);
	}*/

	(*message_packet->witness_ptr)++;
	message_packet = list_remove(cache->last_queue, message_packet);
	packet_destroy(message_packet);

	return;
}

void cache_l1_i_return(struct cache_t *cache, struct cgm_packet_t *message_packet){

	//debug
	/*CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu cleared from mem system\n", cache->name, message_packet->access_id, P_TIME);*/

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
	/*CGM_DEBUG(CPU_cache_debug_file, "%s access_id %llu cycle %llu cleared from mem system\n", cache->name, message_packet->access_id, P_TIME);*/

	/*if(message_packet->access_id == 1759)
	{*/
	/*if(message_packet->set == 62)
	{
		printf("%s id %llu type %d tag %d set %d FINISHED cycle %llu\n",
				cache->name, message_packet->access_id, message_packet->access_type, message_packet->tag, message_packet->set, P_TIME);
	}*/
	/*}*/

	//remove packet from cache queue, global queue, and simulator memory
	message_packet = list_remove(cache->last_queue, message_packet);
	linked_list_add(message_packet->event_queue, message_packet->data);

	/*printf("access_id %llu finished cycles %llu \n", message_packet->access_id, (message_packet->end_cycle - message_packet->start_cycle));*/

	packet_destroy(message_packet);
	return;
}

int cache_get_ORT_size(struct cache_t *cache){

	int size = 0;
	int i = 0;

	for (i = 0; i < cache->mshr_size; i++)
	{
		if(cache->ort[i][0] != -1)
		{
			assert(cache->ort[i][0] != -1 && cache->ort[i][1] != -1 && cache->ort[i][2] != -1);

			//hit in the ORT table
			size++;
		}
	}


	return size;
}

void cache_check_ORT(struct cache_t *cache, struct cgm_packet_t *message_packet){

	int hit_row = 0;
	int num_sets = 0;
	int ort_size = 0;

	int *hit_row_ptr = &hit_row;
	int *num_sets_ptr = &num_sets;
	int *ort_size_ptr = &ort_size;

	/*int i, row;*/

	//get the status of the ORT
	ort_get_row_sets_size(cache, message_packet->tag, message_packet->set, hit_row_ptr, num_sets_ptr, ort_size_ptr);

	//verify ort size
	assert(*ort_size_ptr < cache->mshr_size);


	if(message_packet->access_id == 7101547 && cache->cache_type == l2_cache_t)
	{
		cgm_cache_dump_set(cache, message_packet->set);
		printf("\n");
		ort_dump(cache);
		printf("id %llu type %d set %d tag %d way %d assoc_flag %d cycle %llu\n",
				message_packet->access_id, message_packet->access_type, message_packet->set,
				message_packet->tag, message_packet->way, message_packet->assoc_conflict, P_TIME);
	}


	if((*hit_row_ptr == cache->mshr_size && *num_sets_ptr < cache->assoc) || message_packet->assoc_conflict == 1)
	{
		//unique access and number of outstanding accesses are less than cache associativity
		//i.e. there IS a space in the cache's set and ways for the block on return
		assert(*hit_row_ptr >= 0 && *hit_row_ptr <= cache->mshr_size);

		//we are about to send the packet out, set the assoc conflict flag back to 0.
		if(message_packet->assoc_conflict == 1)
		{
			message_packet->assoc_conflict = 0;
		}

		ort_set_row(cache, message_packet->tag, message_packet->set);

		/*if(message_packet->access_id == 1623278 && cache->cache_type == gpu_l2_cache_t)
		{
			ort_dump(cache);
			cgm_cache_dump_set(cache, message_packet->set);
			fatal("ort pass id %llu assoc %d addr 0x%08x set %d tag %d *hit_row_ptr %d *num_sets_ptr %d cycle %llu\n",
					message_packet->access_id, message_packet->assoc_conflict, message_packet->address, message_packet->set, message_packet->tag, *hit_row_ptr, *num_sets_ptr, P_TIME);
		}*/
	}
	else if(*hit_row_ptr == cache->mshr_size && *num_sets_ptr >= cache->assoc)
	{
		//this is an associativity conflict
		//unique access, but number of outstanding accesses are greater than or equal to cache's number of ways
		//i.e. there IS NOT a space for the block in the cache set and ways on return

		//set the row in the ORT
		ort_set_row(cache, message_packet->tag, message_packet->set);

		message_packet->coalesced = 1;
		message_packet->assoc_conflict = 1;

		message_packet = list_remove(cache->last_queue, message_packet);
		list_enqueue(cache->ort_list, message_packet);

	}
	else if(*hit_row_ptr >= 0 && *hit_row_ptr < cache->mshr_size)
	{
		//non unique access that can be coalesced with another miss
		assert(cache->ort[*hit_row_ptr][0] == message_packet->tag && cache->ort[*hit_row_ptr][1] == message_packet->set && cache->ort[*hit_row_ptr][2] == 1);

		message_packet->coalesced = 1;

		if(message_packet->assoc_conflict == 1)
		{
			fatal("mp assoc conflict check this, may need to set assoc_conflict to 0\n");
		}

		message_packet = list_remove(cache->last_queue, message_packet);
		list_enqueue(cache->ort_list, message_packet);
	}
	else
	{
		fatal("cache_check_ORT(): %s invalid ORT status\n", cache->name);
	}

	return;
}

void cache_put_io_up_queue(struct cache_t *cache, struct cgm_packet_t *message_packet){

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->Tx_queue_top, message_packet);
	advance(cache->cache_io_up_ec);
	return;
}

void cache_put_io_down_queue(struct cache_t *cache, struct cgm_packet_t *message_packet){

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->Tx_queue_bottom, message_packet);
	advance(cache->cache_io_down_ec);
	return;
}

void cache_put_block(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*enum cgm_cache_block_state_t victim_state;*/

	assert(cache->cache_type != l1_i_cache_t);

	//checks
	if(message_packet->access_type == cgm_access_put_clnx)
	{
		assert(message_packet->cache_block_state == cgm_cache_block_exclusive);
	}
	else if(message_packet->access_type == cgm_access_putx)
	{
		if(message_packet->cache_block_state != cgm_cache_block_modified)
		{
			printf("fatal: cache name %s bad block state\n", cache->name);
			assert(message_packet->cache_block_state == cgm_cache_block_modified);
		}
	}
	else if(message_packet->access_type == cgm_access_puts)
	{
		//star todo check this, PUTS is returning an exclusive block something is wrong with the I$ addresses
		assert(message_packet->cache_block_state == cgm_cache_block_shared || message_packet->cache_block_state == cgm_cache_block_exclusive);
	}

	//put the block
	if(cache->cache_type == l3_cache_t)
	{
		//L3 should only ever put exclusive or shared blocks.
		assert(message_packet->cache_block_state == cgm_cache_block_exclusive || message_packet->cache_block_state == cgm_cache_block_shared);

		//set the block data
		cgm_cache_set_block(cache, message_packet->set, message_packet->l3_victim_way, message_packet->tag, message_packet->cache_block_state);
	}
	else if(cache->cache_type == l2_cache_t)
	{
		cgm_cache_set_block(cache, message_packet->set, message_packet->l2_victim_way, message_packet->tag, message_packet->cache_block_state);
	}
	else if(cache->cache_type == l1_d_cache_t || cache->cache_type == l1_i_cache_t)
	{
		cgm_cache_set_block(cache, message_packet->set, message_packet->l1_victim_way, message_packet->tag, message_packet->cache_block_state);
	}
	else
	{
		fatal("cache_put_block(): bad cache type cycle %llu\n", P_TIME);
	}

	//set retry state
	message_packet->access_type = cgm_cache_get_retry_state(message_packet->cpu_access_type);

	message_packet = list_remove(cache->last_queue, message_packet);
	list_enqueue(cache->retry_queue, message_packet);
	advance(cache->ec_ptr);

	return;
}

void cache_coalesed_retry(struct cache_t *cache, int tag, int set){

	struct cgm_packet_t *ort_packet;
	int i = 0;

	//first look for merged accesses
	LIST_FOR_EACH(cache->ort_list, i)
	{
		//get pointer to access in queue and check it's status.
		ort_packet = list_get(cache->ort_list, i);

		if(ort_packet->tag == tag && ort_packet->set == set)
		{
			ort_packet = list_remove_at(cache->ort_list, i);

			ort_packet->access_type = cgm_cache_get_retry_state(ort_packet->cpu_access_type);

			if(((ort_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
			{
				printf("block 0x%08x %s ort pull ID %llu type %d state %d cycle %llu\n",
					(ort_packet->address & cache->block_address_mask), cache->name, ort_packet->access_id, ort_packet->access_type, ort_packet->cache_block_state, P_TIME);
			}

			list_enqueue(cache->retry_queue, ort_packet);
			advance(cache->ec_ptr);

			/*this may cause problems the intent is to run one coalesced
			packet per iteration of the retry state so the timing is correctly charged*/
			return;
		}
	}

	//no coalesced packets remaining now check for packets with cache assoc conflicts
	LIST_FOR_EACH(cache->ort_list, i)
	{
		//get pointer to access in queue and check it's status.
		ort_packet = list_get(cache->ort_list, i);

		if(ort_packet->set == set && ort_packet->assoc_conflict == 1)
		{
			//clear the ORT entry for the assoc miss
			ort_clear(cache, ort_packet);

			ort_packet = list_remove_at(cache->ort_list, i);

			/*if(ort_packet->access_id == 90960)
			{
				printf("pulled from ort_assoc\n");
				getchar();
			}*/

			/*retry the access and it will be a hit then cause
			coalescer to re-enter the set and tag.*/
			ort_packet->coalesced = 0;
			ort_packet->assoc_conflict = 1;
			ort_packet->access_type = cgm_cache_get_retry_state(ort_packet->cpu_access_type);

			if(((ort_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
			{
				printf("block 0x%08x %s ort pull ID %llu type %d state %d cycle %llu\n",
					(ort_packet->address & cache->block_address_mask), cache->name, ort_packet->access_id, ort_packet->access_type, ort_packet->cache_block_state, P_TIME);
			}

			/*if(ort_packet->access_id == 91067)
			{
				fatal("ort pulled 91067\n");

			}*/

			list_enqueue(cache->retry_queue, ort_packet);
			advance(cache->ec_ptr);

			return;
		}
	}

	return;
}

void gpu_cache_coalesed_retry(struct cache_t *cache, int tag, int set){

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
			ort_packet->access_type = cgm_gpu_cache_get_retry_state(ort_packet->gpu_access_type);

			/*if(cache->id == 0)
			{
				printf("%s ort pull access id %llu cycle %llu\n", cache->name, ort_packet->access_id, P_TIME);
			}*/

			list_enqueue(cache->retry_queue, ort_packet);
			advance(cache->ec_ptr);

			/*this may cause problems the intent is to run one coalesced
			packet per iteration of the retry state so the timing is correctly charged*/
			return;
		}
	}

	//no coalesced packets remaining now check for packets with cache assoc conflicts
	LIST_FOR_EACH(cache->ort_list, i)
	{
		//get pointer to access in queue and check it's status.
		ort_packet = list_get(cache->ort_list, i);

		if(ort_packet->set == set && ort_packet->assoc_conflict == 1)
		{
			//clear the ORT entry for the assoc miss
			ort_clear(cache, ort_packet);

			//fatal("GPU assoc conflict this is just a check ok to delete\n");

			ort_packet = list_remove_at(cache->ort_list, i);

			/*retry the access and it will be a hit then cause
			coalescer to re-enter the set and tag.*/
			ort_packet->coalesced = 0;
			ort_packet->assoc_conflict = 1;
			ort_packet->access_type = cgm_gpu_cache_get_retry_state(ort_packet->gpu_access_type);

			if(((ort_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
			{
				printf("block 0x%08x %s ort pull ID %llu type %d state %d cycle %llu\n",
					(ort_packet->address & cache->block_address_mask), cache->name, ort_packet->access_id, ort_packet->access_type, ort_packet->cache_block_state, P_TIME);
			}

			list_enqueue(cache->retry_queue, ort_packet);
			advance(cache->ec_ptr);

			return;
		}
	}


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

void cgm_cache_set_dir_pending_bit(struct cache_t *cache, int set, int way){

	cache->sets[set].blocks[way].directory_entry.entry_bits.pending = 1;

	return;

}

int cgm_cache_get_dir_pending_bit(struct cache_t *cache, int set, int way){

	int pending_bit = 0;

	pending_bit = cache->sets[set].blocks[way].directory_entry.entry_bits.pending;

	return pending_bit;
}

void cgm_cache_clear_dir_pending_bit(struct cache_t *cache, int set, int way){

	cache->sets[set].blocks[way].directory_entry.entry_bits.pending = 0;

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

/*int cgm_cache_get_sown_core(struct cache_t *cache, int set, int way){

	//cycles through the cores and try to match the core id with the share



	return;
}*/

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
	assert(xowner >= 0 && xowner < num_cores);

	return xowner;

}

void cgm_cache_set_block_state(struct cache_t *cache, int set, int way, enum cgm_cache_block_state_t state){

	cache->sets[set].blocks[way].state = state;

	return;
}

/*enum cgm_cache_block_state_t cgm_cache_get_block_state(struct cache_t *cache, int set, int way){

	enum cgm_cache_block_state_t block_state;

	block_state = cache->sets[set].blocks[way].state;

	return block_state;
}*/

void cgm_cache_set_block_flush_pending_bit(struct cache_t *cache, int set, int way){

	//set the bit
	cache->sets[set].blocks[way].flush_pending = 1;

	return;
}

void cgm_cache_set_block_upgrade_pending_bit(struct cache_t *cache, int set, int way){

	//set the bit
	cache->sets[set].blocks[way].upgrade_pending = 1;

	return;
}

void cgm_cache_clear_block_upgrade_pending_bit(struct cache_t *cache, int set, int way){

	//set the bit
	cache->sets[set].blocks[way].upgrade_pending = 0;

	return;

}

void cgm_cache_clear_block_flush_pending_bit(struct cache_t *cache, int set, int way){

	//clear the bit
	cache->sets[set].blocks[way].flush_pending = 0;

	return;
}

int cgm_cache_get_block_flush_pending_bit(struct cache_t *cache, int set, int way){

	int bit;

	bit = cache->sets[set].blocks[way].flush_pending;

	//bit should be between 0 and 1;
	assert(bit >= 0 && bit <= 1);

	return bit;
}



enum cgm_cache_block_state_t cgm_cache_get_block_state(struct cache_t *cache, int set, int way){

	enum cgm_cache_block_state_t victim_state;

	victim_state = cache->sets[set].blocks[way].state;

	return victim_state;
}


void cgm_cache_set_block_transient_state(struct cache_t *cache, int set, int way, enum cgm_cache_block_state_t t_state){

	cache->sets[set].blocks[way].transient_state = t_state;

	if(cache->cache_type == l2_cache_t && set == 172 && t_state == cgm_cache_block_transient)
	{
		cgm_cache_dump_set(cache, set);

		printf("setting t state set %d way %d\n", set, way);

	}
	else if(cache->cache_type == l2_cache_t && set == 172 && t_state == cgm_cache_block_invalid)
	{
		printf("clear t state set %d way %d\n", set, way);
	}

	return;
}

enum cgm_cache_block_state_t cgm_cache_get_block_transient_state(struct cache_t *cache, int set, int way){

	enum cgm_cache_block_state_t t_state;

	t_state = cache->sets[set].blocks[way].transient_state;

	return t_state;
}
void cgm_cache_set_block_address(struct cache_t *cache, int set, int way, unsigned int address){

	cache->sets[set].blocks[way].address = address;

	return;
}

long long cgm_cache_get_block_transient_state_id(struct cache_t *cache, int set, int way){

	long long id;

	id = cache->sets[set].blocks[way].transient_access_id;

	assert(id > 0);

	return id;
}

enum cgm_access_kind_t cgm_gpu_cache_get_retry_state(enum cgm_access_kind_t r_state){

	enum cgm_access_kind_t retry_state = cgm_access_invalid;

	if(r_state == cgm_access_load_s || r_state == cgm_access_load_v)
	{
		retry_state = cgm_access_load_retry;
	}
	else if(r_state == cgm_access_store_v || r_state == cgm_access_nc_store)
	{
		retry_state = cgm_access_store_retry;
	}
	else
	{
		fatal("cgm_cache_get_retry_state(): unrecognized state as %s\n", str_map_value(&cgm_mem_access_strn_map, r_state));
	}

	return retry_state;
}

enum cgm_access_kind_t cgm_cache_get_retry_state(enum cgm_access_kind_t r_state){

	enum cgm_access_kind_t retry_state = cgm_access_invalid;

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
