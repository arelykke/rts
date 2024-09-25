#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>



long long elapsed_ns(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * (10*1000*1000) + (end.tv_nsec - start.tv_nsec);
}

int main() {
    struct timespec start, end;
    long long elapsed_time;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    sched_yield();
    clock_gettime(CLOCK_MONOTONIC, &end);  
    elapsed_time = elapsed_ns(start, end);
    
    printf("Time taken for context switch: %lld nanoseconds\n", elapsed_time);
    
    return 0;
}
