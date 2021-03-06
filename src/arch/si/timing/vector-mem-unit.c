/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <arch/si/emu/emu.h>
#include <arch/si/emu/wavefront.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include <cgm/cgm.h>

#include <arch/si/timing/compute-unit.h>
#include <arch/si/timing/gpu.h>
#include <arch/si/timing/cycle-interval-report.h>
#include <arch/si/timing/vector-mem-unit.h>
#include <arch/si/timing/uop.h>
#include <arch/si/timing/wavefront-pool.h>


long long mem_write_stall = 0;
long long mem_write_complete = 0;
long long num_uops = 0;
long long num_uops_out = 0;

long long mem_buffer = 0;
long long stalls = 0;

void si_vector_mem_complete(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop = NULL;
	int list_entries;
	int i;
	int list_index = 0;

	/* Process completed memory instructions */
	list_entries = list_count(vector_mem->write_buffer);

	/* Sanity check the write buffer */
	assert(list_entries <= si_gpu_vector_mem_width);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(vector_mem->write_buffer, list_index);
		assert(uop);

		/* Uop is not ready */
		if (asTiming(si_gpu)->cycle < uop->write_ready)
		{
			list_index++;
			continue;
		}

		num_uops_out++;

		/* Access complete, remove the uop from the queue */
		list_remove(vector_mem->write_buffer, uop);

		assert(uop->wavefront_pool_entry->lgkm_cnt > 0);
		uop->wavefront_pool_entry->lgkm_cnt--;

		si_trace("si.end_inst id=%lld cu=%d\n", uop->id_in_compute_unit, uop->compute_unit->id);

		/* Free uop */
		si_uop_free(uop);

		/* Statistics */
		vector_mem->inst_count++;
		si_gpu->last_complete_cycle = asTiming(si_gpu)->cycle;
	}

}



