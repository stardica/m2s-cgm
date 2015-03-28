/*
 * interrupt.c
 *
 *  Created on: Mar 28, 2015
 *      Author: stardica
 */

#include <cgm/cgm.h>
#include <cgm/tasking.h>


eventcount volatile *interrupt;



/*
memset(buff,'\0' , 100);
snprintf(buff, 100, "interrupt");
interrupt = new_eventcount(strdup(buff));*/

/*
memset(buff,'\0' , 100);
snprintf(buff, 100, "interrupt");
create_task(interrupt_service_request, DEFAULT_STACK_SIZE, strdup(buff));*/


void interrupt_service_request(void){


	return;
}
