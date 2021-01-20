//
// Created by student on 1/16/21.
//
#include <cstring>
#include <cmath>
#include "stddef.h"
#include <iostream>
#include <unistd.h>


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
    MallocMetadata* pos=(MallocMetadata *)first;
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
    if( size==0 || size> (pow(10,8))){
        return NULL;
    }
    void * ret_block = smalloc(size*num);
    if(ret_block == NULL) return NULL;
    return memset(ret_block, 0, size*num);

}

void sfree(void* p){
    if(p!=NULL){
        MallocMetadata* pos = ((MallocMetadata*)((unsigned long)p - sizeof(MallocMetadata)));
        if(!pos->is_free){
            pos->is_free= true;
            pos->used_bytes=0;
        }
    }
}

void* srealloc(void* oldp, size_t size){
    if( size==0 || size> (pow(10,8))){
        return nullptr;
    }
    if(oldp!= nullptr) {
        MallocMetadata* pos=(MallocMetadata *)oldp;
        pos = pos - 1;
        if (pos->size >= size) {
            pos->used_bytes = size; //not sure
            return oldp;
        }
    }
    void* newp= smalloc(size);
    if(newp==nullptr){
        return nullptr;
    }
    if(oldp!=nullptr) {
        memmove(newp, oldp, size);
        sfree(oldp);
    }
    return newp;
}

size_t _num_free_blocks(){
    if( first== nullptr){
        return 0;
    }
    MallocMetadata* pos= (MallocMetadata *)first;
    size_t counter=0;
    while(pos!= nullptr){
        if(pos->is_free){
            counter++;
        }
        pos=pos->next;
    }
    return counter;
}

size_t _num_free_bytes(){
    if( first== nullptr){
        return 0;
    }
    MallocMetadata* pos= (MallocMetadata *)first;
    size_t counter=0;
    while(pos!= nullptr){
        if(pos->is_free){
            counter+= pos->size; ///maybe not used size of not free?
        }
        pos=pos->next;
    }
    return counter;
}

size_t _num_allocated_blocks(){
    if( first== nullptr){
        return 0;
    }
    MallocMetadata* pos= (MallocMetadata *)first;
    size_t counter=0;
    while(pos!= nullptr){
        counter+=1;
        pos=pos->next;
    }
    return counter;
}

size_t _num_allocated_bytes(){
    if( first== nullptr){
        return 0;
    }
    MallocMetadata* pos= (MallocMetadata *)first;
    size_t counter=0;
    while(pos!= nullptr){
        counter+=pos->size;
        pos=pos->next;
    }
    return counter;
}
size_t _num_meta_data_bytes(){
    return _num_allocated_blocks()*sizeof(MallocMetadata);
}


size_t _size_meta_data(){
    return sizeof (MallocMetadata);
}