#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wiringPi.h>

#define PININ1  8
#define PINOUT1 9
#define PININ2  7
#define PINOUT2 15
#define PININ3  16
#define PINOUT3 1

#define TEST_COUNT        1000
#define TIMEOUT_MS        65
#define DISTURBANCE_COUNT 10

void io_init(void)
{
    wiringPiSetup();

    pinMode(PININ1, INPUT);
    pinMode(PINOUT1, OUTPUT);
    pinMode(PININ2, INPUT);
    pinMode(PINOUT2, OUTPUT);
    pinMode(PININ3, INPUT);
    pinMode(PINOUT3, OUTPUT);

    digitalWrite(PINOUT1, HIGH);
    digitalWrite(PINOUT2, HIGH);
    digitalWrite(PINOUT3, HIGH);
}

int set_cpu(int cpu_id)
{
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_id < 0 || cpu_id >= num_cores)
    {
        return EINVAL;
    }

    cpu_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(cpu_id, &cpu);
    return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}

void *test_thread(void *args)
{
    int pinIn  = *(int *)args;
    int pinOut = (pinIn == PININ1) ? PINOUT1 : (pinIn == PININ2 ? PINOUT2 : PINOUT3);

    struct timespec start, end;
    double          response_times[TEST_COUNT];

    for (int i = 0; i < TEST_COUNT; i++)
    {
        // wait for input signal (busy-wait)
        while (digitalRead(pinIn) != HIGH)
        {
        };

        // get start time
        clock_gettime(CLOCK_MONOTONIC, &start);

        // send response signal
        digitalWrite(pinOut, LOW);
        delay(1);
        digitalWrite(pinOut, HIGH);

        // get end time
        clock_gettime(CLOCK_MONOTONIC, &end);

        response_times[i] = (end.tv_sec - start.tv_sec) * 1e3 + (end.tv_nsec - start.tv_nsec) / 1e6;

        // check for timeout
        if (response_times[i] > TIMEOUT_MS)
        {
            printf('Overflow in subtest %d, response time: %f ms\n', i, response_times[i]);
        }
    }

    return NULL;
}

void *disturbance_thread(void *args)
{
    while (1)
    {
        asm volatile("" ::: "memory");
    }
    return NULL;
}

int main(void)
{
    io_init();

    pthread_t threadA, threadB, threadC;
    int       pinInA = PININ1, pinInB = PININ2, pinInC = PININ3;

    int cpu_id = 0;

    pthread_create(&threadA, NULL, test_thread, &pinInA);
    set_cpu(cpu_id);

    pthread_create(&threadB, NULL, test_thread, &pinInB);
    set_cpu(cpu_id);

    pthread_create(&threadC, NULL, test_thread, &pinInC);
    set_cpu(cpu_id);

    pthread_t disturbance_threads[DISTURBANCE_COUNT];
    for (int i = 0; i < DISTURBANCE_COUNT; i++)
    {
        pthread_create(&disturbance_threads[i], NULL, disturbance_thread, NULL);
        set_cpu(cpu_id);
    }

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);
    pthread_join(threadC, NULL);

    for (int i = 0; i < DISTURBANCE_COUNT; i++)
    {
        pthread_join(disturbance_threads[i], NULL);
    }

    return 0;
}