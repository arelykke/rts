#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdlib.h>


// =========== USING RDTC ================

// uncomment to use on RPi (ARM64)
/*
uint64_t rdtsc(void){
    uint64_t val;
    asm volatile("isb; mrs %0, cntvct_el0; isb; " : "=r"(val) :: "memory"); 
    // You can check the current CPU frequency with $sudo dmesg | grep MHz
    return val;
}
*/

// use is compiled on x86 CPU
uint64_t rdtsc(void){
    unsigned int lo, hi;
    asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
    return((uint64_t)hi << 32) | lo;
}

void measure_latency_rdtsc(int iterations){
    uint64_t start, end;
    clock_t total_time = clock();
    for (int i = 0; i < iterations; i++){
        rdtsc(); // read timer
    }

    total_time = clock() - total_time;
    //printf("RDTSC latency: %f ns per read\n", (double)total_time / CLOCKS_PER_SEC / iterations * 1e9);
}

void measure_resolution_rdtsc(int iterations, int ns_max){
    int histogram[ns_max];
    memset(histogram, 0, sizeof(int) * ns_max);

    uint64_t prev_time = rdtsc();
    for (int i = 1; i < iterations; i++){
        uint64_t current_time = rdtsc();
        int ticks = current_time - prev_time;
        prev_time = current_time;

        // Convert ticks to nanoseconds using the calculated factor
        int ns = ticks * 0.4348;  // Convert ticks to nanoseconds
        if (ns >= 0 && ns < ns_max){
            histogram[ns]++;
        }
    }

    // Print histogram data
    for(int i = 0; i < ns_max; i++){
        printf("%d\n", histogram[i]);
    }
}


//============== USING clock_gettime() ================

void measure_latency_clock_gettime(int iterations) {
    struct timespec ts;
    clock_t start_time = clock();

    for (int i = 0; i < iterations; i++){
        clock_gettime(CLOCK_MONOTONIC, &ts); //read timer
    }

    clock_t total_time = clock() - start_time;
    //printf("clock_gettime() latency: %f ns per read\n", (double)total_time / CLOCKS_PER_SEC / iterations * 1e9);
}

void measure_resolution_clock_gettime(int iterations, int ns_max){
    int histogram[ns_max];
    memset(histogram, 0, sizeof(int) * ns_max);

    struct timespec t1, t2;
    for (int i = 0; i < iterations; i++){
        clock_gettime(CLOCK_MONOTONIC, &t1);
        clock_gettime(CLOCK_MONOTONIC, &t2);

        long ns = (t2.tv_sec - t1.tv_sec) * 1e9 + (t2.tv_nsec - t1.tv_nsec);
        if(ns >= 0 && ns < ns_max){
            histogram[ns]++;
        }
    }

    for(int i = 0; i < ns_max; i++){
        printf("%d\n", histogram[i]);
    }
}

// ================== USING times() ===============
void measure_latency_times(int iterations){
    struct tms ts;
    clock_t total_time = clock();

    for (int i = 0; i < iterations; i++){
        times(&ts); // read timer
    }

    total_time = clock() - total_time;
    //printf("times() latency: %f ns per read\n", (double)total_time / CLOCKS_PER_SEC / iterations * 1e9);
}

void measure_resolution_times(int iterations, int ns_max){
    int histogram[ns_max];
    memset(histogram, 0, sizeof(int) * ns_max);

    struct tms t1, t2;
    long ticks_per_sec = sysconf(_SC_CLK_TCK);
    for (int i = 0; i < iterations; i++){
        clock_t t_start = times(&t1);
        clock_t t_end = times(&t2);

        t_start = t_start*1e9/ticks_per_sec;
        t_end = t_start*1e9/ticks_per_sec;

        int ns = (t_end - t_start);
        if(ns >= 0 && ns < ns_max){
            histogram[ns]++;
        }
    }

    for(int i = 0; i < ns_max; i++){
        printf("%d\n", histogram[i]);
    }
}


int main(int argc, char *argv[]){

    int iterations = 10 * 1000 * 1000;
    int ns_max = 50;

    int choice = atoi(argv[1]);

    if (choice == 1) {
        //printf("Measuring RDTSC latency and resolution...\n");
        measure_latency_rdtsc(iterations);
        measure_resolution_rdtsc(iterations, ns_max);
    }
    else if(choice == 2) {
        //printf("Measuring clock_gettime() latency and resolution...\n");
        measure_latency_clock_gettime(iterations);
        measure_resolution_clock_gettime(iterations, ns_max);
    }
    else if (choice == 3) {
        //printf("Measuring times() latency and resolution...\n");
        measure_latency_times(iterations);
        measure_resolution_times(iterations, ns_max);
    }
    return 0;
}