//
// Created by lukas on 23.02.22.
//

#ifndef HEAPTABLE_CROWHEAPTABLE_H
#define HEAPTABLE_CROWHEAPTABLE_H

#include <cstdint>

class cRowHeapTable {
private:
    char *mData = nullptr;
    unsigned int rowCount = 0;
    unsigned int itemCount = 0;
    unsigned int rowSize = 0;

    inline char* getRowPointer(unsigned int rowId) const;

public:
    cRowHeapTable(const unsigned int attrsSize[], unsigned int length, unsigned int rowCount);
    ~cRowHeapTable();

    bool Insert(char * rec);
    unsigned int Select(unsigned int conditions[][2], std::size_t size) const;
    bool CreateBitmapIndex(unsigned int argsCount);
};

cRowHeapTable::cRowHeapTable(const unsigned int attrsSize[], unsigned int length, unsigned int rowCount) {
    unsigned int size = 0;
    for (int i = 0; i < length; ++i) {
        size += attrsSize[i];
    }
    rowSize = size;
    mData = new char[size * rowCount];
    this->rowCount = rowCount;
}

cRowHeapTable::~cRowHeapTable() {
    if (mData != nullptr) {
        delete [] mData;
        mData = nullptr;
        itemCount = 0;
        rowCount = 0;
    }
}

bool cRowHeapTable::Insert(char *rec) {
    if (itemCount > rowCount) {
        return false;
    }
    auto rowPointer = getRowPointer(itemCount);
    rowPointer = rec;
    itemCount += 1;
    return true;
}

unsigned int cRowHeapTable::Select(unsigned int conditions[][2], std::size_t size) const {
//    for (int i = 0; i < size; ++i) {
//        
//    }
    char * rowPointer = nullptr;
    for (std::size_t i = 0; i < itemCount; ++i) {
        rowPointer = getRowPointer(i);
//        for (int j = 0; j < size; ++j) {
//        }
    }
    return 0;
}




inline char *cRowHeapTable::getRowPointer(unsigned int rowId) const {
    return mData + (rowId * rowSize);
}

bool cRowHeapTable::CreateBitmapIndex(unsigned int) {
    return false;
}

#endif //HEAPTABLE_CROWHEAPTABLE_H
