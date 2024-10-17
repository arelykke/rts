#ifndef BARRIER_H
#define BARRIER_H

#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int trip_count;
} barrier_t;

void barrier_init(barrier_t *barrier, int count);
int barrier_wait(barrier_t *barrier);
void barrier_destroy(barrier_t *barrier);
int getThreadID();
#endif