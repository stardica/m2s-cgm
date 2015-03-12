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

#include <signal.h>
#include <sys/time.h>
#include <m2s.h>
#include <arch/common/arch.h>
#include <arch/common/asm.h>
#include <arch/common/emu.h>
#include <arch/common/timing.h>
#include <arch/common/runtime.h>
#include <arch/x86/emu/checkpoint.h>
#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <arch/x86/emu/isa.h>
#include <arch/x86/emu/loader.h>
#include <arch/x86/emu/syscall.h>
#include <arch/x86/timing/cpu.h>
#include <arch/x86/timing/trace-cache.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/elf-format.h>
#include <lib/util/class.h>

#if CGM
#include <cgm/cgm.h>
#include <cgm/tasking.h>
#else
#include <mem-system/config.h>
#include <mem-system/mem-system.h>
#include <network/net-system.h>
#endif

#include <mem-image/memory.h>
#include <mem-image/mmu.h>
#include <instrumentation/stats.h>

#if GPU
#include <arch/si/asm/asm.h>
#include <arch/si/emu/emu.h>
#include <arch/si/emu/isa.h>
#include <arch/si/timing/gpu.h>
#include <driver/opencl/opencl.h>
//#include <driver/cuda/cuda.h>
//#include <driver/glu/glu.h>
//#include <driver/glut/glut.h>
//#include <driver/glew/glew.h>
//#include <driver/opencl-old/evergreen/opencl.h>
//#include <driver/opengl/opengl.h>
#endif

//star >> added version here because eclipse was complaining.
//this are pointers to strings its like saying visual_file_name[].
//The version number is defined by either make or configure.
static char *m2sversion = "0";
//static char *visual_file_name = "";
static char *ctx_config_file_name = "";
static char *elf_debug_file_name = "";
static char *trace_file_name = "";
static char *glu_debug_file_name = "";
static char *glut_debug_file_name = "";
static char *glew_debug_file_name = "";
static char *opengl_debug_file_name = "";
static char *opencl_debug_file_name = "";
static char *cuda_debug_file_name = "";

static char *x86_call_debug_file_name = "";
static char *x86_ctx_debug_file_name = "";
static char *x86_disasm_file_name = "";
static char *x86_isa_debug_file_name = "";
static char *x86_load_checkpoint_file_name = "";
static char *x86_loader_debug_file_name = "";
static char *x86_save_checkpoint_file_name = "";
static char *x86_sys_debug_file_name = "";
static char *x86_trace_cache_debug_file_name = "";
static enum arch_sim_kind_t x86_sim_kind = arch_sim_kind_functional;


static char *si_disasm_file_name = "";
static char *si_isa_debug_file_name = "";
static char *si_opengl_disasm_file_name = "";
static int si_opengl_disasm_shader_index = 1;
static enum arch_sim_kind_t si_sim_kind = arch_sim_kind_functional;

static char *mem_debug_file_name = "";
static char *net_debug_file_name = "";
static long long m2s_max_time;  /* Max. simulation time in seconds (0 = no limit) */
static long long m2s_loop_iter;  /* Number of iterations in main simulation loop */
static char m2s_sim_id[10];  /* Pseudo-unique simulation ID (5 alpha-numeric digits) */

static volatile int m2s_signal_received;  /* Signal received by handler (0 = none */

static X86Cpu *x86_cpu;

