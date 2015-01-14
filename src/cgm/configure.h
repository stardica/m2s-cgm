/*
 * configure.h
 *
 *  Created on: Nov 26, 2014
 *      Author: stardica
 */


#ifndef CONFIGURE_H_
#define CONFIGURE_H_

#include <stdio.h>
#include <string.h>

//star todo remember to change these as needed.
#define CGMMEMCONFIGPATH "/home/stardica/Desktop/cgm-mem/src/Config.ini"
#define HOSTSIMCONFIGPATH "/home/stardica/Desktop/m2s-cgm/bin/SimpleSingleCore/Intel-i7-4790k-CPU-Config-Single-Core.ini"
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

//global variables
extern int cgmmem_check_config;

/*extern struct queue_config_t{

	int size;

};

extern struct cache_config_t{

	int size;

};

extern struct queue_config_t *q_config;
extern struct cache_config_t *c_config;*/

int cgmmem_configure(void);
int cpu_config(void* user, const char* section, const char* name, const char* value);
int check_config(void* user, const char* section, const char* name, const char* value);
void print_config(void);
int queue_config(void* user, const char* section, const char* name, const char* value);
int cache_config(void* user, const char* section, const char* name, const char* value);
int sysagent_config(void* user, const char* section, const char* name, const char* value);
int memctrl_config(void* user, const char* section, const char* name, const char* value);


#endif /* CONFIGURE_H_ */
