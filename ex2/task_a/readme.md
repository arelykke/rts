a)
Running this program on a Raspberry Pi with 4GB of RAM will likely lead to memory allocation failures due to insufficient memory, and you may observe limited changes in overall memory usage unless the system attempts to handle the allocation through swap.

b)
the difference between memory and swap:
1. speed -> memory(RAM) is much faster than swap
2. volatility -> memory is volatile, while swap is not
3. memory is used for active processes, while swap is used
    when memory is full to allow more processes to run

In essence, while both memory and swap are crucial for a system's performance, RAM is the primary working area for active data, whereas swap serves as a backup to extend available memory at the cost of speed.

c)
changing x_dim from 1000 to 10000 only changes the shape of the matrix, but the allocated memory remains the same

d)
malloc() doesnt access the memory right away, rather its allocated once the program accesses the memory for the first time.
when memset() is called, the program accesses the memory?

