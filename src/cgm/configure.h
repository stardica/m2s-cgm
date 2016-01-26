/*
 * configure.h
 *
 *  Created on: Nov 26, 2014
 *      Author: stardica
 */


#ifndef CONFIGURE_H_
#define CONFIGURE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*#include <lib/util/debug.h>*/
#include <lib/util/config.h>
#include <lib/util/list.h>
#include <lib/util/linked-list.h>
#include <lib/util/string.h>

#include <arch/common/arch.h>
#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/core.h>
#include <arch/x86/timing/thread.h>
#include <arch/si/timing/gpu.h>
#include <arch/si/timing/compute-unit.h>

/*#include <driver/opencl/opencl.c>*/

#include <cgm/cgm.h>
#include <cgm/protocol.h>
#include <cgm/directory.h>
#include <cgm/switch.h>
#include <cgm/hub-iommu.h>
#include <cgm/sys-agent.h>
#include <cgm/mem-ctrl.h>
#include <cgm/dram.h>

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define LOG2(x) ((int) rint((log((double) (x))) / (log(2.0))))

//global variables
extern int cgmmem_check_config;

int cgm_mem_configure(struct mem_t *mem);
int cgm_cpu_configure(void);
int cgm_gpu_configure(void);

int debug_read_config(void* user, const char* section, const char* name, const char* value);
int debug_finish_create(void);
int stats_read_config(void* user, const char* section, const char* name, const char* value);
int stats_finish_create(void);
int cache_read_config(void* user, const char* section, const char* name, const char* value);
int cache_finish_create(void);
int directory_read_config(void* user, const char* section, const char* name, const char* value);
int directory_finish_create(void);
int switch_read_config(void* user, const char* section, const char* name, const char* value);
int switch_finish_create(void);
int sys_agent_config(void* user, const char* section, const char* name, const char* value);
int sys_agent_finish_create(void);
int mem_ctrl_config(void* user, const char* section, const char* name, const char* value);
int mem_ctrl_finish_create(struct mem_t *mem);

//functions run by virtual function
void cpu_configure(Timing *self, struct config_t *config);
void gpu_configure(Timing *self, struct config_t *config);

int check_config(void* user, const char* section, const char* name, const char* value);
void print_config(void);


#endif /* CONFIGURE_H_ */
