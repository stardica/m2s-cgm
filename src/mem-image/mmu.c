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

#include <assert.h>
#include <string.h>

#include <arch/x86/timing/thread.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include <mem-image/mmu.h>

#include <cgm/cgm.h>


/*
 * Global variables
 */

char *mmu_report_file_name = "";

unsigned int mmu_page_size = 1 << 12;  /* 4KB default page size */
unsigned int mmu_log_page_size;
unsigned int mmu_page_mask;

int page_number = 0;

unsigned int max_2 = 0;
unsigned int max_1 = 0;

int mmu_fetch_pid = 0;
int mmu_data_pid = 0;
eventcount volatile *mmu_fetch_ec;
eventcount volatile *mmu_data_ec;
task *mmu_fetch_task;
task *mmu_data_task;

/*
 * Private variables
 */

struct mmu_t *mmu;


/*#include <cgm/cgm.h>*/

/*
 * Private Functions
 */

struct mmu_page_t *mmu_page_access(int address_space_index, enum mmu_address_type_t addr_type, unsigned int addr, enum mmu_access_t access_type){

	//struct mmu_page_t *prev = NULL;
	struct mmu_page_t *page = NULL;
	struct mmu_page_t *text_page = NULL;
	struct mmu_page_t *data_page = NULL;
	struct mmu_page_t *gpu_page = NULL;
	unsigned int tag;
	int index;
	int i = 0;

	enum mmu_page_type_t page_type;

	/* Look for page */
	index = ((addr >> mmu_log_page_size) + address_space_index * 23) % MMU_PAGE_HASH_SIZE;
	tag = addr & ~mmu_page_mask;
	//prev = NULL;

	 // explaining the next line...
	/*the hub_iommu translates from GPU vtl to phy addresses however the hashed pages
	are indexed by their vtl addresses, which doesn't work when trying to reverse translate.
	so a small table is kept in the hub_iommu that records the index of the hashed vtl address
	as the physical address's hash. So, the next line gives you the stored vtl index for a
	quick lookup of the page*/

	if(access_type == mmu_access_load_store)
	{
		page = mmu->page_hash_table[index];
	}
	else if(access_type == mmu_access_gpu || access_type == mmu_access_gpu_coherent)
	{
		page = mmu->page_hash_table[hub_iommu->page_hash_table[index]];
	}

	while (page)
	{
		//check to see if it is a hit and look for both text and data page types
		if(page->page_type == mmu_page_text && mmu_search_page(page, addr_type, addr, address_space_index, tag))
		{
			text_page = page;
			break;
		}
		else if(page->page_type == mmu_page_data && mmu_search_page(page, addr_type, addr, address_space_index, tag))
		{
			data_page = page;
			break;
		}
		else if(page->page_type == mmu_page_gpu && mmu_search_page(page, addr_type, addr, address_space_index, tag))
		{
			gpu_page = page;

			fatal("found a gpu page %d\n", gpu_page->id);

			break;
		}

		//prev = page;
		page = page->next;
	}

	/*old code
	search the page list for a page hit.*/
	/*LIST_FOR_EACH(mmu->page_list, i)
	{
		//pull a page
		page = list_get(mmu->page_list, i);

		//check to see if it is a hit and look for both text and data page types
		if(page->page_type == mmu_page_text && mmu_search_page(page, addr_type, addr, address_space_index, tag))
		{
			text_page = page;
		}
		else if(page->page_type == mmu_page_data && mmu_search_page(page, addr_type, addr, address_space_index, tag))
		{
			data_page = page;
		}
		else if(page->page_type == mmu_page_gpu && mmu_search_page(page, addr_type, addr, address_space_index, tag))
		{
			gpu_page = page;
		}
	}*/

	//figure out if we are looking for a .text or a data page.
	page_type = mmu_get_page_type(access_type);

	if(page_type == mmu_page_text && text_page)
	{
		page = text_page;
	}
	else if(page_type == mmu_page_data && data_page)
	{
		page = data_page;
	}
	else if(page_type == mmu_page_gpu && data_page) //star i changed this.
	{
		page = data_page;
	}
	else
	{

		//star todo fix this, for some reason the page wasn't found when hashed.

		/*old code
		search the page list for a page hit.*/
		LIST_FOR_EACH(mmu->page_list, i)
		{
			//pull a page
			page = list_get(mmu->page_list, i);

			//check to see if it is a hit and look for both text and data page types
			if(page->page_type == mmu_page_text && mmu_search_page(page, addr_type, addr, address_space_index, tag))
			{
				return page;
			}
			else if(page->page_type == mmu_page_data && mmu_search_page(page, addr_type, addr, address_space_index, tag))
			{
				return page;
			}
			else if(page->page_type == mmu_page_gpu && mmu_search_page(page, addr_type, addr, address_space_index, tag))
			{
				return page;
			}
		}

		/*not found for real, create the page */
		warning("mmu_page_access(): gpu access missed page addr 0x%08x creating page...\n", addr);
		page = mmu_create_page(address_space_index, tag, access_type, index, addr);
		assert(page);

		return page;
	}

	return page;
}

int mmu_search_page(struct mmu_page_t *page, enum mmu_address_type_t addr_type, unsigned int addr, int address_space_index, int tag){

	int hit = 0;

	switch (addr_type)
	{
		case mmu_addr_vtl:

			if (page->vtl_addr == tag && page->address_space_index == address_space_index)
			{
				hit = 1;
			}

		break;

		case mmu_addr_phy:

			if (page->phy_addr == tag && page->address_space_index == address_space_index)
			{
				hit = 1;
			}

		break;

		case mmu_addr_guest:
		case mmu_addr_invalid:
			fatal("mmu_search_page(): bad addr type\n");

		break;
	}

	return hit;
}


