#pragma once

#include <assert.h>
#include <stdlib.h>

template<class TKey, class TData>
class cHeapTable
{
private:
	int mCapacity;
	int mCount;
	char* mData;
	int const mRowSize = sizeof(TKey) + sizeof(TData);

private:
	inline char* GetRowPointer(int rowId) const;

public:
	cHeapTable(int capacity);
	~cHeapTable();

	bool Add(const TKey &key, const TData &data);
	bool Get(int rowId, TKey &key, TData &data) const;
	bool Find(const TKey &key, TData &data) const;
};

template<class TKey, class TData>
cHeapTable<TKey, TData>::cHeapTable(int capacity)
{
	mCapacity = capacity;
	mCount = 0;
	mData = new char[mRowSize * capacity];
}

template<class TKey, class TData>
cHeapTable<TKey, TData>::~cHeapTable()
{
	if (mData != NULL)
	{
		delete mData;
		mData = NULL;
		mCapacity = 0;
		mCount = 0;
	}
}

template<class TKey, class TData>
inline char* cHeapTable<TKey, TData>::GetRowPointer(int rowId) const
{
	return mData + rowId * mRowSize;
}

template<class TKey, class TData>
bool cHeapTable<TKey, TData>::Get(int rowId, TKey &key, TData &data) const
{
	bool ret = false;
	assert(rowId >= 0 && rowId < mCount);
	ret = true;

	char* p = GetRowPointer(rowId);
	key = *((TKey*)p);
	data = *((TData*)(p + sizeof(TKey)));

	return ret;
}

template<class TKey, class TData>
bool cHeapTable<TKey, TData>::Add(const TKey &key, const TData &data)
{
  // TODO
}

template<class TKey, class TData>
bool cHeapTable<TKey, TData>::Find(const TKey &key, TData &data) const
{
  // TODO
}

