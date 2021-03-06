
#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cgm/tasking.h>

#include <arch/x86/timing/cpu.h>
#include <lib/util/list.h>

enum interrupt_type_t
{
	non_interrupt = 0,
	system_interrupt,
	opencl_interrupt,
	interrupt_count
};

#include <arch/x86/timing/uop.h>

//save some of the data from the eulator for future use in issue
struct interrupt_t{

	//int uop_id;
	//enum interrupt_type_t interrupt;
	//int syscall_code;
	//int abi_code;
	//unsigned int int_src_ptr;
	//unsigned int int_dest_ptr;
	//unsigned int int_size;
	int core_id;
	struct x86_uop_t *uop;
	X86Thread *thread;
};

//flags
//global flags
extern int opencl_syscall_flag;
extern int syscall_flag;
extern unsigned int int_src_ptr;
extern unsigned int int_dest_ptr;
extern unsigned int int_size;

extern int *interrupt_cores;

//lists
extern struct list_t *interrupt_list;

//tasks
extern eventcount volatile *interrupt;


//interupts
void cgm_interrupt(X86Thread *self, struct x86_uop_t *uop);
void interrupt_init(void);
void interrupt_create(void);
void interrupt_create_tasks(void);
struct interrupt_t *interrupt_service_routine_create(void);
void interrupt_service_routine_destroy(struct interrupt_t *isr);

void interrupt_service_request(void);



#endif /* __INTERRUPT_H__ */
