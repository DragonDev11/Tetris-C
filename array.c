#include "array.h"

int array_duplicate(void* array, int array_size, int array_element_size, void* copy){
    if (!array || !copy) return 0;

    unsigned char* source = (unsigned char*)array;
    unsigned char* destination = (unsigned char*)copy;

    for (int i=0; i<array_size*array_element_size; i++){
        destination[i] = source[i];
    }

    return 1;
}

