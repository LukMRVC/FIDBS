//
// Created by lukas on 09.02.22.
//

#ifndef HEAPTABLE_RECORD_H
#define HEAPTABLE_RECORD_H

template <class TKey, class TData>
class Record {

public:
    TKey key;
    TData data;
    Record(TKey key, TData data);
};

template<class TKey, class TData>
Record<TKey, TData>::Record(TKey key, TData data) {
    this->key = key;
    this->data = data;
}


#endif //HEAPTABLE_RECORD_H
