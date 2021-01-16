//
// Created by student on 1/16/21.
//
#include <unistd.h>
#include "stddef.h"


void* first = NULL;

struct MallocMetadata{
        size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
    size_t used_bytes;
};

void* smalloc(size_t size){
    if( size==0 || size> (pow(10,8))){
        return NULL;
    }
    MallocMetadata* pos=first;
    MallocMetadata* last=NULL;
    while(pos!=NULL){
        if(pos->is_free && pos->size>=size){
            pos->is_free= false;
            pos->used_bytes=size;
            return (pos+1);
        }
        last=pos;
        pos=pos->next;
    }
    void* ret= sbrk(size+sizeof(MallocMetadata));
    if (ret== (void*)-1){
        return NULL;
    }
    MallocMetadata* new_block= (MallocMetadata*)ret;
    new_block->size=size;
    new_block->used_bytes=size;
    new_block->is_free=false;
    new_block->next=NULL;
    if(first==NULL){
        new_block->prev=NULL;
        first=(void*)new_block;
        return (new_block+1);
    }
    last->next=new_block;
    new_block->prev=last;
    return (new_block+1);
}

void* scalloc(size_t num, size_t size){
    void * ret_block = smalloc(size*num);
    if(ret_block == NULL) return NULL;
    memset(ret_block, 0, size);
    return ret_block;
}

void sfree(void* p){
    if(p!=NULL){
        MallocMetadata* pos=p;
        pos=pos-1;
        if(!pos->is_free){
            pos->is_free= false;
            pos->used_bytes=0;
        }
    }
}

void* srealloc(void* oldp, size_t size){

}