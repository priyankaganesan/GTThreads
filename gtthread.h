#ifndef GTTHREAD_H
#define GTTHREAD_H
#define GTTHREAD_CANCELED 1

#include "steque.h"
#include <ucontext.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Define gtthread_t and gtthread_mutex_t types here */
typedef int gtthread_t;

typedef struct node_t
{
	int id;
	ucontext_t* thread_ctx;
	void* returns;
	int canceled;
}node_t;

typedef struct gtthread_mutex_t{
		int id;
		int set;
		int init;
}gtthread_mutex_t;

/*Global variables defined*/
struct itimerval *T;
struct sigaction act;
sigset_t vtalrm;

steque_t *q;
int tid;
long p;

void gtthread_init(long period);
int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg);
int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);


int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);
int  gtthread_mutex_destroy(gtthread_mutex_t *mutex);

/*Helper functions*/
void block_signal(void);
void unblock_signal(void);
void set_alarm(void);
void alarm_handler(int sig);
void wrapper(void* (*start_routine)(void*), void *arg);
void *start_routine(void *in);
#endif
