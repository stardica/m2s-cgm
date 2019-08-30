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

#ifndef MEM_SYSTEM_MMU_H
#define MEM_SYSTEM_MMU_H

#include <cgm/tasking.h>
#include <arch/x86/timing/thread.h>
#include <arch/x86/emu/context.h>
#include <arch/x86/timing/uop.h>



/* Local constants */
#define MMU_PAGE_HASH_SIZE  (1 << 10) /*1024*/
#define MMU_PAGE_LIST_SIZE  (1 << 10) /*1024*/

enum mmu_access_t
{
	mmu_access_invalid = 0,
	mmu_access_fetch,
	mmu_access_load_store,
	mmu_access_gpu,	//add more devices as needed
	mmu_access_gpu_coherent,
	mmu_access_read,
	mmu_access_write,
	mmu_access_execute
};

enum mmu_page_type_t
{
	mmu_page_invalid = 0,
	mmu_page_text,
	mmu_page_data,
	mmu_page_gpu
};

enum mmu_address_type_t
{
	mmu_addr_invalid = 0,
	mmu_addr_vtl,
	mmu_addr_phy,
	mmu_addr_guest
};


struct page_guest_t{

	int guest_pid;	/*guest process id*/
	unsigned int guest_vtl_addr_base;
	unsigned int guest_vtl_addr_top;
	unsigned int host_vtl_addr_base;
	unsigned int host_vtl_addr_top;
	unsigned int size;
};



/* Physical memory page */
struct mmu_page_t
{
	int id;

	struct mmu_page_t *next;

	int address_space_index;  /* Memory map ID */
	unsigned int vtl_addr;  /* Virtual address of host page */
	unsigned int phy_addr;  /* Physical address */

	enum mmu_page_type_t page_type; /*type of page either .text or .data segments*/

	/*star added this
	int link; bit indicating if this page is linked with a quest(s)
	struct list_t *guest_list; list of guests with permission to access this page*/

	/*Statistics*/
	long long num_read_accesses;
	long long num_write_accesses;
	long long num_execute_accesses;
};

/* Memory management unit */
struct mmu_t
{
	int id;

	/*Forward translate variables*/
	int fetch_ready;
	int fetch_address_space_index;
	unsigned int vtl_fetch_address;
	unsigned int phy_fetch_address;

	int issue_width;
	int *data_ready;
	int *data_valid;
	int *data_address_space_index;
	unsigned int *vtl_data_address;
	unsigned int *phy_data_address;

	int num_fault_bits;
	int *fault_bits;

	long long num_processed;
	long long num_coalesced;

	/*CPU request in box*/
	struct list_t *page_list;

	/* star List of quests */
	struct list_t *guest_list;

	/*CPU request in box*/
	//struct list_t *request_queue;

	/* Hash table of pages */
	struct mmu_page_t *page_hash_table[MMU_PAGE_HASH_SIZE];

	/* Report file */
	FILE *report_file;
};

extern int page_number;
extern struct mmu_t *mmu;

extern char *mmu_report_file_name;

extern unsigned int mmu_page_size;
extern unsigned int mmu_page_mask;
extern unsigned int mmu_log_page_size;

extern eventcount volatile *mmu_ec;
//extern eventcount volatile *mmu_data_ec;
extern task *mmu_task;
//extern task *mmu_data_task;

extern long long mmu_access_id;

//extern long long step;

void mmu_init(void);
void mmu_done(void);
void mmu_dump_report(void);

int mmu_address_space_new(void);

//star made changes to this function
//unsigned int mmu_translate(int address_space_index, unsigned int vtl_addr);
unsigned int mmu_translate(int address_space_index, unsigned int vtl_addr, enum mmu_access_t access_type, int *page_fault_ptr);
unsigned int mmu_get_phyaddr(int address_space_index, unsigned int vtl_addr, enum mmu_access_t access_type);
unsigned int mmu_get_vtladdr(int address_space_index, unsigned int phy_addr);
int mmu_get_page_id(int address_space_index, enum mmu_address_type_t addr_type, unsigned int vtl_addr, enum mmu_access_t access_type);
struct mmu_page_t *mmu_page_access(int address_space_index, enum mmu_address_type_t addr_type, unsigned int vtladdr, enum mmu_access_t access_type);
struct mmu_page_t *mmu_create_page(int address_space_index, unsigned int tag, enum mmu_access_t access_type, int index, unsigned int vtl_addr);
enum mmu_page_type_t mmu_get_page_type(enum mmu_access_t access_type);
int mmu_search_page(struct mmu_page_t *page, enum mmu_address_type_t addr_type, unsigned int addr, int address_space_index, int tag);
struct mmu_page_t *mmu_get_page(int address_space_index, unsigned int vtladdr, enum mmu_access_t access_type, int *page_fault_ptr);
int mmu_valid_phy_addr(unsigned int phy_addr);
void mmu_access_stats(unsigned int phy_addr, enum mmu_access_t access);

void mmu_access_page(struct mmu_t *mmu, unsigned int phy_addr, enum mmu_access_t access);

void mmu_add_guest(int address_space_index, int guest_pid, unsigned int guest_pointer, unsigned int host_ptr, unsigned int size);
struct page_guest_t *mmu_create_guest(void);
unsigned int mmu_reverse_translate(int address_space_index, unsigned int phy_addr, enum mmu_access_t access_type);


void mmu_ctrl(void);
int mmu_has_fault(struct mmu_t *mmu);
int mmu_fetch_translate(X86Thread *self, unsigned int block);
int mmu_data_translate(X86Thread *self, struct x86_uop_t *uop);

void mmu_set_fetch_fault_bit(struct mmu_t *mmu, int val);
void mmu_set_data_fault_bit(struct mmu_t *mmu, int row, int val);


int can_access_mmu(void);


unsigned int mmu_forward_translate_guest(int address_space_index, int guest_pid, unsigned int guest_vtl_addr);
unsigned int mmu_reverse_translate_guest(int address_space_index, int guest_pid, unsigned int host_phy_addr);
unsigned int mmu_forward_link_guest_address(int guest_pid, unsigned int vtl_addr);
unsigned int mmu_reverse_link_host_address(int guest_pid, unsigned int phy_addr);


#endif
