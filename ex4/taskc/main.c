#include <pthread.h>
#include <stdio.h>
#include "barrier.h"
#include <dispatch/dispatch.h>

int iterations = 1000 * 1000;
long global_i = 0;

const int threadCount = 2;
barrier_t barr;
pthread_t threads[threadCount];
dispatch_semaphore_t semaphore;

void* inc(void* arg)
{
    barrier_t *barrier = (barrier_t*) arg;

    int local_i = 0;

    printf("Thread %d waiting at barrier\n", getThreadID());
    barrier_wait(barrier);

    printf("Thread %d passed the barrier, start counting...\n", getThreadID());

    for (int i = 0; i < iterations; i++)
    {
        local_i++;

        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
        global_i++;
        dispatch_semaphore_signal(semaphore);
    }

    printf("Thread %d done counting.\n", getThreadID());
    printf("Global: %ld  |   Local: %d\n", global_i, local_i);
    return NULL;
}

int main()
{
    // init barrier
    barrier_init(&barr, threadCount);

    //init semaphore
    semaphore = dispatch_semaphore_create(1);

    // create threads
    for (int t = 0; t < threadCount; t++)
    {
        pthread_create(&threads[t], NULL, inc, &barr);
    }

    // join threads
    for (int t = 0; t < threadCount; t++)
    {
        pthread_join(threads[t], NULL);
    }

    barrier_destroy(&barr);
    dispatch_release(semaphore);

    return 0;
}