struct mmu_page_t *mmu_get_page(int address_space_index, unsigned int vtladdr, enum mmu_access_t access_type)
{
	struct mmu_page_t *prev = NULL;
	struct mmu_page_t *page = NULL;
	struct mmu_page_t *text_page = NULL;
	struct mmu_page_t *data_page = NULL;
	//struct mmu_page_t *gpu_page = NULL;
	unsigned int tag;
	int index;
	/*int num_pages;*/
	//int i = 0;

	enum mmu_page_type_t page_type;

	if(access_type == mmu_access_gpu && GPU_HUB_IOMMU == 1)
	{
		printf("searching for page gpu vtl_addr 0x%08x\n", vtladdr);
	}

	/* Look for page */
	index = ((vtladdr >> mmu_log_page_size) + address_space_index * 23) % MMU_PAGE_HASH_SIZE;
	tag = vtladdr & ~mmu_page_mask;
	prev = NULL;
	page = mmu->page_hash_table[index];

	while (page)
	{
		//check to see if it is a hit and look for both text and data page types
		if(page->page_type == mmu_page_text && mmu_search_page(page, mmu_addr_vtl, vtladdr, address_space_index, tag))
		{
			text_page = page;
			break;
		}
		else if(page->page_type == mmu_page_data && mmu_search_page(page, mmu_addr_vtl, vtladdr, address_space_index, tag))
		{
			data_page = page;
			break;
		}
		else if (page->page_type == mmu_page_gpu && mmu_search_page(page, mmu_addr_vtl, vtladdr, address_space_index, tag))
		{
			fatal("mmu_get_page(): found a gpu page\n");

			//gpu_page = page;

			break;
		}

		prev = page;
		page = page->next;
	}

	//make sure this is null if there was no page...
	if(!page)
		prev = NULL;

	/*old code*/

	/*search the page list for a page hit.*/
	/*LIST_FOR_EACH(mmu->page_list, i)
	{
		//pull a page
		page = list_get(mmu->page_list, i);

		//check to see if it is a hit and look for both text and data page types
		if(page->page_type == mmu_page_text && mmu_search_page(page, mmu_addr_vtl, vtladdr, address_space_index, tag))
		{
			text_page = page;
		}
		else if(page->page_type == mmu_page_data && mmu_search_page(page, mmu_addr_vtl, vtladdr, address_space_index, tag))
		{
			data_page = page;
		}
		else if (page->page_type == mmu_page_gpu && mmu_search_page(page, mmu_addr_vtl, vtladdr, address_space_index, tag))
		{
			gpu_page = page;
		}

		printf("page id %d, page type %d\n", page->id, page->page_type);
	}*/

	/*if(access_type == mmu_access_gpu && gpu_page && GPU_HUB_IOMMU == 1)
	{
		printf("page found\n");
	}
	else if(access_type == mmu_access_gpu && !gpu_page && GPU_HUB_IOMMU == 1)
	{
		printf("page NOT found\n");
	}*/

	//figure out if we are looking for a .text or a data page.
	page_type = mmu_get_page_type(access_type);

	/*either return the page or create a new one
	if the segment types do not match create a new page*/

	/*if i found a text or data page and I am a text or data access return the existing page
	otherwise this is an access that doesn't have a mapped page. create a page now.*/
	if(page_type == mmu_page_text && text_page)
	{
		page = text_page;
	}
	else if(page_type == mmu_page_data && data_page)
	{
		page = data_page;
	}
	else if(page_type == mmu_page_gpu && data_page) //star changed this
	{
		if(access_type == mmu_access_gpu && GPU_HUB_IOMMU == 1)
		{
			printf("data page found id %d\n", data_page->id);
		}

		page = data_page;
	}
	else
	{
		page = mmu_create_page(address_space_index, tag, access_type, index, vtladdr);
	}

	/* Locate page at the head of the hash table for faster subsequent lookup */
	if (prev)
	{
		prev->next = page->next;
		page->next = mmu->page_hash_table[index];
		mmu->page_hash_table[index] = page;
	}

	return page;
}


/* Compare two pages */
static int mmu_page_compare(const void *ptr1, const void *ptr2)
{
	struct mmu_page_t *page1 = (struct mmu_page_t *) ptr1;
	struct mmu_page_t *page2 = (struct mmu_page_t *) ptr2;

	long long num_accesses1;
	long long num_accesses2;

	num_accesses1 = page1->num_read_accesses + page1->num_write_accesses + page1->num_execute_accesses;
	num_accesses2 = page2->num_read_accesses + page2->num_write_accesses + page2->num_execute_accesses;
	if (num_accesses1 < num_accesses2)
		return 1;
	else if (num_accesses1 == num_accesses2)
		return 0;
	else
		return -1;
}




/*
 * Public Functions
 */

