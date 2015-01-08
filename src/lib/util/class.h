/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef LIB_UTIL_CLASS_H
#define LIB_UTIL_CLASS_H


#include <assert.h>
#include <stdio.h>


/*
 * All classes must be defined here. In the latest versions of 'gcc' this is
 * not needed. But multiple definitions of the same type using 'typedef' is
 * not allowed in standard C. This is why they need to be protected by the
 * '#ifdef' guard in this file, instead of appearing in multiple class
 * forward declarations.
 */

#define CLASS_DECLARE(name) 									\
								typedef struct _##name name

//star same as... declares a type Object of struct _Object
//typedef struct _Object Object;


/*
 * Multi2Sim Classes
 */

CLASS_DECLARE(ARMAsm);
CLASS_DECLARE(ARMEmu);
CLASS_DECLARE(ARMCpu);

CLASS_DECLARE(EvgAsm);
CLASS_DECLARE(EvgEmu);
CLASS_DECLARE(EvgGpu);

CLASS_DECLARE(FrmAsm);
CLASS_DECLARE(FrmEmu);
CLASS_DECLARE(FrmGpu);

CLASS_DECLARE(MIPSAsm);
CLASS_DECLARE(MIPSEmu);
CLASS_DECLARE(MIPSCpu);

CLASS_DECLARE(Asm);
CLASS_DECLARE(Emu);
CLASS_DECLARE(Timing);

CLASS_DECLARE(Object);

CLASS_DECLARE(SIAsm);
CLASS_DECLARE(SIEmu);
CLASS_DECLARE(SIGpu);

CLASS_DECLARE(X86Asm);
CLASS_DECLARE(X86Emu);
CLASS_DECLARE(X86Cpu);
CLASS_DECLARE(X86Context);
CLASS_DECLARE(X86Thread);
CLASS_DECLARE(X86Core);


/*
 * Multi2C Classes
 */

CLASS_DECLARE(Node);
CLASS_DECLARE(LeafNode);
CLASS_DECLARE(AbstractNode);
CLASS_DECLARE(CTree);
CLASS_DECLARE(BasicBlock);

CLASS_DECLARE(Llvm2siPhi);
CLASS_DECLARE(Llvm2siSymbol);
CLASS_DECLARE(Llvm2siSymbolTable);
CLASS_DECLARE(Llvm2siBasicBlock);
CLASS_DECLARE(Llvm2siFunction);
CLASS_DECLARE(Llvm2siFunctionUAV);
CLASS_DECLARE(Llvm2siFunctionArg);



/*
 * Class
 */


struct class_t
{
	/* Name of the class */
	//star >> this is a string pointer to a string
	char *name;

	/* Unique identifier of the class, calculated as a hash function of its
	 * name during initialization. */
	unsigned int id;

	/* Total size of the class structure, considering both the user fields
	 * and the '__parent' and '__info' metadata fields. */
	unsigned int size;

	/* Offset where field '__info' can be found, relative to the beginning
	 * of the class data structure. */
	unsigned int info_offset;

	/* Class parent */
	struct class_t *parent;

	/* Class destructor */
	//star >> this points to a function.
	void (*destroy)(void *ptr);

	/* Next class in class list */
	struct class_t *next;
};


struct class_info_t
{
	struct class_info_t *parent;
	struct class_info_t *child;
	struct class_t *c;
};




/*
 * Base class 'Object'
 */

struct _Object
{
	/* Class information, first field */
	struct class_info_t __info;

	
	/*** Virtual functions ***/

	void (*Dump)(Object *self, FILE *f);
};



extern struct class_t ObjectClass;

static inline int isObject(void *p)
{
	assert(((Object *) p)->__info.c);
	assert(((Object *) p)->__info.c->id == ObjectClass.id);
	return 1;
}

static inline Object *asObject(void *p)
{
	assert(isObject(p));
	return (Object *) p;
}

void ObjectCreate(Object *self);
void ObjectDestroy(Object *self);

void ObjectDump(Object *self, FILE *f);




/*
 * Class definition macros
 */


									//star >> class begin
									//static struct class_t *X86EmuParentClass __attribute__((unused)) = &EmuClass;
									//extern struct class_t EmuClass;
									//struct _X86Emu{
									//parent __parent;
									//struct class_info_t __info;

									///* ... user fields follow here ... */



