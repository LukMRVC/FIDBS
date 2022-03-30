//
// Created by lukas on 09.02.22.
//

#ifndef HEAPTABLE_TABLE_H
#define HEAPTABLE_TABLE_H

template <class TKey, class TData>
class Table {
public:
    virtual ~Table() = default;

    virtual bool Add(const TKey &key, const TData &data) = 0;
    virtual bool Find(const TKey &key, TData &data) const = 0;
};
#endif //HEAPTABLE_TABLE_H

#ifndef HEAPTABLE_ABSTRACTHEAPTABLE_H
#define HEAPTABLE_ABSTRACTHEAPTABLE_H

template<class TKey, class TData>
class AbstractHeapTable : public Table<TKey, TData> {
public:
    virtual bool Get(int rowId, TKey &key, TData &data) const = 0;
};

#endif