/*
 * IOMMU.h
 *
 *  Created on: May 2, 2015
 *      Author: stardica
 */

#ifndef __IOMMU_H__
#define __IOMMU_H__

#include <stdio.h>
#include <stdlib.h>

#include <arch/si/timing/gpu.h>
#include <arch/si/emu/emu.h>

#include <lib/util/list.h>
#include <lib/util/string.h>

#include <mem-image/mmu.h>

#include <cgm/tasking.h>
#include <cgm/switch.h>

enum Rx_queue_name
{
	Rx_queue_top_0 = 0,
	Rx_queue_top_1,
	Rx_queue_top_2,
	Rx_queue_top_3,
	Rx_queue_top_4,
	Rx_queue_top_5,
	Rx_queue_top_6,
	Rx_queue_top_7,
	Rx_queue_bottom,
	Rx_queue_num
};

enum Tx_queue_name
{
	Tx_queue_top_0 = 0,
	Tx_queue_top_1,
	Tx_queue_top_2,
	Tx_queue_top_3,
	Tx_queue_top_4,
	Tx_queue_top_5,
	Tx_queue_top_6,
	Tx_queue_top_7,
	Tx_queue_bottom,
	Tx_queue_num
};

enum hub_connect_type_t
{
	hub_to_mc = 0,
	hub_to_l3,
	Thub_connect_type_num
};

extern struct str_map_t Rx_queue_strn_map;
extern struct str_map_t Tx_queue_strn_map;

struct hub_iommu_t{

	char *name;
	int id;
	unsigned int wire_latency;
	unsigned int gpu_l2_num;
	int latency;
	int bus_width;

	struct list_t *switch_queue;
	int switch_id;

	struct list_t **Rx_queue_top;
	struct list_t *Rx_queue_bottom;
	struct list_t *next_queue;
	struct list_t *last_queue;

	struct list_t **Tx_queue_top;
	struct list_t *Tx_queue_bottom;

	//io ctrl
	eventcount volatile **hub_iommu_io_up_ec;
	task **hub_iommu_io_up_tasks;
	eventcount volatile *hub_iommu_io_down_ec;
	task *hub_iommu_io_down_tasks;

	/*translation table*/

	/*reverse lookup Hash table*/
	int page_hash_table[MMU_PAGE_HASH_SIZE];

	unsigned int **translation_table;
	int translation_table_size;

	/*protocol related structures*/

	//mshr control links
	int mshr_size;
	struct mshr_t *mshrs;

	//outstanding request table
	int **ort;
	struct list_t *ort_list;
	int max_coal;


};

extern struct hub_iommu_t *hub_iommu;
extern eventcount volatile *hub_iommu_ec;
extern task *hub_iommu_tasks;
extern int hub_iommu_pid;
extern int hub_iommu_io_up_pid;
extern int hub_iommu_io_down_pid;
extern int hub_iommu_connection_type;

void hub_iommu_init(void);
void hub_iommu_create(void);
void hub_iommu_create_tasks(void (*func)(void));

int hub_iommu_can_access(struct list_t *queue);

//virtual functions that support multiple simulator configurations
void (*hub_iommu_ctrl)(void);
void hub_iommu_ctrl_func(void);
//void hub_iommu_noncoherent_ctrl(void);

struct cgm_packet_t *hub_iommu_get_from_queue(void);

int (*hub_iommu_put_next_queue)(struct cgm_packet_t *message_packet);
int hub_iommu_put_next_queue_func(struct cgm_packet_t *message_packet);
//int hub_iommu_put_next_queue(struct cgm_packet_t *message_packet);

void hub_probe_address(struct cache_t *cache, struct cgm_packet_t *message_packet);

void hub_iommu_io_up_ctrl(void);
void hub_iommu_io_down_ctrl(void);

//iommu functions
void iommu_nc_translate(struct cgm_packet_t *message_packet);
void iommu_translate(struct cgm_packet_t *message_packet);

void iommu_put_translation_table(unsigned int vtl_index, unsigned int phy_index);
int iommu_get_translation_table(unsigned int phy_index);
void iommu_clear_translation_table(int row);

int iommu_translation_table_insert_address(unsigned int address);
unsigned int iommu_translation_table_get_address(int id);
int iommu_get_translation_table_size(void);

#endif /* __IOMMU_H__ */