#define CLASS_BEGIN(name, parent) 													\
									static struct class_t *name##ParentClass 		\
									__attribute__((unused)) = &parent##Class; 		\
									extern struct class_t name##Class; 				\
									struct _##name 									\
									{ 												\
									parent __parent; 								\
									struct class_info_t __info;
									/* ... user fields follow here ... */


									//star >> ends a particular class

									///* Prototype for destructor */
									//void X86EmuDestroy(x86Emu *self);

									///* Check if object is instance of class */

									//static inline int isX86Emu(void *p){
									//return class_instance_of(p, &X86EmuClass);
									//}

									///* Cast to class */

									//asX86Emu returns the pointer to the name?
									//static inline name *asX86Emu(void *p){
									//assert(isX86Emu(p));
									//return (name *) p;
									//}
#define CLASS_END(name) 															\
																					\
									/* ... user fields end here ... */  			\
									};												\
																					\
									/* Prototype for destructor */ 					\
									void name##Destroy(name *self); 				\
																					\
									/* Check if object is instance of class */ 		\
									static inline int is##name(void *p) 			\
									{												\
									return class_instance_of(p, &name##Class); 		\
									}												\
																					\
									/* Cast to class */ 							\
									static inline name *as##name(void *p) 			\
									{												\
									assert(is##name(p)); 							\
									return (name *) p; 								\
									}




									//star >> this is where x86EmuClass is declared.
#define CLASS_IMPLEMENTATION(name) 													\
									struct class_t name##Class;

									//star >> declares an instance of the "name" passed to the macro i.e. x86Emu.
									//initializes the "x86Emuclass" class.

									//typedef struct _x86Emu x86Emu
									//X86Emu instance
									//class_init()
									//x86EmuClass.name = "x86Emu";
									//x86EmuClass.size = sizeof(x86Emu);
									//x86EmuClass.info_offset = (unsigned int) ((void *) &instance.__info - (void *) &instance); Note: instance is thrown away.
									//x86EmuClass.parent = x86EmuParentClass;
									//x86EmuClass.destroy = (void(*)(void *)) x86EmuDestroy;
									//class_register(&x86EmuClass); Note: passes in dereferenced struct x86EmuClass

									//class_init()
									//x86EmuDestroy()
									//class_register()

									//class_init()
									//SIEmuDestroy()
									//class_register()

									//class_init()
									//SIGpuDestroy()
									//class_register()

#define CLASS_REGISTER(_name) 																	\
									({ 															\
									class_init();												\
									_name instance; 											\
									_name##Class.name = #_name; 								\
									_name##Class.size = sizeof(_name);	 						\
									_name##Class.info_offset = (unsigned int) ((void *) &instance.__info - (void *) &instance);				\
									_name##Class.parent = _name##ParentClass; 					\
									_name##Class.destroy = (void(*)(void *)) _name##Destroy; 	\
									class_register(&_name##Class); 								\
									})


//star >> playing with the do while... why have a do while (0)? i modified the code to what is above...
/*#define CLASS_REGISTER(_name) 																\
*								do { 															\
*									_name instance; 											\
*									class_init(); 												\
*									_name##Class.name = #_name; 								\
*									_name##Class.size = sizeof(_name);	 						\
*									_name##Class.info_offset = (unsigned int) ((void *) &instance.__info - (void *) &instance);				\
*									_name##Class.parent = _name##ParentClass; 					\
*									_name##Class.destroy = (void(*)(void *)) _name##Destroy; 	\
*									class_register(&_name##Class); 								\
*									} while (0)
*/


//star >> the ... means it can take multiple arguments.
//new() maybe called in other places with additional arguments for other architectures.
#define new(name, ...) new_ctor(name, Create, ##__VA_ARGS__)


									//star >> returns the pointer _P
									//X86Emu *_P = class_new(&x86EmuClass);
									//x86EmuCreate(_p, x86Emu__VA_ARGS__); Note: _VA_ARGS_ expands to support additional arguments.
									//__p

									//---X86Emu---//
									//X86EmuCreate takes in a single pointer to the x86Emu.

									//---Cpu---//
									//X86CpuCreate takes in a single pointer to an Emu (x86).
									//---Core---//
									//X86CoreCreate >> new(X86Core, self);
									//---Thread---//
									//X86ThreadCreate >> new(X86Thread, self);
									//---X86Context---//
									//X86ContextCreate >> new(X86Context, self);
									//---X86Asm---//
									//X86AsmCreate >> x86_asm = new(X86Asm);

									//---SIEmu---//
									//SIEmuCreate >> si_emu = new(SIEmu);

									//---SIGpu---//
									//SIGpuCreate >> si_gpu = new(SIGpu);

#define new_ctor(name, ctor, ...) 																\
									({ 															\
									name *__p = class_new(&name##Class); 						\
									name##ctor(__p, ##__VA_ARGS__); 							\
									__p; 														\
									})


#define delete(var) class_delete((var))


extern struct class_t *class_list_head;
extern struct class_t *class_list_tail;

void class_init(void);
void class_register(struct class_t *c);
unsigned int class_compute_id(char *name);
void *class_new(struct class_t *c);
void class_delete(void *p);
int class_instance_of(void *p, struct class_t *c);

/* Return the last child class of object in 'p' */
struct class_t *class_of(void *p);

/* Dump information about all classes registered */
void class_dump(FILE *f);


#endif
