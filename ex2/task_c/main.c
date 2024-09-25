#include "array.h"

int main(){
    Array arr = array_new(5);

    for(int i = 0; i < 15; i++){
        array_insertBack(&arr, i);
        array_print(arr);
    }

    array_destroy(arr);
    return 0;
}