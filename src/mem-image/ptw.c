/*
 * ptw.c
 *
 *  Created on: May 16, 2017
 *      Author: stardica
 */

#include <mem-image/ptw.h>



eventcount volatile *ptw_ec;
task *ptw_task;

long long ptw_pid = 0;

long long pages_created = 0;
long long ptw_num_processed = 0;


void ptw_init(void){

	int num_cores = x86_cpu_num_cores;
	char buff[100];
	int i = 0;

	//create event counts
	ptw_ec = (void *) calloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "ptw_ec_%d", i);
		ptw_ec[i] = *(new_eventcount(strdup(buff)));
	}

	//create tasks
	ptw_task = (void *) calloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "ptw_task_%d", i);
		ptw_task[i] = *(create_task(ptw_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	return;
}

void ptw_clear_mmu_fault_bits(struct mmu_t *mmu){

	int i = 0;

	for (i = 0; i < (mmu->issue_width + 1); i++)
		mmu->fault_bits[i] = 0;

	return;
}

void ptw_ctrl(void){

	//my_pid increments for the number of CPU cores. i.e. 0 - 4 for a quad core
	int my_pid = ptw_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	int core_issue_width = x86_cpu_issue_width;

	int i = 0;

	int page_fault = 0;
	int *page_fault_ptr = &page_fault;

	int set = 0;
	int tag = 0;
	unsigned int offset = 0;
	int way = 0;

	int *set_ptr = &set;
	int *tag_ptr = &tag;
	unsigned int *offset_ptr = &offset;
	int *way_ptr = &way;

	int num_advances = 0;

	//int num_faults = 0;

	int err = 0;

	while(1)
	{
		//wait here until there is a job to do
		await(&ptw_ec[my_pid], step); //&mmu_ec[my_pid], step);


		//check each mmu fault bit, resolve address translations, replay accesses (advance MMU again would be good)

		//inst cache accesses
		if(mmu[my_pid].fault_bits[0] == 1)
		{
			//mmu reports TLB miss for this address walk the page table.
			//warning("PTW address to translate 0x%08x\n", mmu[my_pid].vtl_fetch_address);

			//fast way to simulate!!! (not how the "real" system works though),
			//(1) check the page table first, if the page isn't created yet then it wouldn't be in the page table and is a page fault
			//(2) if it is in the page table assume it is also in the PTW, thus not a full page fault
			//(3) if it is in the page table, but has been swapped out, then it is a full page fault.

			//translate the address, if a new page is created its a page fault (PTW is a miss and page not in main memory).
			mmu[my_pid].phy_fetch_address = mmu_translate(mmu[my_pid].fetch_address_space_index, mmu[my_pid].vtl_fetch_address, mmu_access_fetch, page_fault_ptr);

			//warning("PTW translated 0x%08x to 0x%08x fault %d cycle %llu\n", mmu[my_pid].vtl_fetch_address, mmu[my_pid].phy_fetch_address, page_fault, P_TIME);


			//store results in TLB for future look up...
			//probe the address...
			cgm_tlb_probe_address(&i_tlbs[my_pid], mmu[my_pid].vtl_fetch_address, set_ptr, tag_ptr, offset_ptr);

			//warning("PTW probe tag %d set %d way %d\n", tag, set, way);

			//find the transient line in the TLB
			err = cgm_tlb_find_transient_entry(&i_tlbs[my_pid], tag_ptr, set_ptr, way_ptr);

			if(err != 1)
				tlb_dump_set(&i_tlbs[my_pid], set);

			assert(err == 1);

			/*feels weird that this should be invalid, but remember that execution has stopped because of the TLB miss*/
			assert(i_tlbs[my_pid].sets[set].blocks[way].state == cgm_tlb_block_invalid);

			//write the new block in and restart the TLB access
			cgm_tlb_set_entry(&i_tlbs[my_pid], tag, set, way, cgm_tlb_get_ppn_tag(&i_tlbs[my_pid], mmu[my_pid].phy_fetch_address));

			//warning("setting phy tag %d on set %d way %d\n", cgm_tlb_get_ppn_tag(&i_tlbs[my_pid], mmu[my_pid].phy_fetch_address), set, way);

			//mmu[my_pid].fault_bits[0] = 0;

			//if(page_fault)
			//	num_faults++;

			//warning("advancing MMU\n");

			//Advance the MMU for retry
			advance(&mmu_ec[my_pid]);

			num_advances++;
			page_fault = 0;
		}


		//data cache accesses
		for(i=1; i<(core_issue_width +1); i++)
		{

			//inst cache accesses
			if(mmu[my_pid].fault_bits[i] == 1)
			{

				//translate the address, if a new page is created its a page fault (PTW is a miss and page not in main memory).
				mmu[my_pid].phy_data_address[i-1] = mmu_translate(mmu[my_pid].data_address_space_index[i-1], mmu[my_pid].vtl_data_address[i-1], mmu_access_load_store, page_fault_ptr);

				warning("PTW D translated 0x%08x to 0x%08x fault %d cycle %llu\n", mmu[my_pid].vtl_data_address[i-1], mmu[my_pid].phy_data_address[i-1], page_fault, P_TIME);


				//store results in TLB for future look up...
				//probe the address...
				cgm_tlb_probe_address(&d_tlbs[my_pid], mmu[my_pid].vtl_data_address[i-1], set_ptr, tag_ptr, offset_ptr);

				//warning("PTW probe tag %d set %d way %d\n", tag, set, way);

				//find the transient line in the TLB
				err = cgm_tlb_find_transient_entry(&d_tlbs[my_pid], tag_ptr, set_ptr, way_ptr);

				if(err != 1)
					tlb_dump_set(&d_tlbs[my_pid], set);

				assert(err == 1);


				/*feels weird that this should be invalid, but remember that execution has stopped because of the TLB miss*/
				assert(d_tlbs[my_pid].sets[set].blocks[way].state == cgm_tlb_block_invalid);

				//write the new block in and restart the TLB access
				cgm_tlb_set_entry(&d_tlbs[my_pid], tag, set, way, cgm_tlb_get_ppn_tag(&d_tlbs[my_pid], mmu[my_pid].phy_data_address[i-1]));

				//warning("setting phy tag %d on set %d way %d\n", cgm_tlb_get_ppn_tag(&i_tlbs[my_pid], mmu[my_pid].phy_fetch_address), set, way);


				//if(page_fault)
				//	num_faults++;

				//mmu[my_pid].fault_bits[i] = 0;

				//warning("advancing MMU\n");

				//Advance the MMU for retry
				advance(&mmu_ec[my_pid]);

				num_advances++;
				//page_fault = 0;
			}

		}

		if(page_fault > 0)
			P_PAUSE(6000);
		else
			P_PAUSE(8);

		//mmu[my_pid].fault_bits[0] = 0;

		//Advance the MMU for retry

		//for(i=0; i < num_advances; i++)
		//	advance(&mmu_ec[my_pid]);

		ptw_num_processed += num_advances;
		step += num_advances;
		//step++;
		//assert(page_fault == 0);
		num_advances = 0;
		page_fault = 0;

	}

	fatal("ptw_ctrl(): out of loop\n");

	return;
}
