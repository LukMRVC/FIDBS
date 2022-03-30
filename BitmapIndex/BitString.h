//
// Created by lukas on 09.03.22.
//

#ifndef BITMAPINDEX_BITSTRING_H
#define BITMAPINDEX_BITSTRING_H

#include <cstdio>

constexpr short BYTE_SIZE = 8;

class BitString {
public:
    static int getByteSizeFromBits(int bitSize) {
        return (bitSize + BYTE_SIZE - 1) / BYTE_SIZE;
    }

    static void setBitString(char* rec, int bitOffset) {
        rec[bitOffset / 8] |= (1 << (bitOffset % BYTE_SIZE));
    }

    static void printBitString(char * rec, int byteSize) {
        for (int i = byteSize * BYTE_SIZE - 1; i >= 0; --i) {
            auto bitValue = (rec[i / BYTE_SIZE] & (1 << (i % BYTE_SIZE)));
            if (bitValue == 0) {
                printf("0");
            } else {
                printf("1");
            }
            if (i == 8) {
                printf("_");
            }
        }
        printf("\n");
    }

    static bool equals(const char * mask, const char * record, int byteSize) {
        for (int i = 0; i < byteSize; ++i) {
            if ((mask[i] & record[i]) != record[i] ) {
                return  false;
            }
        }
        return true;
    }
};


#endif //BITMAPINDEX_BITSTRING_H
