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

enum scheduler_state_t{

	schedule_cant_process = 0,
	schedule_can_process,
	schedule_stall,
};


struct cgm_packet_t *cache_get_message(struct cache_t *cache){

	struct cgm_packet_t *new_message = NULL;
	enum scheduler_state_t state = schedule_can_process; //priority goes to processor

	/*Ok, the scheduler is a little complicated, i'll add notes as I go...*/

	/*first get the status of the cache's various internal elements*/
	int ort_status = get_ort_status(cache);
	int ort_coalesce_size = list_count(cache->ort_list);
	int rx_top_queue_size = list_count(cache->Rx_queue_top);
	int rx_bottom_queue_size = list_count(cache->Rx_queue_bottom);
	//look below for tx_top_queue
	int tx_bottom_queue_size = list_count(cache->Tx_queue_bottom);
	int retry_queue_size = list_count(cache->retry_queue);
	int pending_queue_size = list_count(cache->pending_request_buffer);
	int write_back_queue_size = list_count(cache->write_back_buffer);
	int coherence_queue_size = list_count(cache->Coherance_Rx_queue);

	//queues should never exceed their max sizes.
	//assert(ort_status <= cache->mshr_size);
	//assert(ort_coalesce_size <= (cache->max_coal + 1));
	//assert(rx_bottom_queue_size <= QueueSize);
	//assert(tx_bottom_queue_size <= QueueSize);
	//assert(retry_queue_size <= QueueSize);
	//assert(pending_queue_size <= QueueSize);
	//assert(write_back_queue_size <= QueueSize);
	//assert(coherence_queue_size <= QueueSize);

	/*printf warnings if a size is exceeded debugging*/

	if(ort_status > cache->mshr_size)
		warning("cache_get_message(): %s MSHR table exceeded size 16 cycle %llu\n", cache->name, P_TIME);

	if(ort_coalesce_size > (cache->max_coal + 1))
		warning("cache_get_message(): %s %s exceeded size %d cycle %llu\n", cache->name, cache->ort_list->name, list_count(cache->ort_list), P_TIME);

	if(cache->cache_type == l1_i_cache_t || cache->cache_type == l1_d_cache_t || cache->cache_type == l2_cache_t || cache->cache_type == l3_cache_t)
	{
		//assert(rx_top_queue_size <= QueueSize);
		if(rx_top_queue_size > QueueSize)
		warning("cache_get_message(): %s %s exceeded size %d cycle %llu\n", cache->name, cache->Rx_queue_top->name, list_count(cache->Rx_queue_top), P_TIME);
	}

	if(rx_bottom_queue_size > QueueSize)
		warning("cache_get_message(): %s %s exceeded size %d cycle %llu\n", cache->name, cache->Rx_queue_bottom->name, list_count(cache->Rx_queue_bottom), P_TIME);

	if(tx_bottom_queue_size > QueueSize)
		warning("cache_get_message(): %s %s exceeded size %d cycle %llu\n", cache->name, cache->Tx_queue_bottom->name, list_count(cache->Tx_queue_bottom), P_TIME);

	if(retry_queue_size > QueueSize)
		warning("cache_get_message(): %s %s exceeded size %d cycle %llu\n", cache->name, cache->retry_queue->name, list_count(cache->retry_queue), P_TIME);

	if(pending_queue_size > QueueSize)
		warning("cache_get_message(): %s %s exceeded size %d cycle %llu\n", cache->name, cache->pending_request_buffer->name, list_count(cache->pending_request_buffer), P_TIME);

	if(write_back_queue_size > QueueSize)
	{
		warning("cache_get_message(): %s %s exceeded size %d cycle %llu\n", cache->name, cache->write_back_buffer->name, list_count(cache->write_back_buffer), P_TIME);

	}

	if(coherence_queue_size > QueueSize)
		warning("cache_get_message(): %s %s exceeded size %d cycle %llu\n", cache->name, cache->Coherance_Rx_queue->name, list_count(cache->Coherance_Rx_queue), P_TIME);

	/*if(cache->cache_type == gpu_s_cache_t || cache->cache_type == gpu_v_cache_t || cache->cache_type == gpu_l2_cache_t)
	assert(rx_top_queue_size <= 64);*/

	/*check if a cache element is full that would prevent us from processing a request*/
	if(ort_status >= cache->mshr_size)
		state = schedule_cant_process;

	if(ort_coalesce_size >= cache->max_coal)
		state = schedule_cant_process;

	if(write_back_queue_size >= QueueSize)
		state = schedule_cant_process;

	if(retry_queue_size >= QueueSize)
		state = schedule_cant_process;

	//star todo change this  if l1 and l2 top queue is full stall, also if l2 and L3 top queue is full stall.
	/*if(cache->cache_type == l2_cache_t || cache->cache_type == l3_cache_t)
	{
		int tx_top_queue_size = list_count(cache->Tx_queue_top);
		assert(tx_top_queue_size <= QueueSize);

		if(tx_top_queue_size == QueueSize)
			state = schedule_cant_process;
	}*/

	if(tx_bottom_queue_size >= QueueSize)
		state = schedule_cant_process;

	if(pending_queue_size >= QueueSize)
		state = schedule_cant_process;


	/*the scheduler needs to determine which queue to pull from or if a stall is needed*/
	/*if the state is still "can process" then check queue statuses and schedule a packet*/
	if(state == schedule_can_process)
	{
		if(retry_queue_size > 0)
		{
			/*pull from the retry queue if there is a packet in retry*/
			new_message = list_get(cache->retry_queue, 0);
			cache->last_queue = cache->retry_queue;
			assert(new_message);
		}
		else if(rx_bottom_queue_size > 0)
		{
			/* if no CPU packet pull from the memory system side*/
			new_message = list_get(cache->Rx_queue_bottom, 0);

			//keep pointer to last queue
			cache->last_queue = cache->Rx_queue_bottom;
			assert(new_message);
		}
		else if (coherence_queue_size > 0)
		{
			new_message = list_get(cache->Coherance_Rx_queue, 0);
			cache->last_queue = cache->Coherance_Rx_queue;
			assert(new_message);
		}
		else if(rx_top_queue_size > 0)
		{
			/*pull from the cpu side*/
			new_message = list_get(cache->Rx_queue_top, 0);

			//keep pointer to last queue
			cache->last_queue = cache->Rx_queue_top;
			assert(new_message);
		}
		else if(write_back_queue_size > 0)
		{
			new_message = cache_search_wb_not_pending_flush(cache);

			/*if all write backs are pending we can't do anything with the write backs*/
			if(!new_message)
			{
				state = schedule_stall;
			}
			else
			{
				cache->last_queue = cache->write_back_buffer;
				assert(new_message);
			}
		}
		else
		{
			fatal("cache_get_message(): %s can process, but didn't find a packet\n", cache->name);
		}

	}
	else
	{
		/*there is an element in the cache that is full, so we can't process a request wait for replies or do something else*/
		assert(state == schedule_cant_process);

		/*if the write back queue has a write back clear a write back*/
		new_message = cache_search_wb_not_pending_flush(cache);

		if(retry_queue_size > 0)
		{
			//pull from the retry queue if we have retry accesses waiting.
			new_message = list_get(cache->retry_queue, 0);
			cache->last_queue = cache->retry_queue;

			if(new_message->access_id == 84000950 || new_message->access_id == 84000974)
				warning("pulling id %llu from %s cycle %llu\n", new_message->access_id, cache->retry_queue->name, P_TIME);

			assert(new_message);
		}
		else if(write_back_queue_size > 0 && new_message)
		{
			cache->last_queue = cache->write_back_buffer;
			assert(new_message);
		}
		else if (rx_bottom_queue_size > 0)
		{
			new_message = list_get(cache->Rx_queue_bottom, 0);
			cache->last_queue = cache->Rx_queue_bottom;

			if(new_message->access_id == 84000950 || new_message->access_id == 84000974)
				warning("pulling id %llu from %s cycle %llu\n", new_message->access_id, cache->Rx_queue_bottom->name, P_TIME);


			assert(new_message);
		}
		else if (coherence_queue_size > 0)
		{
			new_message = list_get(cache->Coherance_Rx_queue, 0);
			cache->last_queue = cache->Coherance_Rx_queue;
			assert(new_message);
		}
		else
		{
			state = schedule_stall;
		}

	}

	return (state == schedule_stall) ? NULL : new_message;


	/*if the ort or the coalescer are full we can't process a CPU request because a miss will overrun the table.*/

	//schedule write back if the wb queue is full.
	/*if(write_back_queue_size >= QueueSize)
	{
		new_message = cache_search_wb_not_pending_flush(cache);

		//star todo check this
		if(new_message == NULL)
			return NULL;

		cache->last_queue = cache->write_back_buffer;
		assert(new_message);
	}
	//schedule write back if the retry queue is empty and the bottom queue is empty and the cache has nothing else to do AND the wb isn't pending a flush.
	else if((ort_status == cache->mshr_size || ort_coalesce_size > cache->max_coal) && write_back_queue_size > 0)
	{
		new_message = cache_search_wb_not_pending_flush(cache);

		//star todo check this
		if(new_message == NULL)
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
	else if(coherence_queue_size > 0 && retry_queue_size == 0)
	{
		new_message = list_get(cache->Coherance_Rx_queue, 0);
		cache->last_queue = cache->Coherance_Rx_queue;
		assert(new_message);
	}
	//last pull from the bottom queue if none of the others have messages.
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
	}*/

	//if we made it here we better have a message.
	/*assert(new_message);
	return new_message;*/
}

int get_ort_status(struct cache_t *cache){

	int i = 0;

	// checks the ort to find an empty row
	for (i = 0; i < cache->mshr_size; i++)
	{
		if(cache->ort[i][0] == -1 && cache->ort[i][1] == -1)
		{
			//hit in the ORT table
			break;
		}
	}
	return i;
}
int cache_search_wb_dup_packets(struct cache_t *cache, int tag, int set){

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

	return j;
}

void cache_dump_queue(struct list_t *queue){

	int i = 0;
	struct cgm_packet_t *packet = NULL;

	LIST_FOR_EACH(queue, i)
	{
		//get pointer to access in queue and check it's status.
		packet = list_get(queue, i);
		printf("\t %s slot %d packet id %llu access type %s addr 0x%08x blk addr 0x%08x pending_bit %d upgrade bit %d start_cycle %llu\n",
				queue->name, i, packet->access_id, str_map_value(&cgm_mem_access_strn_map, packet->access_type), packet->address,
				(packet->address & l3_caches[0].block_address_mask), packet->flush_pending, packet->upgrade_pending, packet->start_cycle);
	}

	return;
}


void cache_dump_write_back(struct cache_t *cache){

	int i = 0;
	struct cgm_packet_t *wb_packet = NULL;

	LIST_FOR_EACH(cache->write_back_buffer, i)
	{
		//get pointer to access in queue and check it's status.
		wb_packet = list_get(cache->write_back_buffer, i);
		printf("\tslot %d wb id %llu flush bit %d blk addr 0x%08x\n", i, wb_packet->write_back_id, wb_packet->flush_pending, wb_packet->address);
	}

	return;
}


struct cgm_packet_t *cache_search_wb_not_pending_flush(struct cache_t *cache){

	int i = 0;
	int j = 0;
	struct cgm_packet_t *wb_packet = NULL;


	LIST_FOR_EACH(cache->write_back_buffer, i)
	{
		//get pointer to access in queue and check it's status.
		wb_packet = list_get(cache->write_back_buffer, i);

		//found block in write back buffer
		if(wb_packet->flush_pending == 0)
		{
			return wb_packet;
		}
	}

