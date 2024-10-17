#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <dispatch/dispatch.h> // Include dispatch for GCD
#include <inttypes.h>
#include "barrier.h" // Custom barrier implementation using pthreads

barrier_t barrier;
pthread_t threadL, threadM, threadH;
dispatch_semaphore_t sem; // Change to dispatch_semaphore_t

void *low_f();
void *med_f();
void *high_f();

void print_pri(pthread_t *thread, const char *s)
{
    struct sched_param param;
    int policy;
    pthread_getschedparam(*thread, &policy, &param);
    printf("b:%i ", param.sched_priority);
    printf("%s\n", s);
}

void busy_wait_ms(int ms)
{
    usleep(ms * 1000); // Simplified busy wait for demo purposes
}

void create_and_start_task(pthread_t *thread, void *function, int policy, int priority)
{
    pthread_attr_t tattr;
    struct sched_param param;

    // Initialize thread attributes
    pthread_attr_init(&tattr);
    pthread_attr_setschedpolicy(&tattr, policy);
    pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
    param.sched_priority = priority;
    pthread_attr_setschedparam(&tattr, &param);
    
    // Create the thread
    pthread_create(thread, &tattr, function, NULL);
}

int main()
{
    // Setting up barrier and semaphore
    int threadNum = 4;
    barrier_init(&barrier, threadNum);
    
    // Initialize semaphore with value 1
    sem = dispatch_semaphore_create(1); // Create GCD semaphore

    // Create tasks
    int policy = SCHED_RR;
    create_and_start_task(&threadL, low_f, policy, 1);
    create_and_start_task(&threadM, med_f, policy, 2);
    create_and_start_task(&threadH, high_f, policy, 3);

    // Sleep for 100 ms while other tasks are started
    usleep(100 * 1000);

    printf("-------------------Start test-------------------\n");
    barrier_wait(&barrier);
    barrier_destroy(&barrier);

    pthread_join(threadL, NULL);
    pthread_join(threadM, NULL);
    pthread_join(threadH, NULL);

    printf("-------------------End test-------------------\n");

    // Clean up GCD semaphore
    dispatch_release(sem); // Release semaphore

    printf("Finished\n");
    exit(EXIT_SUCCESS);
}

void *high_f()
{
    print_pri(&threadH, "H0: high priority waiting for sync\n");
    barrier_wait(&barrier);

    print_pri(&threadH, "H1: high usleep\n");
    usleep(200 * 1000);
    print_pri(&threadH, "H3: high priority thread waits lock\n");
    dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER); // Wait for semaphore
    print_pri(&threadH, "H4: high priority thread has lock\n");
    print_pri(&threadH, "H5: high priority thread runs with lock\n");
    busy_wait_ms(100);
    print_pri(&threadH, "H6: high priority thread runs with lock\n");
    busy_wait_ms(100);
    print_pri(&threadH, "H7: high priority thread return lock\n");
    dispatch_semaphore_signal(sem); // Signal semaphore

    return NULL;
}

void *med_f()
{
    print_pri(&threadM, "M0: med priority waiting for sync\n");
    barrier_wait(&barrier);

    print_pri(&threadM, "M1: med usleep\n");
    usleep(100 * 1000);
    print_pri(&threadM, "M2: med priority thread runs\n");
    busy_wait_ms(100);
    print_pri(&threadM, "M3: med priority thread runs\n");
    busy_wait_ms(100);
    print_pri(&threadM, "M4: med priority thread runs\n");
    busy_wait_ms(100);
    print_pri(&threadM, "M5: med priority thread runs\n");
    busy_wait_ms(100);
    print_pri(&threadM, "M6: med priority thread runs\n");
    busy_wait_ms(100);

    return NULL;
}

void *low_f()
{
    print_pri(&threadL, "L0: low priority waiting for sync\n");
    barrier_wait(&barrier);

    print_pri(&threadL, "L1: low priority thread waits lock\n");
    dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER); // Wait for semaphore
    print_pri(&threadL, "L2: low priority thread has lock\n");
    print_pri(&threadL, "L3: low priority thread runs with lock\n");
    busy_wait_ms(100);
    print_pri(&threadL, "L4: low priority thread runs with lock\n");
    busy_wait_ms(100);
    print_pri(&threadL, "L5: low priority thread runs with lock\n");
    busy_wait_ms(100);
    print_pri(&threadL, "L6: low priority thread return lock\n");
    dispatch_semaphore_signal(sem); // Signal semaphore

    return NULL;
}
