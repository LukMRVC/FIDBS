//
// Created by lukas on 16.02.22.
//

#ifndef HEAPTABLEHASHTABLE_CMEMORY_H
#define HEAPTABLEHASHTABLE_CMEMORY_H


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

    char* New(int size);
    void PrintStat() const;

};

#endif //HEAPTABLEHASHTABLE_CMEMORY_H
