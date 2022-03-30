//
// Created by lukas on 09.03.22.
//

#ifndef BITMAPINDEX_BITMAPINDEX_H
#define BITMAPINDEX_BITMAPINDEX_H

#include "BitString.h"

class BitmapIndex {
private:
    char * mData = nullptr;
    short bitSize = 0;
    int byteSize = 0;
    int attrsCount = 0;
    int * attrsMaxValue = nullptr;
    int * bitIndexAttributeOffset = nullptr;
    int recordCount = 0;
    char * SelectMask;

    unsigned int capacity = 0;
    [[nodiscard]] inline char * getRowPointer(unsigned int rowId) const {
        return mData + rowId * byteSize;
    }

    inline bool shouldColBeIndexed(int max_col_value) const {
        return max_col_value > 0;
    }
public:
    BitmapIndex(const int *attrs_max_value, int attrs_count, unsigned int capacity);
    ~BitmapIndex();

    unsigned long getTotalByteSize() const {
        return byteSize * capacity;
    }


    void createRecord(const char *rec, const unsigned int *attributeSizes);
    int Select(unsigned int conditions[][2], int size) const;
    int Select(const char *) const;
};




#endif //BITMAPINDEX_BITMAPINDEX_H
