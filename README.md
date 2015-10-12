# GTThreads
Advanced Operating Systems Assignment 1
--------------------------------------------------------------------------------------------
Name: Priyanka Ganesan
--------------------------------------------------------------------------------------------
Linux Platform: Ubuntu 14.04
--------------------------------------------------------------------------------------------
Compilation:

GTThreads:
In the top directory(GTThreads), type: 
make

This will give gtthread.a.
To compile a test case, copy it to the top directory and type:
gcc -Wall -pedantic -I. -o main main.c gtthread.a

Dining Philosopher's Problem:
In the top directory(Dining-Philosophers), type: 
make

To run the dining philosophers program type:
./dining_philosophers

--------------------------------------------------------------------------------------------
Preemptive scheduler implementation:

When a thread is created or swapped out, it is inserted at the end of the queue. When a context switch occurs, the thread at the start of the queue is cycled to the back by the round-robin scheduler.

I utilized the steque data structure interface provided. Each node in this data structure contains a void pointer. The thread specific data structure contains fields storing the gtthread_t ID, thread context, and other variables. These two structures are linked.

I've used setitimer() to generate SIGVTALRM signals at time intervals specified by the input to gtthread_init. I have specified the function alarm_handler() as the signal handler. When a signal comes in (preemption), the scheduler is called. The scheduler searches for the next executable thread. It then swaps the contexts of this thread and the current running thread using swapcontext(), which switches execution.

In the event the period is specified to be 0, the timer is not set up, so threads are context switched only on a call to gtthread_yield() or gttread_exit(). This is the case of non-preemption.

* When a thread calls gtthread_yield(), the scheduler is called.
* In the event of gtthread_exit(), the current running thread is terminated, and context is swapped to the next thread in line.
* When gtthread_cancel() is called, the specific thread is canceled, and execution resumes as normal.
* For gtthread_join(), the calling thread waits for the called thread to finish executing, and then returns.

--------------------------------------------------------------------------------------------
Dining Philosophers Solution:

To implement the dining philosophers problem, we use one mutex for each chopstick. The philosopher's eat and think for random amounts of time (implemented by generating a random integer and then looping that many times).

There are 5 philosophers (Philosopher #1 to #5) and 5 chopsticks (chopstick #1 to #5). Chopstick 1 is on the left of Philosopher 1. So he must pick up chopstick 1 and 2 to eat. Similarly for philosophers 2-4. Philosopher 5 must pick up chopstick 5 and 1 to eat.

To prevent deadlocks, I have implemented the resource hierarchy solution. Each philosopher must first pick up the chopstick with the lower number. For all except the last philosopher, this will be the left chopstick. Only the last philosopher will pick up his right chopstick first (his left chopstick is #5 while the right one is #1). This ensures that even if the first 4 philosophers pick up their left chopsticks simultaneously, there will be one chopstick (#5) left on the table since this is the highest numbered chopstick. Therefore it can be picked up by the fourth philosopher who can eat and then release both his chopsticks which can be picked up by the next lower numbered philosopher.

--------------------------------------------------------------------------------------------
Thoughts:

This project was a great learning experience to system level programming. It helped expose us to user level threads, schedulers, and preemption. Kicking off the project required quite some background reading on the functioning of signals and timers, and a lot of reading of man pages of the different pthread functions. Trying to emulate these functions in code was a lot of fun, as was the mutex implementation.

I believe I used the steque data structure implementation quite well given the methods available. I also believe the quality of my code is quite good, as I did not run into any segmentation faults when I started testing it. My code does not handle multiple joins on the same thread.

--------------------------------------------------------------------------------------------

