#pragma once

#include "HashTableNode.h"
#include "Memory.h"
#include <cstring>
#include "Cursor.h"

template <class TData> class cHashTable;
template <class TData> class Cursor;

template<class TData>
class cHashTableNode
{
private:
	bool mEmptyNode;
	cHashTableNode* mNextNode = nullptr;
    unsigned int keySize = 0;
    unsigned int size = 0;
    char * data = nullptr;
    unsigned int recordSize = 0;
    unsigned recordsCount = 0;
    friend class cHashTable<TData>;
    inline unsigned int getRecordOffset(unsigned int recordId) {
        return recordId * recordSize;
    }
    inline char * getRowPointer(unsigned int recordId) {
        return data + getRecordOffset(recordId);
    };

public:
    cHashTableNode(unsigned int keySize, unsigned int nodeSize);
    cHashTableNode();
	~cHashTableNode();

    bool Add(const char *, const TData &data);
    bool IncrementData(const char *);
    bool Find(const char *, Cursor<TData> &);
    bool Find(const char *, TData &);

    cHashTableNode<TData> * getNextNode() const {
        return mNextNode;
    }
    unsigned int getRecordSize() const {
        return recordSize;
    }
    unsigned int getKeySize() const {
        return keySize;
    }
    unsigned int getRecordCount() const {
        return recordsCount;
    }

    char * getDataPointer() const {
        return data;
    }

};

template<class TData>
cHashTableNode<TData>::cHashTableNode()
{
	mNextNode = nullptr;
	mEmptyNode = true;
}

template<class TData>
cHashTableNode<TData>::~cHashTableNode()
{
	if (mNextNode != nullptr)
	{
		delete mNextNode;
		mNextNode = nullptr;
	}

    if (data != nullptr) {
        delete [] data;
        data = nullptr;
    }
}


template<class TData>
cHashTableNode<TData>::cHashTableNode(unsigned int keySize, unsigned int nodeSize) {
    this->keySize = keySize;
    recordsCount = 0;
    size = nodeSize;
    data = new char [nodeSize];
    recordSize = keySize + sizeof (TData);
    memset(data, 0, nodeSize);
}

template<class TData>
bool cHashTableNode<TData>::Add(const char * key, const TData &data) {
    if (getRecordOffset(recordsCount) + recordSize > size) {

        if (mNextNode == nullptr) {
            mNextNode = new cHashTableNode<TData>(keySize, size);
        }
        return mNextNode->Add(key, data);
    }

    auto p = getRowPointer(recordsCount);
    memcpy(p, key, keySize);
    *((TData*)(p + keySize)) = data;

    recordsCount += 1;
    return true;
}

template<class TData>
bool cHashTableNode<TData>::Find(const char * key, TData & out_data)
{
    for (unsigned int i = 0; i < recordsCount; ++i) {
        auto p = getRowPointer(i);

        if (std::memcmp(p, key, keySize) == 0) {
            out_data = *((TData *) (p + keySize));
            return true;
        }
    }

    if (mNextNode != nullptr) {
        return mNextNode->Find(key, out_data);
    }
    return false;
}

template<class TData>
bool cHashTableNode<TData>::Find(const char * key, Cursor<TData> &cursor) {
    for (unsigned int i = 0; i < recordsCount; ++i) {
        char * p = getRowPointer(i);

        if (std::memcmp(p, key, keySize) == 0) {
            cursor.key = key;
            cursor.pointer = p + keySize;
            cursor.currentNode = this;
            cursor.offset = i;
            return true;
        }
    }

    if (mNextNode != nullptr) {
        return mNextNode->Find(key, cursor);
    }
    return false;
}

template<class TData>
bool cHashTableNode<TData>::IncrementData(const char * key) {
    for (int i = 0; i < recordsCount; ++i) {
        auto p = getRowPointer(i);
        if (memcmp(p, key, keySize) == 0) {
            *((TData*)(p + keySize)) += 1;
            return true;
        }
    }
    auto rowPointer = getRowPointer(recordsCount);
    memcpy(rowPointer, key, keySize);
    *((TData*)(rowPointer + keySize)) += 1;
    recordsCount += 1;
    return true;
}

