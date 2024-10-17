#include "barrier.h"
#include <pthread.h>

// init barrier
void barrier_init(barrier_t *barrier, int count)
{
    barrier->trip_count = count;
    barrier->count = 0;
    pthread_mutex_init(&barrier->mutex, NULL);
    pthread_cond_init(&barrier->cond, NULL);
}

// wait at the barrier and designate on thread as the "serial thread"
int barrier_wait(barrier_t *barrier)
{
    pthread_mutex_lock(&barrier->mutex);
    barrier->count++;
    
    int is_serial_thread = 0;

    if (barrier->count >= barrier->trip_count)
    {
        barrier->count = 0;
        pthread_cond_broadcast(&barrier->cond);
        is_serial_thread = 1;
    }
    else
    {
        pthread_cond_wait(&barrier->cond, &barrier->mutex);
    }

    pthread_mutex_unlock(&barrier->mutex);
    return is_serial_thread;
}

// destroy barrier
void barrier_destroy(barrier_t *barrier)
{
    pthread_mutex_destroy(&barrier->mutex);
    pthread_cond_destroy(&barrier->cond);
}

// bs for debugging
int getThreadID()
{
    // convert thread to som kind of int that more easy can represent thread ID
    return (int)(uintptr_t)pthread_self();
}