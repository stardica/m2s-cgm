/*
 * IOMMU.h
 *
 *  Created on: May 2, 2015
 *      Author: stardica
 */

#ifndef __IOMMU_H__
#define __IOMMU_H__

#include <lib/util/list.h>

#include <cgm/tasking.h>


enum queue_name
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
	queue_num
};

extern struct str_map_t queue_strn_map;

struct hub_iommu_t{

	char *name;
	unsigned int wire_latency;
	unsigned int gpu_l2_num;

	struct list_t **Rx_queue_top;
	struct list_t *Rx_queue_bottom;
	struct list_t *next_queue;
	struct list_t *last_queue;

	//star todo add something here for the GPU virtual to physical address translation.
};

extern struct hub_iommu_t *hub_iommu;
extern eventcount volatile *hub_iommu_ec;
extern task *hub_iommu_tasks;
extern int hub_iommu_pid;

void hub_iommu_init(void);
void hub_iommu_create(void);
void hub_iommu_create_tasks(void);
void hub_iommu_ctrl(void);

struct cgm_packet_t *hub_iommu_get_from_queue(void);

#endif /* __IOMMU_H__ */
