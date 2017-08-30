
#ifndef DRAM_H_
#define DRAM_H_

#include <cgm/cgm.h>
#include <cgm/cgm-struct.h>

/*DRAMSim functions*/

#define dram_system_handler_t MultiChannelMemorySystem

#define GET_BLOCK(addr) (addr & ~mem_ctrl->block_mask)
#define KHZ 1000
#define MHZ 1000000
#define GHZ 1000000000

extern char *dramsim_ddr_config_path;
extern char *dramsim_system_config_path;
extern char *dramsim_trace_config_path;
extern char *dramsim_vis_config_path;
extern unsigned int mem_size;
extern unsigned int cpu_freq;

extern int DRAMSim;
extern void *DRAM_object_ptr;

extern eventcount volatile *dramsim;
extern task *dramsim_cpu_clock;

void dramsim_init(void);
void dramsim_create_tasks(void);

void dramsim_create_mem_object(void);
void dramsim_ctrl(void);
void dramsim_print(void);


void dramsim_set_cpu_freq(void);
void dramsim_update_cpu_clock(void);
int dramsim_add_transaction(enum cgm_access_kind_t access_type, unsigned int addr);
void dramsim_register_call_backs(void);
void dramsim_read_complete(unsigned id, long long address, long long clock_cycle);
void dramsim_write_complete(unsigned id, long long address, long long clock_cycle);
void dramsim_power_callback(double a, double b, double c, double d);

/*DRAMSim Bindings*/
void call_print_me(void);
int call_add_transaction(void * mem, int isWrite, unsigned int addr);
void *call_get_memory_system_instance(const char *dev, const char *sys,
		const char *pwd, const char *trc, unsigned int megsOfMemory, const char *visfilename);
void call_set_CPU_clock_speed(void *mem, unsigned int cpuClkFreqHz);
void call_register_call_backs(void *mem, void *read_done, void *write_done, void *report_power);
void call_update(void *mem);

#endif /*DRAM_H_*/
