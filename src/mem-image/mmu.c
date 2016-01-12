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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include <mem-image/mmu.h>


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

/*
 * Private variables
 */

/* Local constants */
#define MMU_PAGE_HASH_SIZE  (1 << 10) /*1024*/
#define MMU_PAGE_LIST_SIZE  (1 << 10) /*1024*/

/* Physical memory page */
struct mmu_page_t
{
	int id;

	struct mmu_page_t *next;

	int address_space_index;  /* Memory map ID */
	unsigned int vtl_addr;  /* Virtual address of page */
	unsigned int phy_addr;  /* Physical address */

	enum mmu_page_type_t page_type; /*type of page either .text or data segments*/

	/*Statistics*/

	long long num_read_accesses;
	long long num_write_accesses;
	long long num_execute_accesses;
};

/* Memory management unit */
struct mmu_t
{
	/* List of pages */
	struct list_t *page_list;

	/* Hash table of pages */
	//struct mmu_page_t *page_hash_table[MMU_PAGE_HASH_SIZE];

	/* Report file */
	FILE *report_file;
};

static struct mmu_t *mmu;


#include <cgm/cgm.h>

/*
 * Private Functions
 */

struct mmu_page_t *mmu_page_access(int address_space_index, unsigned int vtladdr, enum mmu_access_t access_type){

	struct mmu_page_t *prev = NULL;
	struct mmu_page_t *page = NULL;
	struct mmu_page_t *text_page = NULL;
	struct mmu_page_t *data_page = NULL;
	unsigned int tag;
	int index;
	int i = 0;

	enum mmu_page_type_t page_type;

	/* Look for page */
	index = ((vtladdr >> mmu_log_page_size) + address_space_index * 23) % MMU_PAGE_HASH_SIZE;
	tag = vtladdr & ~mmu_page_mask;

	/*search the page list for a page hit.*/
	LIST_FOR_EACH(mmu->page_list, i)
	{
		//pull a page
		page = list_get(mmu->page_list, i);

		//check to see if it is a hit and look for both text and data page types
		if(mmu_search_page(page, vtladdr, address_space_index, tag))
		{
			//assign to text or data page ptr
			if(page->page_type == mmu_page_text)
			{
				text_page = page;
			}
			else if(page->page_type == mmu_page_data)
			{
				data_page = page;
			}
		}
		/*printf("page id %d, page type %d\n", page->id, page->page_type);*/
	}

	//figure out if we are looking for a .text or a data page.
	page_type = mmu_get_page_type(access_type);

	if(page_type == mmu_page_text && text_page)
	{
		return text_page;
	}
	else if(page_type == mmu_page_data && data_page)
	{
		return data_page;
	}
	else
	{
		fatal("mmu_page_access(): page miss\n");
	}



	/*page = mmu->page_hash_table[index];

	while (page)
	{
		if (page->vtl_addr == tag && page->address_space_index == address_space_index)
			break;
		prev = page;
		page = page->next;
	}*/

	assert(page);

	/* Return it */
	return page;
}

int mmu_search_page(struct mmu_page_t *page, unsigned int vtl_addr, int address_space_index, int tag){

	int hit = 0;

	if (page->vtl_addr == tag && page->address_space_index == address_space_index)
	{
		hit = 1;
	}

	return hit;
}


struct mmu_page_t *mmu_get_page(int address_space_index, unsigned int vtladdr, enum mmu_access_t access_type)
{
	//struct mmu_page_t *prev = NULL;
	struct mmu_page_t *page = NULL;
	struct mmu_page_t *text_page = NULL;
	struct mmu_page_t *data_page = NULL;
	unsigned int tag;
	int index;
	/*int num_pages;*/
	int i = 0;

	enum mmu_page_type_t page_type;

	/* Look for page */
	index = ((vtladdr >> mmu_log_page_size) + address_space_index * 23) % MMU_PAGE_HASH_SIZE;
	tag = vtladdr & ~mmu_page_mask;
	//prev = NULL;

	/*search the page list for a page hit.*/
	LIST_FOR_EACH(mmu->page_list, i)
	{
		//pull a page
		page = list_get(mmu->page_list, i);

		//check to see if it is a hit and look for both text and data page types
		if(mmu_search_page(page, vtladdr, address_space_index, tag))
		{
			//assign to text or data page ptr
			if(page->page_type == mmu_page_text)
			{
				text_page = page;
			}
			else if(page->page_type == mmu_page_data)
			{
				data_page = page;
			}
		}
		/*printf("page id %d, page type %d\n", page->id, page->page_type);*/
	}

	//figure out if we are looking for a .text or a data page.
	page_type = mmu_get_page_type(access_type);

	/*either return the page or create a new one
	if the segment types do not match create a new page*/

	/*if i found a text or data page and I am a text or data access return the existing page
	otherwise this is an access that doesn't have a mapped page. create a page now.*/
	if(page_type == mmu_page_text && text_page)
	{
		return text_page;
	}
	else if(page_type == mmu_page_data && data_page)
	{
		return data_page;
	}
	else
	{
		return mmu_create_page(address_space_index, tag, access_type, index, vtladdr);
	}

