//
// Created by lukas on 16.02.22.
//

#ifndef HEAPTABLEHASHTABLE_CMEMORY_H
#define HEAPTABLEHASHTABLE_CMEMORY_H

#include <cstdio>

class cMemory {
private:
    int mCapacity;
    int mSize;
    char * mData = nullptr;

public:
    explicit cMemory(int capacity) {
        mCapacity = capacity;
        mSize = 0;
        mData = new char[capacity];
    }

    ~cMemory() {
        if (mData != nullptr) {
            delete [] mData;
            mCapacity = 0;
            mSize = 0;
            mData = nullptr;
        }
    }

    inline char* New(int size);
    void PrintStat() const;

};

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

#endif //HEAPTABLEHASHTABLE_CMEMORY_H
