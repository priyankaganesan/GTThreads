/**********************************************************************
gtthread_sched.c.  

This file contains the implementation of the scheduling subset of the
gtthreads library.  A simple round-robin queue should be used.
 **********************************************************************/
/*
  Include as needed
*/

#include "gtthread.h"

/* 
   Students should define global variables and helper functions as
   they see fit.
 */
 
void block_signal()
{
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
}

void unblock_signal()
{
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
}

void set_alarm()
{
  /*
    Setting up the handler
  */
  memset (&act, '\0', sizeof(act));
  act.sa_handler = &alarm_handler;
  if (sigaction(SIGVTALRM, &act, NULL) < 0) {
    fprintf(stderr, "sigaction installation failed\n"); 
    perror ("sigaction");
  }

  /* 
    Setting up the alarm
  */
  if (p > 0) {
    T->it_value.tv_sec = T->it_interval.tv_sec = 0;
    T->it_value.tv_usec = T->it_interval.tv_usec = p;
    setitimer(ITIMER_VIRTUAL, T, NULL);
  }
  else{
    T = NULL;
  }
}

void alarm_handler(int sig)
{
  void* current = (node_t*)malloc(sizeof(node_t));
  void* next = (node_t*)malloc(sizeof(node_t));

  block_signal();
  current = steque_front(q);

  steque_cycle(q);
  while(1) {
    next = steque_front(q);
    if((((node_t*)next)->canceled) == 0){
      break;
    }
    steque_cycle(q);
  }

  set_alarm();  
  
  swapcontext (((node_t*)current)->thread_ctx, ((node_t*)next)->thread_ctx);
  unblock_signal();
}

void wrapper(void* (*start_routine)(void*), void *arg)
{
    void *retval;
    retval = (void *) start_routine(arg);  
    gtthread_exit(retval);
    return;
}

/*
  The gtthread_init() function does not have a corresponding pthread equivalent.
  It must be called from the main thread before any other GTThreads
  functions are called. It allows the caller to specify the scheduling
  period (quantum in micro second), and may also perform any other
  necessary initialization.  If period is zero, then thread switching should
  occur only on calls to gtthread_yield().

  Recall that the initial thread of the program (i.e. the one running
  main() ) is a thread like any other. It should have a
  gtthread_t that clients can retrieve by calling gtthread_self()
  from the initial thread, and they should be able to specify it as an
  argument to other GTThreads functions. The only difference in the
  initial thread is how it behaves when it executes a return
  instruction. You can find details on this difference in the man page
  for pthread_create.
 */
void gtthread_init(long period){
  gtthread_t main_id = tid++;

  q = (steque_t*)malloc(sizeof(steque_t));
  steque_init(q);

  void* main_item;
  main_item = (node_t*)malloc(sizeof(node_t));
  ((node_t*)main_item) -> thread_ctx =  (ucontext_t*)malloc(sizeof(ucontext_t));
  ((node_t*)main_item) -> id = main_id;
  getcontext(((node_t*)main_item) -> thread_ctx);
  //TODO: need to init context?

  steque_enqueue(q, main_item);

  /*
    Setting up the signal mask
  */
  sigemptyset(&vtalrm);
  sigaddset(&vtalrm, SIGVTALRM);
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);

  T = (struct itimerval*) malloc(sizeof(struct itimerval));
  p = period;

  set_alarm();
}


/*
  The gtthread_create() function mirrors the pthread_create() function,
  only default attributes are always assumed.
 */
