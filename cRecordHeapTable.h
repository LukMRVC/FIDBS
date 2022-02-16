//
// Created by lukas on 09.02.22.
//

#ifndef HEAPTABLE_CRECORDHEAPTABLE_H
#define HEAPTABLE_CRECORDHEAPTABLE_H
#include "Record.h"
#include "Table.h"
#include <cassert>
#include <cassert>


template <class TKey, class TData>
class cRecordHeapTable : public Table<TKey, TData> {
    int mCapacity;
    int mCount;
    int const mRowSize = sizeof (Record<TKey, TData>);

public:
    explicit cRecordHeapTable(int capacity);
    virtual ~cRecordHeapTable();

    Record<TKey, TData> ** mData = nullptr;
    bool Add(const TKey &key, const TData &data);
    bool Get(int rowId, TKey &key, TData &data) const;
    bool Find(const TKey &key, TData &data) const;
};

template <class TKey, class TData>
cRecordHeapTable<TKey, TData>::cRecordHeapTable(int capacity)
{
    mCapacity = capacity;
    mCount = 0;
    mData = new Record<TKey, TData>*[capacity];
}

template <class TKey, class TData>
cRecordHeapTable<TKey, TData>::~cRecordHeapTable()
{
    if (mData != nullptr)
    {
        for (long i = 0; i < mCount; ++i) {
            delete mData[i];
        }
        delete [] mData;
        mData = nullptr;
        mCapacity = 0;
        mCount = 0;
    }
}

template <class TKey, class TData>
bool cRecordHeapTable<TKey, TData>::Get(int rowId, TKey &key, TData &data) const
{
    bool ret = false;
    assert(rowId >= 0 && rowId < mCount);
    ret = true;

    auto record = mData[rowId];
    key = record->key;
    data = record->data;

    return ret;
}

template <class TKey, class TData>
bool cRecordHeapTable<TKey, TData>::Add(const TKey &key, const TData &data)
{
    bool ret = false;
    assert(mCount < mCapacity);
    ret = true;

    mData[mCount] = new Record(key, data);

    mCount++;
    return ret;
}

template <class TKey, class TData>
bool cRecordHeapTable<TKey, TData>::Find(const TKey &key, TData &data) const
{
    bool ret = false;
    for (long i = 0; i < mCount; ++i) {
        if (mData[i]->key == key) {
            data = mData[i]->data;
            ret = true;
            break;
        }
    }

    return ret;
}

#endif //HEAPTABLE_CRECORDHEAPTABLE_H