	/*page = mmu->page_hash_table[index];*/



	//this searches the page list for an existing page.
	//page = mmu_search_page(page, vtladdr, address_space_index, tag, access_type, page_type);

	/*while(page)
	{
		if (page->vtl_addr == tag && page->address_space_index == address_space_index)
		{
			break;
		}

		prev = page;
		page = page->next;
	}*/
	
	/* Not found */
	/*if (!page)// == NULL || found == 0)
	{
		page = mmu_create_page(address_space_index, tag, access_type, index, vtladdr);
	}*/

	/* Locate page at the head of the hash table for faster subsequent lookup */
	/*if (prev)
	{
		prev->next = page->next;
		page->next = mmu->page_hash_table[index];
		mmu->page_hash_table[index] = page;
	}*/

	/* Return it */
	/*return page;*/
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
	/* Variables derived from page size */
	mmu_log_page_size = log_base2(mmu_page_size);
	mmu_page_mask = mmu_page_size - 1;

	/*printf("%d\n", mmu_log_page_size);
	printf("%d\n", mmu_page_mask);
	getchar();*/

	/* Initialize */
	mmu = xcalloc(1, sizeof(struct mmu_t));
	mmu->page_list = list_create_with_size(MMU_PAGE_LIST_SIZE);

	/* Open report file */
	if (*mmu_report_file_name)
	{
		mmu->report_file = file_open_for_write(mmu_report_file_name);
		if (!mmu->report_file)
			fatal("%s: cannot open report file for MMU", mmu_report_file_name);
	}
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

unsigned int mmu_get_phyaddr(int address_space_index, unsigned int vtl_addr, enum mmu_access_t access_type){

	struct mmu_page_t *page;

	unsigned int offset;
	unsigned int phy_addr;

	page = mmu_page_access(address_space_index, vtl_addr, access_type);

	offset = vtl_addr & mmu_page_mask;
	phy_addr = page->phy_addr | offset;

	return phy_addr;
}

int mmu_get_page_id(int address_space_index, unsigned int vtl_addr, enum mmu_access_t access_type){

	struct mmu_page_t *page;

	page = mmu_page_access(address_space_index, vtl_addr, access_type);

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
	else
	{
		fatal("mmu_get_page_type(): invalid page access_type\n");
	}

	return page_type;
}


struct mmu_page_t *mmu_create_page(int address_space_index, unsigned int tag, enum mmu_access_t access_type, int index, unsigned int vtl_addr){

	struct mmu_page_t *prev = NULL;
	struct mmu_page_t *page = NULL;

	/* Initialize */
	page = xcalloc(1, sizeof(struct mmu_page_t));

	//assign page id
	page->id = page_number;
	page_number++;

	/*printf("---page created %d\n", page_number);*/

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
	else
	{
		fatal("mmu_get_page(): invalid access_type\n");
	}

	/* Insert in page list */
	list_add(mmu->page_list, page);

	/* Insert in page hash table */
	/*page->next = mmu->page_hash_table[index];
	mmu->page_hash_table[index] = page;
	prev = NULL;*/

	return page;
}



unsigned int mmu_translate(int address_space_index, unsigned int vtl_addr, enum mmu_access_t access_type)
{
	struct mmu_page_t *page;

	unsigned int offset;
	unsigned int phy_addr;
	//unsigned int tag;
	//unsigned int index;

	//star added this check. The address space doesn't change for 1 CPU.
	assert(address_space_index == 0);

	page = mmu_get_page(address_space_index, vtl_addr, access_type);
	assert(page);

	//star added these two
	//index = ((vtl_addr >> mmu_log_page_size) + address_space_index * 23) % MMU_PAGE_HASH_SIZE;
	//tag = (vtl_addr & ~mmu_page_mask) >> mmu_log_page_size;

	offset = vtl_addr & mmu_page_mask;
	phy_addr = page->phy_addr | offset;

	//check for a protection fault.
	if(page->page_type == mmu_page_text && access_type != mmu_access_fetch)
	{
		fatal("mmu_translate(): protection fault load or store to text segment type %d vtrl_addr 0x%08x phy_addr 0x%08x page_id %d cycle %llu\n",
				access_type, vtl_addr, phy_addr, mmu_get_page_id(0, vtl_addr, mmu_access_fetch), P_TIME);
		return 0;
	}
	else if(page->page_type == mmu_page_data && access_type != mmu_access_load_store)
	{
		fatal("mmu_translate(): protection fault fetch to text segment type %d vtrl_addr 0x%08x phy_addr 0x%08x page_id %d cycle %llu\n",
				access_type, vtl_addr, phy_addr, mmu_get_page_id(0, vtl_addr, mmu_access_load_store), P_TIME);
		return 0;
	}

	return phy_addr;
}


int mmu_valid_phy_addr(unsigned int phy_addr)
{
	int index;

	index = phy_addr >> mmu_log_page_size;
	return index < mmu->page_list->count;
}


void mmu_access_page(unsigned int phy_addr, enum mmu_access_t access)
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