static char *m2s_help =
		"Syntax:\n"
		"\n"
		"        m2s [<options>] [<x86_binary> [<arg_list>]]\n"
		"\n"
		"The user command-line supports a sequence of command-line options, and can\n"
		"include an x86 ELF binary (executable x86 program) followed by its arguments.\n"
		"The execution of this program will be simulated by Multi2Sim, together with the\n"
		"rest of the x86 programs specified in the context configuration file (option\n"
		"'--ctx-config <file>'). The rest of the possible command-line options are\n"
		"classified in categories and listed next:\n"
		"\n"
		"\n"
		"================================================================================\n"
		"General Options\n"
		"================================================================================\n"
		"\n"
		"  --ctx-config <file>\n"
		"      Use <file> as the context configuration file. This file describes the\n"
		"      initial set of running applications, their arguments, and environment\n"
		"      variables. Type 'm2s --ctx-config-help' for a description of the file\n"
		"      format.\n"
		"\n"
		"  --ctx-config-help\n"
		"      Show a help message describing the format of the context configuration\n"
		"      file, passed to the simulator through option '--ctx-config <file>'.\n"
		"\n"
		"  --elf-debug <file>\n"
		"      Dump debug information related with the analysis of ELF files. Every time\n"
		"      an executable file is open (CPU program of GPU kernel binary), detailed\n"
		"      information about its symbols, sections, strings, etc. is dumped here.\n"
		"\n"
		"  --max-time <time>\n"
		"      Maximum simulation time in seconds. The simulator will stop once this time\n"
		"      is exceeded. A value of 0 (default) means no time limit.\n"
		"\n"
		"  --trace <file>.gz\n"
		"      Generate a trace file with debug information on the configuration of the\n"
		"      modeled CPUs, GPUs, and memory system, as well as their dynamic\n"
		"      simulation. The trace is a compressed plain-text file in format. The user\n"
		"      should watch the size of the generated trace as simulation runs, since\n"
		"      the trace file can quickly become extremely large.\n"
		"\n"
		"  --visual <file>.gz\n"
		"      Run the Multi2Sim Visualization Tool. This option consumes a file\n"
		"      generated with the '--trace' option in a previous simulation. This option\n"
		"      is only available on systems with support for GTK 3.0 or higher.\n"
		"\n"
		"\n"
		"================================================================================\n"
		"x86 CPU Options\n"
		"================================================================================\n"
		"\n"
		"  --x86-config <file>\n"
		"      Configuration file for the x86 CPU timing model, including parameters\n"
		"      describing stage bandwidth, structures size, and other parameters of\n"
		"      processor cores and threads. Type 'm2s --x86-help' for details on the file\n"
		"      format.\n"
		"\n"
		"  --x86-debug-call <file>\n"
		"      Dump debug information about function calls and returns. The control flow\n"
		"      of an x86 program can be observed leveraging ELF symbols present in the\n"
		"      program binary.\n"
		"\n"
		"  --x86-debug-clrt <file>\n"
		"      Debug information for the newer implementation of the OpenCL runtime\n"
		"      library (not available yet).\n"
		"\n"
		"  --x86-debug-ctx <file>\n"
		"      Dump debug information related with context creation, destruction,\n"
		"      allocation, or state change.\n"
		"\n"
		"  --x86-debug-glut <file>\n"
		"      Debug information for GLUT runtime calls performed by an OpenGL program\n"
		"      based on the GLUT library.\n"
		"\n"
		"  --x86-debug-loader <file>\n"
		"      Dump debug information extending the analysis of the ELF program binary.\n"
		"      This information shows which ELF sections and symbols are loaded to the\n"
		"      initial program memory image.\n"
		"\n"
		"  --x86-debug-isa\n"
		"      Debug information for dynamic execution of x86 instructions. Updates on\n"
		"      the processor state can be analyzed using this information.\n"
		"\n"
		"  --x86-debug-opengl <file>\n"
		"      Debug information for OpenGL runtime calls.\n"
		"\n"
		"  --x86-debug-syscall\n"
		"      Debug information for system calls performed by an x86 program, including\n"
		"      system call code, arguments, and return value.\n"
		"\n"
		"  --x86-debug-trace-cache\n"
		"      Debug information for trace cache.\n"
		"\n"
		"  --x86-disasm <file>\n"
		"      Disassemble the x86 ELF file provided in <file>, using the internal x86\n"
		"      disassembler. This option is incompatible with any other option.\n"
		"\n"
		"  --x86-help\n"
		"      Display a help message describing the format of the x86 CPU context\n"
		"      configuration file.\n"
		"\n"
		"  --x86-last-inst <bytes>\n"
		"      Stop simulation when the specified instruction is fetched. Can be used to\n"
		"      trigger a checkpoint with option '--x86-save-checkpoint'. The instruction\n"
		"      must be given as a sequence of hexadecimal digits, including trailing\n"
		"      zeros if needed.\n"
		"\n"
		"  --x86-load-checkpoint <file>\n"
		"      Load a checkpoint of the x86 architectural state, created in a previous\n"
		"      execution of the simulator with option '--x86-save-checkpoint'.\n"
		"\n"
		"  --x86-max-cycles <cycles>\n"
		"      Maximum number of cycles for x86 timing simulation. Use 0 (default) for no\n"
		"      limit. This option is only valid for detailed x86 simulation (option\n"
		"      '--x86-sim detailed').\n"
		"\n"
		"  --x86-max-inst <inst>\n"
		"      Maximum number of x86 instructions. On x86 functional simulation, this\n"
		"      limit is given in number of emulated instructions. On x86 detailed\n"
		"      simulation, it is given as the number of committed (non-speculative)\n"
		"      instructions. Use 0 (default) for unlimited.\n"
		"\n"
		"  --x86-report <file>\n"
		"      File to dump a report of the x86 CPU pipeline, including statistics such\n"
		"      as the number of instructions handled in every pipeline stage, read/write\n"
		"      accesses performed on pipeline queues, etc. This option is only valid for\n"
		"      detailed x86 simulation (option '--x86-sim detailed').\n"
		"\n"
		"  --x86-save-checkpoint <file>\n"
		"      Save a checkpoint of x86 architectural state at the end of simulation.\n"
		"      Useful options to use together with this are '--x86-max-inst' and\n"
		"      '--x86-last-inst' to force the simulation to stop and create a checkpoint.\n"
		"\n"
		"  --x86-sim {functional|detailed}\n"
		"      Choose a functional simulation (emulation) of an x86 program, versus\n"
		"      a detailed (architectural) simulation. Simulation is functional by\n" 	"      default.\n"
		"\n"
		"\n"
		"================================================================================\n"
		"AMD Evergreen GPU Options\n"
		"================================================================================\n"
		"\n"
		"  --evg-calc <prefix>\n"
		"      If this option is set, a kernel execution will cause three GPU occupancy\n"
		"      plots to be dumped in files '<prefix>.<ndrange_id>.<plot>.eps', where\n"
		"      <ndrange_id> is the identifier of the current ND-Range, and <plot> is\n"
		"      {work_items|registers|local_mem}. This options requires 'gnuplot' to be\n"
		"      installed in the system.\n"
		"\n"
		"  --evg-config <file>\n"
		"      Configuration file for the Evergreen GPU timing model, including\n"
		"      parameters such as number of compute units, stream cores, or wavefront\n"
		"      size. Type 'm2s --evg-help' for details on the file format.\n"
		"\n"
		"  --evg-debug-isa <file>\n"
		"      Dump debug information on the Evergreen ISA instructions emulated, and\n"
		"      their updates in the architectural GPU state.\n"
		"\n"
		"  --evg-debug-opencl <file>\n"
		"      Dump debug information on OpenCL system calls performed by the x86 host\n"
		"      program. The information includes OpenCL call code, arguments, and return\n"
		"      values.\n"
		"\n"
		"  --evg-disasm <file>\n"
		"      Disassemble OpenCL kernel binary provided in <file>. This option must be\n"
		"      used with no other options.\n"
		"\n"
		"  --evg-disasm-opengl <file> <index>\n"
		"      Disassemble OpenGL shader binary provided in <file>. The shader identifier\n"
		"      is specified in <index>. This option must be used with no other options.\n"
		"\n"
		"  --evg-help\n"
		"      Display a help message describing the format of the Evergreen GPU\n"
		"      configuration file, passed with option '--evg-config <file>'.\n"
		"\n"
		"  --evg-kernel-binary <file>\n"
		"      Specify OpenCL kernel binary to be loaded when the OpenCL host program\n"
		"      performs a call to 'clCreateProgramWithSource'. Since on-line compilation\n"
		"      of OpenCL kernels is not supported, this is a possible way to load them.\n"
		"\n"
		"  --evg-max-cycles <cycles>\n"
		"      Maximum number of Evergreen GPU cycles for detailed simulation. Use 0\n"
		"      (default) for no limit.\n"
		"\n"
		"  --evg-max-inst <inst>\n"
		"      Maximum number of Evergreen ISA instructions. An instruction executed in\n"
		"      common for a whole wavefront counts as 1 toward this limit. Use 0\n"
		"      (default) for no limit.\n"
		"\n"
		"  --evg-max-kernels <kernels>\n"
		"      Maximum number of Evergreen GPU kernels (0 for no maximum). After the last\n"
		"      kernel finishes execution, the simulator will stop.\n"
		"\n"
		"  --evg-report-kernel <file>\n"
		"      File to dump report of a GPU device kernel emulation. The report includes\n"
		"      statistics about type of instructions, VLIW packing, thread divergence,\n"
		"      etc.\n"
		"\n"
		"  --evg-report <file>\n"
		"      File to dump a report of the GPU pipeline, such as active execution\n"
		"      engines, compute units occupancy, stream cores utilization, etc. Use\n"
		"      together with a detailed GPU simulation (option '--evg-sim detailed').\n"
		"\n"
		"  --evg-sim {functional|detailed}\n"
		"      Functional simulation (emulation) of the AMD Evergreen GPU kernel, versus\n"
		"      detailed (architectural) simulation. Functional simulation is default.\n"
		"\n"
		"\n"
		"================================================================================\n"
		"AMD Southern Islands GPU Options\n"
		"================================================================================\n"
		"\n"
		"  --si-calc <prefix>\n"
		"      If this option is set, a kernel execution will cause three GPU occupancy\n"
		"      plots to be dumped in files '<prefix>.<ndrange_id>.<plot>.eps', where\n"
		"      <ndrange_id> is the identifier of the current ND-Range, and <plot> is\n"
		"      {work_items|registers|local_mem}. This options requires 'gnuplot' to be\n"
		"      installed in the system.\n"
		"\n"
		"  --si-config <file>\n"
		"      Configuration file for the Southern Islands GPU timing model, including\n"
		"      parameters such as number of compute units, stream cores, or wavefront\n"
		"      size. Type 'm2s --si-help' for details on the file format.\n"
		"\n"
		"  --si-debug-isa <file>\n"
		"      Debug information on the emulation of Southern Islands ISA instructions,\n"
		"      including architectural state updates on registers and memory locations.\n"
		"\n"
		"  --si-debug-opencl <file>\n"
		"      Dump debug information on OpenCL system calls performed by the x86 host\n"
		"      program. The information includes OpenCL call code, arguments, and return\n"
		"      values.\n"
		"\n"
		"  --si-disasm <file>\n"
		"      Disassemble a Southern Islands kernel binary. This option is incompatible\n"
		"      with othe command-line options.\n"
		"\n"
		"  --si-dump-default-config <file>\n"
		"      Dumps the default GPU configuration file used for timing simulation.\n"
		"      This cannot be used with any other option.\n"
		"\n"
		"  --si-help\n"
		"      Display a help message describing the format of the Southern Islands GPU\n"
		"      configuration file, passed with option '--si-config <file>'.\n"
		"\n"
		"  --si-max-cycles <cycles>\n"
		"      Maximum number of cycles for the GPU detailed simulation. Use 0 (default)\n"
		"      for no limit.\n"
		"\n"
		"  --si-max-inst <inst>\n"
		"      Maximum number of ISA instructions. An instruction executed by an entire\n"
		"      wavefront counts as 1 toward this limit. Use 0 (default) for no limit.\n"
		"\n"
		"  --si-max-kernels <kernels>\n"
		"      Maximum number of Southern Islands kernels (0 for no maximum). After the\n"
		"      last kernel finishes execution, the simulator will stop.\n"
		"\n"
		"  --si-report <file>\n"
		"      File to dump a report of the GPU pipeline, such as active execution\n"
		"      engines, compute units occupancy, stream cores utilization, etc. Use\n"
		"      together with a detailed GPU simulation (option '--si-sim detailed').\n"
		"\n"
		"  --si-shader-binary <file>\n"
		"      Use <file> as the returned shader binary upon an OpenGL call to\n"
		"      'clLoadProgramWithSource'.\n"
		"\n"
		"  --si-sim {functional|detailed}\n"
		"      Functional (default) or detailed simulation for the AMD Southern Islands\n"
		"      GPU model.\n"
		"\n"
		"\n"
		"================================================================================\n"
		"ARM CPU Options\n"
		"================================================================================\n"
		"\n"
		"  --arm-disasm <file>\n"
		"      Disassemble an ARM binary using Multi2Sim's internal disassembler. This\n"
		"      option is incompatible with any other command-line option.\n"
		"\n"
		"  --arm-debug-loader <file>\n"
		"      Dump debug information extending the analysis of the ELF program binary.\n"
		"      This information shows which ELF sections and symbols are loaded to the\n"
		"      initial program memory image.\n"
		"\n"
		"  --arm-debug-isa <file>\n"
		"      Debug information for dynamic execution of Arm instructions. Updates on\n"
		"      the processor state can be analyzed using this information.\n"
		"\n"
		"\n"
		"================================================================================\n"
		"MIPS Options\n"
		"================================================================================\n"
		"\n"
		"  --mips-disasm <file>\n"
		"      Disassemble an MIPS binary using Multi2Sim's internal disassembler. This\n"
		"      option is incompatible with any other command-line option.\n"
		"\n"
		"  --mips-debug-loader <file>\n"
		"      Dump debug information extending the analysis of the ELF program binary.\n"
		"      This information shows which ELF sections and symbols are loaded to the\n"
		"      initial program memory image.\n"
		"\n"
		"  --mips-debug-isa <file>\n"
		"      Debug information for dynamic execution of Mips instructions. Updates on\n"
		"      the processor state can be analyzed using this information.\n"
		"\n"
		"\n"
		"================================================================================\n"
		"NVIDIA Fermi GPU Options\n"
		"================================================================================\n"
		"\n"
		"  --frm-debug-isa <file>\n"
		"      Debug information on the emulation of Fermi ISA instructions,\n"
		"      including architectural state updates on registers and memory locations.\n"
		"\n"
		"  --frm-debug-cuda <file>\n"
		"      Debug information on the emulation of Fermi CUDA driver APIs.\n"
		"\n"
		"  --frm-disasm <file>\n"
		"      Disassemble a Fermi kernel binary (cubin format). This option is\n"
		"      incompatible with any other command-line option.\n"
		"\n"
		"  --frm-report <file>\n"
		"      File to dump a report of the GPU pipeline, such as active execution\n"
		"      engines, compute units occupancy, stream cores utilization, etc. Use\n"
		"      together with a detailed GPU simulation (option '--frm-sim detailed').\n"
		"\n"
		"  --frm-sim {functional|detailed}\n"
		"      Functional (default) or detailed simulation for the NVIDIA Fermi\n"
		"      GPU model.\n"
		"\n"
		"\n"
		"================================================================================\n"
		"NVIDIA Kepler GPU Options\n"
		"================================================================================\n"
		"\n"
		"  --kpl-disasm <file>\n"
		"      Disassemble a Kepler kernel binary (cubin format). This option is\n"
		"      incompatible with any other command-line option.\n"
		"\n"
		"\n"
		"================================================================================\n"
		"Memory System Options\n"
		"================================================================================\n"
		"\n"
		"  --mem-config <file>\n"
		"      Configuration file for memory hierarchy. Run 'm2s --mem-help' for a\n"
		"      description of the file format.\n"
		"\n"
		"  --mem-debug <file>\n"
		"      Dump debug information about memory accesses, cache memories, main memory,\n"
		"      and directories.\n"
		"\n"
		"  --mem-help\n"
		"      Print help message describing the format of the memory configuration file,\n"
		"      passed to the simulator with option '--mem-config <file>'.\n"
		"\n"
		"  --mem-report\n"
		"      File for a report on the memory hierarchy, including cache hits, misses,\n"
		"      evictions, etc. This option must be used together with detailed simulation\n"
		"      of any CPU/GPU architecture.\n"
		"\n"
		"\n"
		"================================================================================\n"
		"Network Options\n"
		"================================================================================\n"
		"\n"
		"  --net-config <file>\n"
		"      Network configuration file. Networks in the memory hierarchy can be\n"
		"      defined here and referenced in other configuration files. For a\n"
		"      description of the format, use option '--net-help'.\n"
		"\n"
		"  --net-debug\n"
		"      Debug information related with interconnection networks, including packet\n"
		"      transfers, link usage, etc.\n"
		"\n"
		"  --net-help\n"
		"      Print help message describing the network configuration file, passed to\n"
		"      the simulator with option '--net-config <file>'.\n"
		"\n"
		"  --net-injection-rate <rate>\n"
		"      For network simulation, packet injection rate for nodes (e.g. 0.01 means\n"
		"      one packet every 100 cycles on average. Nodes will inject packets into\n"
		"      the network using random delays with exponential distribution with lambda\n"
		"      = <rate>. This option must be used together with '--net-sim'.\n"
		"\n"
		"  --net-max-cycles <cycles>\n"
		"      Maximum number of cycles for network simulation. This option must be used\n"
		"      together with option '--net-sim'.\n"
		"\n"
		"  --net-msg-size <size>\n"
		"      For network simulation, packet size in bytes. An entire packet is assumed\n"
		"      to fit in a node's buffer, but its transfer latency through a link will\n"
		"      depend on the message size and the link bandwidth. This option must be\n"
		"      used together with '--net-sim'.\n"
		"\n"
		"  --net-report <file>\n"
		"      File to dump detailed statistics for each network defined in the network\n"
		"      configuration file (option '--net-config'). The report includes statistics\n"
		"      on bandwidth utilization, network traffic, etc.\n"
		"\n"
		"  --net-visual <file>\n"
		"      File for graphically representing the interconnection network. This file \n"
		"      is an input for a supplementary tool called 'graphplot' which is located \n"
		"      in samples/network folder in multi2sim trunk.\n"
		"\n"
		"  --net-sim <network>\n"
		"      Runs a network simulation using synthetic traffic, where <network> is the\n"
		"      name of a network specified in the network configuration file (option\n"
		"      '--net-config').\n"
		"\n";


