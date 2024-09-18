#include <sys/times.h>
#include <stdio.h>


void busy_wait(clock_t ticks){
    struct tms now;
    clock_t start = times(&now);

    while((times(&now) - start) < ticks){
        for(int i = 0; i < 10000; i++){}
    }

}

int main(){
    printf("Waiting for 3 sec using busytime()...\n");
    busy_wait(300);
    return 0;
}