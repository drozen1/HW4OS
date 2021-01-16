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
    if( size==0 || size> (1e8)){
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



