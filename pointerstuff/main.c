#include <stdio.h>

int main()
{
    int tall = 10;
    int *pTall = &tall;

    printf("Verdien til tall: %d\n", tall);
    printf("Adressen til tall: %p\n", pTall);
    return 0;
}