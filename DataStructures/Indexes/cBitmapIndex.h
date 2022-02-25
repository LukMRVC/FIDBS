//
// Created by lukas on 23.02.22.
//

#ifndef HEAPTABLE_CBITMAPINDEX_H
#define HEAPTABLE_CBITMAPINDEX_H

#include <cstdint>

class cBitmapIndex {
private:
    char * mData = nullptr;

public:
    explicit cBitmapIndex(unsigned int argsCount[], std::size_t length, unsigned int rowCount);
};

cBitmapIndex::cBitmapIndex(unsigned int *argsCount, std::size_t length, unsigned int rowCount) {

}

#endif //HEAPTABLE_CBITMAPINDEX_H
