#pragma once

#include "cHashTableNode.h"
#include "cMemory.h"
#include "Table.h"

template<class TKey, class TData>
class cHashTable
{
private:
	int mSize;
	cHashTableNode<TKey,TData>** mHashTable = nullptr;
	int mItemCount;
	int mNodeCount;
    cMemory *mMemory = nullptr;

private:
	inline int HashValue(const TKey &key) const;

public:
	explicit cHashTable(int size, cMemory *memory);
	~cHashTable();

	bool Add(const TKey &key, const TData &data);
	bool Find(const TKey &key, TData &data) const;
	void PrintStat() const;
};

template<class TKey, class TData>
cHashTable<TKey,TData>::cHashTable(int size, cMemory *memory)
{
	mSize = size;
	mHashTable = new cHashTableNode<TKey,TData>*[size];
	for (int i = 0; i < mSize; i++) {
		mHashTable[i] = nullptr;
	}
    mMemory = memory;
}

template<class TKey, class TData>
cHashTable<TKey, TData>::~cHashTable()
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
}

template<class TKey, class TData>
bool cHashTable<TKey, TData>::Add(const TKey &key, const TData &data)
{
	int hv = HashValue(key);

	if (mHashTable[hv] == nullptr)
	{
        // mMemory is not accessible
        if (mMemory == nullptr) {
            mHashTable[hv] = new cHashTableNode<TKey, TData>();
        } else { //mMemory was given and we can operate on it
            auto memForNode = mMemory->New(sizeof(cHashTableNode<TKey, TData>));
            mHashTable[hv] = new (memForNode)cHashTableNode<TKey, TData>();
        }
        mNodeCount++;
    }

	return mHashTable[hv]->Add(key, data, mMemory, mItemCount, mNodeCount);
}

template<class TKey, class TData>
bool cHashTable<TKey, TData>::Find(const TKey &key, TData &data) const
{
	auto hv = HashValue(key);
    if (mHashTable[hv] == nullptr) {
        return false;
    }
    return mHashTable[hv]->Find(key, data);
}

template<class TKey, class TData>
inline int cHashTable<TKey, TData>::HashValue(const TKey &key) const
{
	return key % mSize;
}

template<class TKey, class TData>
void cHashTable<TKey, TData>::PrintStat() const
{
	printf("HashTable Statistics: Size: %d, ItemCount: %d, NodeCount: %d, Avg. Items/Slot: %.2f.\n",
		mSize, mItemCount, mNodeCount, (float)mItemCount / mSize);
}