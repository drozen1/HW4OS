//
// Created by student on 1/16/21.
//
#include <cstring>
#include <cmath>
#include "stddef.h"
#include <iostream>
#include <unistd.h>


void* first_sbrk = NULL;
void* first_mmap = NULL;

struct MallocMetadata{
    size_t size;
    bool is_free;
    bool is_mmap;
    MallocMetadata* next;
    MallocMetadata* prev;
};

void cutblock(void* pos,size_t size){
    size_t left_size = ((MallocMetadata*)pos)->size - size-size(MallocMetadata);
    ((MallocMetadata*)pos)->size= size;
    MallocMetadata* new_block = (MallocMetadata*)(pos+size(MallocMetadata)+size);
    new_block->size = left_size;
    new_block->is_free = false;
    new_block->is_mmap = false;
    new_block->prev =(MallocMetadata*)pos;
    new_block->next= ((MallocMetadata*)pos)->next;
    ((MallocMetadata*)pos)->next = new_block;
}
MallocMetadata* wildernessChunkIsFree(MallocMetadata* last,size_t size){
    size_t sbrk_size = size- last->size;
    void *ret = sbrk(sbrk_size);
    if (ret == (void *) -1) {
    return NULL;
    }
    last->is_free = false;
    last->size = size;
    assert(last->next == nullptr);
    return last;
}

void* smalloc(size_t size){
    if( size==0 || size> (pow(10,8))){
        return NULL;
    }
    if(size>128 * 1024){
        mmap_hanler(size); //TODO
    }
    MallocMetadata* pos=(MallocMetadata *)first;
    MallocMetadata* last=NULL;
    while(pos!=NULL){
        if(pos->is_free && pos->size>=size){
            if(pos->size-size-size(MallocMetadata)>=128) {
                pos->is_free = false;
                cutblock(pos, size); //TODO
                return (pos + 1);
            }
            else{
                pos->is_free = false;
                return (pos + 1);
            }
        }
        last=pos;
        pos=pos->next;
    }
    if(last!= nullptr && last->is_free){
        return 1+wildernessChunkIsFree(last,size); //TODO:
//        sbrk_size-=(last->size+sizeof(MallocMetadata));
    }
    else {
        void *ret = sbrk(size + sizeof(MallocMetadata));
        if (ret == (void *) -1) {
            return NULL;
        }
        MallocMetadata* new_block= (MallocMetadata*)ret;
        new_block->size=size;
        new_block->is_free=false;
        new_block->next=NULL;
        if(first==NULL){
            new_block->prev=NULL;
            first=(void*)new_block;
            return (new_block+1);
        }
        last->next = new_block;
        new_block->prev=last;
        return (new_block+1);
    }

}

void* scalloc(size_t num, size_t size){
    if( size==0 || size> (pow(10,8))){
        return NULL;
    }
    void * ret_block = smalloc(size*num);
    if(ret_block == NULL) return NULL;
    return memset(ret_block, 0, size*num);

}

MallocMetadata* merge_cells(MallocMetadata* last,MallocMetadata* next, bool is_pos_first){
    assert(last != nullptr && next != nullptr);
    if(last->is_free && next->is_free){
        last->size = last->size +next->size;
        last->next = next->next;
        return last;
    } else if(is_pos_first){
        return last;
    } else{
        return next;
    }
}

void sfree(void* p){
    if(p!=NULL){
        MallocMetadata* pos = ((MallocMetadata*)((unsigned long)p - sizeof(MallocMetadata)));
        if(pos->is_mmap){
            if(first_mmap==pos){ //check if first
                first_mmap=pos->next;
                if(pos->next!= nullptr){
                    pos->next->prev= nullptr;
                }
            }else{ //not first for sure
                pos->prev->next=pos->next;
                if(pos->next!= nullptr){
                    pos->next->prev=pos->prev;
                }
            }
            munmap((void*)(pos), pos->size + META_DATA_SIZE);
        }else{
            if(!pos->is_free){
                pos->is_free= true;
            }
            if(pos->prev!= nullptr){
                pos=merge_cells(pos->prev,pos, false); //TODO: need to check if its free, return the meta date of the block
            }
            if(pos->next!= nullptr){
                pos=merge_cells(pos,pos->next, true); //TODO:
            }
            assert(pos->is_free);
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
        if (pos->is_mmap) { ///mmap case:
            if(pos->size==size){ //TODO: check Piazza case https://piazza.com/class/kg7wfbyqnoc73t?cid=344
                return oldp;
            }
            void* ret= smalloc(size);
            if(ret== nullptr){
                return nullptr;
            }
            memmove(ret, oldp, size);
            sfree(oldp);  // will use munmap
            return ret;
        }else{ ///case A
            if(pos->size>=size){
                if(pos->size-size-size(MallocMetadata)>=128) { //can split it to two
                    cutblock(pos, size); //TODO
                }
                pos->is_free = false;
                return (pos + 1);
            }else{ ///case B
                if(caseB(pos,size)!= nullptr)
                void* ret=caseB(pos,size); //TODO
                if(ret!= nullptr){
                    return ret;
                }else {//caseC
                    void* ret=caseC(pos,size); //TODO
                    if(ret!= nullptr) {
                        return ret;
                    }else{ //caseD
                        void* ret=caseD(pos,size); //TODO
                        if(ret!= nullptr) {
                            return ret;
                        }
                    }
                }
            }
        }
        ///Case E:
        ((MallocMetadata*)((unsigned long)oldp - META_DATA_SIZE))->is_free = true;
        ret = smalloc(size);
        if(ret== nullptr){
            return nullptr;
        }
        ((MallocMetadata*)((unsigned long)oldp - META_DATA_SIZE))->is_free = false;
        memmove(ret, oldp, size);
        if(ret!=oldp){
            sfree(oldp);
        }
    }
    ///oldp is NULL case
    void* newp= smalloc(size);
    if(newp==nullptr){
        return nullptr;
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