	return NULL;
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

int cache_search_pending_request_get_getx_fwd(struct cache_t *cache, unsigned int address){

	int i = 0;
	int hit = 0;
	struct cgm_packet_t *pending_request;

	LIST_FOR_EACH(cache->pending_request_buffer, i)
	{
		//get pointer to access in queue and check it's status.
		pending_request = list_get(cache->pending_request_buffer, i);

		if((pending_request->address & cache->block_address_mask) == address && (pending_request->access_type == cgm_access_get_fwd || pending_request->access_type == cgm_access_getx_fwd))
		{
			hit++;
		}
	}

	return hit;
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

	/*star todo, at this point the code is a mess, we need to come back and clean everything up when the protocol is finished*/
	/*if you don't find the block try looking at the block level*/
	LIST_FOR_EACH(cache->pending_request_buffer, i)
	{
		//get pointer to access in queue and check it's status.
		pending_request = list_get(cache->pending_request_buffer, i);

		if((pending_request->address & cache->block_address_mask) == address)
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

int ort_get_num_coal(struct cache_t *cache, int tag, int set){

	struct cgm_packet_t *ort_packet;
	int i = 0;
	int num_coalesced = 0;

	//first look for merged accesses
	LIST_FOR_EACH(cache->ort_list, i)
	{
		//get pointer to access in queue and check it's status.
		ort_packet = list_get(cache->ort_list, i);

		if(ort_packet->tag == tag && ort_packet->set == set)
		{
			num_coalesced ++;
		}
	}

	return num_coalesced;
}


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
		if(cache->ort[i][0] == tag && cache->ort[i][1] == set)
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

void ort_clear_pending_join_bit(struct cache_t *cache, int row, int tag, int set){

	assert(cache->ort[row][0] == tag && cache->ort[row][1] == set);

	cache->ort[row][2] = 1;

	return;
}


int ort_get_pending_join_bit(struct cache_t *cache, int row, int tag, int set){

	int status = 0;

	assert(cache->ort[row][0] == tag && cache->ort[row][1] == set);

	status = cache->ort[row][2];

	return status;
}

void ort_set_pending_join_bit(struct cache_t *cache, int row, int tag, int set){

	assert(cache->ort[row][0] == tag && cache->ort[row][1] == set);

	cache->ort[row][2] = 0;

	return;
}

int ort_search(struct cache_t *cache, int tag, int set){

	int i = 0;

	for (i = 0; i < cache->mshr_size; i++)
	{
		if(cache->ort[i][0] == tag && cache->ort[i][1] == set)
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
	if(row >= cache->mshr_size)
	{
		ort_dump(cache);
		printf("ort_clear(): row >= cache->mshr_size %s access id %llu block 0x%08x cycle %llu\n",
				cache->name, message_packet->access_id, (message_packet->address & cache->block_address_mask), P_TIME);

		assert(row < cache->mshr_size);
	}

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

void ort_set_valid_bit(struct cache_t *cache,int row){

	cache->ort[row][2] = 0;

	return;
}


void ort_dump(struct cache_t *cache){

	int i = 0;

	for (i = 0; i <  cache->mshr_size; i++)
	{
		printf("\tort row %d tag %d set %d pending action %d\n", i, cache->ort[i][0], cache->ort[i][1], cache->ort[i][2]);
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


struct cache_t *cgm_l3_cache_map(int set){

	int num_cores = x86_cpu_num_cores;
	int map = -1;

	int map_type = l3_caches[0].slice_type;

	if(map_type == 0)
	{
		//only 1 L3 cache.
		map = 0;
	}
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
	return &l3_caches[map];
}

/*int cgm_gpu_cache_map(int cache_id){*/
int cgm_gpu_cache_map(struct cache_t *cache, unsigned int addr){

	int num_cus = si_gpu_num_compute_units;
	int gpu_group_cache_num = (num_cus/4);
	int map = -1;

	unsigned int set = addr & cache->block_address_mask;
	set = set >> cache->log_block_size;

	map = set & (unsigned int) (gpu_group_cache_num - 1);

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

void cgm_cache_print_set_tag(struct cache_t *cache, unsigned int addr){

	int tag = (addr >> (cache->log_block_size + cache->log_set_size));
	int set = ((addr >> cache->log_block_size) & (cache->set_mask));
	//int offset = addr & (cache->block_mask);

	printf("%s addr 0x%08x blk addr 0x%08x set %d tag %d\n",cache->name, addr, addr & cache->block_address_mask, set, tag);

	return;
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

int cgm_cache_find_transient_block(struct cache_t *cache, int *tag_ptr, int *set_ptr, unsigned int *offset_ptr, int *way_ptr, int *state_ptr){

	int set, tag, way;
	//unsigned int offset;

	/* Locate block */
	tag = *(tag_ptr);
	set = *(set_ptr);

	//offset = *(offset_ptr);

	*(state_ptr) = 0;

	for (way = 0; way < cache->assoc; way++)
	{
		if (cache->sets[set].blocks[way].transient_tag == tag && cache->sets[set].blocks[way].transient_state == cgm_cache_block_transient)
		{
			/* Block found */
			*(way_ptr) = way;
			*(state_ptr) = cache->sets[set].blocks[way].state;
			return 1;
		}
	}

	//if here something is wrong
	fatal("cgm_cache_find_transient_block(): blk 0x%08x transient block not found as it should be %s set %d tag %d way %d cycle %llu\n",
			cgm_cache_build_address(cache, set, tag), cache->name, set, tag, way, P_TIME);

	/* Block not found */
	return 0;
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

	cache->sets[set].blocks[way].tag = tag;
	cache->sets[set].blocks[way].state = state;
	cache->sets[set].blocks[way].transient_state = cgm_cache_block_invalid;
	cache->sets[set].blocks[way].transient_tag = tag;
	cache->sets[set].blocks[way].written = 1;

	/*if (cache->cache_type == l2_cache_t && set == 69 && tag == 57)
	{
		printf("\t writing set %d tag %d state %d to way %d cycle %llu\n", set, tag, state, way, P_TIME);
		cgm_cache_dump_set(cache, set);

		if(way == 0)
			run_watch_dog = 1;
	}*/

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

	assert(cache->cache_type == l1_d_cache_t || cache->cache_type == gpu_v_cache_t);
	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);

	enum cgm_cache_block_state_t victim_state;

	//get the victim's state
	victim_state = cgm_cache_get_block_state(cache, set, way);

	if((((cgm_cache_build_address(cache, cache->sets[set].id, cache->sets[set].blocks[way].tag) == WATCHBLOCK) && WATCHLINE)) || DUMP)
	{
		if(LEVEL == 1 || LEVEL == 3)
		{
			printf("block 0x%08x %s evicted cycle %llu\n",
					cgm_cache_build_address(cache, cache->sets[set].id, cache->sets[set].blocks[way].tag),
					cache->name, P_TIME);
		}
	}

	//put the block in the write back buffer if in E/M states
	if (victim_state == cgm_cache_block_modified)
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


void cgm_L2_cache_evict_block(struct cache_t *cache, int set, int way, int sharers, int victim_way){

	assert(cache->cache_type == l2_cache_t || cache->cache_type == gpu_l2_cache_t);

	assert(cache->share_mask > 0);

	enum cgm_cache_block_state_t victim_state;
	//get the victim's state
	victim_state = cgm_cache_get_block_state(cache, set, way);

	if((((cgm_cache_build_address(cache, cache->sets[set].id, cache->sets[set].blocks[way].tag) == WATCHBLOCK) && WATCHLINE)) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s evicted cycle %llu\n",
					cgm_cache_build_address(cache, cache->sets[set].id, cache->sets[set].blocks[way].tag),
					cache->name, P_TIME);
		}
	}

	if(cache->cache_type == gpu_l2_cache_t)
		fatal("l2 gpu evicting sharers %d \n", sharers);


	//if dirty data is found
	if (victim_state == cgm_cache_block_modified || victim_state == cgm_cache_block_exclusive)
	{
		//move the block to the WB buffer
		struct cgm_packet_t *write_back_packet = packet_create();

		init_write_back_packet(cache, write_back_packet, set, way, 0, victim_state);

		write_back_packet->flush_pending = 1;

		list_enqueue(cache->write_back_buffer, write_back_packet);
	}

	/*set the pending bit*/
	/*assert(cgm_cache_get_block_flush_pending_bit(cache, set, way) == 0);
	cgm_cache_set_block_flush_pending_bit(cache, set, way);*/

	struct cgm_packet_t *flush_packet = packet_create();

	init_flush_packet(cache, flush_packet, set, way);

	if(victim_way)
		flush_packet->l2_victim_way = victim_way;

	/*needed for correct routing from L2 to L1 D
	figure out another way to do this*/
	if(cache->cache_type == gpu_l2_cache_t)
	{
		flush_packet->cpu_access_type = cgm_access_load_v;
		flush_packet->l1_cache_id = 0;

		warning("gpu l2 evict, fix this \n");
	}
	else
	{
		flush_packet->cpu_access_type = cgm_access_load;
	}

	//set the block state to invalid
	cgm_cache_set_block_state(cache, set, way, cgm_cache_block_invalid);

	list_enqueue(cache->Tx_queue_top, flush_packet);
	advance(cache->cache_io_up_ec);

	return;
}

void cgm_L3_cache_evict_block(struct cache_t *cache, int set, int way, int sharers, int victim_way){

	enum cgm_cache_block_state_t victim_state;
	int i = 0;
	unsigned char bit_vector;
	int num_cores = x86_cpu_num_cores;
	//int check = 0;

	struct cgm_packet_t *write_back_packet = NULL;
	struct cgm_packet_t *flush_packet = NULL;

	assert(sharers >= 0 && sharers <= num_cores);
	assert(cache->cache_type == l3_cache_t);
	assert(cache->share_mask > 0);

	if((((cgm_cache_build_address(cache, cache->sets[set].id, cache->sets[set].blocks[way].tag) == WATCHBLOCK) && WATCHLINE)) || DUMP)
	{
		if(LEVEL == 2 || LEVEL == 3)
		{
			printf("block 0x%08x %s evicted num_sharers %d cycle %llu\n",
					cgm_cache_build_address(cache, cache->sets[set].id, cache->sets[set].blocks[way].tag),
					cache->name, sharers, P_TIME);
		}
	}

	//get the victim's state
	victim_state = cgm_cache_get_block_state(cache, set, way);

	if(victim_state == cgm_cache_block_modified || victim_state == cgm_cache_block_exclusive)
		assert(sharers == 0 || sharers == 1);


	//if block is in the E/M state dirty data is found
	if (victim_state == cgm_cache_block_modified || victim_state == cgm_cache_block_exclusive)
	{
		//move the block to the WB buffer
		 write_back_packet = packet_create();

		init_write_back_packet(cache, write_back_packet, set, way, 0, victim_state);

		/*if no core has the block don't set it as pending*/

		/*if(write_back_packet->write_back_id == 802006)
		{
			//get the presence bits from the directory
			bit_vector = cache->sets[set].blocks[way].directory_entry.entry;
			bit_vector = bit_vector & cache->share_mask;

			warning("made the wb for %llu shareres = %d vector %d \n", write_back_packet->write_back_id, sharers, bit_vector);
		}*/

		if(sharers > 0)
		{
			//printf("write_back_packet %llu number of sharers %d\n", write_back_packet->write_back_id, sharers);
			write_back_packet->flush_pending = 1;
		}

		list_enqueue(cache->write_back_buffer, write_back_packet);
	}

	//send flush to owning cores
	/*star todo account for different block sizes in L3 L2 L1*/

	//get the presence bits from the directory
	bit_vector = cache->sets[set].blocks[way].directory_entry.entry;
	bit_vector = bit_vector & cache->share_mask;

	int num_messages = 0;

	//only run the loop if the bit vector has a bit set.
	if(bit_vector > 0)
	{
		for(i = 0; i < num_cores; i++)
		{
			//for each core that has a copy of the cache block send the eviction
			if((bit_vector & 1) == 1)
			{
				flush_packet = packet_create();

				init_flush_packet(cache, flush_packet, set, way);

				flush_packet->cpu_access_type = cgm_access_store;

				if(victim_way)
					flush_packet->l3_victim_way = victim_way;

				/*if(flush_packet->write_back_id == 802006)
				{
					warning("sending flush to %s\n", l2_caches[i].name);
				}*/

				//update routing
				flush_packet->dest_id = str_map_string(&node_strn_map, l2_caches[i].name);
				flush_packet->dest_name = str_map_value(&l2_strn_map, flush_packet->dest_id);
				flush_packet->src_name = cache->name;
				flush_packet->src_id = str_map_string(&node_strn_map, cache->name);

				list_enqueue(cache->Tx_queue_top, flush_packet);
				advance(cache->cache_io_up_ec);

				num_messages++;
			}

			//shift the vector to the next position and continue
			bit_vector = bit_vector >> 1;
		}
	}

	/*make sure these two are aligned.*/
	assert(num_messages == sharers);

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

int cgm_cache_get_block_usage(struct cache_t *cache){

	int count = 0;
	int set = 0;
	int way = 0;

	for (set = 0; set < cache->num_sets; set++)
	{
		for (way = 0; way < cache->assoc; way++)
		{
			if( cache->sets[set].blocks[way].written == 1)
			{
				count++;
			}
		}
	}

	return count;
}


void cgm_cache_dump_set(struct cache_t *cache, int set){

	int i = 0;

	for(i=0; i < cache->assoc; i++)
	{
		printf("\tcache %s set %d way %d tag %d blk_state %s tran_state %s tran_tag %d upgrade_pending %d cycle %llu\n",
				cache->name, set, i, cache->sets[set].blocks[i].tag,
				str_map_value(&cgm_cache_block_state_map, cache->sets[set].blocks[i].state),
				str_map_value(&cgm_cache_block_state_map, cache->sets[set].blocks[i].transient_state),
				cache->sets[set].blocks[i].transient_tag,
				cache->sets[set].blocks[i].upgrade_pending, P_TIME);
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

	/*if(way < 0 || way >= cache->assoc)
	{
		cgm_cache_dump_set(cache, set);
		ort_dump(cache);
		fatal("cgm_cache_get_victim_for_wb(): no room for wb in cache set %d way %d\n", set, way);
	}

	assert(way >= 0 && way < cache->assoc);*/

	return way;
}


int cgm_cache_get_victim(struct cache_t *cache, int set, int tran_tag){

	//int way = -1;
	int i = 0;

	struct cache_block_t *block;

	assert(set >= 0 && set < cache->num_sets);

	/*if(cache->policy == cache_policy_first_available)
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
	}*/
	if(cache->policy == cache_policy_lru)
	{
		//get the tail block.
		block = cache->sets[set].way_tail;

		//the block should not be in the transient state.


		for(i = 0; i < cache->assoc; i++)
		{
			assert(block->transient_state == cgm_cache_block_invalid || block->transient_state == cgm_cache_block_transient);

			if(block->transient_state == cgm_cache_block_invalid && block->directory_entry.entry_bits.pending == 0)
			{
				block->transient_state = cgm_cache_block_transient;
				block->transient_tag = tran_tag;
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

void cache_store_stats(struct cgm_stats_t *cgm_stat_container){


	int num_cores = x86_cpu_num_cores;
	int i = 0;

	//caches
	for(i = 0; i < num_cores; i++)
	{
		cgm_stat_container->l1_i_Occupancy[i] = l1_i_caches[i].Occupancy;
		cgm_stat_container->l1_i_CoalescePut[i] = l1_i_caches[i].CoalescePut;
		cgm_stat_container->l1_i_CoalesceGet[i] = l1_i_caches[i].CoalesceGet;
		cgm_stat_container->l1_i_TotalHits[i] = l1_i_caches[i].TotalHits;
		cgm_stat_container->l1_i_TotalMisses[i] = l1_i_caches[i].TotalMisses;
		cgm_stat_container->l1_i_TotalAdvances[i] = l1_i_caches[i].TotalAdvances;
		cgm_stat_container->l1_i_TotalAcesses[i] = l1_i_caches[i].TotalAcesses;
		cgm_stat_container->l1_i_TotalReads[i] = l1_i_caches[i].TotalReads;
		cgm_stat_container->l1_i_TotalWrites[i] = l1_i_caches[i].TotalWrites;
		cgm_stat_container->l1_i_TotalGets[i] = l1_i_caches[i].TotalGets;
		cgm_stat_container->l1_i_TotalGet[i] = l1_i_caches[i].TotalGet;
		cgm_stat_container->l1_i_TotalGetx[i] = l1_i_caches[i].TotalGetx;
		cgm_stat_container->l1_i_TotalUpgrades[i] = l1_i_caches[i].TotalUpgrades;
		cgm_stat_container->l1_i_TotalReadMisses[i] = l1_i_caches[i].TotalReadMisses;
		cgm_stat_container->l1_i_TotalWriteMisses[i] = l1_i_caches[i].TotalWriteMisses;
		//cgm_stat_container->l1_i_TotalWriteBacks[i] = l1_i_caches[i].TotalWriteBacks;
		cgm_stat_container->l1_i_invalid_hits[i] = l1_i_caches[i].invalid_hits;
		cgm_stat_container->l1_i_assoc_conflict[i] = l1_i_caches[i].assoc_conflict;
		cgm_stat_container->l1_i_UpgradeMisses[i] = l1_i_caches[i].UpgradeMisses;
		cgm_stat_container->l1_i_retries[i] = l1_i_caches[i].retries;
		cgm_stat_container->l1_i_mshr_entries[i] = l1_i_caches[i].mshr_entries;
		cgm_stat_container->l1_i_Stalls[i] = l1_i_caches[i].Stalls;
		cgm_stat_container->l1_i_WbMerges[i] = l1_i_caches[i].WbMerges;
		cgm_stat_container->l1_i_MergeRetries[i] = l1_i_caches[i].MergeRetries;
		cgm_stat_container->l1_i_EvictInv[i] = l1_i_caches[i].EvictInv;
		cgm_stat_container->l1_i_TotalWriteBackRecieved[i] = l1_i_caches[i].TotalWriteBackRecieved;
		cgm_stat_container->l1_i_TotalWriteBackSent[i] = l1_i_caches[i].TotalWriteBackSent;
		cgm_stat_container->l1_i_TotalWriteBackDropped[i] = l1_i_caches[i].TotalWriteBackDropped;

		cgm_stat_container->l1_i_TotalDowngrades[i] = l1_i_caches[i].TotalDowngrades;
		cgm_stat_container->l1_i_TotalGetxFwdInvals[i] = l1_i_caches[i].TotalGetxFwdInvals;
		cgm_stat_container->l1_i_TotalUpgradeAcks[i] = l1_i_caches[i].TotalUpgradeAcks;
		cgm_stat_container->l1_i_TotalUpgradeInvals[i] = l1_i_caches[i].TotalUpgradeInvals;
		cgm_stat_container->l1_i_TotalWriteBlocks[i] = l1_i_caches[i].TotalWriteBlocks;


		cgm_stat_container->l1_d_Occupancy[i] = l1_d_caches[i].Occupancy;
		cgm_stat_container->l1_d_TotalAdvances[i] = l1_d_caches[i].TotalAdvances;
		cgm_stat_container->l1_d_TotalAcesses[i] = l1_d_caches[i].TotalAcesses;
		cgm_stat_container->l1_d_TotalMisses[i] = l1_d_caches[i].TotalMisses;
		cgm_stat_container->l1_d_TotalHits[i] = l1_d_caches[i].TotalHits;
		cgm_stat_container->l1_d_TotalReads[i] = l1_d_caches[i].TotalReads;
		cgm_stat_container->l1_d_TotalWrites[i] = l1_d_caches[i].TotalWrites;
		cgm_stat_container->l1_d_TotalGets[i] = l1_d_caches[i].TotalGets;
		cgm_stat_container->l1_d_TotalGet[i] = l1_d_caches[i].TotalGet;
		cgm_stat_container->l1_d_TotalGetx[i] = l1_d_caches[i].TotalGetx;
		cgm_stat_container->l1_d_TotalUpgrades[i] = l1_d_caches[i].TotalUpgrades;
		cgm_stat_container->l1_d_TotalReadMisses[i] = l1_d_caches[i].TotalReadMisses;
		cgm_stat_container->l1_d_TotalWriteMisses[i] = l1_d_caches[i].TotalWriteMisses;
		//cgm_stat_container->l1_d_TotalWriteBacks[i] = l1_d_caches[i].TotalWriteBacks;
		cgm_stat_container->l1_d_invalid_hits[i] = l1_d_caches[i].invalid_hits;
		cgm_stat_container->l1_d_assoc_conflict[i] = l1_d_caches[i].assoc_conflict;
		cgm_stat_container->l1_d_UpgradeMisses[i] = l1_d_caches[i].UpgradeMisses;
		cgm_stat_container->l1_d_retries[i] = l1_d_caches[i].retries;
		cgm_stat_container->l1_d_CoalescePut[i] = l1_d_caches[i].CoalescePut;
		cgm_stat_container->l1_d_CoalesceGet[i] = l1_d_caches[i].CoalesceGet;
		cgm_stat_container->l1_d_mshr_entries[i] = l1_d_caches[i].mshr_entries;
		cgm_stat_container->l1_d_Stalls[i] = l1_d_caches[i].Stalls;
		cgm_stat_container->l1_d_WbMerges[i] = l1_d_caches[i].WbMerges;
		cgm_stat_container->l1_d_MergeRetries[i] = l1_d_caches[i].MergeRetries;
		cgm_stat_container->l1_d_EvictInv[i] = l1_d_caches[i].EvictInv;
		cgm_stat_container->l1_d_TotalWriteBackRecieved[i] = l1_d_caches[i].TotalWriteBackRecieved;
		cgm_stat_container->l1_d_TotalWriteBackSent[i] = l1_d_caches[i].TotalWriteBackSent;
		cgm_stat_container->l1_d_TotalWriteBackDropped[i] = l1_d_caches[i].TotalWriteBackDropped;

		cgm_stat_container->l1_d_TotalDowngrades[i] = l1_d_caches[i].TotalDowngrades;
		cgm_stat_container->l1_d_TotalGetxFwdInvals[i] = l1_d_caches[i].TotalGetxFwdInvals;
		cgm_stat_container->l1_d_TotalUpgradeAcks[i] = l1_d_caches[i].TotalUpgradeAcks;
		cgm_stat_container->l1_d_TotalUpgradeInvals[i] = l1_d_caches[i].TotalUpgradeInvals;
		cgm_stat_container->l1_d_TotalWriteBlocks[i] = l1_d_caches[i].TotalWriteBlocks;


		cgm_stat_container->l2_Occupancy[i] = l2_caches[i].Occupancy;
		cgm_stat_container->l2_TotalAdvances[i] = l2_caches[i].TotalAdvances;
		cgm_stat_container->l2_TotalAcesses[i] = l2_caches[i].TotalAcesses;
		cgm_stat_container->l2_TotalMisses[i] = l2_caches[i].TotalMisses;
		cgm_stat_container->l2_TotalHits[i] = l2_caches[i].TotalHits;
		cgm_stat_container->l2_TotalReads[i] = l2_caches[i].TotalReads;
		cgm_stat_container->l2_TotalWrites[i] = l2_caches[i].TotalWrites;
		cgm_stat_container->l2_TotalGets[i] = l2_caches[i].TotalGets;
		cgm_stat_container->l2_TotalGet[i] = l2_caches[i].TotalGet;
		cgm_stat_container->l2_TotalGetx[i] = l2_caches[i].TotalGetx;
		cgm_stat_container->l2_TotalUpgrades[i] = l2_caches[i].TotalUpgrades;
		cgm_stat_container->l2_TotalReadMisses[i] = l2_caches[i].TotalReadMisses;
		cgm_stat_container->l2_TotalWriteMisses[i] = l2_caches[i].TotalWriteMisses;
		//cgm_stat_container->l2_TotalWriteBacks[i] = l2_caches[i].TotalWriteBacks;
		cgm_stat_container->l2_invalid_hits[i] = l2_caches[i].invalid_hits;
		cgm_stat_container->l2_assoc_conflict[i] = l2_caches[i].assoc_conflict;
		cgm_stat_container->l2_UpgradeMisses[i] = l2_caches[i].UpgradeMisses;
		cgm_stat_container->l2_TotalUpgradeAcks[i] = l2_caches[i].TotalUpgradeAcks;
		cgm_stat_container->l2_retries[i] = l2_caches[i].retries;
		cgm_stat_container->l2_CoalescePut[i] = l2_caches[i].CoalescePut;
		cgm_stat_container->l2_CoalesceGet[i] = l2_caches[i].CoalesceGet;
		cgm_stat_container->l2_mshr_entries[i] = l2_caches[i].mshr_entries;
		cgm_stat_container->l2_Stalls[i] = l2_caches[i].Stalls;
		cgm_stat_container->l2_WbMerges[i] = l2_caches[i].WbMerges;
		cgm_stat_container->l2_MergeRetries[i] = l2_caches[i].MergeRetries;
		cgm_stat_container->l2_EvictInv[i] = l2_caches[i].EvictInv;
		cgm_stat_container->l2_TotalWriteBackRecieved[i] = l2_caches[i].TotalWriteBackRecieved;
		cgm_stat_container->l2_TotalWriteBackSent[i] = l2_caches[i].TotalWriteBackSent;
		cgm_stat_container->l2_TotalWriteBackDropped[i] = l2_caches[i].TotalWriteBackDropped;
		cgm_stat_container->l2_TotalWriteBlocks[i] = l2_caches[i].TotalWriteBlocks;
		cgm_stat_container->l2_LoadMisses[i] = l2_caches[i].LoadMisses;
		cgm_stat_container->l2_TotalUpgradeInvals[i] = l2_caches[i].TotalUpgradeInvals;


		cgm_stat_container->l2_gets_[i] = l2_caches[i].l2_gets_;
		cgm_stat_container->l2_get_[i] = l2_caches[i].l2_get_;
		cgm_stat_container->l2_getx_[i] = l2_caches[i].l2_getx_;
		cgm_stat_container->l2_write_back_[i] = l2_caches[i].l2_write_back_;
		cgm_stat_container->l2_write_block_[i] = l2_caches[i].l2_write_block_;
		cgm_stat_container->l2_downgrade_ack_[i] = l2_caches[i].l2_downgrade_ack_;
		cgm_stat_container->l2_get_nack_[i] = l2_caches[i].l2_get_nack_;
		cgm_stat_container->l2_getx_nack_[i] = l2_caches[i].l2_getx_nack_;
		cgm_stat_container->l2_get_fwd_[i] = l2_caches[i].l2_get_fwd_;
		cgm_stat_container->l2_downgrade_nack_[i] = l2_caches[i].l2_downgrade_nack_;
		cgm_stat_container->l2_getx_fwd_[i] = l2_caches[i].l2_getx_fwd_;
		cgm_stat_container->l2_getx_fwd_inval_ack_[i] = l2_caches[i].l2_getx_fwd_inval_ack_;
		cgm_stat_container->l2_getx_fwd_nack_[i] = l2_caches[i].l2_getx_fwd_nack_;
		cgm_stat_container->l2_upgrade_[i] = l2_caches[i].l2_upgrade_;
		cgm_stat_container->l2_upgrade_ack_[i] = l2_caches[i].l2_upgrade_ack_;
		cgm_stat_container->l2_upgrade_nack_[i] = l2_caches[i].l2_upgrade_nack_;
		cgm_stat_container->l2_upgrade_putx_n_[i] = l2_caches[i].l2_upgrade_putx_n_;
		cgm_stat_container->l2_upgrade_inval_[i] = l2_caches[i].l2_upgrade_inval_;
		cgm_stat_container->l2_flush_block_[i] = l2_caches[i].l2_flush_block_;
		cgm_stat_container->l2_flush_block_ack_[i] = l2_caches[i].l2_flush_block_ack_;


		cgm_stat_container->l3_Occupancy[i] = l3_caches[i].Occupancy;
		cgm_stat_container->l3_TotalAdvances[i] = l3_caches[i].TotalAdvances;
		cgm_stat_container->l3_TotalAcesses[i] = l3_caches[i].TotalAcesses;
		cgm_stat_container->l3_TotalMisses[i] = l3_caches[i].TotalMisses;
		cgm_stat_container->l3_TotalHits[i] = l3_caches[i].TotalHits;
		cgm_stat_container->l3_TotalReads[i] = l3_caches[i].TotalReads;
		cgm_stat_container->l3_TotalWrites[i] = l3_caches[i].TotalWrites;
		cgm_stat_container->l3_TotalGets[i] = l3_caches[i].TotalGets;
		cgm_stat_container->l3_TotalGet[i] = l3_caches[i].TotalGet;
		cgm_stat_container->l3_TotalGetx[i] = l3_caches[i].TotalGetx;
		cgm_stat_container->l3_TotalUpgrades[i] = l3_caches[i].TotalUpgrades;
		cgm_stat_container->l3_TotalReadMisses[i] = l3_caches[i].TotalReadMisses;
		cgm_stat_container->l3_TotalWriteMisses[i] = l3_caches[i].TotalWriteMisses;
		//cgm_stat_container->l3_TotalWriteBacks[i] = l3_caches[i].TotalWriteBacks;
		cgm_stat_container->l3_invalid_hits[i] = l3_caches[i].invalid_hits;
		cgm_stat_container->l3_assoc_conflict[i] = l3_caches[i].assoc_conflict;
		cgm_stat_container->l3_UpgradeMisses[i] = l3_caches[i].UpgradeMisses;
		cgm_stat_container->l3_retries[i] = l3_caches[i].retries;
		cgm_stat_container->l3_CoalescePut[i] = l3_caches[i].CoalescePut;
		cgm_stat_container->l3_CoalesceGet[i] = l3_caches[i].CoalesceGet;
		cgm_stat_container->l3_mshr_entries[i] = l3_caches[i].mshr_entries;
		cgm_stat_container->l3_Stalls[i] = l3_caches[i].Stalls;
		cgm_stat_container->l3_WbMerges[i] = l3_caches[i].WbMerges;
		cgm_stat_container->l3_MergeRetries[i] = l3_caches[i].MergeRetries;
		cgm_stat_container->l3_EvictInv[i] = l3_caches[i].EvictInv;
		cgm_stat_container->l3_TotalWriteBackRecieved[i] = l3_caches[i].TotalWriteBackRecieved;
		cgm_stat_container->l3_TotalWriteBackSent[i] = l3_caches[i].TotalWriteBackSent;
		cgm_stat_container->l3_TotalSharingWriteBackSent[i] = l3_caches[i].TotalSharingWriteBackSent;
		cgm_stat_container->l3_TotalWriteBackDropped[i] = l3_caches[i].TotalWriteBackDropped;
		cgm_stat_container->l3_TotalWriteBlocks[i] = l3_caches[i].TotalWriteBlocks;
		cgm_stat_container->l3_TotalUpgradeInvals[i] = l3_caches[i].TotalUpgradeInvals;


		cgm_stat_container->l3_gets_[i] = l3_caches[i].l3_gets_;
		cgm_stat_container->l3_get_[i] = l3_caches[i].l3_get_;
		cgm_stat_container->l3_getx_[i] = l3_caches[i].l3_getx_;
		cgm_stat_container->l3_write_back_[i] = l3_caches[i].l3_write_back_;
		cgm_stat_container->l3_flush_block_ack_[i] = l3_caches[i].l3_flush_block_ack_;
		cgm_stat_container->l3_write_block_[i] = l3_caches[i].l3_write_block_;
		cgm_stat_container->l3_downgrade_ack_[i] = l3_caches[i].l3_downgrade_ack_;
		cgm_stat_container->l3_downgrade_nack_[i] = l3_caches[i].l3_downgrade_nack_;
		cgm_stat_container->l3_getx_fwd_ack_[i] = l3_caches[i].l3_getx_fwd_ack_;
		cgm_stat_container->l3_getx_fwd_nack_[i] = l3_caches[i].l3_getx_fwd_nack_;
		cgm_stat_container->l3_getx_fwd_upgrade_nack_[i] = l3_caches[i].l3_getx_fwd_upgrade_nack_;
		cgm_stat_container->l3_get_fwd_upgrade_nack_[i] = l3_caches[i].l3_get_fwd_upgrade_nack_;
		cgm_stat_container->l3_upgrade_[i] = l3_caches[i].l3_upgrade_;
		cgm_stat_container->l3_upgrade_ack_[i] = l3_caches[i].l3_upgrade_ack_;

	}

	return;
}

void cache_reset_stats(void){

	int num_cores = x86_cpu_num_cores;
	int i = 0;

	//caches
	for(i = 0; i < num_cores; i++)
	{
		l1_i_caches[i].Occupancy = 0;
		l1_i_caches[i].CoalescePut = 0;
		l1_i_caches[i].CoalesceGet = 0;
		l1_i_caches[i].TotalHits = 0;
		l1_i_caches[i].TotalMisses = 0;
		l1_i_caches[i].WbMerges = 0;
		l1_i_caches[i].MergeRetries = 0;
		l1_i_caches[i].EvictInv = 0;
		l1_i_caches[i].TotalWriteBackRecieved = 0;
		l1_i_caches[i].TotalWriteBackSent = 0;
		l1_i_caches[i].TotalWriteBackDropped = 0;
		l1_i_caches[i].Stalls = 0;
		l1_i_caches[i].TotalDowngrades = 0;
		l1_i_caches[i].TotalGetxFwdInvals = 0;
		l1_i_caches[i].TotalUpgradeAcks = 0;
		l1_i_caches[i].TotalUpgradeInvals = 0;
		l1_i_caches[i].TotalWriteBlocks = 0;

		l1_i_caches[i].TotalAdvances = 0;
		l1_i_caches[i].TotalAcesses = 0;
		l1_i_caches[i].TotalReads = 0;
		l1_i_caches[i].TotalWrites = 0;
		l1_i_caches[i].TotalGets = 0;
		l1_i_caches[i].TotalGet = 0;
		l1_i_caches[i].TotalGetx = 0;
		l1_i_caches[i].TotalUpgrades = 0;
		l1_i_caches[i].TotalReadMisses = 0;
		l1_i_caches[i].TotalWriteMisses = 0;
		//l1_i_caches[i].TotalWriteBacks = 0;
		l1_i_caches[i].invalid_hits = 0;
		l1_i_caches[i].assoc_conflict = 0;
		l1_i_caches[i].UpgradeMisses = 0;
		l1_i_caches[i].retries = 0;

		l1_i_caches[i].mshr_entries = 0;


		l1_d_caches[i].Occupancy = 0;
		l1_d_caches[i].TotalAdvances = 0;
		l1_d_caches[i].WbMerges = 0;
		l1_d_caches[i].MergeRetries = 0;
		l1_d_caches[i].EvictInv = 0;
		l1_d_caches[i].TotalWriteBackRecieved = 0;
		l1_d_caches[i].TotalWriteBackSent = 0;
		l1_d_caches[i].TotalWriteBackDropped = 0;
		l1_d_caches[i].CoalescePut = 0;
		l1_d_caches[i].CoalesceGet = 0;
		l1_d_caches[i].Stalls = 0;
		l1_d_caches[i].TotalDowngrades = 0;
		l1_d_caches[i].TotalGetxFwdInvals = 0;
		l1_d_caches[i].TotalUpgradeAcks = 0;
		l1_d_caches[i].TotalUpgradeInvals = 0;
		l1_d_caches[i].TotalWriteBlocks = 0;



		l1_d_caches[i].TotalAcesses = 0;
		l1_d_caches[i].TotalMisses = 0;
		l1_d_caches[i].TotalHits = 0;
		l1_d_caches[i].TotalReads = 0;
		l1_d_caches[i].TotalWrites = 0;
		l1_d_caches[i].TotalGets = 0;
		l1_d_caches[i].TotalGet = 0;
		l1_d_caches[i].TotalGetx = 0;
		l1_d_caches[i].TotalUpgrades = 0;
		l1_d_caches[i].TotalReadMisses = 0;
		l1_d_caches[i].TotalWriteMisses = 0;
		//l1_d_caches[i].TotalWriteBacks = 0;
		l1_d_caches[i].invalid_hits = 0;
		l1_d_caches[i].assoc_conflict = 0;
		l1_d_caches[i].UpgradeMisses = 0;
		l1_d_caches[i].retries = 0;

		l1_d_caches[i].mshr_entries = 0;


		l2_caches[i].Occupancy = 0;
		l2_caches[i].TotalAdvances = 0;
		l2_caches[i].TotalAcesses = 0;
		l2_caches[i].WbMerges = 0;
		l2_caches[i].MergeRetries = 0;
		l2_caches[i].EvictInv = 0;
		l2_caches[i].TotalWriteBackRecieved = 0;
		l2_caches[i].TotalWriteBackSent = 0;
		l2_caches[i].TotalWriteBackDropped = 0;
		l2_caches[i].Stalls = 0;
		l2_caches[i].TotalDowngrades = 0;
		l2_caches[i].TotalGetxFwdInvals = 0;
		l2_caches[i].UpgradeMisses = 0;
		l2_caches[i].TotalUpgradeAcks = 0;
		l2_caches[i].TotalUpgradeInvals = 0;
		l2_caches[i].TotalWriteBlocks = 0;
		l2_caches[i].LoadMisses = 0;

		l2_caches[i].TotalMisses = 0;
		l2_caches[i].TotalHits = 0;
		l2_caches[i].TotalReads = 0;
		l2_caches[i].TotalWrites = 0;
		l2_caches[i].TotalGets = 0;
		l2_caches[i].TotalGet = 0;
		l2_caches[i].TotalGetx = 0;
		l2_caches[i].TotalUpgrades = 0;
		l2_caches[i].TotalReadMisses = 0;
		l2_caches[i].TotalWriteMisses = 0;
		//l2_caches[i].TotalWriteBacks = 0;
		l2_caches[i].invalid_hits = 0;
		l2_caches[i].assoc_conflict = 0;

		l2_caches[i].retries = 0;
		l2_caches[i].CoalescePut = 0;
		l2_caches[i].CoalesceGet = 0;
		l2_caches[i].mshr_entries = 0;


		l2_caches[i].l2_gets_ = 0;
		l2_caches[i].l2_get_ = 0;
		l2_caches[i].l2_getx_ = 0;
		l2_caches[i].l2_write_back_ = 0;
		l2_caches[i].l2_write_block_ = 0;
		l2_caches[i].l2_downgrade_ack_ = 0;
		l2_caches[i].l2_get_nack_ = 0;
		l2_caches[i].l2_getx_nack_ = 0;
		l2_caches[i].l2_get_fwd_ = 0;
		l2_caches[i].l2_downgrade_nack_ = 0;
		l2_caches[i].l2_getx_fwd_ = 0;
		l2_caches[i].l2_getx_fwd_inval_ack_ = 0;
		l2_caches[i].l2_getx_fwd_nack_ = 0;
		l2_caches[i].l2_upgrade_ = 0;
		l2_caches[i].l2_upgrade_ack_ = 0;
		l2_caches[i].l2_upgrade_nack_ = 0;
		l2_caches[i].l2_upgrade_putx_n_ = 0;
		l2_caches[i].l2_upgrade_inval_ = 0;
		l2_caches[i].l2_flush_block_ = 0;
		l2_caches[i].l2_flush_block_ack_ = 0;


		l3_caches[i].Occupancy = 0;
		l3_caches[i].TotalAdvances = 0;
		l3_caches[i].TotalAcesses = 0;
		l3_caches[i].WbMerges = 0;
		l3_caches[i].MergeRetries = 0;
		l3_caches[i].EvictInv = 0;
		l3_caches[i].TotalWriteBackRecieved = 0;
		l3_caches[i].TotalWriteBackSent = 0;
		l3_caches[i].TotalSharingWriteBackSent = 0;
		l3_caches[i].TotalWriteBackDropped = 0;
		l3_caches[i].Stalls = 0;
		l3_caches[i].TotalDowngrades = 0;
		l3_caches[i].TotalGetxFwdInvals = 0;
		l3_caches[i].TotalUpgradeAcks = 0;
		l3_caches[i].TotalUpgradeInvals = 0;
		l3_caches[i].TotalWriteBlocks = 0;


		l3_caches[i].TotalMisses = 0;
		l3_caches[i].TotalHits = 0;
		l3_caches[i].TotalReads = 0;
		l3_caches[i].TotalWrites = 0;
		l3_caches[i].TotalGets = 0;
		l3_caches[i].TotalGet = 0;
		l3_caches[i].TotalGetx = 0;
		l3_caches[i].TotalUpgrades = 0;
		l3_caches[i].TotalReadMisses = 0;
		l3_caches[i].TotalWriteMisses = 0;
		//l3_caches[i].TotalWriteBacks = 0;
		l3_caches[i].invalid_hits = 0;
		l3_caches[i].assoc_conflict = 0;
		l3_caches[i].UpgradeMisses = 0;
		l3_caches[i].retries = 0;
		l3_caches[i].CoalescePut = 0;
		l3_caches[i].CoalesceGet = 0;
		l3_caches[i].mshr_entries = 0;


		l3_caches[i].l3_gets_ = 0;
		l3_caches[i].l3_get_ = 0;
		l3_caches[i].l3_getx_ = 0;
		l3_caches[i].l3_write_back_ = 0;
		l3_caches[i].l3_flush_block_ack_ = 0;
		l3_caches[i].l3_write_block_ = 0;
		l3_caches[i].l3_downgrade_ack_ = 0;
		l3_caches[i].l3_downgrade_nack_ = 0;
		l3_caches[i].l3_getx_fwd_ack_ = 0;
		l3_caches[i].l3_getx_fwd_nack_ = 0;
		l3_caches[i].l3_getx_fwd_upgrade_nack_ = 0;
		l3_caches[i].l3_get_fwd_upgrade_nack_ = 0;
		l3_caches[i].l3_upgrade_ = 0;
		l3_caches[i].l3_upgrade_ack_ = 0;

	}

	return;
}

void cache_dump_stats(struct cgm_stats_t *cgm_stat_container){

	int num_cores = x86_cpu_num_cores;
	//int num_cus = si_gpu_num_compute_units;
	//int gpu_group_cache_num = (num_cus/4);
	int i = 0;
	int blocks_written = 0;

	/*CPU caches*/
	for(i = 0; i < num_cores; i++)
	{
		/*CGM_STATS(cgm_stats_file, ";---Core %d---\n", i);
		CGM_STATS(cgm_stats_file, "[L1_I_Cache_%d]\n", i);*/
		CGM_STATS(cgm_stats_file, "l1_i_%d_Occupancy = %llu\n", i, cgm_stat_container->l1_i_Occupancy[i]);
		if(cgm_stat_container->stats_type == systemStats)
		{
			CGM_STATS(cgm_stats_file, "l1_i_%d_OccupancyPct = %0.4f\n", i, ((double) cgm_stat_container->l1_i_Occupancy[i]/(double) P_TIME)*100);
		}
		else if (cgm_stat_container->stats_type == parallelSection)
		{
			CGM_STATS(cgm_stats_file, "l1_i_%d_OccupancyPct = %0.4f\n", i, ((double) cgm_stat_container->l1_i_Occupancy[i]/(double) cgm_stat_container->total_parallel_section_cycles)*100);
		}
		else
		{
			fatal("cache_dump_stats(): bad container type\n");
		}
		CGM_STATS(cgm_stats_file, "l1_i_%d_Stalls = %llu\n", i, cgm_stat_container->l1_i_Stalls[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_CoalescePut = %llu\n", i, cgm_stat_container->l1_i_CoalescePut[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_CoalesceGet = %llu\n", i, cgm_stat_container->l1_i_CoalesceGet[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_WbMerges = %llu\n", i, cgm_stat_container->l1_i_WbMerges[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_MergeRetries = %llu\n", i, cgm_stat_container->l1_i_MergeRetries[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_EvictInv = %llu\n", i, cgm_stat_container->l1_i_EvictInv[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_WbRecieved = %llu\n", i, cgm_stat_container->l1_i_TotalWriteBackRecieved[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_WbSent = %llu\n", i, cgm_stat_container->l1_i_TotalWriteBackSent[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_SharingWbSent = %llu\n", i, (long long)0);
		CGM_STATS(cgm_stats_file, "l1_i_%d_WbDropped = %llu\n", i, cgm_stat_container->l1_i_TotalWriteBackDropped[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_UpgradeMisses = %llu\n", i, cgm_stat_container->l1_i_UpgradeMisses[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalMisses = %llu\n", i, cgm_stat_container->l1_i_TotalMisses[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalHits = %llu\n", i, (cgm_stat_container->l1_i_TotalHits[i]));
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalAdvances = %llu\n", i, cgm_stat_container->l1_i_TotalAdvances[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_AveCyclesPerAdvance = %0.2f\n", i, ((double) cgm_stat_container->l1_i_Occupancy[i]/ (double) cgm_stat_container->l1_i_TotalAdvances[i]));
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalAccesses = %llu\n", i, cgm_stat_container->l1_i_TotalAcesses[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalDowngrades = %llu\n", i, cgm_stat_container->l1_i_TotalDowngrades[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalGetxFwdInvals = %llu\n", i, cgm_stat_container->l1_i_TotalGetxFwdInvals[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalUpgradeAcks = %llu\n", i, cgm_stat_container->l1_i_TotalUpgradeAcks[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalUpgradeInvals = %llu\n", i, cgm_stat_container->l1_i_TotalUpgradeInvals[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalWriteBlocks = %llu\n", i, cgm_stat_container->l1_i_TotalWriteBlocks[i]);

		CGM_STATS(cgm_stats_file, "l1_i_%d_MissRate = %0.2f\n", i,
				(double) (cgm_stat_container->l1_i_TotalMisses[i])/(double) (cgm_stat_container->l1_i_TotalAcesses[i] - cgm_stat_container->l1_i_TotalMisses[i]));
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalReads = %llu\n", i, cgm_stat_container->l1_i_TotalReads[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalReadMisses = %llu\n", i, cgm_stat_container->l1_i_TotalReadMisses[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_ReadMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_i_TotalReadMisses[i]/(double) cgm_stat_container->l1_i_TotalReads[i]));
		//CGM_STATS(cgm_stats_file, "l1_i_%d_TotalWrites = %llu\n", i, cgm_stat_container->l1_i_TotalWrites[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalWrites = %llu\n", i, (long long)0);
		//CGM_STATS(cgm_stats_file, "l1_i_%d_TotalWriteMisses = %llu\n", i, cgm_stat_container->l1_i_TotalWriteMisses[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalWriteMisses = %llu\n", i, (long long)0);
		//CGM_STATS(cgm_stats_file, "l1_i_%d_WriteMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_i_TotalWriteMisses[i]/(double) cgm_stat_container->l1_i_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l1_i_%d_WriteMissRate = %0.2f\n", i, (float)0);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalGets = %llu\n", i, cgm_stat_container->l1_i_TotalGets[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalGet = %llu\n", i, cgm_stat_container->l1_i_TotalGet[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalGetx = %llu\n", i, cgm_stat_container->l1_i_TotalGetx[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_GetsMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_i_TotalGets[i]/(double) cgm_stat_container->l1_i_TotalReads[i]));
		//CGM_STATS(cgm_stats_file, "l1_i_%d_GetMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_i_TotalGet[i]/(double) cgm_stat_container->l1_i_TotalReads[i]));
		CGM_STATS(cgm_stats_file, "l1_i_%d_GetMissRate = %0.2f\n", i, (float)0);
		//CGM_STATS(cgm_stats_file, "l1_i_%d_GetxMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_i_TotalGetx[i]/(double) cgm_stat_container->l1_i_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l1_i_%d_GetxMissRate = %0.2f\n", i, (float)0);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalUpgrades = %llu\n", i, cgm_stat_container->l1_i_TotalUpgrades[i]);
		//CGM_STATS(cgm_stats_file, "l1_i_%d_UpgradeMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_i_TotalUpgrades[i]/(double) cgm_stat_container->l1_i_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l1_i_%d_UpgradeMissRate = %0.2f\n", i, (float)0);
		CGM_STATS(cgm_stats_file, "l1_i_%d_TotalWriteBacks = %llu\n", i, cgm_stat_container->l1_i_TotalWriteBacks[i]);
		blocks_written = cgm_cache_get_block_usage(&l1_i_caches[i]);
		CGM_STATS(cgm_stats_file, "l1_i_%d_CacheUtilization = %0.2f\n", i, ((double) blocks_written)/(double) (l1_i_caches[i].num_sets * l1_i_caches[i].assoc));
		/*CGM_STATS(cgm_stats_file, "\n");*/

		/*CGM_STATS(cgm_stats_file, "[L1_D_Cache_%d]\n", i);*/
		CGM_STATS(cgm_stats_file, "l1_d_%d_Occupancy = %llu\n", i, cgm_stat_container->l1_d_Occupancy[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_Stalls = %llu\n", i, cgm_stat_container->l1_d_Stalls[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_CoalescePut = %llu\n", i, cgm_stat_container->l1_d_CoalescePut[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_CoalesceGet = %llu\n", i, cgm_stat_container->l1_d_CoalesceGet[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_WbMerges = %llu\n", i, cgm_stat_container->l1_d_WbMerges[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_MergeRetries = %llu\n", i, cgm_stat_container->l1_d_MergeRetries[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_EvictInv = %llu\n", i, cgm_stat_container->l1_d_EvictInv[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_WbRecieved = %llu\n", i, cgm_stat_container->l1_d_TotalWriteBackRecieved[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_WbSent = %llu\n", i, cgm_stat_container->l1_d_TotalWriteBackSent[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_SharingWbSent = %llu\n", i, (long long)0);
		CGM_STATS(cgm_stats_file, "l1_d_%d_WbDropped = %llu\n", i, cgm_stat_container->l1_d_TotalWriteBackDropped[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalDowngrades = %llu\n", i, cgm_stat_container->l1_d_TotalDowngrades[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalGetxFwdInvals = %llu\n", i, cgm_stat_container->l1_d_TotalGetxFwdInvals[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalUpgradeAcks = %llu\n", i, cgm_stat_container->l1_d_TotalUpgradeAcks[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalUpgradeInvals = %llu\n", i, cgm_stat_container->l1_d_TotalUpgradeInvals[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalWriteBlocks = %llu\n", i, cgm_stat_container->l1_d_TotalWriteBlocks[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_AveCyclesPerAdvance = %0.2f\n", i, ((double) cgm_stat_container->l1_d_Occupancy[i]/ (double) cgm_stat_container->l1_d_TotalAdvances[i]));
		if(cgm_stat_container->stats_type == systemStats)
		{
			CGM_STATS(cgm_stats_file, "l1_d_%d_OccupancyPct = %0.2f\n", i, ((double)cgm_stat_container->l1_d_Occupancy[i]/(double)P_TIME)*100);
		}
		else if (cgm_stat_container->stats_type == parallelSection)
		{
			//fatal("occ %llu cycles %llu then %0.2f\n", cgm_stat_container->l1_d_occupancy[i], cgm_stat_container->total_parallel_section_cycles, cgm_stat_container->l1_d_occupancy[i]/cgm_stat_container->total_parallel_section_cycles);
			CGM_STATS(cgm_stats_file, "l1_d_%d_OccupancyPct = %0.2f\n", i, ((double)cgm_stat_container->l1_d_Occupancy[i]/(double)cgm_stat_container->total_parallel_section_cycles)*100);
		}
		else
		{
			fatal("cache_dump_stats(): bad container type\n");
		}

		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalAdvances = %llu\n", i, cgm_stat_container->l1_d_TotalAdvances[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalAccesses = %llu\n", i, cgm_stat_container->l1_d_TotalAcesses[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_UpgradeMisses = %llu\n", i, cgm_stat_container->l1_d_UpgradeMisses[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalHits = %llu\n", i, (cgm_stat_container->l1_d_TotalAcesses[i] - cgm_stat_container->l1_d_TotalMisses[i]));
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalMisses = %llu\n", i, cgm_stat_container->l1_d_TotalMisses[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_MissRate = %0.2f\n", i,
				(double) (cgm_stat_container->l1_d_TotalMisses[i])/(double) (cgm_stat_container->l1_d_TotalAcesses[i]- cgm_stat_container->l1_d_TotalMisses[i]));
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalReads = %llu\n", i, cgm_stat_container->l1_d_TotalReads[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalReadMisses = %llu\n", i, cgm_stat_container->l1_d_TotalReadMisses[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_ReadMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_d_TotalReadMisses[i] / (double) cgm_stat_container->l1_d_TotalReads[i]));
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalWrites = %llu\n", i, cgm_stat_container->l1_d_TotalWrites[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalWriteMisses = %llu\n", i, cgm_stat_container->l1_d_TotalWriteMisses[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_WriteMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_d_TotalWriteMisses[i] / (double) cgm_stat_container->l1_d_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalGets = %llu\n", i, cgm_stat_container->l1_d_TotalGets[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalGet = %llu\n", i, cgm_stat_container->l1_d_TotalGet[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalGetx = %llu\n", i, cgm_stat_container->l1_d_TotalGetx[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_GetsMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_d_TotalGets[i] / (double) cgm_stat_container->l1_d_TotalReads[i]));
		CGM_STATS(cgm_stats_file, "l1_d_%d_GetMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_d_TotalGet[i] / (double) cgm_stat_container->l1_d_TotalReads[i]));
		CGM_STATS(cgm_stats_file, "l1_d_%d_GetxMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_d_TotalGetx[i] / (double) cgm_stat_container->l1_d_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalUpgrades = %llu\n", i, cgm_stat_container->l1_d_TotalUpgrades[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_UpgradeMissRate = %0.2f\n", i, ((double) cgm_stat_container->l1_d_TotalUpgrades[i] / (double) cgm_stat_container->l1_d_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l1_d_%d_TotalWriteBacks = %llu\n", i, cgm_stat_container->l1_d_TotalWriteBacks[i]);
		blocks_written = cgm_cache_get_block_usage(&l1_d_caches[i]);
		CGM_STATS(cgm_stats_file, "l1_d_%d_CacheUtilization = %0.2f\n", i, ((double) blocks_written)/(double) (l1_d_caches[i].num_sets * l1_d_caches[i].assoc));
		/*CGM_STATS(cgm_stats_file, "\n");*/

		/*CGM_STATS(cgm_stats_file, "[L2_Cache_%d]\n", i);*/
		CGM_STATS(cgm_stats_file, "l2_%d_Occupancy = %llu\n", i, cgm_stat_container->l2_Occupancy[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_Stalls = %llu\n", i, cgm_stat_container->l2_Stalls[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_CoalescePut = %llu\n", i, cgm_stat_container->l2_CoalescePut[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_CoalesceGet = %llu\n", i, cgm_stat_container->l2_CoalesceGet[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_UpgradeMisses = %llu\n", i, cgm_stat_container->l2_UpgradeMisses[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_WbMerges = %llu\n", i, cgm_stat_container->l2_WbMerges[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_MergeRetries = %llu\n", i, cgm_stat_container->l2_MergeRetries[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_EvictInv = %llu\n", i, cgm_stat_container->l2_EvictInv[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_WbRecieved = %llu\n", i, cgm_stat_container->l2_TotalWriteBackRecieved[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_WbSent = %llu\n", i, cgm_stat_container->l2_TotalWriteBackSent[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_SharingWbSent = %llu\n", i, (long long)0);
		CGM_STATS(cgm_stats_file, "l2_%d_WbDropped = %llu\n", i, cgm_stat_container->l2_TotalWriteBackDropped[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalDowngrades = %llu\n", i, cgm_stat_container->l2_TotalDowngrades[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalGetxFwdInvals = %llu\n", i, cgm_stat_container->l2_TotalGetxFwdInvals[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalUpgradeAcks = %llu\n", i, cgm_stat_container->l2_TotalUpgradeAcks[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalUpgradeInvals = %llu\n", i, cgm_stat_container->l2_TotalUpgradeInvals[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalWriteBlocks = %llu\n", i, cgm_stat_container->l2_TotalWriteBlocks[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_AveCyclesPerAdvance = %0.2f\n", i, ((double) cgm_stat_container->l2_Occupancy[i]/ (double) cgm_stat_container->l2_TotalAdvances[i]));
		if(cgm_stat_container->stats_type == systemStats)
		{
			CGM_STATS(cgm_stats_file, "l2_%d_OccupancyPct = %0.2f\n", i, ((double) cgm_stat_container->l2_Occupancy[i]/(double)P_TIME)*100);
		}
		else if (cgm_stat_container->stats_type == parallelSection)
		{
			CGM_STATS(cgm_stats_file, "l2_%d_OccupancyPct = %0.2f\n", i, ((double)cgm_stat_container->l2_Occupancy[i]/(double)cgm_stat_container->total_parallel_section_cycles)*100);
		}
		else
		{
			fatal("cache_dump_stats(): bad container type\n");
		}

		CGM_STATS(cgm_stats_file, "l2_%d_TotalAdvances = %llu\n", i, cgm_stat_container->l2_TotalAdvances[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalAccesses = %llu\n", i, cgm_stat_container->l2_TotalAcesses[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalHits = %llu\n", i, (cgm_stat_container->l2_TotalAcesses[i] - cgm_stat_container->l2_TotalMisses[i]));
		CGM_STATS(cgm_stats_file, "l2_%d_TotalMisses = %llu\n", i, cgm_stat_container->l2_TotalMisses[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalLoadMisses = %llu\n", i, cgm_stat_container->l2_LoadMisses[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_MissRate = %0.2f\n", i,
				(double) (cgm_stat_container->l2_TotalMisses[i])/(double) (cgm_stat_container->l2_TotalAcesses[i] - cgm_stat_container->l2_TotalMisses[i]));
		CGM_STATS(cgm_stats_file, "l2_%d_TotalReads = %llu\n", i, cgm_stat_container->l2_TotalReads[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalReadMisses = %llu\n", i, cgm_stat_container->l2_TotalReadMisses[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_ReadMissRate = %0.2f\n", i, ((double) cgm_stat_container->l2_TotalReadMisses[i] / (double) cgm_stat_container->l2_TotalReads[i]));
		CGM_STATS(cgm_stats_file, "l2_%d_TotalWrites = %llu\n", i, cgm_stat_container->l2_TotalWrites[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalWriteMisses = %llu\n", i, cgm_stat_container->l2_TotalWriteMisses[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_WriteMissRate = %0.2f\n", i, ((double) cgm_stat_container->l2_TotalWriteMisses[i] / (double) cgm_stat_container->l2_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l2_%d_TotalGets = %llu\n", i, cgm_stat_container->l2_TotalGets[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalGet = %llu\n", i, cgm_stat_container->l2_TotalGet[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalGetx = %llu\n", i, cgm_stat_container->l2_TotalGetx[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_TotalUpgrades = %llu\n", i, cgm_stat_container->l2_TotalUpgrades[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_GetsMissRate = %0.2f\n", i, ((double) cgm_stat_container->l2_TotalGets[i] / (double) cgm_stat_container->l2_TotalReads[i]));
		CGM_STATS(cgm_stats_file, "l2_%d_GetMissRate = %0.2f\n", i, ((double) cgm_stat_container->l2_TotalGet[i] / (double) cgm_stat_container->l2_TotalReads[i]));
		CGM_STATS(cgm_stats_file, "l2_%d_GetxMissRate = %0.2f\n", i, ((double) cgm_stat_container->l2_TotalGetx[i] / (double) cgm_stat_container->l2_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l2_%d_UpgradeMissRate = %0.2f\n", i, ((double) cgm_stat_container->l2_TotalUpgrades[i] / (double) cgm_stat_container->l2_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l2_%d_TotalWriteBacks = %llu\n", i, cgm_stat_container->l2_TotalWriteBacks[i]);
		blocks_written = cgm_cache_get_block_usage(&l2_caches[i]);
		CGM_STATS(cgm_stats_file, "l2_%d_CacheUtilization = %0.2f\n", i, ((double) blocks_written)/(double) (l2_caches[i].num_sets * l2_caches[i].assoc));
		/*CGM_STATS(cgm_stats_file, "\n");*/

		/*CGM_STATS(cgm_stats_file, "[L3_Cache_%d]\n", i);*/
		CGM_STATS(cgm_stats_file, "l3_%d_Occupancy = %llu\n", i, cgm_stat_container->l3_Occupancy[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_Stalls = %llu\n", i, cgm_stat_container->l3_Stalls[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_CoalescePut = %llu\n", i, cgm_stat_container->l3_CoalescePut[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_CoalesceGet = %llu\n", i, cgm_stat_container->l3_CoalesceGet[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_WbMerges = %llu\n", i, cgm_stat_container->l3_WbMerges[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_MergeRetries = %llu\n", i, cgm_stat_container->l3_MergeRetries[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_EvictInv = %llu\n", i, cgm_stat_container->l3_EvictInv[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_WbRecieved = %llu\n", i, cgm_stat_container->l3_TotalWriteBackRecieved[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_WbSent = %llu\n", i, cgm_stat_container->l3_TotalWriteBackSent[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_SharingWbSent = %llu\n", i, cgm_stat_container->l3_TotalSharingWriteBackSent[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_WbDropped = %llu\n", i, cgm_stat_container->l3_TotalWriteBackDropped[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalDowngrades = %llu\n", i, cgm_stat_container->l3_TotalDowngrades[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalGetxFwdInvals = %llu\n", i, cgm_stat_container->l3_TotalGetxFwdInvals[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalUpgradeAcks = %llu\n", i, cgm_stat_container->l3_TotalUpgradeAcks[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalUpgradeInvals = %llu\n", i, cgm_stat_container->l3_TotalUpgradeInvals[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalWriteBlocks = %llu\n", i, cgm_stat_container->l3_TotalWriteBlocks[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_AveCyclesPerAdvance = %0.2f\n", i, ((double) cgm_stat_container->l3_Occupancy[i]/ (double) cgm_stat_container->l3_TotalAdvances[i]));
		if(cgm_stat_container->stats_type == systemStats)
		{
			CGM_STATS(cgm_stats_file, "l3_%d_OccupancyPct = %0.2f\n", i, ((double)cgm_stat_container->l3_Occupancy[i]/(double)P_TIME)*100);
		}
		else if (cgm_stat_container->stats_type == parallelSection)
		{
			CGM_STATS(cgm_stats_file, "l3_%d_OccupancyPct = %0.2f\n", i, ((double) cgm_stat_container->l3_Occupancy[i]/(double) cgm_stat_container->total_parallel_section_cycles)*100);
		}
		else
		{
			fatal("cache_dump_stats(): bad container type\n");
		}

		CGM_STATS(cgm_stats_file, "l3_%d_TotalAdvances = %llu\n", i, cgm_stat_container->l3_TotalAdvances[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalAccesses = %llu\n", i, cgm_stat_container->l3_TotalAcesses[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalHits = %llu\n", i, (cgm_stat_container->l3_TotalAcesses[i] - cgm_stat_container->l3_TotalMisses[i]));
		CGM_STATS(cgm_stats_file, "l3_%d_TotalMisses = %llu\n", i, cgm_stat_container->l3_TotalMisses[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_UpgradeMisses = %llu\n", i, cgm_stat_container->l3_UpgradeMisses[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_MissRate = %0.2f\n", i,
				(double) (cgm_stat_container->l3_TotalMisses[i])/(double) (cgm_stat_container->l3_TotalAcesses[i] - cgm_stat_container->l3_TotalMisses[i]));
		CGM_STATS(cgm_stats_file, "l3_%d_TotalReads = %llu\n", i, cgm_stat_container->l3_TotalReads[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalReadMisses = %llu\n", i, cgm_stat_container->l3_TotalReadMisses[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_ReadMissRate = %0.2f\n", i, ((double) cgm_stat_container->l3_TotalReadMisses[i] / (double) cgm_stat_container->l3_TotalReads[i]));
		CGM_STATS(cgm_stats_file, "l3_%d_TotalWrites = %llu\n", i, cgm_stat_container->l3_TotalWrites[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalWriteMisses = %llu\n", i, cgm_stat_container->l3_TotalWriteMisses[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_WriteMissRate = %0.2f\n", i, ((double) cgm_stat_container->l3_TotalWriteMisses[i] / (double) cgm_stat_container->l3_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l3_%d_TotalGets = %llu\n", i, cgm_stat_container->l3_TotalGets[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalGet = %llu\n", i, cgm_stat_container->l3_TotalGet[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalGetx = %llu\n", i, cgm_stat_container->l3_TotalGetx[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_TotalUpgrades = %llu\n", i, cgm_stat_container->l3_TotalUpgrades[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_GetsMissRate = %0.2f\n", i, ((double) cgm_stat_container->l3_TotalGets[i] / (double) cgm_stat_container->l3_TotalReads[i]));
		CGM_STATS(cgm_stats_file, "l3_%d_GetMissRate = %0.2f\n", i, ((double) cgm_stat_container->l3_TotalGet[i] / (double) cgm_stat_container->l3_TotalReads[i]));
		CGM_STATS(cgm_stats_file, "l3_%d_GetxMissRate = %0.2f\n", i, ((double) cgm_stat_container->l3_TotalGetx[i] / (double) cgm_stat_container->l3_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l3_%d_UpgradeMissRate = %0.2f\n", i, ((double) cgm_stat_container->l3_TotalUpgrades[i] / (double) cgm_stat_container->l3_TotalWrites[i]));
		CGM_STATS(cgm_stats_file, "l3_%d_TotalWriteBacks = %llu\n", i, cgm_stat_container->l3_TotalWriteBacks[i]);
		blocks_written = cgm_cache_get_block_usage(&l3_caches[i]);
		CGM_STATS(cgm_stats_file, "l3_%d_CacheUtilization = %0.2f\n", i, ((double) blocks_written)/(double) (l3_caches[i].num_sets * l3_caches[i].assoc));
		/*CGM_STATS(cgm_stats_file, "\n");*/
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

	long long occ_start = 0;

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

		/*stats*/
		occ_start = P_TIME;

		//peak at a message from the input queues.
		message_packet = cache_get_message(&(l1_i_caches[my_pid]));

		/*star todo fix this, these should be related to the message type.
		This can be done more efficiently than a caret blanch stall */
		if (message_packet == NULL || !cache_can_access_Tx_bottom(&(l1_i_caches[my_pid])))
		{
			//the cache state is preventing the cache from working this cycle stall
			l1_i_caches[my_pid].Stalls++;

			//warning("l1_i_cache_ctrl(): %s stalling \n", l1_i_caches[my_pid].name);

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

			/*stats*/

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

		/*stats occupancy*/
		l1_i_caches[my_pid].Occupancy += (P_TIME - occ_start);
	}
	/* should never get here*/
	fatal("l1_i_cache_ctrl task is broken\n");
	return;
}

void l1_d_cache_ctrl(void){

	int my_pid = l1_d_pid++;
	int num_cores = x86_cpu_num_cores;
	//int num_cus = si_gpu_num_compute_units;
	long long step = 1;

	struct cgm_packet_t *message_packet;
	/*struct cgm_packet_t *wb_packet;*/
	enum cgm_access_kind_t access_type;
	long long access_id = 0;
	long long occ_start = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{
		//wait here until there is a job to do.
		await(&l1_d_cache[my_pid], step);

		l1_d_caches[my_pid].TotalAdvances = l1_d_cache[my_pid].count;
		occ_start = P_TIME;

		//get the message out of the queue
		message_packet = cache_get_message(&(l1_d_caches[my_pid]));

		//star todo this can be refined a lot.
		if (message_packet == NULL || !cache_can_access_Tx_bottom(&(l1_d_caches[my_pid])))
		{
			//the cache state is preventing the cache from working this cycle stall.
			l1_d_caches[my_pid].Stalls++;

			//warning("l1_d_cache_ctrl(): %s stalling cycle %llu\n", l1_d_caches[my_pid].name, P_TIME);
			P_PAUSE(1);
		}
		else
		{
			step++;

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
			else if (access_type == cgm_access_puts || access_type == cgm_access_putx
					|| access_type == cgm_access_put_clnx || access_type == cgm_access_upgrade_putx)
			{

				if(l1_d_caches[my_pid].l1_d_write_block(&(l1_d_caches[my_pid]), message_packet))
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
			else if (access_type == cgm_access_flush_block)
			{
				//Call back function (cgm_mesi_l1_d_inval)
				l1_d_caches[my_pid].l1_d_flush_block(&(l1_d_caches[my_pid]), message_packet);

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
			else if (access_type == cgm_access_get_nack)
			{
				//Call back function (cgm_mesi_l1_d_downgrade)
				l1_d_caches[my_pid].l1_d_load_nack(&(l1_d_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_getx_nack)
			{
				//Call back function (cgm_mesi_l1_d_downgrade)
				l1_d_caches[my_pid].l1_d_store_nack(&(l1_d_caches[my_pid]), message_packet);
			}
			else
			{
				fatal("l1_d_cache_ctrl(): %s access_id %llu bad access type %s at cycle %llu\n",
						l1_d_caches[my_pid].name, access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}

		/*stats occupancy*/
		l1_d_caches[my_pid].Occupancy += (P_TIME - occ_start);

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

	long long occ_start = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	while(1)
	{
		/*wait here until there is a job to do.*/
		await(&l2_cache[my_pid], step);

		/*stats*/
		l2_caches[my_pid].TotalAdvances = l2_cache[my_pid].count;
		occ_start = P_TIME;

		//check the top or bottom rx queues for messages.
		message_packet = cache_get_message(&(l2_caches[my_pid]));

		if (message_packet == NULL || !cache_can_access_Tx_bottom(&(l2_caches[my_pid])) || !cache_can_access_Tx_top(&(l2_caches[my_pid])))
		{
			//the cache state is preventing the cache from working this cycle stall.
			//warning("l2_cache_ctrl(): %s stalling \n", l2_caches[my_pid].name);
			l2_caches[my_pid].Stalls++;

			//warning("l2_cache_ctrl(): %s stalling \n", l2_caches[my_pid].name);
			P_PAUSE(1);
		}
		else
		{

			/*if(list_count(l2_caches[my_pid].Tx_queue_bottom) >= QueueSize)
				fatal("here\n");*/

			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			wd_current_set = message_packet->set;
			wd_current_tag = message_packet->tag;

			/*printf("%s running id %llu type %s cycle %llu\n",
					l2_caches[my_pid].name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);*/

			/*stats*/
			if(message_packet->access_type == cgm_access_gets)
				mem_system_stats->l2_total_fetch_requests++;
			else if(message_packet->access_type == cgm_access_get)
				mem_system_stats->l2_total_load_requests++;
			else if(message_packet->access_type == cgm_access_getx)
				mem_system_stats->l2_total_store_requests++;



			if(access_type == cgm_access_gets || access_type == cgm_access_fetch_retry)//
			{
				//Call back function (cgm_mesi_l2_gets)
				l2_caches[my_pid].l2_gets(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_gets_++;
			}
			else if(access_type == cgm_access_get || access_type == cgm_access_load_retry)//
			{
				//Call back function (cgm_mesi_l2_get)
				l2_caches[my_pid].l2_get(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_get_++;
			}
			else if(access_type == cgm_access_getx || access_type == cgm_access_store_retry)//
			{
				//Call back function (cgm_mesi_l2_getx)

				//will run again if getx results in upgrade request at L2 level.
				if(!l2_caches[my_pid].l2_getx(&(l2_caches[my_pid]), message_packet))
					step--;

				/*stats*/
				l2_caches[my_pid].l2_getx_++;
			}
			else if(access_type == cgm_access_write_back)//
			{
				//Call back function (cgm_mesi_l2_write_back)

				//if the write back was internally scheduled decrement the counter.
				if(!l2_caches[my_pid].l2_write_back(&(l2_caches[my_pid]), message_packet))
					step--;

				/*stats*/
				l2_caches[my_pid].l2_write_back_++;
			}
			else if(access_type == cgm_access_puts || access_type == cgm_access_putx || access_type == cgm_access_put_clnx)//
			{
				//Call back function (cgm_mesi_l2_put)
				l2_caches[my_pid].l2_write_block(&(l2_caches[my_pid]), message_packet);

				//run again
				step--;

				/*stats*/
				l2_caches[my_pid].l2_write_block_++;
			}
			else if(access_type == cgm_access_downgrade_ack)//
			{
				//Call back function (cgm_mesi_l2_downgrade_ack)
				l2_caches[my_pid].l2_downgrade_ack(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_downgrade_ack_++;
			}
			else if(access_type == cgm_access_get_nack)//
			{
				//Call back function (cgm_mesi_l2_downgrade_ack)
				l2_caches[my_pid].l2_get_nack(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_get_nack_++;
			}
			else if(access_type == cgm_access_getx_nack)//
			{
				//Call back function (cgm_mesi_l2_downgrade_ack)
				l2_caches[my_pid].l2_getx_nack(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_getx_nack_++;
			}
			else if(access_type == cgm_access_get_fwd)//
			{
				//Call back function (cgm_mesi_l2_get_fwd)
				l2_caches[my_pid].l2_get_fwd(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_get_fwd_++;
			}
			else if(access_type == cgm_access_downgrade_nack)//
			{
				//Call back function (cgm_mesi_l2_get_fwd)
				l2_caches[my_pid].l2_downgrade_nack(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_downgrade_nack_++;
			}
			else if(access_type == cgm_access_getx_fwd || message_packet->access_type == cgm_access_upgrade_getx_fwd)//
			{
				//Call back function (cgm_mesi_l2_getx_fwd)
				l2_caches[my_pid].l2_getx_fwd(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_getx_fwd_++;
			}
			else if(access_type == cgm_access_getx_fwd_inval_ack)//
			{
				//Call back function (cgm_mesi_l2_getx_fwd_inval_ack)
				l2_caches[my_pid].l2_getx_fwd_inval_ack(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_getx_fwd_inval_ack_++;
			}
			else if(access_type == cgm_access_getx_fwd_nack)//
			{
				//Call back function (cgm_mesi_l2_getx_fwd_inval_ack)
				l2_caches[my_pid].l2_getx_fwd_nack(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_getx_fwd_nack_++;
			}
			else if(access_type == cgm_access_upgrade)//
			{
				//Call back function (cgm_mesi_l2_getx_fwd_inval_ack)
				if(!l2_caches[my_pid].l2_upgrade(&(l2_caches[my_pid]), message_packet))
					step--;

				/*stats*/
				l2_caches[my_pid].l2_upgrade_++;
			}
			else if (access_type == cgm_access_upgrade_ack)//
			{
				//Call back function (cgm_mesi_l2_upgrade_nack)
				if(!l2_caches[my_pid].l2_upgrade_ack(&(l2_caches[my_pid]), message_packet))
					step--;

				/*stats*/
				l2_caches[my_pid].l2_upgrade_ack_++;
			}
			else if (access_type == cgm_access_upgrade_nack)//
			{
				//Call back function (cgm_mesi_l2_getx_fwd_inval_ack)
				l2_caches[my_pid].l2_upgrade_nack(&(l2_caches[my_pid]), message_packet);

				step--;

				/*stats*/
				l2_caches[my_pid].l2_upgrade_nack_++;
			}
			else if(access_type == cgm_access_upgrade_putx_n)//
			{
				//Call back function (cgm_mesi_l2_upgrade_putx_n)
				l2_caches[my_pid].l2_upgrade_putx_n(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_upgrade_putx_n_++;
			}
			else if (access_type == cgm_access_upgrade_inval)//
			{
				//Call back function (cgm_mesi_l2_upgrade_inval)
				l2_caches[my_pid].l2_upgrade_inval(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_upgrade_inval_++;
			}
			else if (access_type == cgm_access_flush_block)
			{
					//Call back function (cgm_mesi_l2_inval)
				l2_caches[my_pid].l2_flush_block(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_flush_block_++;
			}
			else if (access_type == cgm_access_flush_block_ack)
			{
				//Call back function (cgm_mesi_l2_inval_ack)
				l2_caches[my_pid].l2_flush_block_ack(&(l2_caches[my_pid]), message_packet);

				/*stats*/
				l2_caches[my_pid].l2_flush_block_ack_++;
			}
			else
			{
				fatal("l2_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
						access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}

		/*stats occupancy*/
		l2_caches[my_pid].Occupancy += (P_TIME - occ_start);

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

	long long occ_start = 0;

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

		l3_caches[my_pid].TotalAdvances = l3_cache[my_pid].count;
		occ_start = P_TIME;

		//get the message out of the queue
		message_packet = cache_get_message(&(l3_caches[my_pid]));

		if (message_packet == NULL || !cache_can_access_Tx_bottom(&(l3_caches[my_pid])) || !cache_can_access_Tx_top(&(l3_caches[my_pid])))
		{
			//star todo add more detail here look for replies/request, process them is there respective queue is ok.
			//the cache state is preventing the cache from working this cycle stall.
			//warning("l3_cache_ctrl(): %s stalling \n", l3_caches[my_pid].name);


			l3_caches[my_pid].Stalls++;

			//warning("l3_cache_ctrl(): %s stalling \n", l3_caches[my_pid].name);

			P_PAUSE(1);
		}
		else
		{
			step++;


			access_type = message_packet->access_type;
			access_id = message_packet->access_id;


			if(message_packet->access_type == cgm_access_gets)
				mem_system_stats->l3_total_fetch_requests++;
			else if(message_packet->access_type == cgm_access_get)
				mem_system_stats->l3_total_load_requests++;
			else if(message_packet->access_type == cgm_access_getx)
				mem_system_stats->l3_total_store_requests++;

			/*printf("%s running id %llu type %s cycle %llu\n",
					l3_caches[my_pid].name, message_packet->access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);*/

			if(access_type == cgm_access_gets || access_type == cgm_access_fetch_retry)
			{
				//via call back function (cgm_mesi_l3_gets)
				l3_caches[my_pid].l3_gets(&(l3_caches[my_pid]), message_packet);

				/*stats*/
				l3_caches[my_pid].l3_gets_++;

			}
			else if(access_type == cgm_access_get || access_type == cgm_access_load_retry)
			{
				//via call back function (cgm_mesi_l3_get)
				l3_caches[my_pid].l3_get(&(l3_caches[my_pid]), message_packet);

				/*stats*/
				l3_caches[my_pid].l3_get_++;
			}
			else if(access_type == cgm_access_getx || access_type == cgm_access_store_retry)
			{
				//via call back function (cgm_mesi_l3_getx)
				l3_caches[my_pid].l3_getx(&(l3_caches[my_pid]), message_packet);

				/*stats*/
				l3_caches[my_pid].l3_getx_++;
			}
			else if(access_type == cgm_access_write_back)
			{
				//via call back function (cgm_mesi_l3_write_back)
				if(!l3_caches[my_pid].l3_write_back(&(l3_caches[my_pid]), message_packet))
					step--;

				l3_caches[my_pid].l3_write_back_++;
			}
			else if(access_type == cgm_access_flush_block_ack)
			{
				//via call back function (cgm_mesi_l3_get_fwd_nack)
				l3_caches[my_pid].l3_flush_block_ack(&(l3_caches[my_pid]), message_packet);
				//run again and pull the message_packet as a new access

				l3_caches[my_pid].l3_flush_block_ack_++;
			}
			else if (access_type == cgm_access_mc_put)
			{
				//via call back function (cgm_mesi_l3_write_block)
				l3_caches[my_pid].l3_write_block(&(l3_caches[my_pid]), message_packet);

				//retry state set so run again
				step--;

				l3_caches[my_pid].l3_write_block_++;
			}
			else if(access_type == cgm_access_downgrade_ack)
			{
				//via call back function (cgm_mesi_l3_downgrade_ack)
				l3_caches[my_pid].l3_downgrade_ack(&(l3_caches[my_pid]), message_packet);

				l3_caches[my_pid].l3_downgrade_ack_++;
			}
			else if(access_type == cgm_access_downgrade_nack)
			{
				//via call back function (cgm_mesi_l3_downgrade_nack)
				l3_caches[my_pid].l3_downgrade_nack(&(l3_caches[my_pid]), message_packet);

				l3_caches[my_pid].l3_downgrade_nack_++;
			}
			else if(access_type == cgm_access_getx_fwd_ack)
			{
				//via call back function (cgm_mesi_l3_getx_fwd_ack)
				l3_caches[my_pid].l3_getx_fwd_ack(&(l3_caches[my_pid]), message_packet);

				l3_caches[my_pid].l3_getx_fwd_ack_++;
			}
			else if(access_type == cgm_access_getx_fwd_nack)
			{
				//via call back function (cgm_mesi_l3_get_fwd_nack)
				l3_caches[my_pid].l3_getx_fwd_nack(&(l3_caches[my_pid]), message_packet);

				l3_caches[my_pid].l3_getx_fwd_nack_++;
			}
			else if(access_type == cgm_access_getx_fwd_upgrade_nack)
			{
				//via call back function (cgm_mesi_l3_get_fwd_nack)
				l3_caches[my_pid].l3_getx_fwd_upgrade_nack(&(l3_caches[my_pid]), message_packet);
				//run again and pull the message_packet as a new access

				l3_caches[my_pid].l3_getx_fwd_upgrade_nack_++;
			}
			else if(access_type == cgm_access_get_fwd_upgrade_nack)
			{
				//via call back function (cgm_mesi_l3_get_fwd_nack)
				l3_caches[my_pid].l3_get_fwd_upgrade_nack(&(l3_caches[my_pid]), message_packet);

				l3_caches[my_pid].l3_get_fwd_upgrade_nack_++;

			}
			else if(access_type == cgm_access_upgrade)
			{
				//via call back function (cgm_mesi_l3_upgrade)
				if(!l3_caches[my_pid].l3_upgrade(&(l3_caches[my_pid]), message_packet))
					step--;

				l3_caches[my_pid].l3_upgrade_++;
			}
			/*else if(access_type == cgm_access_upgrade_ack)
			{
				//via call back function (cgm_mesi_l3_upgrade)
				l3_caches[my_pid].l3_upgrade_ack(&(l3_caches[my_pid]), message_packet);


			}*/
			else
			{
				fatal("l3_cache_ctrl_0(): access_id %llu bad access type %s at cycle %llu\n",
						access_id, str_map_value(&cgm_mem_access_strn_map, message_packet->access_type), P_TIME);
			}
		}

		//assert((P_TIME - occ_start) == 48);

		/*stats occupancy*/
		l3_caches[my_pid].Occupancy += (P_TIME - occ_start);

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

		if (message_packet == NULL || !cache_can_access_top(&gpu_l2_caches[cgm_gpu_cache_map(&gpu_s_caches[my_pid], message_packet->address)]))
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

			if (access_type == cgm_access_load || access_type == cgm_access_load_retry)
			{
				//Call back function (cgm_nc_gpu_s_load)
				gpu_s_caches[my_pid].gpu_s_load(&(gpu_s_caches[my_pid]), message_packet);
			}
			//star todo this is wrong change this to put NOT puts
			else if (access_type == cgm_access_put_clnx)
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

		//star todo fix this
		if (message_packet == NULL || !cache_can_access_Tx_bottom(&(gpu_v_caches[my_pid])))
		{
			P_PAUSE(1);
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

			if(access_type == cgm_access_load || access_type == cgm_access_load_retry) // || access_type == cgm_access_loadx_retry) //star remove loadx
			{
				//Call back function (gpu_cache_access_load)
				gpu_v_caches[my_pid].gpu_v_load(&(gpu_v_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_store || access_type == cgm_access_nc_store || access_type == cgm_access_store_retry) // || access_type == cgm_access_storex_retry) //star remove storex
			{
				//Call back function (gpu_l1_cache_access_store)
				gpu_v_caches[my_pid].gpu_v_store(&(gpu_v_caches[my_pid]), message_packet);
			}
			else if (access_type == cgm_access_put_clnx || access_type == cgm_access_putx)
			{
				//Call back function (gpu_cache_access_put)
				gpu_v_caches[my_pid].gpu_v_write_block(&(gpu_v_caches[my_pid]), message_packet);

				//entered retry state run again.
				step--;
			}
			else if (access_type == cgm_access_inv)
			{
				gpu_v_caches[my_pid].gpu_v_inval(&(gpu_v_caches[my_pid]), message_packet);
			}
			/*else if (access_type == cgm_access_flush_block)
			{
				//Call back function (cgm_mesi_l1_d_inval)
				l1_d_caches[my_pid].l1_d_flush_block(&(l1_d_caches[my_pid]), message_packet);

			}*/
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

		if (message_packet == NULL || !cache_can_access_Tx_bottom(&(gpu_l2_caches[my_pid])) || !cache_can_access_Tx_top(&(gpu_l2_caches[my_pid])))
		{
			//the cache state is preventing the cache from working this cycle stall.
			//printf("%s stalling cycle %llu\n", gpu_l2_caches[my_pid].name, P_TIME);
			/*printf("stalling\n");*/
			gpu_l2_caches[my_pid].Stalls++;
			/*future_advance(&gpu_l2_cache[my_pid], etime.count + 2);*/
			P_PAUSE(1);
		}
		else
		{

			step++;

			access_type = message_packet->access_type;
			access_id = message_packet->access_id;

			if(access_type == cgm_access_get || access_type == cgm_access_load_retry)
			{
				//Call back function (gpu_cache_access_get)
				gpu_l2_caches[my_pid].gpu_l2_get(&gpu_l2_caches[my_pid], message_packet);
			}
			else if(access_type == cgm_access_getx || access_type == cgm_access_store_retry)
			{
				//Call back function (cgm_mesi_gpu_l2_getx)
				gpu_l2_caches[my_pid].gpu_l2_getx(&gpu_l2_caches[my_pid], message_packet);
			}
			else if(access_type == cgm_access_mc_put || access_type == cgm_access_putx)
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
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l1_i_caches[my_pid].cache_io_down_ec, step);

		if(list_count(l2_caches[my_pid].Rx_queue_top) > QueueSize)
		{
			P_PAUSE(1);
		}
		else
		{
			step++;

			message_packet = list_dequeue(l1_i_caches[my_pid].Tx_queue_bottom);
			assert(message_packet);
			assert(message_packet->access_type == cgm_access_gets);

			transfer_time = (message_packet->size/l1_i_caches[my_pid].bus_width);

			if(transfer_time == 0)
				transfer_time = 1;

			P_PAUSE(transfer_time);

			//drop into the next virtual lane correct queue.
			list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);
			advance(&l2_cache[my_pid]);

			/*stats*/

			l2_caches[my_pid].TotalAcesses++;
			assert(message_packet->access_type == cgm_access_gets);
			if(message_packet->access_type == cgm_access_gets)
				l2_caches[my_pid].TotalReads++;
		}
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

		message_packet = list_get(l1_d_caches[my_pid].Tx_queue_bottom, 0);
		assert(message_packet);

		transfer_time = (message_packet->size/l1_d_caches[my_pid].bus_width);

		if(transfer_time == 0)
			transfer_time = 1;

		//drop into the next correct virtual lane/queue.
		if(message_packet->access_type == cgm_access_get || message_packet->access_type == cgm_access_getx
				|| message_packet->access_type == cgm_access_upgrade)
		{

			if(list_count(l2_caches[my_pid].Rx_queue_top) >= QueueSize)
			{
				P_PAUSE(1);
			}
			else
			{
				step++;

				P_PAUSE(transfer_time);

				message_packet = list_remove(l1_d_caches[my_pid].Tx_queue_bottom, message_packet);
				list_enqueue(l2_caches[my_pid].Rx_queue_top, message_packet);
				advance(&l2_cache[my_pid]);

				/*stats*/
				l2_caches[my_pid].TotalAcesses++;

				if(message_packet->access_type == cgm_access_get)
					l2_caches[my_pid].TotalReads++;

				if (message_packet->access_type == cgm_access_getx || message_packet->access_type == cgm_access_upgrade)
					l2_caches[my_pid].TotalWrites++;
			}
		}
		else if(message_packet->access_type == cgm_access_flush_block_ack || message_packet->access_type == cgm_access_downgrade_ack
				|| message_packet->access_type == cgm_access_getx_fwd_inval_ack || message_packet->access_type == cgm_access_write_back)
		{

			if(list_count(l2_caches[my_pid].Coherance_Rx_queue) >= QueueSize)
			{
				P_PAUSE(1);
			}
			else
			{
				step++;

				P_PAUSE(transfer_time);

				message_packet = list_remove(l1_d_caches[my_pid].Tx_queue_bottom, message_packet);
				list_enqueue(l2_caches[my_pid].Coherance_Rx_queue, message_packet);
				advance(&l2_cache[my_pid]);

				/*stats*/
				l2_caches[my_pid].TotalAcesses++;
			}
		}
		else
		{
			fatal("l1_d_cache_down_io_ctrl(): invalid access type\n");
		}
	}

	fatal("l1_d_cache_down_io_ctrl(): out of while loop\n");

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

		message_packet = list_get(l2_caches[my_pid].Tx_queue_top, 0);
		assert(message_packet);

		//star todo fix this we need a top and bottom bus_width
		transfer_time = (message_packet->size/l1_i_caches[my_pid].bus_width);

		if(transfer_time == 0)
			transfer_time = 1;

		//drop into the correct l1 cache queue and lane.
		if (message_packet->cpu_access_type == cgm_access_fetch)
		{
			assert(message_packet->access_type == cgm_access_puts);

			if(list_count(l1_i_caches[my_pid].Rx_queue_bottom) > QueueSize)
			{
				P_PAUSE(1);
			}
			else
			{
				step++;

				P_PAUSE(transfer_time);

				message_packet = list_remove(l2_caches[my_pid].Tx_queue_top, message_packet);
				list_enqueue(l1_i_caches[my_pid].Rx_queue_bottom, message_packet);
				advance(&l1_i_cache[my_pid]);
			}
		}
		else if (message_packet->cpu_access_type == cgm_access_load || message_packet->cpu_access_type == cgm_access_store)
		{
			if(message_packet->access_type == cgm_access_puts || message_packet->access_type == cgm_access_putx
					|| message_packet->access_type == cgm_access_put_clnx || message_packet->access_type == cgm_access_get_nack
					|| message_packet->access_type == cgm_access_getx_nack)
			{

				if(list_count(l1_d_caches[my_pid].Rx_queue_bottom) > QueueSize)
				{
					P_PAUSE(1);
				}
				else
				{
					step++;

					P_PAUSE(transfer_time);

					message_packet = list_remove(l2_caches[my_pid].Tx_queue_top, message_packet);
					list_enqueue(l1_d_caches[my_pid].Rx_queue_bottom, message_packet);
					advance(&l1_d_cache[my_pid]);
				}
			}
			else if(message_packet->access_type == cgm_access_flush_block || message_packet->access_type == cgm_access_upgrade_ack
					|| message_packet->access_type == cgm_access_downgrade || message_packet->access_type == cgm_access_getx_fwd_inval
					|| message_packet->access_type == cgm_access_upgrade_inval)
			{

				if(list_count(l1_d_caches[my_pid].Coherance_Rx_queue) > QueueSize)
				{
					P_PAUSE(1);
				}
				else
				{
					step++;

					P_PAUSE(transfer_time);

					message_packet = list_remove(l2_caches[my_pid].Tx_queue_top, message_packet);
					list_enqueue(l1_d_caches[my_pid].Coherance_Rx_queue, message_packet);
					advance(&l1_d_cache[my_pid]);
				}
			}
			else
			{
				fatal("l2_cache_up_io_ctrl(): bad access type\n");
			}
		}
		else
		{
			fatal("l2_cache_up_io_ctrl(): bad cpu access type %s\n", str_map_value(&cgm_mem_access_strn_map, message_packet->access_type));
		}
	}
	return;
}

void l2_cache_down_io_ctrl(void){

	int my_pid = l2_down_io_pid++;
	long long step = 1;

	struct cgm_packet_t *message_packet = NULL;
	struct cache_t *l3_cache_ptr = NULL;

	int transfer_time = 0;
	long long queue_depth = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l2_caches[my_pid].cache_io_down_ec, step);

		if(list_count(switches[my_pid].north_queue) >= QueueSize)
		{
			P_PAUSE(1);
		}
		else
		{
			step++;

			message_packet = list_dequeue(l2_caches[my_pid].Tx_queue_bottom);
			assert(message_packet);

			/*access_id = message_packet->access_id;*/

			//star todo fix this we need a top and bottom bus_width
			transfer_time = (message_packet->size/l2_caches[my_pid].bus_width);

			if(transfer_time == 0)
				transfer_time = 1;

			SYSTEM_PAUSE(transfer_time);

			//drop in to the switch queue
			list_enqueue(switches[my_pid].north_queue, message_packet);
			advance(&switches_ec[my_pid]);

			/*stats*/
			//store_stat_bandwidth(bytes_tx, my_pid, transfer_time, l2_caches[my_pid].bus_width);

			switches[my_pid].north_rx_inserts++;
			queue_depth = list_count(switches[my_pid].north_queue);

			/*max depth*/
			if(queue_depth > switches[my_pid].north_rxqueue_max_depth)
				switches[my_pid].north_rxqueue_max_depth = queue_depth;

			/*ave depth = ((old count * old data) + next data) / next count*/
			switches[my_pid].north_rxqueue_ave_depth =
				((((double) switches[my_pid].north_rx_inserts - 1) * switches[my_pid].north_rxqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].north_rx_inserts;

			l3_cache_ptr = cgm_l3_cache_map(message_packet->set);
			assert(l3_cache_ptr);

			l3_cache_ptr->TotalAcesses++;
			if(message_packet->access_type == cgm_access_gets || message_packet->access_type == cgm_access_get)
			{
				l3_cache_ptr->TotalReads++;
			}
			else if(message_packet->access_type == cgm_access_getx || message_packet->access_type == cgm_access_upgrade)
			{
				l3_cache_ptr->TotalWrites++;
			}

		}

	}

	fatal("l2_cache_down_io_ctrl(): out of while loop\n");

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
	long long queue_depth = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l3_caches[my_pid].cache_io_up_ec, step);

		if(list_count(switches[my_pid].south_queue) > QueueSize)
		{
			SYSTEM_PAUSE(1);
		}
		else
		{
			step++;

			message_packet = list_dequeue(l3_caches[my_pid].Tx_queue_top);
			assert(message_packet);

			transfer_time = (message_packet->size/l3_caches[my_pid].bus_width);

			if(transfer_time == 0)
				transfer_time = 1;

			SYSTEM_PAUSE(transfer_time);

			/*if(message_packet->access_id == 9031342 || message_packet->access_id == 9031352)
			{
				warning("L3 sending %llu cycle %llu\n", message_packet->access_id, P_TIME);
				fflush(stderr);
			}*/

			//drop in to the switch queue
			list_enqueue(switches[my_pid].south_queue, message_packet);
			advance(&switches_ec[my_pid]);

			/*stats*/
			switches[my_pid].south_rx_inserts++;
			queue_depth = list_count(switches[my_pid].south_queue);
			/*max depth*/
			if(queue_depth > switches[my_pid].south_rxqueue_max_depth)
				switches[my_pid].south_rxqueue_max_depth = queue_depth;

			/*ave depth = ((old count * old data) + next data) / next count*/
			switches[my_pid].south_rxqueue_ave_depth =
				((((double) switches[my_pid].south_rx_inserts - 1) * switches[my_pid].south_rxqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].south_rx_inserts;
		}
	}

	fatal("l3_cache_up_io_ctrl(): out of while loop\n");

	return;

}

void l3_cache_down_io_ctrl(void){

	int my_pid = l3_down_io_pid++;
	long long step = 1;

	int transfer_time = 0;
	long long queue_depth = 0;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(l3_caches[my_pid].cache_io_down_ec, step);

		if(list_count(switches[my_pid].south_queue) >= QueueSize)
		{
			SYSTEM_PAUSE(1);
		}
		else
		{
			step++;

			message_packet = list_dequeue(l3_caches[my_pid].Tx_queue_bottom);
			assert(message_packet);

			//star todo fix this we need a top and bottom bus_width
			transfer_time = (message_packet->size/l3_caches[my_pid].bus_width);

			if(transfer_time == 0)
				transfer_time = 1;

			SYSTEM_PAUSE(transfer_time);

			//drop in to the switch queue
			list_enqueue(switches[my_pid].south_queue, message_packet);
			advance(&switches_ec[my_pid]);

			/*stats*/
			switches[my_pid].south_rx_inserts++;
			queue_depth = list_count(switches[my_pid].south_queue);
			/*max depth*/
			if(queue_depth > switches[my_pid].south_rxqueue_max_depth)
				switches[my_pid].south_rxqueue_max_depth = queue_depth;

			/*ave depth = ((old count * old data) + next data) / next count*/
			switches[my_pid].south_rxqueue_ave_depth =
				((((double) switches[my_pid].south_rx_inserts - 1) * switches[my_pid].south_rxqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].south_rx_inserts;

		}
	}

	fatal("l3_cache_down_io_ctrl(): out of while loop\n");
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

		fatal("l3 io down\n");

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
		list_enqueue(gpu_l2_caches[cgm_gpu_cache_map(&gpu_s_caches[my_pid], message_packet->address)].Rx_queue_top, message_packet);
		advance(&gpu_l2_cache[cgm_gpu_cache_map(&gpu_s_caches[my_pid], message_packet->address)]);
	}

	return;
}

void gpu_v_cache_down_io_ctrl(void){

	int my_pid = gpu_v_io_pid++;
	long long step = 1;
	struct cgm_packet_t *message_packet;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(gpu_v_caches[my_pid].cache_io_down_ec, step);

		message_packet = list_get(gpu_v_caches[my_pid].Tx_queue_bottom, 0);
		assert(message_packet);

		transfer_time = (message_packet->size/gpu_v_caches[my_pid].bus_width);

		if(transfer_time == 0)
			transfer_time = 1;


		//drop into the next correct virtual lane/queue.
		if(message_packet->access_type == cgm_access_get || message_packet->access_type == cgm_access_getx
				|| message_packet->access_type == cgm_access_upgrade)
		{
			if(list_count(gpu_l2_caches[cgm_gpu_cache_map(&gpu_v_caches[my_pid], message_packet->address)].Rx_queue_top) >= QueueSize)
			{
				P_PAUSE(1);
			}
			else
			{
				step++;

				P_PAUSE(transfer_time);

				message_packet = list_remove(gpu_v_caches[my_pid].Tx_queue_bottom, message_packet);
				list_enqueue(gpu_l2_caches[cgm_gpu_cache_map(&gpu_v_caches[my_pid], message_packet->address)].Rx_queue_top, message_packet);
				advance(&gpu_l2_cache[cgm_gpu_cache_map(&gpu_v_caches[my_pid], message_packet->address)]);

				/*stats*/
				gpu_l2_caches[cgm_gpu_cache_map(&gpu_v_caches[my_pid], message_packet->address)].TotalAcesses++;

				if(message_packet->access_type == cgm_access_get)
					gpu_l2_caches[cgm_gpu_cache_map(&gpu_v_caches[my_pid], message_packet->address)].TotalReads++;

				if (message_packet->access_type == cgm_access_getx || message_packet->access_type == cgm_access_upgrade)
					gpu_l2_caches[cgm_gpu_cache_map(&gpu_v_caches[my_pid], message_packet->address)].TotalWrites++;
			}
		}
		else if(message_packet->access_type == cgm_access_flush_block_ack || message_packet->access_type == cgm_access_downgrade_ack
				|| message_packet->access_type == cgm_access_getx_fwd_inval_ack || message_packet->access_type == cgm_access_write_back)
		{

			if(list_count(gpu_l2_caches[cgm_gpu_cache_map(&gpu_v_caches[my_pid], message_packet->address)].Coherance_Rx_queue) >= QueueSize)
			{
				P_PAUSE(1);
			}
			else
			{
				step++;

				P_PAUSE(transfer_time);

				message_packet = list_remove(gpu_v_caches[my_pid].Tx_queue_bottom, message_packet);
				list_enqueue(gpu_l2_caches[cgm_gpu_cache_map(&gpu_v_caches[my_pid], message_packet->address)].Coherance_Rx_queue, message_packet);
				advance(&gpu_l2_cache[cgm_gpu_cache_map(&gpu_v_caches[my_pid], message_packet->address)]);

				/*stats*/
				gpu_l2_caches[cgm_gpu_cache_map(&gpu_v_caches[my_pid], message_packet->address)].TotalAcesses++;
			}
		}
		else
		{
			fatal("gpu_v_cache_down_io_ctrl(): invalid access type\n");
		}
	}

	fatal("gpu_v_cache_down_io_ctrl(): out of while loop\n");

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

		message_packet = list_get(gpu_l2_caches[my_pid].Tx_queue_top, 0);
		assert(message_packet);

		transfer_time = (message_packet->size/gpu_v_caches[my_pid].bus_width);

		if(transfer_time == 0)
			transfer_time = 1;

		//drop into the correct l1 cache queue and lane.
		if(message_packet->access_type == cgm_access_puts || message_packet->access_type == cgm_access_putx
				|| message_packet->access_type == cgm_access_put_clnx || message_packet->access_type == cgm_access_get_nack
				|| message_packet->access_type == cgm_access_getx_nack)
		{

			if(list_count(gpu_v_caches[message_packet->l1_cache_id].Rx_queue_bottom) >= QueueSize)
			{
				P_PAUSE(1);
			}
			else
			{
				step++;

				P_PAUSE(transfer_time);

				message_packet = list_remove(gpu_l2_caches[my_pid].Tx_queue_top, message_packet);
				list_enqueue(gpu_v_caches[message_packet->l1_cache_id].Rx_queue_bottom, message_packet);
				advance(&gpu_v_cache[message_packet->l1_cache_id]);
			}
		}
		else if(message_packet->access_type == cgm_access_flush_block || message_packet->access_type == cgm_access_upgrade_ack
				|| message_packet->access_type == cgm_access_downgrade || message_packet->access_type == cgm_access_getx_fwd_inval
				|| message_packet->access_type == cgm_access_upgrade_inval)
		{

			if(list_count(gpu_v_caches[message_packet->l1_cache_id].Coherance_Rx_queue) > QueueSize)
			{
				P_PAUSE(1);
			}
			else
			{
				step++;

				P_PAUSE(transfer_time);

				message_packet = list_remove(gpu_l2_caches[my_pid].Tx_queue_top, message_packet);
				list_enqueue(gpu_v_caches[message_packet->l1_cache_id].Coherance_Rx_queue, message_packet);
				advance(&gpu_v_cache[message_packet->l1_cache_id]);
			}
		}
		else
		{
			fatal("gpu_l2_cache_up_io_ctrl(): bad access type\n");
		}
	}

	return;
}

void gpu_l2_cache_down_io_ctrl(void){

	int my_pid = gpu_l2_down_io_pid++;
	long long step = 1;
	struct cgm_packet_t *message_packet = NULL;
	struct cache_t *l3_cache_ptr = NULL;
	int transfer_time = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(gpu_l2_caches[my_pid].cache_io_down_ec, step);

		if(list_count(hub_iommu->Rx_queue_top[my_pid]) >= QueueSize)
		{
			P_PAUSE(1);
		}
		else
		{
			step++;

			message_packet = list_dequeue(gpu_l2_caches[my_pid].Tx_queue_bottom);
			assert(message_packet);


			transfer_time = (message_packet->size/gpu_l2_caches[my_pid].bus_width);

			if(transfer_time == 0)
				transfer_time = 1;

			SYSTEM_PAUSE(transfer_time);

			list_enqueue(hub_iommu->Rx_queue_top[my_pid], message_packet);

			advance(hub_iommu_ec);

			/*stats*/
			if(hub_iommu_connection_type == hub_to_l3)
			{
				l3_cache_ptr = cgm_l3_cache_map(message_packet->set);
				assert(l3_cache_ptr);
				l3_cache_ptr->TotalAcesses++;
			}
		}
	}

	return;
}


void cache_get_transient_block(struct cache_t *cache, struct cgm_packet_t *message_packet, int *cache_block_hit_ptr, int *cache_block_state_ptr){

	//similar to cache_get_block_status(), but returns the transient block way regardless of block state.
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

	//look for the block in the cache
	*(cache_block_hit_ptr) = cgm_cache_find_transient_block(cache, tag_ptr, set_ptr, offset_ptr, way_ptr, cache_block_state_ptr);

	//store the decode in the packet for now.
	message_packet->tag = tag;
	message_packet->set = set;
	message_packet->offset = offset;
	message_packet->way = way;


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

	//look for the block in the cache
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

/*void cache_access_stats(struct cache_t *cache, int hit, int state){

	cgm_cache_block_invalid = 0,
	cgm_cache_block_noncoherent,1
	cgm_cache_block_modified, 2
	cgm_cache_block_owned, 3
	cgm_cache_block_exclusive,4
	cgm_cache_block_shared, 5
	cgm_cache_block_transient,6
	cgm_cache_block_flush,6
	cgm_cache_block_null,7
	cgm_cache_block_state_num

	l1_i_cache_t,
	l1_d_cache_t,
	l2_cache_t,
	l3_cache_t,
	gpu_s_cache_t,
	gpu_v_cache_t,
	gpu_l2_cache_t

	//for all cache types
	assert(hit == 0 || hit == 1);
	if(hit == 0)
	{
		cache->TotalMisses++;
	}
	else if(hit == 1)
	{
		cache->TotalHits++;
	}

	//specific stats to collect
	switch(cache->cache_type)
	{

		case l1_i_cache_t:
			//don't need anything here for now...
			break;

		case l1_d_cache_t:

			if(hit == 0)
			{
				if(state == cgm_cache_block_invalid)
				{
					//

				}
			}

			break;

		case l2_cache_t:

			break;

		case l3_cache_t:

			break;

		case gpu_s_cache_t:

			break;

		case gpu_v_cache_t:

			break;

		case gpu_l2_cache_t:

			break;

		default:
			fatal("cache_access_stats() cache missing cache type\n");
			break;
	}

	return;
}*/

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

	/*stats*/
	long long mem_lat = message_packet->end_cycle - message_packet->start_cycle;
	if(mem_lat >= HISTSIZE)
	{

		/*cgm_parallel_stats->end_parallel_section_cycle =  P_TIME;
		cgm_parallel_stats->total_parallel_section_cycles = cgm_parallel_stats->end_parallel_section_cycle - cgm_parallel_stats->start_parallel_section_cycle;

		cgm_store_stats(cgm_parallel_stats);

		cgm_dump_summary();*/

		fatal("cache_l1_i_return(): %s increase HISTSIZE %llu access id %llu blk_addr 0x%08x type %d start_cycle %llu end_cycle %llu total_lat %llu\n",
				cache->name, mem_lat, message_packet->access_id, message_packet->address & cache->block_address_mask, message_packet->access_type,
				message_packet->start_cycle, message_packet->end_cycle, mem_lat);
	}

	mem_system_stats->cpu_total_fetch_replys++;
	mem_system_stats->fetch_lat_hist[mem_lat]++;

	if(message_packet->access_id == 1)
		mem_system_stats->first_mem_access_lat = mem_lat;


	assert(message_packet->protocol_case != invalid);
	if(message_packet->protocol_case == L1_hit)
	{
		mem_system_stats->fetch_l1_hits++;
	}
	else if (message_packet->protocol_case == L2_hit)
	{
		mem_system_stats->fetch_l2_hits++;
	}
	else if (message_packet->protocol_case == L3_hit)
	{
		mem_system_stats->fetch_l3_hits++;
	}
	else if (message_packet->protocol_case == memory)
	{
		mem_system_stats->fetch_memory++;
	}
	else
	{
		fatal("cache_l1_i_return(): message_packet->protocol_case is invalid\n");
	}

	/*for deadlock problems*/
	last_committed_fetch_access_id = message_packet->access_id;
	last_committed_fetch_access_blk = message_packet->address & cache->block_address_mask;

	//remove packet from cache queue, global queue, and simulator memory
	message_packet = list_remove(cache->last_queue, message_packet);
	remove_from_global(message_packet->access_id);

	packet_destroy(message_packet);
	return;
}

void cache_l1_d_return(struct cache_t *cache, struct cgm_packet_t *message_packet){

	/*stats*/
	long long mem_lat = message_packet->end_cycle - message_packet->start_cycle;
	assert(message_packet->start_cycle != 0);
	if(mem_lat >= HISTSIZE)
		fatal("cache_l1_d_return(): %s increase HISTSIZE %llu access id %llu blk_addr 0x%08x type %d start_cycle %llu end_cycle %llu total_lat %llu\n",
				cache->name, mem_lat, message_packet->access_id, message_packet->address & cache->block_address_mask, message_packet->access_type,
				message_packet->start_cycle, message_packet->end_cycle, mem_lat);

	/*if(mem_lat >= 2973)
	fatal("cache_l1_d_return(): %s increase HISTSIZE %llu access id %llu blk_addr 0x%08x type %d start_cycle %llu end_cycle %llu total_lat %llu\n",
				cache->name, mem_lat, message_packet->access_id, message_packet->address & cache->block_address_mask, message_packet->access_type,
				message_packet->start_cycle, message_packet->end_cycle, mem_lat);*/


	if(message_packet->cpu_access_type == cgm_access_load)
	{
		mem_system_stats->cpu_total_load_replys++;
		mem_system_stats->load_lat_hist[mem_lat]++;

		if(message_packet->protocol_case == L1_hit)
		{
			mem_system_stats->load_l1_hits++;
		}
		else if (message_packet->protocol_case == L2_hit)
		{
			mem_system_stats->load_l2_hits++;
		}
		else if (message_packet->protocol_case == L3_hit)
		{
			mem_system_stats->load_l3_hits++;
		}
		else if (message_packet->protocol_case == memory)
		{
			mem_system_stats->load_memory++;
		}
		else
			fatal("cache_l1_d_return(): message_packet->protocol_case is invalid\n");
	}
	else if(message_packet->cpu_access_type == cgm_access_store)
	{
		mem_system_stats->cpu_total_store_replys++;
		mem_system_stats->store_lat_hist[mem_lat]++;

		if(message_packet->protocol_case == L1_hit)
		{
			mem_system_stats->store_l1_hits++;
		}
		else if (message_packet->protocol_case == L2_hit)
		{
			mem_system_stats->store_l2_hits++;

		}
		else if (message_packet->protocol_case == L3_hit)
		{
			mem_system_stats->store_l3_hits++;
		}
		else if (message_packet->protocol_case == memory)
		{
			mem_system_stats->store_memory++;
		}
	}

	/*info for deadlock problems*/
	last_committed_lsq_access_id = message_packet->access_id;
	last_committed_lsq_access_blk = message_packet->address & cache->block_address_mask;

	//remove packet from cache queue, global queue, and simulator memory
	message_packet = list_remove(cache->last_queue, message_packet);
	linked_list_add(message_packet->event_queue, message_packet->data);

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

	//get the status of the ORT
	ort_get_row_sets_size(cache, message_packet->tag, message_packet->set, hit_row_ptr, num_sets_ptr, ort_size_ptr);

	//verify ort size
	assert(*ort_size_ptr < cache->mshr_size);


	if((*hit_row_ptr == cache->mshr_size && *num_sets_ptr < cache->assoc) || message_packet->assoc_conflict == 1)
	{
		/*if(message_packet->access_id == 72735041)
			warning("coal case 1\n");*/


		//unique access and number of outstanding accesses are less than cache associativity
		//i.e. there IS a space in the cache's set and ways for the block on return
		assert(*hit_row_ptr >= 0 && *hit_row_ptr <= cache->mshr_size);

		//we are about to send the packet out, set the assoc conflict flag back to 0.
		if(message_packet->assoc_conflict == 1)
		{
			message_packet->assoc_conflict = 0;
		}

		ort_set_row(cache, message_packet->tag, message_packet->set);

	}
	else if(*hit_row_ptr == cache->mshr_size && *num_sets_ptr >= cache->assoc)
	{

		/*if(message_packet->access_id == 72735041)
			warning("coal case 2\n");*/

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
		/*if(message_packet->access_id == 72735041)
			warning("coal case 3\n");*/

		//non unique access that can be coalesced with another miss
		assert(cache->ort[*hit_row_ptr][0] == message_packet->tag && cache->ort[*hit_row_ptr][1] == message_packet->set);

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
	assert(message_packet);

	list_enqueue(cache->Tx_queue_top, message_packet);
	advance(cache->cache_io_up_ec);
	return;
}

void cache_put_io_down_queue(struct cache_t *cache, struct cgm_packet_t *message_packet){

	message_packet = list_remove(cache->last_queue, message_packet);
	assert(message_packet);

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

long long ort_pull = 0;
long long assoc = 0;

void cache_coalesed_retry(struct cache_t *cache, int tag, int set, long long access_id){

	struct cgm_packet_t *ort_packet;
	int i = 0;
	long long oldest_packet = 0;

	//first look for merged accesses
	LIST_FOR_EACH(cache->ort_list, i)
	{
		//get pointer to access in queue and check it's status.
		ort_packet = list_get(cache->ort_list, i);

		if(ort_packet->tag == tag && ort_packet->set == set)
		{
			ort_packet = list_remove_at(cache->ort_list, i);

			if(cache->cache_type == gpu_l2_cache_t || cache->cache_type == gpu_v_cache_t)
			{
				ort_packet->access_type = cgm_gpu_cache_get_retry_state(ort_packet->gpu_access_type);
			}
			else
			{
				ort_packet->access_type = cgm_cache_get_retry_state(ort_packet->cpu_access_type);
			}

			if(((ort_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
			{

				if((LEVEL == 1 || LEVEL == 3) && (cache->cache_type == l1_i_cache_t || cache->cache_type == l1_d_cache_t))
				{
					printf("block 0x%08x %s ort pull ID %llu type %d state %d cycle %llu\n",
					(ort_packet->address & cache->block_address_mask), cache->name, ort_packet->access_id, ort_packet->access_type, ort_packet->cache_block_state, P_TIME);
				}
				else if((LEVEL == 2 || LEVEL == 3) && (cache->cache_type == l2_cache_t || cache->cache_type == l3_cache_t))
				{
					printf("block 0x%08x %s ort pull ID %llu type %d state %d cycle %llu\n",
					(ort_packet->address & cache->block_address_mask), cache->name, ort_packet->access_id, ort_packet->access_type, ort_packet->cache_block_state, P_TIME);
				}
			}

			list_enqueue(cache->retry_queue, ort_packet);
			advance(cache->ec_ptr);

			/*Stats*/
			cache->CoalesceGet++;

			/*this may cause problems the intent is to run one coalesced
			packet per iteration of the retry state so the timing is correctly charged*/
			return;
		}
	}

	oldest_packet = get_oldest_packet(cache, set);

	//no coalesced packets remaining now check for packets with cache assoc conflicts
	LIST_FOR_EACH(cache->ort_list, i)
	{
		//get pointer to access in queue and check it's status.
		ort_packet = list_get(cache->ort_list, i);

		if(ort_packet->set == set && ort_packet->assoc_conflict == 1)
		{

			if(ort_packet->access_id == 5643812)
			{
				warning("found it i was accessed by %llu on %llu\n", access_id, P_TIME);
			}

			assert(ort_packet->start_cycle >= oldest_packet);

			//clear the ORT entry for the assoc miss
			ort_clear(cache, ort_packet);

			ort_packet = list_remove_at(cache->ort_list, i);

			/*retry the access and it will be a hit then cause
			coalescer to re-enter the set and tag.*/
			ort_packet->coalesced = 0;
			ort_packet->assoc_conflict = 1;

			if(cache->cache_type == gpu_l2_cache_t || cache->cache_type == gpu_v_cache_t)
			{
				ort_packet->access_type = cgm_gpu_cache_get_retry_state(ort_packet->gpu_access_type);
			}
			else
			{
				ort_packet->access_type = cgm_cache_get_retry_state(ort_packet->cpu_access_type);
			}

			if(((ort_packet->address & cache->block_address_mask) == WATCHBLOCK) && WATCHLINE)
			{
				printf("block 0x%08x %s ort pull ID %llu type %d state %d cycle %llu\n",
					(ort_packet->address & cache->block_address_mask), cache->name, ort_packet->access_id, ort_packet->access_type, ort_packet->cache_block_state, P_TIME);
			}

			list_enqueue(cache->retry_queue, ort_packet);
			advance(cache->ec_ptr);

			/*Stats*/
			cache->CoalesceGet++;

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

#define SETDIR(cache_id, id)													\
						if(cache_id == id)										\
						{														\
							cache->sets[set].blocks[way].directory_entry.entry_bits.p##id = 1;	\
						}




void cgm_cache_set_dir(struct cache_t *cache, int set, int way, int cache_id){

	int num_cores = x86_cpu_num_cores;
	/*unsigned long long position = 0;
	unsigned long long bit_set = 1;*/

	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);
	assert(cache_id > (-1) && cache_id < (num_cores + 1)); //+1 is hub-iommu

	SETDIR(cache_id, 0);
	SETDIR(cache_id, 1);
	SETDIR(cache_id, 2);
	SETDIR(cache_id, 3);
	SETDIR(cache_id, 4);
	SETDIR(cache_id, 5);
	SETDIR(cache_id, 6);
	SETDIR(cache_id, 7);
	SETDIR(cache_id, 8);
	SETDIR(cache_id, 9);
	SETDIR(cache_id, 10);
	SETDIR(cache_id, 11);
	SETDIR(cache_id, 12);
	SETDIR(cache_id, 13);
	SETDIR(cache_id, 14);
	SETDIR(cache_id, 15);

	/*if(cache_id == 0)
	{
		cache->sets[set].blocks[way].directory_entry.entry_bits.p0 = 1;
	}
	else if(cache_id == 1)
	{
		cache->sets[set].blocks[way].directory_entry.entry_bits.p1 = 1;
	}
	else if(cache_id == 2)
	{
		cache->sets[set].blocks[way].directory_entry.entry_bits.p2 = 1;
	}
	else if(cache_id == 3)
	{
		cache->sets[set].blocks[way].directory_entry.entry_bits.p3 = 1;
	}
	else if(cache_id == 4)
	{
		//printf("here 4\n");

		cache->sets[set].blocks[way].directory_entry.entry_bits.p4 = 1;
	}
	else if(cache_id == 5)
	{

		//printf("here 5\n");
		cache->sets[set].blocks[way].directory_entry.entry_bits.p5 = 1;
	}
	else if(cache_id == 6)
	{
		//printf("here 6\n");

		cache->sets[set].blocks[way].directory_entry.entry_bits.p6 = 1;
	}
	else if(cache_id == 7)
	{
		//printf("here 7\n");

		cache->sets[set].blocks[way].directory_entry.entry_bits.p7 = 1;
	}
	else if(cache_id == 8)
	{
		//printf("here 8\n");

		cache->sets[set].blocks[way].directory_entry.entry_bits.p8 = 1;
	}
	else
	{
		fatal("cgm_cache_set_dir(): current dir implementation supports up to 4 cores.\n");
	}*/

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


#define MATCHCORE(cache_id, bit)																 \
		if(cache_id == bit && cache->sets[set].blocks[way].directory_entry.entry_bits.p##bit == 1) \
		{																						 \
			core_match++;																		 \
		}

int cgm_cache_is_owning_core(struct cache_t *cache, int set, int way, int cache_id){

	int core_match = 0;
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;

	/*max(num_cores, num_cus)*/

	MATCHCORE(cache_id, 0);
	MATCHCORE(cache_id, 1);
	MATCHCORE(cache_id, 2);
	MATCHCORE(cache_id, 3);
	MATCHCORE(cache_id, 4);
	MATCHCORE(cache_id, 5);
	MATCHCORE(cache_id, 6);
	MATCHCORE(cache_id, 7);
	MATCHCORE(cache_id, 8);
	MATCHCORE(cache_id, 9);
	MATCHCORE(cache_id, 10);
	MATCHCORE(cache_id, 11);
	MATCHCORE(cache_id, 12);
	MATCHCORE(cache_id, 13);
	MATCHCORE(cache_id, 14);
	MATCHCORE(cache_id, 15);

	/*if(cache_id == 0 && cache->sets[set].blocks[way].directory_entry.entry_bits.p0 == 1)
	{
		core_match++;
	}
	else if(cache_id == 1 && cache->sets[set].blocks[way].directory_entry.entry_bits.p1 == 1)
	{
		core_match++;
	}
	else if(cache_id == 2 && cache->sets[set].blocks[way].directory_entry.entry_bits.p2 == 1)
	{
		core_match++;
	}
	else if(cache_id == 3 && cache->sets[set].blocks[way].directory_entry.entry_bits.p3 == 1)
	{
		core_match++;
	}
	else if(cache_id == 4 && cache->sets[set].blocks[way].directory_entry.entry_bits.p4 == 1)
	{
		core_match++;
	}
	else if(cache_id == 5 && cache->sets[set].blocks[way].directory_entry.entry_bits.p5 == 1)
	{
		core_match++;
	}
	else if(cache_id == 6 && cache->sets[set].blocks[way].directory_entry.entry_bits.p6 == 1)
	{
		core_match++;
	}
	else if(cache_id == 7 && cache->sets[set].blocks[way].directory_entry.entry_bits.p7 == 1)
	{
		core_match++;
	}
	else if(cache_id == 8 && cache->sets[set].blocks[way].directory_entry.entry_bits.p8 == 1)
	{
		core_match++;
	}
	else if(cache_id == 9 && cache->sets[set].blocks[way].directory_entry.entry_bits.p9 == 1)
	{
		core_match++;
	}
	else if(cache_id == 10 && cache->sets[set].blocks[way].directory_entry.entry_bits.p10 == 1)
	{
		core_match++;
	}
	else if(cache_id == 11 && cache->sets[set].blocks[way].directory_entry.entry_bits.p11 == 1)
	{
		core_match++;
	}
	else if(cache_id == 12 && cache->sets[set].blocks[way].directory_entry.entry_bits.p12 == 1)
	{
		core_match++;
	}
	else if(cache_id == 13 && cache->sets[set].blocks[way].directory_entry.entry_bits.p13 == 1)
	{
		core_match++;
	}
	else if(cache_id == 14 && cache->sets[set].blocks[way].directory_entry.entry_bits.p14 == 1)
	{
		core_match++;
	}
	else if(cache_id == 15 && cache->sets[set].blocks[way].directory_entry.entry_bits.p15 == 1)
	{
		core_match++;
	}*/



	/*-----------------*/

	if(cache->cache_type == l3_cache_t && (cache_id < 0 && cache_id >= num_cores))
	{
		fatal("cgm_cache_is_owning_core(): cache id broken...\n");
	}
	else if (cache->cache_type == gpu_l2_cache_t && (cache_id < 0 && cache_id >= num_cus))
	{
		fatal("cgm_cache_is_owning_core(): cache id broken...\n");
	}

	assert(core_match == 0 || core_match == 1);

	return core_match;
}

int cgm_cache_get_num_shares(struct cache_t *cache, int set, int way){

	int sharers = 0;
	int num_cores = x86_cpu_num_cores;
	int i = 0;
	unsigned long long bit_vector;

	/*get the number of shares, mask away everything but the the share bit field
	and take the log of the value to get the number of sharers*/

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

	/*if(cache->cache_type == l2_cache_t && set == 69 && way == 0 && state == 0)
		printf("caught the error\n");*/

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

int cgm_cache_get_block_upgrade_pending_bit(struct cache_t *cache, int set, int way){

	return cache->sets[set].blocks[way].upgrade_pending;

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

	/*if(cache->cache_type == l2_cache_t && set == 172 && t_state == cgm_cache_block_transient)
	{
		cgm_cache_dump_set(cache, set);

		printf("setting t state set %d way %d\n", set, way);

	}
	else if(cache->cache_type == l2_cache_t && set == 172 && t_state == cgm_cache_block_invalid)
	{
		printf("clear t state set %d way %d\n", set, way);
	}*/

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

long long get_oldest_packet(struct cache_t *cache, int set){

	long long start = 0xFFFFFFFFFFFFFF;
	long long i = 0;
	struct cgm_packet_t *ort_packet;

	LIST_FOR_EACH(cache->ort_list, i)
	{
		//get pointer to access in queue and check it's status.
		ort_packet = list_get(cache->ort_list, i);

		if(ort_packet->set == set && ort_packet->assoc_conflict == 1)
		{
			if(ort_packet->start_cycle < start)
			{
				start = ort_packet->start_cycle;
			}
		}
	}

	return start;
}

enum cgm_access_kind_t cgm_gpu_cache_get_retry_state(enum cgm_access_kind_t r_state){

	enum cgm_access_kind_t retry_state = cgm_access_invalid;

	if(cgm_gpu_cache_protocol == cgm_protocol_non_coherent)
	{
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
			fatal("cgm_gpu_cache_get_retry_state(): unrecognized state as %s\n", str_map_value(&cgm_mem_access_strn_map, r_state));
		}
	}
	else if (cgm_gpu_cache_protocol == cgm_protocol_mesi)
	{
		if(r_state == cgm_access_load_s || r_state == cgm_access_load_v)
		{
			retry_state = cgm_access_loadx_retry;
		}
		else if(r_state == cgm_access_store_v || r_state == cgm_access_nc_store)
		{
			retry_state = cgm_access_storex_retry;
		}
		else
		{
			fatal("cgm_gpu_cache_get_retry_state(): unrecognized state as %s\n", str_map_value(&cgm_mem_access_strn_map, r_state));
		}
	}
	else
	{
		fatal("cgm_gpu_cache_get_retry_state(): protocol\n");
	}

	assert(retry_state != cgm_access_invalid);

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
		fatal("cgm_cache_get_retry_state(): CPU unrecognized state as %s \n", str_map_value(&cgm_mem_access_strn_map, r_state));
	}



	return retry_state;
}

int cache_validate_block_flushed_from_core(int core_id, unsigned int addr){

	struct cgm_packet_t *l1_write_back_packet = NULL;
	struct cgm_packet_t *l2_write_back_packet = NULL;

	int l1_set = 0;
	int l1_tag = 0;
	unsigned int l1_offset = 0;
	int l1_way = 0;

	int *l1_set_ptr = &l1_set;
	int *l1_tag_ptr = &l1_tag;
	unsigned int *l1_offset_ptr = &l1_offset;
	int *l1_way_ptr = &l1_way;

	int l2_set = 0;
	int l2_tag = 0;
	unsigned int l2_offset = 0;
	int l2_way = 0;

	int *l2_set_ptr = &l2_set;
	int *l2_tag_ptr = &l2_tag;
	unsigned int *l2_offset_ptr = &l2_offset;
	int *l2_way_ptr = &l1_way;

	int l1_cache_block_hit = 0;
	int l1_cache_block_state = 0;

	int l2_cache_block_hit = 0;
	int l2_cache_block_state = 0;

	int *l1_cache_block_hit_ptr = &l1_cache_block_hit;
	int *l1_cache_block_state_ptr = &l1_cache_block_state;

	int *l2_cache_block_hit_ptr = &l2_cache_block_hit;
	int *l2_cache_block_state_ptr = &l2_cache_block_state;

	int l1_hit = 0;
	int l2_hit = 0;
	int l1_wb_hit = 0;
	int l2_wb_hit = 0;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(&l1_d_caches[core_id], addr, l1_set_ptr, l1_tag_ptr, l1_offset_ptr);

	//look for the block in the cache
	*(l1_cache_block_hit_ptr) = cgm_cache_find_block(&l1_d_caches[core_id], l1_tag_ptr, l1_set_ptr, l1_offset_ptr, l1_way_ptr, l1_cache_block_state_ptr);

	//search the WB buffer for the data if in WB the block is either in the E or M state so return

	l1_write_back_packet = cache_search_wb(&l1_d_caches[core_id], l1_tag, l1_set);

	if(l1_write_back_packet)
		l1_wb_hit = 1;


	if(*l1_cache_block_hit_ptr == 1 || l1_write_back_packet)
		l1_hit = 1;


	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(&l2_caches[core_id], addr, l2_set_ptr, l2_tag_ptr, l2_offset_ptr);

	//look for the block in the cache
	*(l2_cache_block_hit_ptr) = cgm_cache_find_block(&l2_caches[core_id], l2_tag_ptr, l2_set_ptr,l2_offset_ptr, l2_way_ptr, l2_cache_block_state_ptr);

	//search the WB buffer for the data if in WB the block is either in the E or M state so return
	l2_write_back_packet = cache_search_wb(&l2_caches[core_id], l2_tag, l2_set);

	if(l2_write_back_packet)
		l2_wb_hit = 1;

	if(*l2_cache_block_hit_ptr == 1 || l2_write_back_packet)
		l2_hit = 1;


	if(l1_hit == 1 || l2_hit ==1)
	{

		/*fatal("\terror check l1_hit %d l2_hit %d cycle %llu\n", l1_hit, l2_hit, P_TIME);*/
		printf("\terror check l1_hit_ptr %d l1_state_ptr %d\n"
			  "\terror check l2_hit_ptr %d l2_state_ptr %d\n"
			  "\terror L1_wb id %d L2_wb id %d\n",
			  *l1_cache_block_hit_ptr, *l1_cache_block_state_ptr,
			  *l2_cache_block_hit_ptr, *l2_cache_block_hit_ptr,
			  l1_wb_hit, l2_wb_hit);

		printf("block 0x%08x searching for set %d tag %d\n", addr, l1_set, l1_tag);
		cgm_cache_print_set_tag(&l1_d_caches[core_id], addr);
		if(l1_write_back_packet)
		{
			printf("block 0x%08x l1 wb_addr 0x%08x id %llu wb_set %d wb_tag %d\n",
				addr, l1_write_back_packet->address, l1_write_back_packet->write_back_id, l1_write_back_packet->set, l1_write_back_packet->tag);
		}
		else
		{
			printf("l1 no wb found\n");
		}

		printf("block 0x%08x searching for set %d tag %d\n", addr, l2_set, l2_tag);
		cgm_cache_print_set_tag(&l2_caches[core_id], addr);
		cgm_cache_dump_set(&l2_caches[core_id], l2_set);
		if(l2_write_back_packet)
		{
			printf("block 0x%08x l2 wb_addr 0x%08x id %llu wb_set %d wb_tag %d\n",
				addr, l2_write_back_packet->address, l2_write_back_packet->write_back_id, l2_write_back_packet->set, l2_write_back_packet->tag);

			cgm_cache_print_set_tag(&l2_caches[core_id], l2_write_back_packet->address);

		}
		else
		{
			printf("l2 no wb found\n");
		}

		fatal("cache_validate_block_flushed_from_core(): block not where it ought to be\n");

		return 1;
	}

	return 0;
}


int cache_validate_block_flushed_from_l1(int core_id, unsigned int addr){

	int hit = 0;
	struct cgm_packet_t *write_back_packet = NULL;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;

	int cache_block_hit;
	int cache_block_state;
	int *cache_block_hit_ptr = &cache_block_hit;
	int *cache_block_state_ptr = &cache_block_state;

	//probe the address for set, tag, and offset.
	cgm_cache_probe_address(&l1_d_caches[core_id], addr, set_ptr, tag_ptr, offset_ptr);

	//look for the block in the cache
	*(cache_block_hit_ptr) = cgm_cache_find_block(&l1_d_caches[core_id], tag_ptr, set_ptr, offset_ptr, way_ptr, cache_block_state_ptr);

		//search the WB buffer for the data if in WB the block is either in the E or M state so return
	write_back_packet = cache_search_wb(&l1_d_caches[core_id], tag, set);

	if(*cache_block_hit_ptr == 1 || write_back_packet)
		hit = 1;

	return hit;
}


