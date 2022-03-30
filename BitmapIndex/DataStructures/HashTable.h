#pragma once

#include <cstdint>
#include "HashTableNode.h"
#include "Memory.h"
#include "Cursor.h"
#include <string_view>
#include <cmath>

template<class TData>
class cHashTable
{
private:
	int mSize = 0;
	cHashTableNode<TData>** mHashTable = nullptr;
	int mItemCount = 0;
	int mNodeCount = 0;
    cMemory *mMemory = nullptr;
    unsigned int nodeSize = 0;
    unsigned int keySize = 0;
    unsigned int * bitShifts = nullptr;
	inline int HashValue(const char *) const;
    char * selectKey = nullptr;
public:
    cHashTable(unsigned int, unsigned int, unsigned int, int*, unsigned int);

	~cHashTable();

    bool Add(const char *, const TData &data, bool recursive = false);
    bool IncrementData(const char *);
    bool Find(const char *, TData &data, bool recursive = false) const;
    bool Find(char *, Cursor<TData> &) const;
    bool Select(const char *, TData &data) const;
	void PrintStat() const;
};

/*
template<class TData>
cHashTable<TData>::cHashTable(int size, cMemory *memory)
{
    printf("Hash table slot size is %d\n", size);
	mSize = size;
	mHashTable = new cHashTableNode<TData>*[size];
	for (int i = 0; i < mSize; i++) {
		mHashTable[i] = nullptr;
	}
    mMemory = memory;
}
*/

template<class TData>
cHashTable<TData>::cHashTable(
        unsigned int itemCount,unsigned int nodeSize, unsigned int keySize,
        int * attrs_max_value, unsigned int cols
    ) {
    this->nodeSize = nodeSize;
    this->keySize = keySize;
    this->mSize = itemCount / (nodeSize / (keySize + sizeof (TData)));
    mHashTable = new cHashTableNode<TData>*[mSize];
    bitShifts = new unsigned int[keySize];
    for (int i = 0; i < mSize; ++i) {
        mHashTable[i] = nullptr;
    }

    selectKey = new char [keySize];

    auto bitShiftIndex = 0;
    for (int i = 0; i < cols; ++i) {
        if (attrs_max_value[i] > 0) {
            auto sum = 0;
            if (bitShiftIndex > 0) {
                sum = bitShifts[bitShiftIndex - 1];
            }
            auto log = (int)std::ceil(std::log2(attrs_max_value[i]));
            bitShifts[bitShiftIndex++] = sum + log;
        }
    }
}

template<class TData>
cHashTable<TData>::~cHashTable()
{
    if (mMemory == nullptr) {
        if (mHashTable != nullptr) {
            for (int i = 0; i < mSize; ++i) {
                if (mHashTable[i] != nullptr) {
                    delete mHashTable[i];
                    mHashTable[i] = nullptr;
                }
            }
        }
    }

    delete [] mHashTable;
    mHashTable = nullptr;

    if (selectKey != nullptr) {
        delete [] selectKey;
        selectKey = nullptr;
    }
}

template<class TData>
bool cHashTable<TData>::Add(const char * key, const TData &data, bool recursive)
{
	int hv = HashValue(key);

	if (mHashTable[hv] == nullptr)
	{
        // mMemory is not accessible
        if (mMemory == nullptr) {
            mHashTable[hv] = new cHashTableNode<TData>(keySize, nodeSize);
        } else { //mMemory was given and we can operate on it
            auto memForNode = mMemory->New(sizeof(cHashTableNode<TData>));
            mHashTable[hv] = new (memForNode)cHashTableNode<TData>();
        }
        mNodeCount++;
        printf("HashTable new Node creating!, Slot count: %d\n", mNodeCount);
    }

//    if (recursive) {
//        return mHashTable[hv]->AddRecursive(key, data, mMemory, mItemCount, mNodeCount);
//    } else {
//        return mHashTable[hv]->Add(key, data, mMemory, mItemCount, mNodeCount);
//    }
    return mHashTable[hv]->Add(key, data);
}

template<class TData>
bool cHashTable<TData>::Find(const char * key, TData &data, bool recursive) const
{
	auto hv = HashValue(key);
    if (mHashTable[hv] == nullptr) {
        return false;
    }

    return mHashTable[hv]->Find(key, data);
}

template<class TData>
inline int cHashTable<TData>::HashValue(const char * key) const
{
    unsigned long hash = 0;
    for (int i = 0; i < keySize; ++i) {
        hash += key[i] << bitShifts[i];
    }
	return hash % mSize;
}

template<class TData>
void cHashTable<TData>::PrintStat() const
{
	printf("HashTable Statistics: Size: %d, ItemCount: %ld, NodeCount: %ld, Avg. Items/Slot: %.2f.\n",
		mSize, mItemCount, mNodeCount, (float)(mItemCount / mSize));
}

template<class TData>
bool cHashTable<TData>::Find(const char * key, Cursor<TData> & cursor) const {
    auto hv = HashValue(key);
    if (mHashTable[hv] == nullptr) {
        return false;
    }
    return mHashTable[hv]->Find(key, cursor);
}

template<class TData>
bool cHashTable<TData>::Select(const char *query, TData &data) const {


    return Find(selectKey, data);
}

template<class TData>
bool cHashTable<TData>::IncrementData(const char * key) {
    int hv = HashValue(key);

    if (mHashTable[hv] == nullptr)
    {
        // mMemory is not accessible
        if (mMemory == nullptr) {
            mHashTable[hv] = new cHashTableNode<TData>(keySize, nodeSize);
        } else { //mMemory was given and we can operate on it
            auto memForNode = mMemory->New(sizeof(cHashTableNode<TData>));
            mHashTable[hv] = new (memForNode)cHashTableNode<TData>();
        }
        mNodeCount++;
    }
    return mHashTable[hv]->IncrementData(key);
}
