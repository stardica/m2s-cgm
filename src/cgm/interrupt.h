/*
 * interrupt.h
 *
 *  Created on: Mar 28, 2015
 *      Author: stardica
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <cgm/cgm.h>
#include <cgm/tasking.h>

extern eventcount volatile *interrupt;


//interupts
void interrupt_init(void);
void interrupt_service_request(void);





#endif /* __INTERRUPT_H__ */
