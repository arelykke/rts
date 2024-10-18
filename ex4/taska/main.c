#include <pthread.h>
#include <stdio.h>
#include "barrier.h"

// mutexes or semaphores are not used, in order to demonstrate issues with threading


int iterations = 1e6;
long global_i = 0;

const int threadCount = 2;
barrier_t barr;
pthread_t threads[threadCount];

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
        global_i++;
    }

    printf("Thread %d done counting.\n", getThreadID());
    printf("Global: %ld  |   Local: %d\n", global_i, local_i);
    return NULL;
}

int main()
{
    // init barrier
    barrier_init(&barr, threadCount);

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

    return 0;
}