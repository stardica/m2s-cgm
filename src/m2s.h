/*
 * m2s.h
 *
 *  Created on: Jan 25, 2015
 *      Author: stardica
 */


#ifndef M2S_H_
#define M2S_H_

//GPU 1 or 0 takes in and out all of the GPU and runtime code
//CGM 1 or 0 takes in and out the new memory system
#define GPU 1
#define CGM 1
//MSG turns on and off verbose notes
// MSG 1 outputs configuration data.
// MSG 2 outputs memory system runtime data.
#define MSG 0
//print task related messages.
#define TSK 0

#define INT 1

//m2s prototypes
void m2s_loop(void);


#endif /*M2S_H_*/
