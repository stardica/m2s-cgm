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

#include <lib/util/list.h>
#include <lib/util/string.h>
/*#include <lib/util/debug.h>*/

/*#include <cgm/cgm.h>*/
/*#include <cgm/protocol.h>*/
#include <cgm/tasking.h>
#include <cgm/switch.h>

/*
#include <lib/util/list.h>

#include <cgm/tasking.h>
*/


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

extern struct str_map_t Rx_queue_strn_map;
extern struct str_map_t Tx_queue_strn_map;

struct hub_iommu_t{

	char *name;
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

	unsigned int **translation_table;
	int translation_table_size;

	//virtual functions that support multiple simulator configurations
	void (*hub_iommu_translate)(struct cgm_packet_t *message_packet);
};

extern struct hub_iommu_t *hub_iommu;
extern eventcount volatile *hub_iommu_ec;
extern task *hub_iommu_tasks;

extern int hub_iommu_pid;
extern int hub_iommu_io_up_pid;
extern int hub_iommu_io_down_pid;

void hub_iommu_init(void);
void hub_iommu_create(void);
void hub_iommu_create_tasks(void);
void hub_iommu_ctrl(void);

void hub_iommu_io_up_ctrl(void);
void hub_iommu_io_down_ctrl(void);

struct cgm_packet_t *hub_iommu_get_from_queue(void);
void hub_iommu_put_next_queue(struct cgm_packet_t *message_packet);
int hub_iommu_can_access(struct list_t *queue);


//iommu functions

void iommu_translate(struct cgm_packet_t *message_packet);
unsigned int iommu_get_phy_address(unsigned int address);
unsigned int iommu_get_vtl_address(unsigned int address, int id);

int iommu_translation_table_insert_address(unsigned int address);
unsigned int iommu_translation_table_get_address(int id);
int iommu_get_translation_table_size(void);

#endif /* __IOMMU_H__ */
