#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "njvm.h"
#include "garbage.h"
#include "bigint.h"

void *heap;
HeapHalf half1;
HeapHalf half2;
HeapHalf *currentHalf;
void *freePointer;
int size;
bool purge = false;
bool stats = false;
int stack_n = 64;
int heap_n = 8192;
int heap_half_size;

void allocateHeap() {
    int heap_size = heap_n * 1024;
    int heap_half_size = heap_size / 2;

    heap = malloc(heap_size);

    half1.start = heap;
    half1.end = heap + heap_size;
    half2.start = half1.end;
    half2.end = heap + heap_size;

    currentHalf = &half1;
    freePointer = currentHalf->start;

    if(purge) {
        memset(heap, 0, heap_size);
    }
}

void allocateStack() {
    int stack_size = stack_n * 1024;
    stack = malloc(stack_size);

    if(purge) {
        memset(stack, 0, stack_size);
    }
}

void * relocate(ObjRef orig, void **newFreePointer) {
    ObjRef copy;
    if(orig == NULL) {
        copy = NULL;
    }
    else if(orig->brokenHeart) {
        copy = orig->forwardPointer;
    }
    else {
        unsigned long size;
        if(IS_PRIM(orig)) {
            size = sizeof(orig) + orig->size;
        } 
        else {
            size = sizeof(orig) + (sizeof(void *) * GET_SIZE(orig));
        }
        memcpy(*newFreePointer, orig, size);
        copy = *newFreePointer;
        orig->brokenHeart = true;
        orig->forwardPointer = *newFreePointer;
        *newFreePointer += size;
    }
    return copy;
}

void collect() {
    void *newFreePointer;
    HeapHalf *newHalf;

    if(currentHalf->start == half1.start) {
        newHalf = &half2;
    }
    else {
        newHalf = &half1;
    }
    newFreePointer = newHalf->start;

    returnRegister = relocate(returnRegister, &newFreePointer);
    bip.op1 = relocate(bip.op1, &newFreePointer);
    bip.op2 = relocate(bip.op2, &newFreePointer);
    bip.rem = relocate(bip.rem, &newFreePointer);
    bip.res = relocate(bip.res, &newFreePointer);

    for(int i = 0; i < sdaSize; i++) {
        sda[i] = relocate(sda[i], &newFreePointer);
    }

    for(int i = 0; i < sp; i++) {
        if(stack[i].isObjRef) {
            stack[i].u.objRef = relocate(stack[i].u.objRef, &newFreePointer);
        }
    }
    
    void *scan = newHalf->start;
    while(scan != newFreePointer) {
        ObjRef obj = scan;
        if(!IS_PRIM(obj)) {
            int size = GET_SIZE(obj);
            for(int i = 0; i < size; i++) {
                GET_REFS(obj)[i] = relocate(GET_REFS(obj)[i], &newFreePointer);
            }
        }
        scan += sizeof(Obj) + obj->size;
    }

    if(purge) {
        memset(currentHalf->start, 0, heap_half_size);
    }
    currentHalf = newHalf;
    freePointer = newFreePointer;
}