#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <string.h>


long long elapsed_ns(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * (10*1000*1000) + (end.tv_nsec - start.tv_nsec);
}

int main() {
    struct timespec start, end;
    long long elapsed_time;

    int ns_max = 10000;
    int iterations = 10 * 1000 * 1000;
    int histogram[ns_max];
    memset(histogram, 0, sizeof(int) * ns_max);

    for (int i = 0; i < iterations; i++)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
        sched_yield();
        clock_gettime(CLOCK_MONOTONIC, &end);

        elapsed_time = elapsed_ns(start, end);

        if(elapsed_time >= 0 && elapsed_time < ns_max)
        {
            histogram[elapsed_time]++;
        }
    }

    for (int i = 0; i < ns_max; i++)
    {
        printf("%d\n", histogram[i]);
    }
    
    return 0;
}
