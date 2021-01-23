//
// Created by student on 1/16/21.
//
#include <cstring>
#include <cmath>
#include "stddef.h"
#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <tgmath.h>
#include <sys/mman.h>


void* first_sbrk = NULL;
void* first_mmap = NULL;

struct MallocMetadata{
    size_t size;
    bool is_free;
    bool is_mmap;
    MallocMetadata* next;
    MallocMetadata* prev;
};

MallocMetadata* merge_cells(MallocMetadata* last,MallocMetadata* next, bool is_pos_first){
    assert(last != nullptr && next != nullptr);
    if(last->is_free && next->is_free){
        last->size = last->size +next->size+sizeof (MallocMetadata);
        last->next = next->next;
        if(next->next!= nullptr){
            next->next->prev=last;
        }
        return last;
    } else if(is_pos_first){
        return last;
    } else{
        return next;
    }
}

void cutblock(void* pos,size_t size){
    size_t left_size = ((MallocMetadata*)pos)->size - size-sizeof(MallocMetadata);
    ((MallocMetadata*)pos)->size= size;
    MallocMetadata* new_block = (MallocMetadata*)((unsigned long)pos +sizeof(MallocMetadata)+size);
    new_block->size = left_size;
    new_block->is_free = true;
    new_block->is_mmap = false;
    new_block->prev =(MallocMetadata*)pos;
    new_block->next= ((MallocMetadata*)pos)->next;
    ((MallocMetadata*)pos)->next = new_block;
    if(new_block->next!= nullptr){
        new_block->next->prev=new_block;
    }
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

void* mmap_hanler(size_t size){
    void* ret_ptr = mmap(NULL, size + sizeof(MallocMetadata), PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ret_ptr == (void*)(-1)) {
        return nullptr;
    }
    MallocMetadata* pos=(MallocMetadata *)ret_ptr;
    pos->is_free= false;
    pos->size=size;
    pos->prev= nullptr;
    pos->next= nullptr;
    pos->is_mmap= true;
    if(first_mmap== nullptr){
        first_mmap=ret_ptr; //void* of pos
    }else{
        MallocMetadata* firstMMap=(MallocMetadata *)first_mmap;
        pos->next=firstMMap;
        firstMMap->prev=pos;
        first_mmap=ret_ptr;
    }
    return (void*)( (unsigned long )first_mmap + sizeof (MallocMetadata));
}

void* caseB(MallocMetadata* pos,size_t size){
    if(pos->prev!= nullptr&& pos->prev->is_free && (pos->prev->size+pos->size+sizeof(MallocMetadata)>= size)){
        pos->is_free= true;
        void* src=(void*)(pos+1);
        size_t old_size=pos->size;
        pos=merge_cells(pos->prev,pos,false);
        pos->is_free= false;
        ///merged! check if we can cut
        void* dest=(void*)(pos+1);
        memmove(dest, src, old_size);
        if(pos->size-size>=128+sizeof(MallocMetadata)){
            cutblock(pos, size);
        }

        return dest;
    }
    return nullptr;
}
void* caseC(MallocMetadata* pos,size_t size){
    if(pos->next!= nullptr&& pos->next->is_free && (pos->next->size+pos->size+sizeof(MallocMetadata)>= size)){
        pos->is_free= true;
        pos=merge_cells(pos,pos->next,true);
        pos->is_free= false;
        ///merged! check if we can cut
        if(pos->size-size>=128+sizeof(MallocMetadata)){
            cutblock(pos, size);
        }
        return pos+1;
    }
    return nullptr;
}

void* caseD(MallocMetadata* pos,size_t size){
    if(pos->next!= nullptr&& pos->next->is_free && pos->prev!= nullptr&& pos->prev->is_free&&
            pos->next->size+pos->size+2*sizeof(MallocMetadata)+pos->prev->size>= size){
        pos->is_free= true;
        void* src=(void*)(pos+1);
        size_t old_size=pos->size;
        pos=merge_cells(pos->prev,pos,false);
        void* dest=(void*)(pos+1);
        pos=merge_cells(pos,pos->next,true);
        pos->is_free= false;
        memmove(dest, src, old_size);
        ///merged! check if we can cut
        if(pos->size-size>=128+sizeof(MallocMetadata)){
            cutblock(pos, size);
        }

        return dest;
//        return pos+1;
    }
    return nullptr;
}


void* smalloc(size_t size){
    if( size==0 || size> (pow(10,8))){
        return NULL;
    }
    if(size>128 * 1024){
        void* ret= mmap_hanler(size);
        if(ret== nullptr){
            return nullptr;
        }
        return ret;
    }
    MallocMetadata* pos=(MallocMetadata *)first_sbrk;
    MallocMetadata* last=NULL;
    while(pos!=NULL){
        if(pos->is_free && pos->size>=size){
            if(pos->size-size>=128+sizeof(MallocMetadata)) {
                pos->is_free = false;
                cutblock(pos, size);
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
        return 1+wildernessChunkIsFree(last,size);
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
        if(first_sbrk==NULL){
            new_block->prev=NULL;
            first_sbrk=(void*)new_block;
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
    void* ret= memset(ret_block, 0, size*num);
    return ret;
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
            munmap((void*)(pos), pos->size + sizeof (MallocMetadata));
        }else{
            if(!pos->is_free){
                pos->is_free= true;
            }
            if(pos->prev!= nullptr){
                pos=merge_cells(pos->prev,pos, false);
            }
            if(pos->next!= nullptr){
                pos=merge_cells(pos,pos->next, true);
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
                if(pos->size-size>=128+sizeof(MallocMetadata)) { //can split it to two
                    cutblock(pos, size);
                }
                pos->is_free = false;
                return (pos + 1);
            }else{ ///case B
                void* ret=caseB(pos,size);
                if(ret!= nullptr){
                    return ret;
                }else {///caseC
                    void* ret=caseC(pos,size);
                    if(ret!= nullptr) {
                        return ret;
                    }else{ ///caseD
                        void* ret=caseD(pos,size);
                        if(ret!= nullptr) {
                            return ret;
                        }
                    }
                }
            }
        }
        ///Case E:
        size_t copy_size= ((MallocMetadata*)((unsigned long)oldp - sizeof(MallocMetadata)))->size;
        ((MallocMetadata*)((unsigned long)oldp - sizeof(MallocMetadata)))->is_free = true;
        void* ret = smalloc(size);
        if(ret== nullptr){
            return nullptr;
        }
        /////((MallocMetadata*)((unsigned long)oldp - sizeof(MallocMetadata)))->is_free = false;

        if(ret!=oldp){
            memmove(ret, oldp, copy_size);
            sfree(oldp);
        }
        return ret;
    }
    ///oldp is NULL case
    void* newp= smalloc(size);
    if(newp==nullptr){
        return nullptr;
    }
    return newp;
}

size_t _num_free_blocks(){
    if( first_sbrk== nullptr){
        return 0;
    }
    MallocMetadata* pos= (MallocMetadata *)first_sbrk;
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
    if( first_sbrk== nullptr){
        return 0;
    }
    MallocMetadata* pos= (MallocMetadata *)first_sbrk;
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
    size_t counter=0;
    if( first_sbrk== nullptr && first_mmap== nullptr){
        return counter;
    }
    MallocMetadata* pos= (MallocMetadata *)first_sbrk;
    while(pos!= nullptr){
        counter+=1;
        pos=pos->next;
    }
    if( first_mmap== nullptr){
        return counter;
    }
    MallocMetadata* posMMap= (MallocMetadata *)first_mmap;
    while(posMMap!= nullptr){
        counter+=1;
        posMMap=posMMap->next;
    }
    return counter;
}

size_t _num_allocated_bytes(){
    size_t counter=0;
    if( first_sbrk== nullptr && first_mmap== nullptr){
        return counter;
    }
    MallocMetadata* pos= (MallocMetadata *)first_sbrk;
    while(pos!= nullptr){
        counter+=pos->size;
        pos=pos->next;
    }
    MallocMetadata* posMMap= (MallocMetadata *)first_mmap;
    while(posMMap!= nullptr){
        counter+=posMMap->size;
        posMMap=posMMap->next;
    }
    return counter;

}
size_t _num_meta_data_bytes(){
    return _num_allocated_blocks()*sizeof(MallocMetadata);
}


size_t _size_meta_data(){
    return sizeof (MallocMetadata);
}