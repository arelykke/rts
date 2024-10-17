#include <pthread.h>
#include <stdio.h>
#include "barrier.h" // custom barrier implementation using pthreads
long global_i = 0;
int iterations = 1000*1000;
const int thread_count = 200;
pthread_mutex_t lock; 
barrier_t barrier;
pthread_t threads[thread_count];

int getThreadID(pthread_t thread)
{
    // convert thread to som kind of int that more easy can represent thread ID
    return (int)(uintptr_t)thread;
}



void* inc(void* arg){
    int local_i = 0;

    barrier_t *barrier = (barrier_t*) arg;
    printf("Thread %d waiting at the barrier...\n", getThreadID(pthread_self()));

    // wait at the barrier and if this thread is the serial thread
    if (barrier_wait(barrier))
    {
        printf("Thread %d is the serial thread!\n", getThreadID(pthread_self()));
    } 

    printf("Thread %d passed the barrier\n", getThreadID(pthread_self()));

    printf("Thread %d started iterating..\n", getThreadID(pthread_self()));

    for (int i = 0; i < iterations; i++)
    {
        local_i++;

        pthread_mutex_lock(&lock);
        global_i++;
        pthread_mutex_unlock(&lock);
    }
    pthread_mutex_lock(&lock);
    printf("Local i = %d    |   Global i = %ld \n", local_i, global_i);
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(){



    // init barrier and &lock
    barrier_init(&barrier, thread_count);
    pthread_mutex_init(&lock, NULL);

    // create threads
    for (int t = 0; t < thread_count; t++)
    {
        pthread_create(&threads[t], NULL, inc, &barrier);
    }

    // wait for all threads to complete
    for (int t = 0; t < thread_count; t++)
    {
        pthread_join(threads[t], NULL);
    }

    // destroy barrier and &lock
    barrier_destroy(&barrier);
    pthread_mutex_destroy(&lock);

    return 0;
}


