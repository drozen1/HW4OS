#include <unistd.h>
#include "stddef.h"

void* smalloc(size_t size){
    if( size==0 || size> (1e8)){
        return NULL;
    }
    void* ret= sbrk(size);
    if (ret== (void*)-1){
        return NULL;
    }
    return ret;
}
