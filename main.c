#include "gtthread.h"

/* Tests creation.
   Should print "Hello World!" */

void *start_routine(void *in)
{
  int i=0;
  for(i=0;i<10;i++){
    printf("Executing thread %d. Parameter is %c\n", gtthread_self(), *(char*)in);
    gtthread_yield();
  }

  gtthread_cancel((gtthread_self()+1)%5);
  fflush(stdout);
  return (void*)5;
}

int main() {
  int i=0, result =0;
  char params[6]="abcde";
  void* thread_return = NULL;
  gtthread_t t1, t2, t3, t4, t5;

  gtthread_init(100);
  printf("Main started. Executing %d\n", gtthread_self());
  gtthread_create( &t1, start_routine, &params[0]);
  gtthread_create( &t2, start_routine, &params[1]);
  gtthread_create( &t3, start_routine, &params[2]);
  gtthread_create( &t4, start_routine, &params[3]);
  gtthread_create( &t5, start_routine, &params[4]);

  for (i=0;i<10;i++){
    printf("Executing thread %d.\n", gtthread_self());
    gtthread_yield();
  }

  result = gtthread_join(t1, &thread_return);
  if (0 != result) {
    fprintf(stderr, "Failed to join consumer thread: %s\n", strerror(result));
    gtthread_exit(NULL);
  }
  printf("Thread 5 returned %d from join\n", (int)thread_return);

  gtthread_join(t2, NULL);
  gtthread_join(t3, NULL);
  gtthread_join(t4, NULL);
  gtthread_join(t5, NULL);
  printf("All joins completed\n");

  // gtthread_exit(0);
  return 0;
}
