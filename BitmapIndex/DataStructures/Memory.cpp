//
// Created by lukas on 06.04.22.
//


#include "Memory.h"
#include <cstdio>

void cMemory::PrintStat() const {
    printf("cMemory::PrintStat(): Capacity: %d, Size %d, Utilization: %.2f\n",
           mCapacity, mSize, (double)mSize / mCapacity);
}

char *cMemory::New(int size) {
    char * mem = nullptr;
    if (mSize + size >= mCapacity) {
        printf("Critical error: cMemory::new(): There is not enough memory.\n");
        mem = nullptr;
    } else {
        mem = mData + mSize;
        mSize += size;
    }
    return mem;
}