// star >> static message
static char *m2s_err_note = "Please type 'm2s --help' for a list of valid Multi2Sim command-line options.\n";

// star >>  outputs an error message
// fatal is defined in debug.c/.h
static void m2s_need_argument(int argc, char **argv, int argi)
{
	if (argi == argc - 1)
		fatal("option %s requires one argument.\n%s", argv[argi], m2s_err_note);
}


static void m2s_read_command_line(int *argc_ptr, char **argv)
{
	// star >> declarations
	int argc = *argc_ptr;
	int argi;
	int arg_discard = 0;
	int err;
	char *net_sim_last_option = NULL;
	//char *dram_sim_last_option = NULL;

	// star >> argi is used as a temp counter
	// star >> 590 - 1519 assign various parameters based on command line options input.
	for (argi = 1; argi < argc; argi++)
	{
		/*
		 * General Options
		 */

		/* Context configuration file */
		if (!strcmp(argv[argi], "--ctx-config"))
		{
			m2s_need_argument(argc, argv, argi);
			ctx_config_file_name = argv[++argi];
			continue;
		}

		/* Help for context configuration file format */
		if (!strcmp(argv[argi], "--ctx-config-help"))
		{
			fprintf(stderr, "%s", x86_loader_help);
			continue;
		}

		/* ELF debug file */
		if (!strcmp(argv[argi], "--elf-debug"))
		{
			m2s_need_argument(argc, argv, argi);
			elf_debug_file_name = argv[++argi];
			continue;
		}

		/* Show help */
		if (!strcmp(argv[argi], "--help"))
		{
			fprintf(stderr, "%s", m2s_help);
			continue;
		}

		/* Simulation time limit */
		if (!strcmp(argv[argi], "--max-time"))
		{
			m2s_need_argument(argc, argv, argi);
			m2s_max_time = str_to_llint(argv[argi + 1], &err);
			if (err)
				fatal("option %s, value '%s': %s", argv[argi],
						argv[argi + 1], str_error(err));
			argi++;
			continue;
		}

		/* Simulation trace */
		if (!strcmp(argv[argi], "--trace"))
		{
			m2s_need_argument(argc, argv, argi);
			trace_file_name = argv[++argi];
			continue;
		}


		/*
		 * x86 CPU Options
		 */

		/* CPU configuration file */
		if (!strcmp(argv[argi], "--x86-config"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_config_file_name = argv[++argi];
			continue;
		}

		/* Function calls debug file */
		if (!strcmp(argv[argi], "--x86-debug-call"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_call_debug_file_name = argv[++argi];
			continue;
		}

		/* OpenCL runtime debug file */
		if (!strcmp(argv[argi], "--x86-debug-opencl"))
		{
			m2s_need_argument(argc, argv, argi);
			opencl_debug_file_name = argv[++argi];
			continue;
		}

		/* Context debug file */
		if (!strcmp(argv[argi], "--x86-debug-ctx"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_ctx_debug_file_name = argv[++argi];
			continue;
		}

		/* CUDA runtime debug file */
		if (!strcmp(argv[argi], "--x86-debug-cuda"))
		{
			m2s_need_argument(argc, argv, argi);
			cuda_debug_file_name = argv[++argi];
			continue;
		}


		/* GLUT debug file OpenGL Utility Toolkit*/
		if (!strcmp(argv[argi], "--x86-debug-glut"))
		{
			m2s_need_argument(argc, argv, argi);
			glut_debug_file_name = argv[++argi];
			continue;
		}

		/* GLEW debug file The OpenGL Extension Wrangler Library*/
		if (!strcmp(argv[argi], "--x86-debug-glew"))
		{
			m2s_need_argument(argc, argv, argi);
			glew_debug_file_name = argv[++argi];
			continue;
		}

		/* GLU debug file */
		if (!strcmp(argv[argi], "--x86-debug-glu"))
		{
			m2s_need_argument(argc, argv, argi);
			glu_debug_file_name = argv[++argi];
			continue;
		}

		/* ISA debug file */
		if (!strcmp(argv[argi], "--x86-debug-isa"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_isa_debug_file_name = argv[++argi];
			continue;
		}

		/* Loader debug file */
		if (!strcmp(argv[argi], "--x86-debug-loader"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_loader_debug_file_name = argv[++argi];
			continue;
		}

		/* OpenGL debug file */
		if (!strcmp(argv[argi], "--x86-debug-opengl"))
		{
			m2s_need_argument(argc, argv, argi);
			opengl_debug_file_name = argv[++argi];
			continue;
		}

		/* System call debug file */
		if (!strcmp(argv[argi], "--x86-debug-syscall"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_sys_debug_file_name = argv[++argi];
			continue;
		}

		/* Trace cache debug file */
		if (!strcmp(argv[argi], "--x86-debug-trace-cache"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_trace_cache_debug_file_name = argv[++argi];
			continue;
		}


		/* x86 disassembler */
		if (!strcmp(argv[argi], "--x86-disasm"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_disasm_file_name = argv[++argi];
			continue;
		}

		/* Help for x86 CPU configuration file */
		if (!strcmp(argv[argi], "--x86-help"))
		{
			fprintf(stderr, "%s", x86_config_help);
			continue;
		}

		/* Last x86 instruction */
		if (!strcmp(argv[argi], "--x86-last-inst"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_emu_last_inst_size = hex_str_to_byte_array(x86_emu_last_inst_bytes, argv[++argi], sizeof x86_emu_last_inst_bytes);
			continue;
		}

		/* CPU load checkpoint file name */
		if (!strcmp(argv[argi], "--x86-load-checkpoint"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_load_checkpoint_file_name = argv[++argi];
			continue;
		}

		/* Maximum number of cycles */
		if (!strcmp(argv[argi], "--x86-max-cycles"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_emu_max_cycles = str_to_llint(argv[argi + 1], &err);
			if (err)
				fatal("option %s, value '%s': %s", argv[argi],
						argv[argi + 1], str_error(err));
			argi++;
			continue;
		}

		/* Maximum number of instructions */
		if (!strcmp(argv[argi], "--x86-max-inst"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_emu_max_inst = str_to_llint(argv[argi + 1], &err);
			if (err)
				fatal("option %s, value '%s': %s", argv[argi],
						argv[argi + 1], str_error(err));
			argi++;
			continue;
		}

		/* File name to save checkpoint */
		if (!strcmp(argv[argi], "--x86-save-checkpoint"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_save_checkpoint_file_name = argv[++argi];
			continue;
		}

		/* x86 CPU pipeline report */
		if (!strcmp(argv[argi], "--x86-report"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_cpu_report_file_name = argv[++argi];
			continue;
		}

		/* x86 simulation accuracy */
		if (!strcmp(argv[argi], "--x86-sim"))
		{
			m2s_need_argument(argc, argv, argi);
			//star >> test output here.
			//printf("star >> %s", argv[++argi]);
			//argi = --argi;
			x86_sim_kind = str_map_string_err_msg(&arch_sim_kind_map, argv[++argi], "invalid value for --x86-sim.");
			continue;
		}


#if GPU
		/*
		 * Southern Islands GPU Options
		 */

		//Help for Southern Islands configuration file
		if (!strcmp(argv[argi], "--si-help"))
		{
			fprintf(stderr, "%s", si_gpu_config_help);
			continue;
		}

		//Southern Islands GPU occupancy calculation plots
		if (!strcmp(argv[argi], "--si-calc"))
		{
			m2s_need_argument(argc, argv, argi);
			si_gpu_calc_file_name = argv[++argi];
			continue;
		}

		//Southern Islands ISA debug file
		if (!strcmp(argv[argi], "--si-debug-isa"))
		{
			m2s_need_argument(argc, argv, argi);
			si_isa_debug_file_name = argv[++argi];
			continue;
		}

		//Southern Islands GPU configuration file
		if (!strcmp(argv[argi], "--si-config"))
		{
			m2s_need_argument(argc, argv, argi);
			si_gpu_config_file_name = argv[++argi];
			continue;
		}

		//Dump Southern Islands default configuration file
		if (!strcmp(argv[argi], "--si-dump-default-config"))
		{
			m2s_need_argument(argc, argv, argi);
			si_gpu_dump_default_config_file_name = argv[++argi];
			continue;
		}

		//Souther Islands disassembler
		if (!strcmp(argv[argi], "--si-disasm"))
		{
			m2s_need_argument(argc, argv, argi);
			si_disasm_file_name = argv[++argi];
			continue;
		}

		//Souther Islands OpenGL shader binary disassembler
		if (!strcmp(argv[argi], "--si-disasm-opengl"))
		{
			if (argc != 4)
				fatal("option '%s' requires two argument.\n%s",
						argv[argi], m2s_err_note);
			si_opengl_disasm_file_name = argv[++argi];
			si_opengl_disasm_shader_index = atoi(argv[++argi]);
			continue;
		}

		//Southern Islands OpenGL binary
		if (!strcmp(argv[argi], "--si-shader-binary"))
		{
			m2s_need_argument(argc, argv, argi);
			si_emu_opengl_binary_name = argv[++argi];
			continue;
		}

		//Maximum number of cycles
		if (!strcmp(argv[argi], "--si-max-cycles"))
		{
			m2s_need_argument(argc, argv, argi);
			si_emu_max_cycles = str_to_llint(argv[argi + 1], &err);
			if (err)
				fatal("option %s, value '%s': %s", argv[argi],
						argv[argi + 1], str_error(err));
			argi++;
			continue;
		}

		//Maximum number of instructions
		if (!strcmp(argv[argi], "--si-max-inst"))
		{
			m2s_need_argument(argc, argv, argi);
			si_emu_max_inst = str_to_llint(argv[argi + 1], &err);
			if (err)
				fatal("option %s, value '%s': %s", argv[argi],
						argv[argi + 1], str_error(err));
			argi++;
			continue;
		}

		//Maximum number of kernels
		if (!strcmp(argv[argi], "--si-max-kernels"))
		{
			m2s_need_argument(argc, argv, argi);
			si_emu_max_kernels = atoi(argv[++argi]);
			continue;
		}

		//Southern Islands GPU timing report
		if (!strcmp(argv[argi], "--si-report"))
		{
			m2s_need_argument(argc, argv, argi);
			si_gpu_report_file_name = argv[++argi];
			continue;
		}

		//Southern Islands simulation accuracy
		if (!strcmp(argv[argi], "--si-sim"))
		{
			m2s_need_argument(argc, argv, argi);
			si_sim_kind = str_map_string_err_msg(&arch_sim_kind_map, argv[++argi], "invalid value for --si-sim.");
			continue;
		}

#endif

		/*
		 * Memory System Options
		 */

		/* Memory hierarchy configuration file */
		if (!strcmp(argv[argi], "--mem-config"))
		{
			m2s_need_argument(argc, argv, argi);
#if CGM
			cgm_config_file_name_and_path = argv[++argi];
			//printf("\nconfig file name is %s\n", cgm_config_file_name_and_path);
			//fflush(stdout);
			//getchar();
			continue;
		}
#else
			mem_config_file_name = argv[++argi];
			continue;
		}

		/* Memory hierarchy debug file */
		if (!strcmp(argv[argi], "--mem-debug"))
		{
			m2s_need_argument(argc, argv, argi);
			mem_debug_file_name = argv[++argi];
			continue;
		}

		/* Help for memory hierarchy configuration file */
		if (!strcmp(argv[argi], "--mem-help"))
		{
			fprintf(stderr, "%s", mem_config_help);
			continue;
		}

		/* Memory hierarchy report */
		if (!strcmp(argv[argi], "--mem-report"))
		{
			m2s_need_argument(argc, argv, argi);
			mem_report_file_name = argv[++argi];
			continue;
		}

		/*
		 * Network Options
		 */

		/* Interconnect debug file */
		if (!strcmp(argv[argi], "--net-debug"))
		{
			m2s_need_argument(argc, argv, argi);
			net_debug_file_name = argv[++argi];
			continue;
		}

		/* Network configuration file */
		if (!strcmp(argv[argi], "--net-config"))
		{
			m2s_need_argument(argc, argv, argi);
			net_config_file_name = argv[++argi];
			continue;
		}

		/* Help for network configuration file */
		if (!strcmp(argv[argi], "--net-help"))
		{
			fprintf(stderr, "%s", net_config_help);
			continue;
		}

		/* Injection rate for network simulation */
		if (!strcmp(argv[argi], "--net-injection-rate"))
		{
			m2s_need_argument(argc, argv, argi);
			net_sim_last_option = argv[argi];
			argi++;
			net_injection_rate = atof(argv[argi]);
			continue;
		}

		/* Traffic Pattern */
		if (!strcmp(argv[argi], "--net-traffic-pattern"))
		{
			m2s_need_argument(argc, argv, argi);
			net_sim_last_option = argv[argi];
			net_traffic_pattern = argv[++argi];
			continue;
		}
		/* Cycles for network simulation */
		if (!strcmp(argv[argi], "--net-max-cycles"))
		{
			m2s_need_argument(argc, argv, argi);
			net_sim_last_option = argv[argi];
			net_max_cycles = str_to_llint(argv[argi + 1], &err);
			if (err)
				fatal("option %s, value '%s': %s", argv[argi],
						argv[argi + 1], str_error(err));
			argi++;
			continue;
		}

		/* Network message size */
		if (!strcmp(argv[argi], "--net-msg-size"))
		{
			m2s_need_argument(argc, argv, argi);
			net_sim_last_option = argv[argi];
			argi++;
			net_msg_size = atoi(argv[argi]);
			continue;
		}

		/* Network report file */
		if (!strcmp(argv[argi], "--net-report"))
		{
			m2s_need_argument(argc, argv, argi);
			net_report_file_name = argv[++argi];
			continue;
		}

		/* Network visual file*/
		if (!strcmp(argv[argi], "--net-visual"))
		{
			m2s_need_argument(argc, argv, argi);
			net_visual_file_name = argv[++argi];
			continue;
		}
		/* Network simulation */
		if (!strcmp(argv[argi], "--net-sim"))
		{
			m2s_need_argument(argc, argv, argi);
			net_sim_network_name = argv[++argi];
			continue;
		}
#endif
		
		/*
		 * Rest
		 */

		/* Invalid option */
		if (argv[argi][0] == '-')
		{
			fatal("'%s' is not a valid command-line option.\n%s", argv[argi], m2s_err_note);
		}

		/* End of options */
		break;
	}



	//star >> 1522 - 1582 declares a message. The if statements will display the message with the incompatible option.
	/* Options only allowed for x86 detailed simulation */
	if (x86_sim_kind == arch_sim_kind_functional)
	{
		char *msg = "option '%s' not valid for functional x86 simulation.\n\tPlease use option '--x86-sim detailed' as well.\n";

		if (*x86_config_file_name)
			fatal(msg, "--x86-config");
		if (x86_emu_max_cycles)
			fatal(msg, "--x86-max-cycles");
		if (*x86_cpu_report_file_name)
			fatal(msg, "--x86-report");
	}

	if (*x86_disasm_file_name && argc > 3)
		fatal("option '--x86-disasm' is incompatible with other options.");

#if CGM

#else
	if (!*net_sim_network_name && net_sim_last_option)
		fatal("option '%s' requires '--net-sim'", net_sim_last_option);
	if (*net_sim_network_name && !*net_config_file_name)
		fatal("option '--net-sim' requires '--net-config'");
#endif

	//Options that only make sense for GPU detailed simulation
#if GPU
	if (si_sim_kind == arch_sim_kind_functional)
	{
		char *msg = "option '%s' not valid for functional GPU simulation.\n\tPlease use option '--si-sim detailed' as well.\n";
		if (*si_gpu_config_file_name)
			fatal(msg, "--si-config");
		if (si_emu_max_cycles)
			fatal(msg, "--si-max-cycles");
		if (*si_gpu_report_file_name)
			fatal(msg, "--si-report");
	}

	//star >> checking for presence of an option and to many or to little parameters.
	/* Other checks */
	if (*si_disasm_file_name && argc > 3)
		fatal("option '--si-disasm' is incompatible with any other options.");
	if (*si_gpu_dump_default_config_file_name && argc > 3)
		fatal("option '--si-dump-default-config' is incompatible with any other options.");
	if (*si_opengl_disasm_file_name && argc != 4)
		fatal("option '--si-disasm-opengl' is incompatible with any other options.");
#endif



	//star >> this casts away command line entries until all entries have been processed.
	/* Discard arguments used as options */
	arg_discard = argi - 1;
	for (argi = 1; argi < argc - arg_discard; argi++)
		argv[argi] = argv[argi + arg_discard];
	*argc_ptr = argc - arg_discard;
}


static void m2s_load_programs(int argc, char **argv)
{

	Elf32_Ehdr ehdr;

	/* Load guest program specified in the command line */
	if (argc > 1)
	{
		/* Load program depending on architecture */
		//star >> returns valid loaded sturct with ELF header info (elf-format.c).
		elf_file_read_header(argv[1], &ehdr);

		//star >> e_machine contains architecture type i.e. Intel 80386 (EM_386) for x86 simulation
		switch (ehdr.e_machine)
		{
			//star >> each function in this switch comes from different places...
			case EM_386:
				//star >> from EMU.c/.h

				X86EmuLoadContextFromCommandLine(x86_emu, argc - 1, argv + 1);

			break;

			default:
			fatal("%s: unsupported ELF architecture", argv[1]);
		}
	}

	//the rest of this function isn't run for our examples
	/* Continue processing the context configuration file, if specified. */
	if (!*ctx_config_file_name)
		return;


	//uncomment this if some of the benchmarks don't run.

/*
	Open file
	config = config_create(ctx_config_file_name);
	if (*ctx_config_file_name)
		config_load(config);

	Iterate through consecutive contexts
	for (id = 0; ; id++)
	{
		 Read section
		snprintf(section, sizeof section, "Context %d", id);
		if (!config_section_exists(config, section))
			break;

		 Read executable full path
		exe_file_name = config_read_string(config, section, "Exe", "");
		cwd_path = config_read_string(config, section, "Cwd", "");
		file_full_path(exe_file_name, cwd_path, exe_full_path, sizeof exe_full_path);

		 Load context depending on architecture
		elf_file_read_header(exe_full_path, &ehdr);
		switch (ehdr.e_machine)
		{
		case EM_386:
			X86EmuLoadContextsFromConfig(x86_emu, config, section);
			break;

		default:
			fatal("%s: unsupported ELF architecture", argv[1]);
		}
	}

	 Close file
	config_check(config);
	config_free(config);

*/

}


static void m2s_dump(FILE *f)
{
	arch_for_each((arch_callback_func_t) arch_dump, f);
}


static void m2s_dump_summary(FILE *f)
{
	double time_in_sec;
	long long cycles;

	/* No summary dumped if no simulation was run */
	if (m2s_loop_iter < 2)
		return;

	/* Header */
	fprintf(f, "\n");
	fprintf(f, ";\n");
	fprintf(f, "; Simulation Statistics Summary\n");
	fprintf(f, ";\n");
	fprintf(f, "\n");

	/* Calculate statistics */
	time_in_sec = (double) esim_real_time() / 1.0e6;

	/* General statistics */
	fprintf(f, "[ General ]\n");
	fprintf(f, "RealTime = %.2f [s]\n", time_in_sec);
	fprintf(f, "SimEnd = %s\n", str_map_value(&esim_finish_map, esim_finish));

	/* General detailed simulation statistics */
	if (esim_time)
	{
		cycles = esim_cycle();
		fprintf(f, "SimTime = %.2f [ns]\n", esim_time / 1000.0);
		fprintf(f, "Frequency = %d [MHz]\n", esim_frequency);
		fprintf(f, "Cycles = %lld\n", cycles);
	}

	/* End */
	fprintf(f, "\n");

	/* Summary for all architectures */
	arch_for_each((arch_callback_func_t) arch_dump_summary, f);
}


/* Signal handler while functional simulation loop is running */
static void m2s_signal_handler(int signum)
{
	/* If a signal SIGINT has been caught already and not processed, it is
	 * time to not defer it anymore. Execution ends here. */
	if (m2s_signal_received == signum && signum == SIGINT)
	{
		fprintf(stderr, "SIGINT received\n");
		exit(1);
	}

	/* Just record that we are receiving a signal. It is not a good idea to
	 * process it now, since we might be interfering some critical
	 * execution. The signal will be processed at the end of the simulation
	 * loop iteration. */
	m2s_signal_received = signum;
}


static void m2s_signal_process(void)
{
	/* Process signal */
	switch (m2s_signal_received)
	{

		case SIGINT:
		{
			/* Second time signal was received, abort. */
			if (esim_finish)
				abort();

			/* Try to normally finish simulation */
			esim_finish = esim_finish_signal;
			warning("signal SIGINT received");
			break;
		}

		case SIGUSR1:
		case SIGUSR2:
		{
			long long time_in_dsec;
			char file_name[MAX_STRING_SIZE];
			FILE *f;

			/* Report file name */
			time_in_dsec = (double) esim_real_time() / 1.0e4;
			snprintf(file_name, sizeof file_name, "m2s-%s-%lld", m2s_sim_id, time_in_dsec);
			warning("user signal received, dumping report in '%s'\n", file_name);

			/* Create report file */
			f = fopen(file_name, "wt");
			if (!f)
			{
				warning("%s: failed to write on file\n", file_name);
				break;
			}

			/* Dump summary or detailed report */
			if (m2s_signal_received == SIGUSR1)
				m2s_dump_summary(f);
			else
				m2s_dump(f);

			/* Close */
			fclose(f);
			break;
		}


		default:

			fprintf(stderr, "Signal %d received\n", m2s_signal_received);
			exit(1);
		}

		/* Signal already processed */
		m2s_signal_received = 0;
}


static void m2s_init(void)
{
	//printf("inside m2s_init");
	struct timeval tv;
	unsigned int min_id;
	unsigned int max_id;
	unsigned int id;

	/* Classes */
	CLASS_REGISTER(Asm);
	CLASS_REGISTER(Emu);
	CLASS_REGISTER(Timing);

	/* Compute simulation ID */
	gettimeofday(&tv, NULL);
	min_id = str_alnum_to_int("10000");
	max_id = str_alnum_to_int("ZZZZZ");
	id = (tv.tv_sec * 1000000000 + tv.tv_usec) % (max_id - min_id + 1) + min_id;
	str_int_to_alnum(m2s_sim_id, sizeof m2s_sim_id, id);

	/* Initial information  with fix*/
	fprintf(stderr, "\n");
	fprintf(stderr, "; Multi2Sim %s - ", m2sversion);
	fprintf(stderr, "A Simulation Framework for CPU-GPU Heterogeneous Computing\n");
	fprintf(stderr, "; Please use command 'm2s --help' for a list of command-line options.\n");
	fprintf(stderr, "; Simulation alpha-numeric ID: %s\n", m2s_sim_id);
	fprintf(stderr, "\n");

}

void m2s_loop(void){


	int num_emu_active;
	int num_timing_active;

	/* Install signal handlers */
	signal(SIGINT, &m2s_signal_handler);
	signal(SIGABRT, &m2s_signal_handler);
	signal(SIGUSR1, &m2s_signal_handler);
	signal(SIGUSR2, &m2s_signal_handler);

	printf("---Simulation Start---\n");
	fflush(stdout);

	/* Simulation loop */
	while (!esim_finish)
	{
		/* Run iteration for all architectures. This function returns the number
		 * of architectures actively running emulation, as well as the number of
		 * architectures running an active timing simulation. */
		arch_run(&num_emu_active, &num_timing_active);


		/* Event-driven simulation. Only process events and advance to next global
		 * simulation cycle if any architecture performed a useful timing simulation.
		 * The argument 'num_timing_active' is interpreted as a flag TRUE/FALSE. */
		esim_process_events(num_timing_active);

		/* If neither functional nor timing simulation was performed for any architecture,
		 * it means that all guest contexts finished execution - simulation can end. */
		if (!num_emu_active && !num_timing_active)
			esim_finish = esim_finish_ctx;

		/* Count loop iterations, and check for limit in simulation time only every
		 * 128k iterations. This avoids a constant overhead of system calls. */
		m2s_loop_iter++;

		//star >> added this to get some status output while running long benchmarks.
		PrintCycle(SKIP);


		if (m2s_max_time && !(m2s_loop_iter & ((1 << 17) - 1)) && esim_real_time() > m2s_max_time * 1000000)
			esim_finish = esim_finish_max_time;


		/* Signal received */
		if (m2s_signal_received)
		{
			printf("---m2s_signal_received---\n");
			m2s_signal_process();
		}

	}

	//printf("loop = %lld\n", m2s_loop_iter);
	//printf("exiting\n");
	/* Restore default signal handlers */
	signal(SIGABRT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGUSR1, SIG_DFL);
	signal(SIGUSR2, SIG_DFL);
}


void sim_end(void){

	//printf("etime.count %lld\n", etime.count);

	printf("\n---Simulation End (CGM)---\n");
	fflush(stdout);

	//star >> don't need this for simple/short runs benchmarks
	/* Save architectural state checkpoint */
	//if (x86_save_checkpoint_file_name[0])
	//X86EmuSaveCheckpoint(x86_emu, x86_save_checkpoint_file_name);

	/* Flush event-driven simulation, only if the reason for simulation
	 * completion was not a simulation stall. If it was, draining the
	 * event-driven simulation could cause another stall! */
	if (esim_finish != esim_finish_stall)
		esim_process_all_events();

	//star >> execution is pretty much done here. The remainder is summary output and cleanup.

	/* Dump statistics summary */
	//m2s_dump_summary(stderr);
	cgm_dump_summary();


	/* x86 */
	if (x86_cpu)
		delete(x86_cpu);
	X86CpuDone();

	/* Finalization of architectures */
	arch_done();

	/* Finalization of runtimes */
#if GPU
	runtime_done();
	opencl_done();
	//cuda_done();
#endif

	/* Finalization of network and memory system */
	/* Finalization of drivers */
	/* Finalization of libraries */
	mmu_done();
	esim_done();
	trace_done();
	debug_done();
	mhandle_done();

	return;
}



int main(int argc, char **argv)
{

	/* Global initialization and welcome message */
	m2s_init();

	/* Read command line */
	m2s_read_command_line(&argc, argv);

	//star >> given command line "--x86-disasm" this will output a disassembly in formatted text then exit.
	/* x86 disassembler tool */
	if (*x86_disasm_file_name)
		x86_asm_disassemble_binary(x86_disasm_file_name);

#if GPU
	/* Southern Islands disassembler tool */
	  if (*si_disasm_file_name)
		  si_disasm(si_disasm_file_name);

	/* Southern Islands OpenGL disassembler tool */
	  if (*si_opengl_disasm_file_name)
		  si_emu_opengl_disasm(si_opengl_disasm_file_name, si_opengl_disasm_shader_index);

	/* Southern Islands dump config file */
	 if (*si_gpu_dump_default_config_file_name)
		 si_gpu_dump_default_config(si_gpu_dump_default_config_file_name);
#endif

	/* Debug */
	//star >> may need to uncomment some of these if there are problems.
	debug_init();
	elf_debug_category = debug_new_category(elf_debug_file_name);
	x86_context_debug_category = debug_new_category(x86_ctx_debug_file_name);
	x86_context_isa_debug_category = debug_new_category(x86_isa_debug_file_name);
	x86_context_call_debug_category = debug_new_category(x86_call_debug_file_name);
	x86_loader_debug_category = debug_new_category(x86_loader_debug_file_name);
	x86_sys_debug_category = debug_new_category(x86_sys_debug_file_name);
	x86_trace_cache_debug_category = debug_new_category(x86_trace_cache_debug_file_name);

#if CGM
	//star took out mem debug
#else
	mem_debug_category = debug_new_category(mem_debug_file_name);
	net_debug_category = debug_new_category(net_debug_file_name);
#endif


#if GPU
	opencl_debug_category = debug_new_category(opencl_debug_file_name);
	si_isa_debug_category = debug_new_category(si_isa_debug_file_name);
	//glu_debug_category = debug_new_category(glu_debug_file_name);
	//glut_debug_category = debug_new_category(glut_debug_file_name);
	//glew_debug_category = debug_new_category(glew_debug_file_name);
	//opengl_debug_category = debug_new_category(opengl_debug_file_name);
	//cuda_debug_category = debug_new_category(cuda_debug_file_name);
#endif

	/* Initialization of runtimes */
	//star >> runtime_inti() creates an empty runtime list of type list_t
	runtime_init();


	//star add these various runtimes to the runtime list
#if GPU
	runtime_register("OpenCL", "OpenCL", "m2s-opencl", 329, (runtime_abi_func_t) opencl_abi_call);
	//runtime_register("Old OpenCL", "m2s-opencl-old", "m2s-opencl-old", 325, (runtime_abi_func_t) evg_opencl_abi_call);
	//runtime_register("GLUT", "glut", "m2s-glut", 326, (runtime_abi_func_t) glut_abi_call);
	//runtime_register("OpenGL", "GL", "m2s-opengl", 327, (runtime_abi_func_t) opengl_abi_call);
	//runtime_register("CUDA", "cuda", "m2s-cuda", 328, (runtime_abi_func_t) cuda_abi_call);
	//runtime_register("CudaRT", "cudart", "m2s-cuda", 328, (runtime_abi_func_t) cuda_abi_call);
	//runtime_register("GLEW", "GLEW", "m2s-glew", 330, (runtime_abi_func_t) glew_abi_call);
	//runtime_register("GLU", "GLU", "m2s-glu", 331, (runtime_abi_func_t) glu_abi_call);

	/* Initialization of GPU drivers */
	//star >> this does nothing at the moment? opencl_init() is an empty function in onpencl.c.
	opencl_init();
	//cuda_init();
#endif


	/* Initialization of libraries */
	esim_init();
	trace_init(trace_file_name);

	/* Initialization of architectures */
	//star >> looks like all architectures are registered whether you need them or not.
	//star >> x86_emu_init calls openGL, GLUT, etc libraries.
	//sets up the architecture for x86 arch_x86 is a pointer to a struct.

#if GPU
	arch_southern_islands = arch_register("SouthernIslands", "si", si_sim_kind, si_emu_init, si_emu_done, si_gpu_read_config, si_gpu_init, si_gpu_done);
#endif

	arch_x86 = arch_register("x86", "x86", x86_sim_kind, x86_emu_init, x86_emu_done, X86CpuReadConfig, NULL, NULL);
	arch_init();

#if GPU
	arch_set_emu(arch_southern_islands, asEmu(si_emu));
	arch_set_timing(arch_southern_islands, asTiming(si_gpu));
#endif

	/* x86
	 * The code above and below is in an intermediate state on the process
	 * of converting all architectures' disassemblers, emulators and timing
	 * simulators into independent classes. All those arch_register calls
	 * and their associated call-backs will go away.
	 * For now, this has been done for x86 only. Notice how variable 'x86_cpu'
	 * is now defined privately only in this file. This code will look much
	 * better once the process finish. But now we need to release 4.2...
	 */

	X86CpuInit();
	arch_set_emu(arch_x86, asEmu(x86_emu));

	if (x86_sim_kind == arch_sim_kind_detailed)
	{
		x86_cpu = new(X86Cpu, x86_emu);
		arch_set_timing(arch_x86, asTiming(x86_cpu));
	}

	//star >> added instrumentation
	//instrumentation_init();

	/* Network and memory system */




	//CGM is the replacement memory system.
#if CGM
	cgm_init();
	cgm_configure();

#else
	//this is old m2s code for the memory system and network.
	net_init();
	mem_system_init();
#endif

	mmu_init();

	/* Load architectural state checkpoint */
	//star >> only runs if you load a checkpoint file.
	//Commented these out for testing purposes.
	//if (x86_load_checkpoint_file_name[0])
	//	X86EmuLoadCheckpoint(x86_emu, x86_load_checkpoint_file_name);

	/* Load programs */
	//star >> stars the ELF parsing work.
	m2s_load_programs(argc, argv);



	//run ends here if CGM is running.
	//sim_send contains all of the "done" functions.
#if CGM

	simulate(sim_end);

	/* End */
	//return 0;

#else

	/* Multi2Sim Central Simulation Loop */
	m2s_loop();

	printf("---Simulation End---\n");
	fflush(stdout);

	//star >> don't need this for simple/short runs benchmarks
	/* Save architectural state checkpoint */
	//if (x86_save_checkpoint_file_name[0])
	//X86EmuSaveCheckpoint(x86_emu, x86_save_checkpoint_file_name);

	/* Flush event-driven simulation, only if the reason for simulation
	 * completion was not a simulation stall. If it was, draining the
	 * event-driven simulation could cause another stall! */
	if (esim_finish != esim_finish_stall)
		esim_process_all_events();


	//star >> execution is pretty much done here. The remainder is summary output and cleanup.

	/* Dump statistics summary */
	m2s_dump_summary(stderr);


	/* x86 */
	if (x86_cpu)
		delete(x86_cpu);
	X86CpuDone();

	/* Finalization of architectures */
	arch_done();

	/* Finalization of runtimes */
#if GPU
	runtime_done();
	opencl_done();
	//cuda_done();
#endif

	/* Finalization of network and memory system */
	mem_system_done();
	net_done();
	mmu_done();

	/* Finalization of drivers */
	/* Finalization of libraries */
	esim_done();
	trace_done();
	debug_done();
	mhandle_done();

	/* End */
	return 0;

#endif

return 0;

}
