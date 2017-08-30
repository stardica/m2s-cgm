
#ifndef __TASKING_H__
#define __TASKING_H__

#include <setjmp.h>

#include <cgm/contexts.h>
//#include "mytime.h"

#define TASKING_MAGIC_NUMBER 0x5a5a5a5a

#ifdef TIME_64
typedef unsigned long long Time_t;
#else
typedef long long Time_t;
#endif

typedef Time_t count_t;

/* 
 * The first three fields of task and event count must be the same
 *  so that the wakeupcall package can work.
 */


long long temp_time;

typedef struct task_s {
	struct task_s *tasklist;	/* pointer to next task on the same list */
	char *name;			/* task name */
	count_t count;		/* argument to await */
  
	void *cdata2;		/* argument (don't ask why)! */
	void *arg2;			/* arg to the function! */
	void (*f)(void *);	/* startup function if necessary */

	int id;				/* task id */
	unsigned magic;		/* stack overflow check */
	context_t c;		/* context switch info */
}task;

extern task *wakeup_task;

typedef volatile struct eventcount_s eventcount;

struct eventcount_s {
	task * tasklist;	/* list of tasks waiting on this event */
	char * name;		/* string name of eventcount */
	count_t count;		/* current value of event */
	volatile struct eventcount_s* eclist; /* pointer to next eventcount */
};



extern eventcount etime;	/* time counter */
typedef volatile count_t ticket_t;




void epause(count_t);			/* wait argument time units */
void await(eventcount *, count_t);    	/* wait for event >= arg */
count_t ticket(ticket_t *);	 	        /* atomically inc counter */
void advance(eventcount *);			/* increment eventcount */
void future_advance(eventcount *, count_t);
void initialize_event_count(eventcount *, count_t, char *);
eventcount *new_eventcount (char *name);
void delete_event_count(eventcount *);
task * create_task(void (*)(void), unsigned, char *);
task * create_task2(void (*)(void *), void *, unsigned, char *);
void remove_last_task (task *);
void set_id(unsigned id);
unsigned get_id(void);
char * get_task_name (void);	/* get current task name */
void initialize_wakeupcall (task*);
/*void wakeupcall (void *arg);*/
void wakeupcall (void);
void end_wakeupcall(long long final_time);
void tasking_dump(void);
count_t get_time (void);
void set_id(unsigned id);

/* return 1 if await would block, 0 otherwise */
#define await_check(ec,cnt) ((ec)->count < (cnt))

/* if you need to wait, call await_wait */
#define await_wait(ec,cnt)  switch_context(ec,cnt)

void simulate (void (*cleanup)(void));
void task_write (FILE *fp, task *t, int ctxt); /* ctxt = 1 if save ctxt */
void task_read  (FILE *fp, task *t, int ctxt); /* 0 otherwise */
void count_write (FILE *fp, count_t c);
void count_read (FILE *fp, count_t *c);

extern task *curtask;

#endif /* __TASKING_H__ */
