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

#ifndef ARCH_COMMON_ASM_H
#define ARCH_COMMON_ASM_H

#include <stdio.h>

#include <lib/util/class.h>


/*
 *	//Class 'Asm'
 *	//Class static struct class_t *AsmParentClass __attribute__((unused)) = &ObjectClass;
 *
 *	extern struct class_t AsmClass;
 *
 *	struct _Asm
 *	{
 *		Object __parent;
 *		struct class_info_t __info;
 *
 *		//Architecture that it belongs to
 *		struct arch_t *arch;
 *	};
 *
 *	//Prototype for destructor
 *	void AsmDestroy(Asm *self);
 *
 *	//Check if object is instance of class
 *	static inline int isAsm(void *p)
 *	{
 *		return class_instance_of(p, &AsmClass);
 *	}
 *
 *	//Cast to class
 *	static inline Asm *asAsm(void *p)
 *	{
 *		((isAsm(p)) ? (void) (0) : __assert_fail ("isAsm(p)", "/home/stardica/Desktop/m2srev2019/src/arch/common/asm.h", 59, __PRETTY_FUNCTION__));
 *		return (Asm *) p;
 *	}
 */


CLASS_BEGIN(Asm, Object)
	
	/* Architecture that it belongs to */
	struct arch_t *arch;

CLASS_END(Asm)


void AsmCreate(Asm *self);
void AsmDestroy(Asm *self);


/*
 * Non-Class Functions
 */

/* Function used when processing format strings in 'asm.dat'. Given a pointer
 * to the middle of a format string in 'fmt', return whether 'token' is
 * present at that position. If it is, the token length is returned in
 * 'length', which the disassembler can use to advance its position in the
 * format string. */
int asm_is_token(char *fmt, char *token, int *length);


#endif

