./<program> <method> 

1: rdtsc
2: clock_gettime()
3: time()

./<program> <method> | gnuplot -p -e "plot '<cat' with boxes"


to test latency, uncomment printf() statements