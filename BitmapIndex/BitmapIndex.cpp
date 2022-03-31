//
// Created by Lukas on 30.03.2022.
//

#include "BitmapIndex.h"
#include "BitString.h"
#include <cstring>
#include <stdexcept>

BitmapIndex::BitmapIndex(const int *attrs_max_value, int attrs_count, unsigned int capacity) {
    auto maxValuesSum = 0;
    attrsMaxValue = new int[attrs_count];
    attrsCount = attrs_count;
    bitIndexAttributeOffset = new int[attrs_count];
    auto bitOffset = 0;
    for (int i = 0; i < attrs_count; ++i) {
        attrsMaxValue[i] = attrs_max_value[i];
        bitIndexAttributeOffset[i] = bitOffset;
        if (!shouldColBeIndexed(attrs_max_value[i])) continue;
        maxValuesSum += attrs_max_value[i];
        bitOffset += attrs_max_value[i];
    }
    bitSize = (short)maxValuesSum;
    byteSize = BitString::getByteSizeFromBits(bitSize);
    mData = new char [byteSize * capacity];
    memset(mData, 0, byteSize * capacity);
    SelectMask = new char [byteSize];
    this->capacity = capacity;
}

BitmapIndex::~BitmapIndex() {
    if (mData != nullptr) {
        delete [] mData;
        mData = nullptr;
        delete [] attrsMaxValue;
        attrsMaxValue = nullptr;

        delete [] SelectMask;
        SelectMask = nullptr;
    }
}

void BitmapIndex::createRecord(const char *rec, const unsigned int *attributeSizes) {
    auto indexRecord = getRowPointer(recordCount);
    unsigned int offset = 0;
    for (int j = 0; j < attrsCount; ++j) {
        if (attrsMaxValue[j] != -1) {
            // byte col value
            char colValue = *(rec + offset);
            if (colValue > attrsMaxValue[j]) {
                throw std::runtime_error("Col value is bigger than max allowed value!");
            }
//            printf("%d ", colValue);
            BitString::setBitString(indexRecord, bitIndexAttributeOffset[j] + colValue);
        }
        offset += attributeSizes[j];
    }
    recordCount += 1;
//    printf("\n");
//    BitString::printBitString(indexRecord, byteSize);
}

int BitmapIndex::Select(unsigned int conditions[][2], int size) const {
    int rowsFound = 0;
    memset(SelectMask, 0, byteSize);
    for (int j = 0; j < size; ++j) {
        auto col = conditions[j][0];
        auto col_value = conditions[j][1];
        BitString::setBitString(SelectMask, bitIndexAttributeOffset[col] + col_value);
    }

    uint64_t maxBytesValue = 0;
    for (int i = 0; i < byteSize; ++i) {
        maxBytesValue |= 0xff << (i * 8);
    }

    for (int i = 0; i < recordCount; ++i) {
        auto indexRecord = getRowPointer(i);
        if (BitString::equals(SelectMask, indexRecord, maxBytesValue)) {
            rowsFound += 1;
        }
    }

    return rowsFound;
}

int BitmapIndex::Select(const char * query) const {
    int rowsFound = 0;
    memset(SelectMask, 0, byteSize);
    for (int col = 0; col < attrsCount; ++col) {
        if (!shouldColBeIndexed(attrsMaxValue[col])) {
            continue;
        }
        auto col_value = query[col];
        if (col_value < 0) {
            for (int i = 0; i < attrsMaxValue[col]; ++i) {
                BitString::setBitString(SelectMask, bitIndexAttributeOffset[col] + i);
            }
        } else {
            BitString::setBitString(SelectMask, bitIndexAttributeOffset[col] + col_value);
        }
    }

    uint64_t maxBytesValue = 0;
    uint64_t byteVals = 0xff;
    for (int i = 0; i < byteSize; ++i) {
        maxBytesValue |= byteVals << (i * 8);
    }

    auto indexRecord = getRowPointer(0);
    auto maxLoops = recordCount >> 1;
    for (int i = 0; i < maxLoops; ++i) {
        if (BitString::equals(SelectMask, indexRecord, maxBytesValue)) {
            rowsFound += 1;
        }
        indexRecord += byteSize;
        if (BitString::equals(SelectMask, indexRecord, maxBytesValue)) {
            rowsFound += 1;
        }
        indexRecord += byteSize;
    }

    if (recordCount & 1) {
        indexRecord += byteSize;
        if (BitString::equals(SelectMask, indexRecord, maxBytesValue)) {
            rowsFound += 1;
        }
    }


    return rowsFound;
}