int gtthread_create(gtthread_t *thread,
		    void *(*start_routine)(void *),
		    void *arg){
  //TODO init check
  void* new_item;
  *thread = tid++;

  new_item = (node_t*)malloc(sizeof(node_t));
  ((node_t*)new_item) -> id = *thread;
  ((node_t*)new_item) -> thread_ctx = (ucontext_t*)malloc(sizeof(ucontext_t));
  ((node_t*)new_item) -> canceled = 0;
  getcontext(((node_t*)new_item) -> thread_ctx);

  ((node_t*)new_item) -> thread_ctx -> uc_link = 0;
  ((node_t*)new_item) -> thread_ctx -> uc_stack.ss_sp = malloc(SIGSTKSZ);
  ((node_t*)new_item) -> thread_ctx -> uc_stack.ss_size = SIGSTKSZ;
  ((node_t*)new_item) -> thread_ctx -> uc_stack.ss_flags = 0;
  makecontext(((node_t*)new_item) -> thread_ctx, (void (*)(void)) wrapper, 2, start_routine, arg);

  steque_enqueue(q, new_item);

  return 0;
}

/*
  The gtthread_join() function is analogous to pthread_join.
  All gtthreads are joinable.
 */
//TODO If the thread calling pthread_join() is canceled, then the target thread shall not be detached.

int gtthread_join(gtthread_t thread, void **status){
  int i = 0;
  steque_node_t* current;
  //TODO: No need to block and unblock signals?
  if (gtthread_equal(thread, gtthread_self())){
    printf("cannot join same thread\n");
    return -1;
  }

  //Searching for node with gtthread_t thread
  current = q->front;
  for (i = 0; (i < q->N); i++) {
    if(gtthread_equal(thread, ((node_t*)current -> item) -> id)) {
      break;
    }
    current = current -> next;
  }

  //Checking for canceled thread, blocking otherwise
  while (1){
    if (((node_t*)current -> item) -> canceled == 1){
      if(status != NULL){
        *status = ((node_t*)current -> item) -> returns;
      }
      break;
    }
    gtthread_yield();
  }     

  return 0;
}

/*
  The gtthread_exit() function is analogous to pthread_exit.
 */
void gtthread_exit(void* retval)
{
  int flag = 0, i = 0;
  //Node to be deleted
  steque_node_t* temp = q->front;
  void* exit_item = steque_front(q);

  block_signal();
  //Setting the canceled flag and return value
  ((node_t*)exit_item) -> returns = retval;
  ((node_t*)exit_item) -> canceled = 1;

  //Check if this is the only executing thread. Exit.
  for (i = 0; (i < q->N); i++) {
  	if(((node_t*)temp -> item) -> canceled == 0){
  		flag = 1 ;
  		break;
  	}
  	temp = temp -> next;
  }
  if(flag == 0){
    steque_destroy(q);
    exit(0);
  }
  unblock_signal();

  raise(SIGVTALRM);
}


/*
  The gtthread_yield() function is analogous to pthread_yield, causing
  the calling thread to relinquish the cpu and place itself at the
  back of the schedule queue.
 */
void gtthread_yield(void){
  raise (SIGVTALRM);
}

/*
  The gtthread_yield() function is analogous to pthread_equal,
  returning zero if the threads are the same and non-zero otherwise.
 */
int  gtthread_equal(gtthread_t t1, gtthread_t t2){
  if (t1 == t2){
    return 1;
  }
  return 0;
}

/*
  The gtthread_cancel() function is analogous to pthread_cancel,
  allowing one thread to terminate another asynchronously.
 */
int  gtthread_cancel(gtthread_t thread){
  int i = 0;
  steque_node_t* current = q->front;
  block_signal();
  if (gtthread_equal(thread, gtthread_self())){
    gtthread_exit((void*)GTTHREAD_CANCELED);
  }
  for (i = 0; (i < q->N); i++) {
    if(gtthread_equal(thread, ((node_t*)current -> item) -> id)) {
      if (((node_t*)current -> item) -> canceled == 1){
        return ESRCH;
      }
      else{
        ((node_t*)current -> item) -> canceled = 1;
        ((node_t*)current -> item) -> returns = (void*)GTTHREAD_CANCELED;
        break;
      }      
    }
    current = current -> next;
  }
  unblock_signal();
  return 0;
}

/*
  Returns calling thread.
 */
gtthread_t gtthread_self(void){
  return ((node_t*)steque_front(q))->id;
}