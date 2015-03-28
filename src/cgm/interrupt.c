/*
 * interrupt.c
 *
 *  Created on: Mar 28, 2015
 *      Author: stardica
 */

#include <stdio.h>
#include <string.h>

#include <cgm/interrupt.h>
#include <cgm/cgm.h>
#include <cgm/tasking.h>



eventcount volatile *interrupt;


void interrupt_init(void){

	char buff[100];

	//event count
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "interrupt");
	interrupt = new_eventcount(strdup(buff));

	//task
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "interrupt");
	create_task(interrupt_service_request, DEFAULT_STACK_SIZE, strdup(buff));

	return;
}


void interrupt_service_request(void){





	return;
}
