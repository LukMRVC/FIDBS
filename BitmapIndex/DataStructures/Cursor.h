//
// Created by lukas on 23.03.22.
//

#ifndef BITMAPINDEX_CURSOR_H
#define BITMAPINDEX_CURSOR_H

#include "HashTableNode.h"
#include <cstring>

template <class TData> class cHashTableNode;

template<class TData>
class Cursor {
public:
    const char * key = nullptr;
    cHashTableNode<TData> * currentNode = nullptr;
    char * pointer = nullptr;
    bool nextRecord(TData & data);
    unsigned int offset = 0;
};

template<class TData>
bool Cursor<TData>::nextRecord(TData &data) {
    for (int i = offset; i < currentNode->getRecordCount(); ++i) {
        auto p = currentNode->getDataPointer() + i * currentNode->getRecordSize();
        if (std::memcmp(key, p, currentNode->getKeySize()) == 0) {
            data = *((TData*)(p + currentNode->getKeySize() ));
            offset = i + 1;
            return true;
        }
    }

    if (currentNode->getNextNode() != nullptr) {
        currentNode = currentNode->getNextNode();
        pointer = currentNode->getDataPointer();
        offset = 0;
        return nextRecord(data);
    }
    return false;
}


#endif //BITMAPINDEX_CURSOR_H
