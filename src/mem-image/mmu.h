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


enum mmu_access_t
{
	mmu_access_invalid = 0,
	mmu_access_fetch,
	mmu_access_load_store,

	mmu_access_read,
	mmu_access_write,
	mmu_access_execute
};


enum mmu_page_type_t
{
	mmu_page_invalid = 0,
	mmu_page_text,
	mmu_page_data
};

enum mmu_address_type_t
{
	mmu_addr_invalid = 0,
	mmu_addr_vtl,
	mmu_addr_phy
};

extern char *mmu_report_file_name;

extern unsigned int mmu_page_size;
extern unsigned int mmu_page_mask;
extern unsigned int mmu_log_page_size;

void mmu_init(void);
void mmu_done(void);
void mmu_dump_report(void);

int mmu_address_space_new(void);

//star made changes to this function
//unsigned int mmu_translate(int address_space_index, unsigned int vtl_addr);
unsigned int mmu_translate(int address_space_index, unsigned int vtl_addr, enum mmu_access_t access_type);
unsigned int mmu_get_phyaddr(int address_space_index, unsigned int vtl_addr, enum mmu_access_t access_type);
unsigned int mmu_get_vtladdr(int address_space_index, unsigned int phy_addr);
int mmu_get_page_id(int address_space_index, unsigned int vtl_addr, enum mmu_access_t access_type);
struct mmu_page_t *mmu_page_access(int address_space_index, unsigned int vtladdr, enum mmu_access_t access_type);
struct mmu_page_t *mmu_create_page(int address_space_index, unsigned int tag, enum mmu_access_t access_type, int index, unsigned int vtl_addr);
enum mmu_page_type_t mmu_get_page_type(enum mmu_access_t access_type);
int mmu_search_page(struct mmu_page_t *page, enum mmu_address_type_t addr_type, unsigned int addr, int address_space_index, int tag);
struct mmu_page_t *mmu_get_page(int address_space_index, unsigned int vtladdr, enum mmu_access_t access_type);

int mmu_valid_phy_addr(unsigned int phy_addr);

void mmu_access_stats(unsigned int phy_addr, enum mmu_access_t access);

#endif
