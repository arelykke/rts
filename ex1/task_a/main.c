#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/times.h>
#include <stdlib.h>


void busy_wait_clock(struct timespec t){
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    struct timespec then = {now.tv_sec + t.tv_sec, now.tv_nsec + t.tv_nsec};


    while((now.tv_sec < then.tv_sec) || (now.tv_sec == then.tv_sec && now.tv_nsec < then.tv_nsec)){
        for(int i = 0; i < 10000; i++){}
        clock_gettime(CLOCK_MONOTONIC, &now);
    }
}


void busy_wait_times(clock_t ticks){
    struct tms now;
    clock_t start = times(&now);

    while(times(&now) - start < ticks){
        for(int i = 0; i < 10000; i++){}
    }
}

int main(int argc, char *argv[]){
    if (argc < 2) {
        printf("Usage: %s <method>\n", argv[0]);
        printf("1 = clock_gettime()\n");
        printf("2 = times()\n");
        printf("3 = sleep()\n");
        return 1;
    }

    int choice = atoi(argv[1]);  // Convert argument to integer

    if (choice == 1) {
        printf("Waiting for 1 second using clock_gettime()\n");
        struct timespec t = {1, 0};
        busy_wait_clock(t);
    } else if (choice == 2) {
        printf("Waiting for 1 second using times()\n");
        clock_t ticks = sysconf(_SC_CLK_TCK);  // Get clock ticks per second
        busy_wait_times(ticks);
    } else if (choice == 3) {
        printf("Waiting for 1 second using sleep()\n");
        sleep(1);
    } else {
        printf("Invalid choice. Please enter 1, 2, or 3.\n");
    }

    return 0;
}