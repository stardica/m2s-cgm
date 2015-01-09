/*
 * cgm-mem.h
 *
 *  Created on: Nov 24, 2014
 *      Author: stardica
 */


#ifndef CGMMEM_H_
#define CGMMEM_H_

#include <cgm-mem/tasking.h>


//test memory is 8bit i.e. 0x00 - 0xFF
extern char test_mem[256];
extern int host_sim_cpu_num;
extern int host_sim_cpu_core_num;
extern int host_sim_cpu_thread_num;
extern int host_sim_cpu_fetch_queue_size;
extern int host_sim_cpu_lsq_queue_size;

extern long long int mem_cycle;

#define TESTSCRIPTPATH "/home/stardica/Desktop/cgm-mem/src/TestScript.ini"

// globals for tasking
extern eventcount *start;
extern eventcount *stop;



//function prototypes
void cgm_mem_init(void);
void cgm_mem_structure_init(void);
void cgm_mem_task_init(void);
void cgm_mem_threads_init(void);
void cgm_mem_sim_loop(void);
void testmem_init(void);
void cleanup(void);


//for testing purposes
int system_test(void);
void cgm_mem_dump(void);
void test_run(void* user, const char* section, const char* name, const char* value);
void load_fetch(char * string);
void load_issue(char * string);
void store_issue(char * string);




#endif /* CGMMEM_H_ */