void mmu_init()
{

	int num_cores = x86_cpu_num_cores;
	char buff[100];
	int i = 0;
	int j = 0;
	int core_issue_width = x86_cpu_issue_width;

	//create event counts
	mmu_fetch_ec = (void *) xcalloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "mmu_fetch_ec_%d", i);
		mmu_fetch_ec[i] = *(new_eventcount(strdup(buff)));
	}

	//create event counts
	mmu_data_ec = (void *) xcalloc(num_cores, sizeof(eventcount));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "mmu_data_ec_%d", i);
		mmu_data_ec[i] = *(new_eventcount(strdup(buff)));
	}

	//create tasks
	mmu_fetch_task = (void *) xcalloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "mmu_task_%d", i);
		mmu_fetch_task[i] = *(create_task(mmu_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	/*mmu_data_task = (void *) xcalloc(num_cores, sizeof(task));
	for(i = 0; i < num_cores; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "mmu_task_%d", i);
		mmu_data_task[i] = *(create_task(mmu_data_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}*/

	/* Variables derived from page size */
	mmu_log_page_size = log_base2(mmu_page_size);
	mmu_page_mask = mmu_page_size - 1;


	/* Initialize */
	mmu = (void *) xcalloc(num_cores, sizeof(struct mmu_t));

	for(i = 0; i < num_cores; i++)
	{
		mmu[i].id = i;

		mmu[i].page_list = list_create_with_size(MMU_PAGE_LIST_SIZE);

		/*star added these*/
		//mmu->request_queue = list_create();
		mmu[i].guest_list = list_create();

		//fetch stuff
		mmu[i].fetch_ready = 1;

		//data stuff
		mmu[i].issue_width = core_issue_width;
		mmu[i].data_ready = (int *)xcalloc(core_issue_width, sizeof(int));
		mmu[i].data_valid = (int *)xcalloc(core_issue_width, sizeof(int));
		mmu[i].data_address_space_index = (int *)xcalloc(core_issue_width, sizeof(int));

		//set up MMY initial state
		for(j = 0; j < core_issue_width; j++)
			mmu[i].data_ready[j] = 1;

		mmu[i].vtl_data_address = (unsigned int *)xcalloc(core_issue_width, sizeof(unsigned int));
		mmu[i].phy_data_address = (unsigned int *)xcalloc(core_issue_width, sizeof(unsigned int));

		mmu[i].num_fault_bits = core_issue_width + 1;
		mmu[i].fault_bits = (int *)xcalloc(mmu[i].num_fault_bits, sizeof(int));


		/* Open report file */
		if (*mmu_report_file_name)
		{
			mmu[i].report_file = file_open_for_write(mmu_report_file_name);
			if (!mmu[i].report_file)
				fatal("%s: cannot open report file for MMU", mmu_report_file_name);
		}

		//warning("makeing mmu %d\n", mmu[i].id);
	}
	//getchar();

}


void mmu_done()
{
	int i;

	/* Dump report */
	mmu_dump_report();

	/* Free pages */
	for (i = 0; i < list_count(mmu->page_list); i++)
		free(list_get(mmu->page_list, i));
	list_free(mmu->page_list);

	/* Free MMU */
	free(mmu);
}


void mmu_dump_report(void)
{
	struct mmu_page_t *page;

	FILE *f;
	int i;

	long long num_accesses;

	/* Report file */
	f = mmu->report_file;
	if (!f)
		return;

	/* Sort list of pages it as per access count */
	list_sort(mmu->page_list, mmu_page_compare);

	/* Header */
	fprintf(f, "%5s %5s %9s %9s %10s %10s %10s %10s\n", "Idx", "MemID", "VtlAddr", "PhyAddr", "Accesses", "Read", "Write", "Exec");
	for (i = 0; i < 77; i++)
		fprintf(f, "-");
	fprintf(f, "\n");

	/* Dump */
	for (i = 0; i < list_count(mmu->page_list); i++)
	{
		page = list_get(mmu->page_list, i);
		num_accesses = page->num_read_accesses + page->num_write_accesses + page->num_execute_accesses;
		fprintf(f, "%5d %5d %9x %9x %10lld %10lld %10lld %10lld\n",
			i + 1, page->address_space_index, page->vtl_addr, page->phy_addr, num_accesses,
			page->num_read_accesses, page->num_write_accesses,
			page->num_execute_accesses);
	}
	fclose(f);
}

/* Obtain an identifier for a new virtual address space */
int mmu_address_space_new(void)
{
	static int mmu_address_space_index;

	return mmu_address_space_index++;
}

unsigned int mmu_get_vtladdr(int address_space_index, unsigned int phy_addr){

	struct mmu_page_t *page = NULL;

	unsigned int offset;
	unsigned int phy_addr_tag;
	unsigned int vtl_addr;
	int i = 0;


	/*get the offset and page tag*/
	offset = phy_addr & mmu_page_mask;
	phy_addr_tag = phy_addr & ~mmu_page_mask;

	/*find the page in the MMU*/
	LIST_FOR_EACH(mmu->page_list, i)
	{
		//pull a page
		page = list_get(mmu->page_list, i);

		//check to see if it is a hit and look for both text and data page types
		if(mmu_search_page(page, mmu_addr_phy, phy_addr, address_space_index, phy_addr_tag))
		{
			break;
		}
	}

	assert(page);
	vtl_addr = page->vtl_addr | offset;

	return vtl_addr;
}

unsigned int mmu_get_phyaddr(int address_space_index, unsigned int vtl_addr, enum mmu_access_t access_type){

	struct mmu_page_t *page;

	unsigned int offset;
	unsigned int phy_addr;

	page = mmu_page_access(address_space_index, mmu_addr_vtl, vtl_addr, access_type);
	assert(page);

	offset = vtl_addr & mmu_page_mask;
	phy_addr = page->phy_addr | offset;

	return phy_addr;
}

int mmu_get_page_id(int address_space_index, enum mmu_address_type_t addr_type, unsigned int vtl_addr, enum mmu_access_t access_type){

	struct mmu_page_t *page;

	page = mmu_page_access(address_space_index, addr_type, vtl_addr, access_type);

	return page->id;
}

enum mmu_page_type_t mmu_get_page_type(enum mmu_access_t access_type){

	enum mmu_page_type_t page_type = mmu_page_invalid;

	if(access_type == mmu_access_fetch)
	{
		page_type = mmu_page_text;
	}
	else if(access_type == mmu_access_load_store)
	{
		page_type = mmu_page_data;
	}
	else if(access_type == mmu_access_gpu)
	{
		page_type = mmu_page_gpu;
	}
	else if(access_type == mmu_access_gpu_coherent)
	{
		page_type = mmu_page_data;
	}
	else
	{
		fatal("mmu_get_page_type(): invalid page access_type\n");
	}

	return page_type;
}


void mmu_add_guest(int address_space_index, int guest_pid, unsigned int guest_ptr, unsigned int host_ptr, unsigned int size){

	struct page_guest_t *guest;

	unsigned int blk_addr_mask = 0x3F;
	unsigned int quest_base_blk_aligned = 0;
	unsigned int quest_top_blk_aligned = 0;

	unsigned int host_base_blk_aligned = 0;
	unsigned int host_top_blk_aligned = 0;

	unsigned int host_base_aligned_size = 0;
	unsigned int host_top_aligned_size = 0;

	/*int host_vtl_addr_index = 0;

	unsigned int cpu_phy_address = 0;
	unsigned int vtl_index = 0;
	unsigned int phy_index = 0;

	unsigned int offset = 0;*/

	/*add the guest device to the page guest list*/
	guest = mmu_create_guest();
	guest->guest_pid = guest_pid;


	////////
	//bottom
	////////

	//get the blk address of the base of the address range for the host and the guest
	//calculate the size of the shift required to align the blocks...
	host_base_blk_aligned = host_ptr & ~blk_addr_mask;
	host_base_aligned_size = host_ptr - host_base_blk_aligned;
	assert(host_base_aligned_size <= blk_addr_mask); //better not be larger than a single block.

	/////
	//top
	/////

	host_top_blk_aligned = ((host_ptr + size) & ~blk_addr_mask) + blk_addr_mask; //gets the last byte of the offset
	host_top_aligned_size = host_top_blk_aligned - (host_ptr + size);
	assert(host_top_aligned_size <= blk_addr_mask); //better not be larger than a single block.


	/*calculate the guest base aligned address (aligned to the CPU's blk)*/
	quest_base_blk_aligned = guest_ptr - host_base_aligned_size;
	quest_top_blk_aligned = guest_ptr + size + host_top_aligned_size;


	//set the values

	guest->guest_vtl_addr_base = quest_base_blk_aligned;
	guest->guest_vtl_addr_top = quest_top_blk_aligned;

	guest->host_vtl_addr_base = host_base_blk_aligned;
	guest->host_vtl_addr_top = host_top_blk_aligned;


	/*guest->guest_vtl_addr_base = guest_ptr;
	guest->guest_vtl_addr_top = guest->guest_vtl_addr_base + size;

	guest->host_vtl_addr_base = host_ptr;
	guest->host_vtl_addr_top = guest->host_vtl_addr_base + size;*/

	//save size
	guest->size = size + host_base_aligned_size + host_top_aligned_size;


	/*printf("guest_base 0x%08x host_base 0x%08x guest_top 0x%08x host_top 0x%08x\n",
			guest_ptr, host_ptr, (guest_ptr + size), (host_ptr + size));

	printf("guest_base 0x%08x guest top 0x%08x\n",
			guest->guest_vtl_addr_base, guest->guest_vtl_addr_top);

	printf("host_base 0x%08x host_top 0x%08x\n",
			guest->host_vtl_addr_base, guest->host_vtl_addr_top);

	printf("size %u bottom_size %u top_size %u total %u\n",
			size, host_base_aligned_size, host_top_aligned_size, (size + host_base_aligned_size + host_top_aligned_size));*/


	/*GPU's vtl address has on io-mmu table needs to result in correct CPU vtl addr hash to find the page
	cpu_phy_address = mmu_get_phyaddr(address_space_index, guest->host_vtl_addr_base, mmu_access_load_store);

	build the host_vtl_addr
	offset = (cpu_phy_address & mmu_page_mask);

	if((mmu_page_mask - offset) < size)
		fatal("fixme\n");*/


	/*for future translations*/
	/*vtl_index = ((guest->guest_vtl_addr_base >> mmu_log_page_size) + address_space_index * 23) % MMU_PAGE_HASH_SIZE;
	phy_index = ((cpu_phy_address >> mmu_log_page_size) + address_space_index * 23) % MMU_PAGE_HASH_SIZE;

	hub_iommu->page_hash_table[phy_index] = vtl_index;*/

	/*printf("guest_base 0x%08x guest_base_blk 0x%08x host_base 0x%08x host_base_blk 0x%08x\n",
			guest->guest_vtl_addr_base, (guest->guest_vtl_addr_base & ~blk_addr_mask), guest->host_vtl_addr_base, (guest->host_vtl_addr_base & ~blk_addr_mask));*/


	if(GPU_HUB_IOMMU == 1)
		printf("mmu guest added id %d guest base address 0x%08x host base address 0x%08x guest top address 0x%08x host top addr 0x%08x size %u\n",
				guest_pid, guest_ptr, host_ptr, guest->guest_vtl_addr_top, guest->host_vtl_addr_top, size);

	//fatal("end\n");

	list_enqueue(mmu->guest_list, guest);

	return;
}

unsigned int mmu_forward_link_guest_address(int guest_pid, unsigned int guest_vtl_addr){

	struct page_guest_t *guest = NULL;
	unsigned int host_vtl_addr = 0;
	unsigned int guest_shift = 0;

	int i = 0;
	int hit = 0;

	/*search the page list for a page hit.*/
	LIST_FOR_EACH(mmu->guest_list, i)
	{
		guest = list_get(mmu->guest_list, i);

		//check to see if page is linked to a quest
		if(guest_pid == guest->guest_pid && guest_vtl_addr >= guest->guest_vtl_addr_base && guest_vtl_addr <= guest->guest_vtl_addr_top)
		{
			hit = 1;

			//calculate the equivalent host address.
			guest_shift = guest_vtl_addr - guest->guest_vtl_addr_base;
			host_vtl_addr = guest->host_vtl_addr_base + guest_shift;
		}
	}

	if(hit != 1)
	{
		warning("mmu_forward_link_guest_address(): failed guest id %d guest_vtl_addr 0x%08x cycle %llu\n", guest_pid, guest_vtl_addr, P_TIME);
		getchar();
	}

	assert(hit == 1);
	return host_vtl_addr;
}

unsigned int mmu_reverse_link_host_address(int guest_pid, unsigned int host_vtl_addr){

	struct page_guest_t *guest = NULL;
	unsigned int guest_vtl_addr = 0;
	unsigned int host_shift = 0;

	int i = 0;
	int hit = 0;

	/*search the page list for a page hit.*/
	LIST_FOR_EACH(mmu->guest_list, i)
	{
		guest = list_get(mmu->guest_list, i);

		//check to see if page is linked to a quest
		if(guest_pid == guest->guest_pid && host_vtl_addr >= guest->host_vtl_addr_base && host_vtl_addr <= guest->host_vtl_addr_top)
		{
			hit = 1;

			//calculate the equivalent guest address.
			host_shift = host_vtl_addr - guest->host_vtl_addr_base;
			guest_vtl_addr = guest->guest_vtl_addr_base + host_shift;
		}
	}

	if(hit != 1)
	{
		warning("trying to link host vtl addr 0x%08x\n", host_vtl_addr);
	}


	assert(hit == 1);
	return guest_vtl_addr;
}


unsigned int mmu_reverse_translate(int address_space_index, unsigned int phy_addr, enum mmu_access_t access_type){

	struct mmu_page_t *page;
	unsigned int vtl_addr;
	unsigned int offset;

	assert(address_space_index == 1);

	page = mmu_page_access(address_space_index, mmu_addr_phy, phy_addr, access_type);
	assert(page);

	/*build the host_vtl_addr*/
	offset = (phy_addr & mmu_page_mask);
	vtl_addr = page->vtl_addr | offset;

	return vtl_addr;
}

unsigned int mmu_reverse_translate_guest(int address_space_index, int guest_pid, unsigned int host_phy_addr){

	struct mmu_page_t *page;
	//unsigned int host_vtl_addr;
	unsigned int guest_vtl_addr;
	unsigned int offset;

	assert(address_space_index == 0);

	page = mmu_page_access(address_space_index, mmu_addr_phy, host_phy_addr, mmu_access_gpu_coherent);
	assert(page);

	/*build the host_vtl_addr*/
	offset = (host_phy_addr & mmu_page_mask);
	//host_vtl_addr = page->vtl_addr | offset;

	guest_vtl_addr = page->vtl_addr | offset;


	/*translate to the guest address space*/

	/*before change*/
	//guest_vtl_addr = mmu_reverse_link_host_address(guest_pid, host_vtl_addr);

	/*fatal("mmu reverse trans page %d guest id %d host_phy_addr 0x%08x offset 0x%08x host_vtl_addr 0x%08x guest_vtl_addr 0x%08x\n",
			page->id, guest_pid, host_phy_addr, offset, host_vtl_addr, guest_vtl_addr);*/

	return guest_vtl_addr;
}

int can_access_mmu(void){

	//int busy = 0;

	/*//check if request queue is full
	if(QueueSize <= list_count(thread->i_cache_ptr[thread->core->id].Rx_queue_top))
	{
		return 0;
	}*/

	//mmu is accessible.
	return 0;
}

int mmu_get_free_port(struct mmu_t *mmu, unsigned int addr, int *port_num){

	int i = 0;

	/*mmu->data_ready[0] = 0;
	mmu->data_ready[1] = 0;
	mmu->data_ready[2] = 0;
	mmu->data_ready[3] = 0;*/

	for(i = 0; i < mmu->issue_width; i++)
	{
		if((mmu->data_ready[i] == 1) && (mmu->vtl_data_address[i] != addr))
		{
			*port_num = i;
			return 1;
		}

	}

	return 0;
}


enum uop_translation_status{
	uop_trans_invalid = 0,
	new_request,
	in_process,
	complete,
	number_status_types
};

int mmu_is_access_pending(struct mmu_t *mmu, unsigned int addr, int address_space_index, int *port_num){

	int i = 0;
	int state = 0;

	for(i = 0; i < mmu->issue_width; i++)
	{
		if((mmu->data_ready[i] == 0) && (mmu->data_valid[i] == 1) && (mmu->vtl_data_address[i] == addr) && (mmu->data_address_space_index[i] == address_space_index))
		{
			state = 1;
			*port_num = i;
			break;
		}
	}

	return state;
}

int mmu_is_access_complete(struct mmu_t *mmu, unsigned int addr, int address_space_index, int *port_num){

	int i = 0;
	int state = 0;

	for(i = 0; i < mmu->issue_width; i++)
	{
		if((mmu->data_ready[i] == 1) && (mmu->data_valid[i] == 1) && (mmu->vtl_data_address[i] == addr) && (mmu->data_address_space_index[i] == address_space_index))
		{
			//translation complete take the address
			state = 1;
			*port_num = i;
			break;
		}
	}

	return state;
}

enum uop_translation_status mmu_get_status(struct mmu_t *mmu, unsigned int addr, int address_space_index, int *port_num){

	enum uop_translation_status status;

	//is this a new access?
	if(mmu_is_access_pending(mmu, addr, address_space_index, port_num))
		status = in_process;
	else if(mmu_is_access_complete(mmu, addr, address_space_index, port_num))
		status = complete;
	else
		status = new_request;

	return status;
}



int mmu_data_translate(X86Thread *self, struct x86_uop_t *uop){

	int stall = 1;
	int port_num = -1;
	int *port_num_ptr = &port_num;
	enum uop_translation_status status = uop_trans_invalid;

	//if(uop->id == 521)
	//	warning("trans start id %llu vtl addr 0x%08x phy addr 0x%08x opcode %d cycle %llu\n", uop->id, uop->uinst->address, uop->phy_addr, uop->uinst->opcode, P_TIME);

	//get the status
	status = mmu_get_status(&mmu[self->core->id], uop->uinst->address, self->ctx->address_space_index, port_num_ptr);


	//if(uop->id == 521)
		//fatal("trans start status %d id %llu vtl addr 0x%08x phy addr 0x%08x opcode %d cycle %llu\n",
		//		status, uop->id, uop->uinst->address, uop->phy_addr, uop->uinst->opcode, P_TIME);

	//fatal("status is %d\n", status);

	//need switch here...
	//new access
		//if fault quit
		//if no port free quit
		//if port free process
	//in process
		//if fault quit
		//if in process quit
	//complete
		//provide translated address

	switch(status)
	{

		case uop_trans_invalid:
		case number_status_types:
			fatal("mmu_data_translate(): Invalid access state");
			break;

		case new_request:

			/*ORDER MATTERS HERE*/
			assert(port_num == -1);

			//if MMU has fault don't process
			if(mmu_has_fault(&mmu[self->core->id]))
			{
				/*MMU is faulted in either i or D cache, can't process now*/
				stall = 0;
				return stall;
			}

			//check for free port
			if(!mmu_get_free_port(&mmu[self->core->id], uop->uinst->address, port_num_ptr))
			{
				//all ports are busy
				stall = 0;
				return stall;
			}

			//if(uop->id == 521)
			//	warning("trans start id %llu vtl addr 0x%08x phy addr 0x%08x opcode %d cycle %llu\n", uop->id, uop->uinst->address, uop->phy_addr, uop->uinst->opcode, P_TIME);

			mmu[self->core->id].vtl_data_address[port_num] = uop->uinst->address;
			mmu[self->core->id].data_ready[port_num] = 0;
			mmu[self->core->id].data_valid[port_num] = 1;
			mmu[self->core->id].data_address_space_index[port_num] = self->ctx->address_space_index;


			advance(&mmu_fetch_ec[self->core->id]);
			//warning("advance mmmu cycle %llu\n", P_TIME);
			stall = 0;

			//warning("starting id %llu port num %d cycle %llu\n", uop->id, port_num, P_TIME);

			break;

		case in_process:

			assert(mmu[self->core->id].data_ready[port_num] == 0);
			assert(mmu[self->core->id].vtl_data_address[port_num] == uop->uinst->address);

			//warning("inprog id %llu port num %d cycle %llu\n", uop->id, port_num, P_TIME);

			stall = 0;

			break;

		case complete:

			//mmu complete with current translation
			assert(uop->uinst->address == mmu[self->core->id].vtl_data_address[port_num]);

			//if(uop->id == 20204)
			//	warning("tans 0x%08x", mmu_translate(0, 0x080480cc, mmu_access_load_store));

			//warning("complete id %llu op %d port num %d cycle %llu vtl 0x%08x uop phy 0x%08x mmu 0x%08x\n", uop->id, uop->uinst->opcode, port_num, P_TIME, uop->uinst->address, uop->phy_addr, mmu[self->core->id].phy_data_address[port_num]);

			if(uop->phy_addr != mmu[self->core->id].phy_data_address[port_num])
				warning("---uop %llu physical address miss match on translation cycle %llu---\n", uop->id, P_TIME);

			//assert(uop->phy_addr == mmu[self->core->id].phy_data_address[port_num]);
			mmu[self->core->id].data_valid[port_num] = 0;
			uop->phy_addr = mmu[self->core->id].phy_data_address[port_num];

			//if(uop->id == 521)
			//	fatal("trans start id %llu vtl addr 0x%08x phy addr 0x%08x opcode %d cycle %llu\n", uop->id, uop->uinst->address, uop->phy_addr, uop->uinst->opcode, P_TIME);


			/*MMU statistics*/
			if (*mmu_report_file_name)
				mmu_access_page(&mmu[self->core->id], uop->uinst->address, mmu_access_execute);

			//fatal("complete id %llu port num %d cycle %llu uop 0x%08x mmu 0x%08x\n", uop->id, port_num, P_TIME, uop->phy_addr, mmu[self->core->id].phy_data_address[port_num]);
			return stall;

			break;

	}

	return stall;
}

int mmu_has_fault(struct mmu_t *mmu){

	int fault_status = 0;
	int i = 0;

	for(i = 0; i < mmu->num_fault_bits; i++)
	{
		if(mmu->fault_bits[i] == 1)
		{
			fault_status = 1;
			break;
		}
	}

	return fault_status;
}


int mmu_fetch_translate(X86Thread *self, unsigned int block){

	int stall = 1;

	/*ORDER MATTERS HERE*/


	//if translation complete for the current address space index then continue
	if ((mmu[self->core->id].fetch_ready == 1) &&
			(mmu[self->core->id].fetch_address_space_index == self->ctx->address_space_index) &&
			(self->fetch_neip == mmu[self->core->id].vtl_fetch_address))
	{
		//mmu complete with current translation
		assert(self->fetch_neip == mmu[self->core->id].vtl_fetch_address);
		self->fetch_block = block;
		self->fetch_address = mmu[self->core->id].phy_fetch_address;

		/*MMU statistics*/
		if (*mmu_report_file_name)
			mmu_access_page(&mmu[self->core->id], self->fetch_address, mmu_access_execute);

		return stall;
	}

	//if MMU has fault don't process
	if(mmu_has_fault(&mmu[self->core->id]))
	{
		/*MMU is faulted in either i or D cache, can't process now*/
		stall = 0;
		return stall;
	}


	//compare the vtrl address to the address in the MMU
	if((mmu[self->core->id].fetch_ready == 1) &&
			(self->fetch_neip != mmu[self->core->id].vtl_fetch_address))
	{
		//warning("fetch mmu request %llu\n", P_TIME);

		//mmu is ready for new translation, advance and return
		mmu[self->core->id].vtl_fetch_address = self->fetch_neip;
		mmu[self->core->id].fetch_ready = 0;
		mmu[self->core->id].fetch_address_space_index = self->ctx->address_space_index;
		advance(&mmu_fetch_ec[self->core->id]);
		//warning("advance mmmu cycle %llu\n", P_TIME);
		stall = 0;
	}
	else
	{
		assert(mmu[self->core->id].fetch_ready == 0);
		assert(self->fetch_neip == mmu[self->core->id].vtl_fetch_address);

		stall = 0;
	}

	return stall;
}

/*void mmu_data_ctrl(void){

	int my_pid = mmu_data_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;
	//long long occ_start = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	//unsigned int phy_address;
	//unsigned int vtl_address;

	int i = 0;

	while(1)
	{

		await(&mmu_data_ec[my_pid], step); //&mmu_ec[my_pid], step);
		step++;

		fatal("data_ctrl\n");

	}

	fatal("mmu_data_ctrl(): out of loop\n");

	return;
}*/

void mmu_ctrl(void){

	//my_pid increments for the number of CPU cores. i.e. 0 - 4 for a quad core
	int my_pid = mmu_fetch_pid++;
	int num_cores = x86_cpu_num_cores;
	long long step = 1;
	//long long occ_start = 0;

	assert(my_pid <= num_cores);
	set_id((unsigned int)my_pid);

	//unsigned int phy_address;
	//unsigned int vtl_address;

	int i = 0;
	int num_translations = 0;
	//int num_requests = 0;

	//int cache_block_hit;
	//int cache_block_state;
	//int *cache_block_hit_ptr = &cache_block_hit;
	//int *cache_block_state_ptr = &cache_block_state;
	//struct cgm_packet_t *write_back_packet = NULL;

	//int set = 0;
	//int tag = 0;
	//unsigned int offset = 0;
	//int way = 0;

	//int *set_ptr = &set;
	//int *tag_ptr = &tag;
	//unsigned int *offset_ptr = &offset;
	//int *way_ptr = &way;

	while(1)
	{
		//wait here until there is a job to do
		await(&mmu_fetch_ec[my_pid], step); //&mmu_ec[my_pid], step);

		//printf("mmu fetc %d data %d\n", mmu[my_pid].fetch_ready, mmu[my_pid].data_ready[0]);

		assert(mmu[my_pid].fetch_ready == 0
				|| mmu[my_pid].data_ready[0] == 0
				|| mmu[my_pid].data_ready[1] == 0
				|| mmu[my_pid].data_ready[2] == 0
				|| mmu[my_pid].data_ready[3] == 0);

		//ORDER MATTERS HERE!!!!!
		//check caches first this "current" cpu cycle
		//put all TLB hits in the right registers, mark complete and raise exceptions where there is a miss.

		//check the I_TLB
		if(mmu[my_pid].fetch_ready == 0)
		{
			//there is a fetch translation waiting....

			//probe the address



			mmu[my_pid].phy_fetch_address = mmu_translate(mmu[my_pid].fetch_address_space_index, mmu[my_pid].vtl_fetch_address, mmu_access_fetch);
			mmu[my_pid].fetch_ready = 1;
			num_translations++;
		}


		//check the D_TLB









		for(i = 0; i < mmu[my_pid].issue_width; i++)
		{
			if(mmu[my_pid].data_ready[i] == 0)
			{
				mmu[my_pid].phy_data_address[i] = mmu_translate(mmu[my_pid].data_address_space_index[i], mmu[my_pid].vtl_data_address[i], mmu_access_load_store);
				mmu[my_pid].data_ready[i] = 1;
				num_translations++;
			}
		}

		//if ALL hits charge single cycle and end
		P_PAUSE(1);

		//on a single TLB miss the processor will trap to OS. raise exception flag and access PTW.

		/*if(num_translations > 2)
		{
			printf("translations %d cycle %llu\n", num_translations, P_TIME);
			getchar();
		}*/

		step += num_translations;
		num_translations = 0;

		/*for(i = 0; i < mmu[my_pid].issue_width; i++)
		{
			if(mmu[my_pid].data_ready[i] == 0)
			{
				mmu[my_pid].phy_data_address[i] = mmu_translate(mmu[my_pid].data_address_space_index[i], mmu[my_pid].vtl_data_address[i], mmu_access_load_store);
				mmu[my_pid].data_ready[i] = 1;
			}
		}*/
	}

	fatal("mmu_fetch_ctrl(): out of loop\n");

	return;
}


unsigned int mmu_forward_translate_guest(int address_space_index, int guest_pid, unsigned int guest_vtl_addr){

	struct mmu_page_t *page;
	//unsigned int host_vtl_addr;
	unsigned int host_phy_addr;
	unsigned int offset;

	int vtl_index = 0;
	int phy_index = 0;

	assert(address_space_index == 0);
	/* link the guest and host pages*/
	//before
	//host_vtl_addr = mmu_forward_link_guest_address(guest_pid, guest_vtl_addr);
	//page = mmu_page_access(address_space_index, mmu_addr_vtl, host_vtl_addr, mmu_access_load_store);

	page = mmu_page_access(address_space_index, mmu_addr_vtl, guest_vtl_addr, mmu_access_load_store);
	assert(page);

	if(GPU_HUB_IOMMU == 1)
		printf("mmu trans page %d guest id %d guest_vtl_addr 0x%08x\n", page->id, guest_pid, guest_vtl_addr);
		//printf("mmu trans page %d guest id %d host vtl_addr 0x%08x\n", page->id, guest_pid, host_vtl_addr);


	//offset = host_vtl_addr & mmu_page_mask;
	offset = guest_vtl_addr & mmu_page_mask;
	host_phy_addr = page->phy_addr | offset;

	/*for future reverse translations*/
	vtl_index = ((guest_vtl_addr >> mmu_log_page_size) + address_space_index * 23) % MMU_PAGE_HASH_SIZE;
	phy_index = ((host_phy_addr >> mmu_log_page_size) + address_space_index * 23) % MMU_PAGE_HASH_SIZE;

	hub_iommu->page_hash_table[phy_index] = vtl_index;


	return host_phy_addr;
}


struct page_guest_t *mmu_create_guest(void){

	struct page_guest_t *new_guest;

	new_guest = xcalloc(1, sizeof(struct page_guest_t));

	return new_guest;

}

struct mmu_page_t *mmu_create_page(int address_space_index, unsigned int tag, enum mmu_access_t access_type, int index, unsigned int vtl_addr){

	//struct mmu_page_t *prev = NULL;
	struct mmu_page_t *page = NULL;
	//char buff[100];

	/* Initialize */
	page = xcalloc(1, sizeof(struct mmu_page_t));

	//assign page id
	page->id = page_number;
	page_number++;

	page->vtl_addr = tag;
	page->address_space_index = address_space_index;
	page->phy_addr = list_count(mmu->page_list) << mmu_log_page_size;

	/*star added this here,
	this tells us if the page is a .text or data page.*/
	if(access_type == mmu_access_fetch)
	{
		page->page_type = mmu_page_text;
	}
	else if(access_type == mmu_access_load_store)
	{
		page->page_type = mmu_page_data;
	}
	else if(access_type == mmu_access_gpu)
	{
		if(access_type == mmu_access_gpu && GPU_HUB_IOMMU == 1)
		{
			printf("created page for gpu %d\n", page->id);
		}

		page->page_type = mmu_page_data;
	}
	else
	{
		fatal("mmu_get_page(): invalid access_type\n");
	}

	/* Insert in page list */
	list_add(mmu->page_list, page);

	/* Insert in page hash table */
	page->next = mmu->page_hash_table[index];
	mmu->page_hash_table[index] = page;
	//prev = NULL;

	return page;
}



unsigned int mmu_translate(int address_space_index, unsigned int vtl_addr, enum mmu_access_t access_type)
{
	struct mmu_page_t *page;

	unsigned int offset;
	unsigned int phy_addr;
	int vtl_index = 0;
	int phy_index = 0;

	if(access_type == mmu_access_gpu && GPU_HUB_IOMMU == 1)
	{
		printf("translating gpu vtl_addr 0x%08x\n", vtl_addr);
	}

	//star added this check. The address space doesn't change for 1 CPU.
	//printf("address_space_index %d\n", address_space_index);
	//assert(address_space_index == 0);

	page = mmu_get_page(address_space_index, vtl_addr, access_type);
	assert(page);



	offset = vtl_addr & mmu_page_mask;
	phy_addr = page->phy_addr | offset;

	//fatal("vtl addr 0x%08x offset 0x%08x page phy addr 0x%08x phy 0x%08x \n", vtl_addr, offset, page->phy_addr, phy_addr);


	//check for a protection fault.
	if(page->page_type == mmu_page_text && access_type != mmu_access_fetch)
	{
		fatal("mmu_translate(): 1 protection fault load or store to text segment type %d vtrl_addr 0x%08x phy_addr 0x%08x page_id %d cycle %llu\n",
				access_type, vtl_addr, phy_addr, mmu_get_page_id(0, mmu_addr_vtl, vtl_addr, mmu_access_fetch), P_TIME);
	}
	else if(page->page_type == mmu_page_data && (access_type != mmu_access_load_store && access_type != mmu_access_gpu))
	{
		fatal("mmu_translate(): 2 protection fault fetch to data segment type %d vtrl_addr 0x%08x phy_addr 0x%08x page_id %d cycle %llu\n",
				access_type, vtl_addr, phy_addr, mmu_get_page_id(0, mmu_addr_vtl, vtl_addr, mmu_access_load_store), P_TIME);
	}
	else if(page->page_type == mmu_page_gpu && access_type != mmu_access_gpu)
	{
		fatal("mmu_translate(): 3 protection fault fetch to text segment type %d vtrl_addr 0x%08x phy_addr 0x%08x page_id %d cycle %llu\n",
				access_type, vtl_addr, phy_addr, mmu_get_page_id(0, mmu_addr_vtl, vtl_addr, mmu_access_gpu), P_TIME);
	}

	/*for future reverse translations*/
	if(access_type == mmu_access_gpu)
	{
		vtl_index = ((vtl_addr >> mmu_log_page_size) + address_space_index * 23) % MMU_PAGE_HASH_SIZE;
		phy_index = ((phy_addr >> mmu_log_page_size) + address_space_index * 23) % MMU_PAGE_HASH_SIZE;

		hub_iommu->page_hash_table[phy_index] = vtl_index;
	}

	return phy_addr;
}


int mmu_valid_phy_addr(unsigned int phy_addr)
{
	int index;

	index = phy_addr >> mmu_log_page_size;
	return index < mmu->page_list->count;
}


void mmu_access_page(struct mmu_t *mmu, unsigned int phy_addr, enum mmu_access_t access)
{
	struct mmu_page_t *page;
	int index;

	/* Get page */
	index = phy_addr >> mmu_log_page_size;
	page = list_get(mmu->page_list, index);
	if (!page)
		return;

	/* Record access */
	switch (access)
	{
	case mmu_access_read:
		page->num_read_accesses++;
		break;

	case mmu_access_write:
		page->num_write_accesses++;
		break;

	case mmu_access_execute:
		page->num_execute_accesses++;
		break;

	default:
		panic("%s: invalid access", __FUNCTION__);
	}
}