void si_vector_mem_write(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;
	/*capture the stalls here*/
	/*with the way this is implemented, the GPU stalls when no uops are written in a given cycle*/
	int uops_completed = 0;

	list_entries = list_count(vector_mem->mem_buffer);

	/* Sanity check the mem buffer */
	assert(list_entries <= si_gpu_vector_mem_max_inflight_mem_accesses);

	for (i = 0; i < list_entries; i++)
	{

		uop = list_get(vector_mem->mem_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (uop->global_mem_witness)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached. */
		if (instructions_processed > si_gpu_vector_mem_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				vector_mem->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Sanity check write buffer */
		assert(list_count(vector_mem->write_buffer) <= si_gpu_vector_mem_write_buffer_size);

		/* Stop if the write buffer is full. */
		if (list_count(vector_mem->write_buffer) == si_gpu_vector_mem_write_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				vector_mem->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Access complete, remove the uop from the queue */
		uop->write_ready = asTiming(si_gpu)->cycle + si_gpu_vector_mem_write_latency;

		/* In the above context, access means any of the 
		 * mod_access calls in si_vector_mem_mem. Means all 
		 * inflight accesses for uop are done */
		if(si_spatial_report_active)
		{
			if (uop->vector_mem_write)
			{
				si_report_global_mem_finish(uop->compute_unit, uop->num_global_mem_write);
			}
			else if (uop->vector_mem_read)
			{
				si_report_global_mem_finish(uop->compute_unit, uop->num_global_mem_read);
			}
			else
			{
				fatal("%s: invalid access kind", __FUNCTION__);
			}
		}

		list_remove(vector_mem->mem_buffer, uop);
		list_enqueue(vector_mem->write_buffer, uop);

		/*stats*/
		uops_completed++;

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"mem-w\"\n", uop->id_in_compute_unit, 
			vector_mem->compute_unit->id, uop->wavefront->id, 
			uop->id_in_wavefront);
	}

	assert(uops_completed >= 0 && uops_completed <= si_gpu_vector_mem_width);

	/*stats*/
	/*if(list_entries > 0)
	{
		if(uops_completed >= 1 && uops_completed <= si_gpu_vector_mem_width)
		{
			cpu_gpu_stats->cu_total_busy[vector_mem->compute_unit->id]++;
		}
		else
		{
			assert(uops_completed == 0);

			cpu_gpu_stats->cu_total_stalls[vector_mem->compute_unit->id]++;
		}
	}*/


}



void si_vector_mem_mem(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop;
	struct si_work_item_uop_t *work_item_uop;
	struct si_work_item_t *work_item;
	int work_item_id;
	int instructions_processed = 0;
	int list_entries;
	int i;
#if CGM
	enum cgm_access_kind_t access_kind = cgm_access_invalid;
#else
	enum mod_access_kind_t access_kind_m2s;
#endif
	int list_index = 0;

	//int access_count = 0;
	//int stalls = 0;

	/*catch the gpu stall if it occurs*/
	//int mem_stall = 0;

	/*currently we are just 1 at a time*/
	if(si_gpu_vector_mem_width > 1 || si_gpu_vector_mem_width < 1)
		fatal("si_vector_mem_write(): vector mem width not equal\n");

	//if(vector_mem->compute_unit->id == 0)
	//gpu_iter++;

	list_entries = list_count(vector_mem->read_buffer);
	
	/* Sanity check the read buffer */
	assert(list_entries <= si_gpu_vector_mem_read_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(vector_mem->read_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (asTiming(si_gpu)->cycle < uop->read_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached. */
		if (instructions_processed > si_gpu_vector_mem_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				vector_mem->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Sanity check mem buffer */
		assert(list_count(vector_mem->mem_buffer) <= si_gpu_vector_mem_max_inflight_mem_accesses);

		//printf("current count %d cycle %llu\n", list_count(vector_mem->mem_buffer), P_TIME);

		/* Stall if there is not room in the memory buffer */
		if (list_count(vector_mem->mem_buffer) == si_gpu_vector_mem_max_inflight_mem_accesses)
		{
			//warning("mem buffer stalls %d core %d\n", list_count(vector_mem->mem_buffer), vector_mem->compute_unit->id);

		//	if(vector_mem->compute_unit->id == 0)
			//	mem_buffer++;

			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				vector_mem->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		assert(uop->wavefront->work_item_count <= 64);

		//star added this...
		/*Stall if there is no room in the vector cache's input queue*/
		if (uop->wavefront->work_item_count > (uop->wavefront->work_item_count - list_count(vector_mem->compute_unit->gpu_v_cache_ptr[vector_mem->compute_unit->id].Rx_queue_top)))
		{
			/*check for system arch integrity*/
			/*if the l1 input queue is not large enough the GPU will never make progress in this implementation...*/
			assert(uop->wavefront->work_item_count <= uop->wavefront->work_item_count && uop->wavefront->work_item_count > 0);

			list_index++;
			continue;
		}

		/* Set the access type */
#if CGM
		if (uop->vector_mem_write && !uop->glc)
			access_kind = cgm_access_nc_store;
		else if (uop->vector_mem_write && uop->glc)
			access_kind = cgm_access_store;
		else if (uop->vector_mem_read)
			access_kind = cgm_access_load;
#else
		if (uop->vector_mem_write && !uop->glc)
			access_kind_m2s = mod_access_nc_store;
		else if (uop->vector_mem_write && uop->glc)
			access_kind_m2s = mod_access_store;
		else if (uop->vector_mem_read)
			access_kind_m2s = mod_access_load;
#endif
		else 
			fatal("%s: invalid access kind", __FUNCTION__);

		/* Access global memory */
		assert(!uop->global_mem_witness);

		//int curr_size = (GPUQueueSize - list_count(vector_mem->compute_unit->gpu_v_cache_ptr[vector_mem->compute_unit->id].Rx_queue_top));

		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
		{
			work_item = uop->wavefront->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];

			uop->global_mem_witness--;
#if CGM
			//star fixes for some broken code...
			if(work_item_uop->global_mem_access_addr >= 0x00000000 && work_item_uop->global_mem_access_addr <=0x0000003F)
			{
				//warning("bad address from the GPU why 0x%08x?\n", work_item_uop->global_mem_access_addr);
				work_item_uop->global_mem_access_addr = 0x081354a8;
				//fatal("caught a bad uop->id %llu\n", uop->id);
			}

			//fatal("caught GPU access 0x%08x\n", work_item_uop->global_mem_access_addr);

			assert(access_kind != cgm_access_invalid);
			cgm_vector_access(vector_mem, access_kind, work_item_uop->global_mem_access_addr, &uop->global_mem_witness);
#else
			mod_access(vector_mem->compute_unit->vector_cache, access_kind_m2s, work_item_uop->global_mem_access_addr, &uop->global_mem_witness, NULL, NULL, NULL);
#endif
		}

		//printf("abs %d queue %d\n", abs(uop->global_mem_witness),  (GPUQueueSize - list_count(vector_mem->compute_unit->gpu_v_cache_ptr[vector_mem->compute_unit->id].Rx_queue_top)));
		//assert(abs(uop->global_mem_witness) <= curr_size);

		//fatal("uop->global_mem_witness %d id %llu\n", uop->global_mem_witness, uop->id);

		if(si_spatial_report_active)
		{
			if (uop->vector_mem_write)
			{
				uop->num_global_mem_write += uop->global_mem_witness;
				si_report_global_mem_inflight(uop->compute_unit, uop->num_global_mem_write);
			}
			else if (uop->vector_mem_read)
			{
				uop->num_global_mem_read += uop->global_mem_witness;
				si_report_global_mem_inflight(uop->compute_unit, uop->num_global_mem_read);
			}
			else
				fatal("%s: invalid access kind", __FUNCTION__);
		}

		/* Transfer the uop to the mem buffer */
		list_remove(vector_mem->read_buffer, uop);
		list_enqueue(vector_mem->mem_buffer, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"mem-m\"\n", uop->id_in_compute_unit, 
			vector_mem->compute_unit->id, uop->wavefront->id, 
			uop->id_in_wavefront);
	}
}

void si_vector_mem_read(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(vector_mem->decode_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= si_gpu_vector_mem_decode_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(vector_mem->decode_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (asTiming(si_gpu)->cycle < uop->decode_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached. */
		if (instructions_processed > si_gpu_vector_mem_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				vector_mem->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Sanity check the read buffer */
		assert(list_count(vector_mem->read_buffer) <= si_gpu_vector_mem_read_buffer_size);

		/* Stop if the read buffer is full. */
		if (list_count(vector_mem->read_buffer) == si_gpu_vector_mem_read_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				vector_mem->compute_unit->id, 
				uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		uop->read_ready = asTiming(si_gpu)->cycle + si_gpu_vector_mem_read_latency;

		list_remove(vector_mem->decode_buffer, uop);
		list_enqueue(vector_mem->read_buffer, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"mem-r\"\n", uop->id_in_compute_unit, 
			vector_mem->compute_unit->id, uop->wavefront->id, 
			uop->id_in_wavefront);
	}
}

void si_vector_mem_decode(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(vector_mem->issue_buffer);

	/* Sanity check the issue buffer */
	assert(list_entries <= si_gpu_vector_mem_issue_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(vector_mem->issue_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop not ready yet */
		if (asTiming(si_gpu)->cycle < uop->issue_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached. */
		if (instructions_processed > si_gpu_vector_mem_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", uop->id_in_compute_unit, vector_mem->compute_unit->id, uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Sanity check the decode buffer */
		assert(list_count(vector_mem->decode_buffer) <= si_gpu_vector_mem_decode_buffer_size);

		/* Stall if the decode buffer is full. */
		if (list_count(vector_mem->decode_buffer) == si_gpu_vector_mem_decode_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", uop->id_in_compute_unit, vector_mem->compute_unit->id, uop->wavefront->id, uop->id_in_wavefront);
			list_index++;
			continue;
		}

		uop->decode_ready = asTiming(si_gpu)->cycle + si_gpu_vector_mem_decode_latency;

		num_uops++;

		list_remove(vector_mem->issue_buffer, uop);
		list_enqueue(vector_mem->decode_buffer, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"mem-d\"\n", uop->id_in_compute_unit, vector_mem->compute_unit->id, uop->wavefront->id, uop->id_in_wavefront);
	}
}

long long v_mem = 0;

void si_vector_mem_run(struct si_vector_mem_unit_t *vector_mem)
{
	/* Local Data Share stages */
	si_vector_mem_complete(vector_mem);
	si_vector_mem_write(vector_mem);
	si_vector_mem_mem(vector_mem);
	si_vector_mem_read(vector_mem);
	si_vector_mem_decode(vector_mem);
}
