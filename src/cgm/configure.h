/*
 * configure.h
 *
 *  Created on: Nov 26, 2014
 *      Author: stardica
 */


#ifndef CONFIGURE_H_
#define CONFIGURE_H_

#include <lib/util/config.h>
#include <arch/common/arch.h>
#include <arch/x86/timing/core.h>
#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/thread.h>

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define LOG2(x) ((int) rint((log((double) (x))) / (log(2.0))))

//global variables

/*extern struct queue_config_t{

	int size;

};

extern struct cache_config_t{

	int size;

};

extern struct queue_config_t *q_config;
extern struct cache_config_t *c_config;*/

extern int cgmmem_check_config;


int cgm_mem_configure(void);
int cgm_cpu_configure(void);
int cgm_gpu_configure(void);
int cpu_configure(Timing *self, struct config_t *config);
int gpu_configure(Timing *self, struct config_t *config);


int check_config(void* user, const char* section, const char* name, const char* value);

//
void print_config(void);
int queue_config(void* user, const char* section, const char* name, const char* value);
int cache_config(void* user, const char* section, const char* name, const char* value);
int sysagent_config(void* user, const char* section, const char* name, const char* value);
int memctrl_config(void* user, const char* section, const char* name, const char* value);


#endif /* CONFIGURE_H_ */
