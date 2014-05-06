#include "proxy_threads.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
HANDLE threads[NUM_THREADS];
#else
#include <unistd.h>
pthread_t threads[NUM_THREADS];
#endif

int thread_args[NUM_THREADS];
int completed_threads[NUM_THREADS];
int thread_ids[NUM_THREADS];

int get_next_thread_id()
{
    for(;;)
    {
        for(int i=0; i<NUM_THREADS; i++)
        {
            if(completed_threads[i] == -1)
            {
                completed_threads[i] = 0;
                return i;
            }
            if(completed_threads[i] == 1)
            {
#ifdef _MSC_VER
                CloseHandle(threads[i]);
#else
                pthread_join(threads[i], NULL);
#endif
                completed_threads[i] = 0;
                return i;
            }
        }
        printf("!!!!!!!!! All threads are busy - cannot accept new connection!!!!!!!\n");
        sleep(10);
    }
}

void new_thread(void * routine, int thread_arg)
{
    int new_thread_id = get_next_thread_id();
    thread_args[new_thread_id] = thread_arg;

#ifdef _MSC_VER
    threads[new_thread_id] = CreateThread(NULL,
                                          0,
                                             (LPTHREAD_START_ROUTINE)routine,
                                             (void*)(thread_ids + new_thread_id),
                                          0,
                                          NULL);
#else
    int rc = pthread_create(&threads[new_thread_id], 
                            NULL, 
                            (void*(*)(void*))routine, 
                            (void*)(thread_ids+new_thread_id));
    if(rc)
    {
        printf("ERROR CREATING GAME THREAD: %d\n", rc);
        exit(1);
    }
#endif
}

void init_threads()
{
    for(int i=0; i<NUM_THREADS; i++)
    {
        completed_threads[i] = -1;
        thread_ids[i] = i;
    }
}
