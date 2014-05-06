#ifndef PROXY_THREADS_H
#define PROXY_THREADS_H

#define NUM_THREADS 100

#ifdef _MSC_VER
#include "windows.h"
extern HANDLE threads[];
#ifndef sleep
#define sleep(t) Sleep(1000*t)
#endif
#else
#include <pthread.h>
extern pthread_t threads[];
#endif

extern int thread_args[];
extern int completed_threads[];

//I could have accomplished the same thing with pointer math, but I'd rather burn a bit of memory and 
//not be ridiculous:
extern int thread_ids[];

int get_next_thread_id();
void new_thread(void * routine, int thread_arg);

//Practically the last line of code I write has me thinking I should go to a class mechanism.  =]
void init_threads();

#endif
