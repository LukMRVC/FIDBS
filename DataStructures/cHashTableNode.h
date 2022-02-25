#pragma once

#include "cHashTableNode.h"
#include "cMemory.h"

template <class TKey, class TData> class cHashTable;

template<class TKey, class TData>
class cHashTableNode
{
private:
	bool mEmptyNode;
	TKey mKey;
	TData mData;
	cHashTableNode* mNextNode;
    friend class cHashTable<TKey, TData>;

public:
	cHashTableNode();
	~cHashTableNode();

    bool Add(const TKey &key, const TData &data, cMemory * memory, int &itemCount, int &nodeCount);
    bool AddRecursive(const TKey &key, const TData &data, cMemory * memory, int &itemCount, int &nodeCount);
    bool Find(const TKey &key, TData &data) const;
    bool FindRecursive(const TKey &key, TData &data) const;
};

template<class TKey, class TData>
cHashTableNode<TKey, TData>::cHashTableNode()
{
	mNextNode = nullptr;
	mEmptyNode = true;
}

template<class TKey, class TData>
cHashTableNode<TKey, TData>::~cHashTableNode()
{
	if (mNextNode != nullptr)
	{
		delete mNextNode;
		mNextNode = nullptr;
	}
}

template<class TKey, class TData>
__attribute__ ((noinline)) bool cHashTableNode<TKey, TData>::Add(const TKey &key, const TData &data, cMemory *memory, int &itemCount, int &nodeCount)
{
    // if is empty node, simply add the key and data
    // if is not empty, check key, if mKey and key are same, return false
    // if nextNode is nullptr, allocate and insert the data
    auto nextNodeInChain = this;
    while (!nextNodeInChain->mEmptyNode) {
        if (nextNodeInChain->mKey == key) {
            return false;
        }
        if (nextNodeInChain->mNextNode == nullptr) {
            if (memory == nullptr) {
                nextNodeInChain->mNextNode = new cHashTableNode<TKey, TData>();
            } else {
                auto mem = memory->New(sizeof (cHashTableNode<TKey, TData>));
                nextNodeInChain->mNextNode = new (mem)cHashTableNode<TKey, TData>();
            }
            nodeCount++;
            nextNodeInChain = nextNodeInChain->mNextNode;
            break;
        }
        nextNodeInChain = nextNodeInChain->mNextNode;
    }

    nextNodeInChain->mKey = key;
    nextNodeInChain->mData = data;
    nextNodeInChain->mEmptyNode = false;
    itemCount++;
    return true;
}

template<class TKey, class TData>
__attribute__ ((noinline)) bool cHashTableNode<TKey, TData>::AddRecursive(const TKey &key, const TData &data, cMemory *memory, int &itemCount, int &nodeCount)
{
    bool ret = true;

    if (!mEmptyNode) {
        if (mKey == key) {
            ret = false;
        }
        else {
            if (mNextNode == nullptr) {
                if (memory == nullptr) {
                    mNextNode = new cHashTableNode<TKey, TData>();
                } else {
                    auto mem = memory->New(sizeof (cHashTableNode<TKey, TData>));
                    mNextNode = new (mem)cHashTableNode<TKey, TData>();
                }
                nodeCount++;
            }
            ret = mNextNode->Add(key, data, memory, itemCount, nodeCount);
        }
    } else {
        mKey = key;
        mData = data;
        mEmptyNode = false;
        itemCount++;
        ret = true;
    }
    return ret;
}

template<class TKey, class TData>
bool __attribute__ ((noinline)) cHashTableNode<TKey, TData>::Find(const TKey &key, TData &data) const
{
    auto nextNodeInChain = this;
    while (nextNodeInChain != nullptr) {
        if (nextNodeInChain->mKey == key) {
            data = nextNodeInChain->mData;
            return true;
        }
        nextNodeInChain = nextNodeInChain->mNextNode;
    }
    return false;
}

template<class TKey, class TData>
bool __attribute__ ((noinline)) cHashTableNode<TKey, TData>::FindRecursive(const TKey &key, TData &data) const {
    if (mKey == key) {
        data = mData;
        return true;
    } else {
        if (mNextNode == nullptr) {
            return false;
        } else {
            return mNextNode->Find(key, data);
        }
    }
}