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

#ifndef X86_ARCH_TIMING_FU_H
#define X86_ARCH_TIMING_FU_H

#include <lib/util/class.h>

#include <arch/x86/timing/uop.h>


/* Forward declarations */
struct config_t;


/*
 * Class 'X86Core'
 */

void X86CoreInitFunctionalUnits(X86Core *self);
void X86CoreFreeFunctionalUnits(X86Core *self);

void X86CoreDumpFunctionalUnitsReport(X86Core *self, FILE *f);

int X86CoreReserveFunctionalUnit(X86Core *self, struct x86_uop_t *uop);
void X86CoreReleaseAllFunctionalUnits(X86Core *self);




/*
 * Public
 */

#define X86_FU_RES_MAX  10

/* WARNING:
 * For every entry in this enumeration, array 'x86_fu_name' should be modified
 * accordingly in 'fu.c'. Array 'x86_fu_res_pool' containing the default
 * configuration for each functional unit should also be updated. */
enum x86_fu_class_t
{
	x86_fu_none = 0,

	x86_fu_int_add, 		//1
	x86_fu_int_mult,		//2
	x86_fu_int_div, 		//3

	x86_fu_effaddr, 		//4
	x86_fu_logic, 			//5

	x86_fu_float_simple,	//6
	x86_fu_float_add,		//7
	x86_fu_float_comp,		//8
	x86_fu_float_mult,		//9
	x86_fu_float_div,		//10
	x86_fu_float_complex,	//11

	x86_fu_xmm_int_add,		//12
	x86_fu_xmm_int_mult,	//13
	x86_fu_xmm_int_div,		//14

	x86_fu_xmm_logic,		//15

	x86_fu_xmm_float_add,	//16
	x86_fu_xmm_float_comp,	//17
	x86_fu_xmm_float_mult,	//18
	x86_fu_xmm_float_div,	//19
	x86_fu_xmm_float_conv,	//20
	x86_fu_xmm_float_complex,	//21

	x86_fu_count			//22
};

struct x86_fu_t
{
	long long cycle_when_free[x86_fu_count][X86_FU_RES_MAX];
	long long accesses[x86_fu_count];
	long long denied[x86_fu_count];
	long long waiting_time[x86_fu_count];
};

struct x86_fu_res_t
{
	int count;
	int oplat;
	int issuelat;
};

extern char *x86_fu_name[x86_fu_count];
extern struct x86_fu_res_t x86_fu_res_pool[x86_fu_count];
extern enum x86_fu_class_t x86_fu_class_table[x86_uinst_opcode_count];

void X86ReadFunctionalUnitsConfig(struct config_t *config);
void X86DumpFunctionalUnitsConfig(FILE *f);


#endif

