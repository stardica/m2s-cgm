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
 *  You should have received as copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_COMMON_EMU_H
#define ARCH_COMMON_EMU_H

#include <lib/util/class.h>


/*	//Class 'Emu'
 * 	//Base class for the emulators of all architectures.
 *
 * 	static struct class_t *EmuParentClass __attribute__((unused)) = &ObjectClass;
 * 	extern struct class_t EmuClass;
 *
 * 	struct _Emu
 * 	{
 * 		Object __parent;
 * 		struct class_info_t __info;
 *
 * 		//Name of emulator
 * 		char *name;
 *
 * 		//Architecture it belongs to
 * 		struct arch_t *arch;
 *
 * 		//Timer keeping track of emulator activity
 * 		struct m2s_timer_t *timer;
 *
 * 		//Number of emulated instructions
 * 		long long instructions;
 *
 * 		//Virtual functions
 * 		//Run one iteration of the emulation loop for the architecture. If
 * 		//there was an active emulation, the function returns TRUE. This is
 * 		//a virtual and abstract function (no implementation for 'Emu').
 * 		int (*Run)(Emu *self);
 *
 * 		//Dump statistics summary
 * 		void (*DumpSummary)(Emu *self, FILE *f);
 * 	};
 * 	//Prototype for destructor
 * 	void EmuDestroy(Emu *self);
 *
 * 	//Check if object is instance of class
 * 	static inline int isEmu(void *p)
 * 	{
 * 		return class_instance_of(p, &EmuClass);
 * 	}
 *
 * 	//Cast to class
 * 	static inline Emu *asEmu(void *p)
 * 	{
 * 		((isEmu(p)) ? (void) (0) : __assert_fail ("isEmu(p)", "/home/stardica/Desktop/m2srev2019/src/arch/common/emu.h", 103, __PRETTY_FUNCTION__));
 * 		return (Emu *) p;
 * 	}
 */


CLASS_BEGIN(Emu, Object)

	/* Name of emulator */
	char *name;

	/* Architecture it belongs to */
	struct arch_t *arch;
	
	/* Timer keeping track of emulator activity */
	struct m2s_timer_t *timer;

	/* Number of emulated instructions */
	long long instructions;


	/*** Virtual functions ***/

	/* Run one iteration of the emulation loop for the architecture. If
	 * there was an active emulation, the function returns TRUE. This is
	 * a virtual and abstract function (no implementation for 'Emu'). */
	int (*Run)(Emu *self);

	/* Dump statistics summary */
	void (*DumpSummary)(Emu *self, FILE *f);

CLASS_END(Emu)


void EmuCreate(Emu *self, char *name);
void EmuDestroy(Emu *self);
void EmuRun(void);

/* Virtual function from class 'Object' */
void EmuDump(Object *self, FILE *f);

/* Virtual function to print statistics summary. The statistics part that is
 * common for all architectures is done in this implementation, which should be
 * invoked by the child. */
void EmuDumpSummary(Emu *self, FILE *f);


#endif
