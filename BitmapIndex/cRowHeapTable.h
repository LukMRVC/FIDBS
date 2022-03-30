//
// Created by lukas on 02.03.22.
//

#ifndef BITMAPINDEX_CROWHEAPTABLE_H
#define BITMAPINDEX_CROWHEAPTABLE_H

#define NODE_SIZE 4096

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include "TableSchema.h"
#include "BitmapIndex.h"
#include "DataStructures/HashTable.h"

class cRowHeapTable {
private:
    char *mData = nullptr;
    unsigned int capacity = 0;
    unsigned int rowCount = 0;
    unsigned int rowSize = 0;
    unsigned int cols = 0;
    unsigned int *attributeSizes = nullptr;
    unsigned int *attributeOffsets = nullptr;
    const TableSchema * schema = nullptr;
    inline void reserve(unsigned int reserve_capacity) {
        capacity = reserve_capacity;
        this->mData = new char [reserve_capacity * rowSize];
    }

    BitmapIndex * bitmapIndex = nullptr;
    [[nodiscard]] inline char * getRowPointer(unsigned int rowId) const {
        return mData + rowId * rowSize;
    }

    cHashTable<int> * statistics = nullptr;
public:
    cHashTable<int> * hashIndex = nullptr;

    [[nodiscard]] const BitmapIndex * getBitmapIndex() const {
        return bitmapIndex;
    }

    cRowHeapTable(const int attrsSize[], unsigned int length, unsigned int capacity = 0);
    cRowHeapTable(const TableSchema*);
    ~cRowHeapTable();

    bool Insert(char * rec);
    bool ReadFile(const char * filename);
    unsigned int Select(unsigned int conditions[][2], std::size_t size) const;
    unsigned int Select(const char *) const;
    unsigned int SelectWithIndex(unsigned int conditions[][2], std::size_t size) const;
    unsigned int SelectWithIndex(const char *) const;
    bool Find(char *, Cursor<int> &) const;

    bool get(int rowId, char * data) const;
    static void generateRecord(char *rec, const int attr_sizes[], const int *attrs_max_value, int attr_count);

    bool createBitmapIndex(const int * attrs_max_value, int attr_count);
    bool createBitmapIndex();
    unsigned long getTotalByteSize() {
        return capacity * rowSize;
    }

    unsigned int getRowCount() const {
        return rowCount;
    }

    void createHashTableIndex(const int attr_order[]);
};

cRowHeapTable::~cRowHeapTable() {
    if (mData != nullptr) {
        delete[] mData;
        mData = nullptr;
    }

    if (attributeSizes != nullptr) {
        delete [] attributeSizes;
        attributeSizes = nullptr;
    }

    if (attributeOffsets != nullptr) {
        delete[] attributeOffsets;
        attributeOffsets = nullptr;
    }

    capacity = 0;
    rowCount = 0;
    rowSize = 0;
    cols = 0;

    if (bitmapIndex != nullptr) {
        delete bitmapIndex;
        bitmapIndex = nullptr;
    }

    if (statistics != nullptr) {
        delete statistics;
        statistics = nullptr;
    }
}

cRowHeapTable::cRowHeapTable(const int *attrsSize, unsigned int length, unsigned int capacity) {
    unsigned int size = 0;
    attributeSizes = new unsigned int[length];
    attributeOffsets = new unsigned int[length];
    cols = length;
    for (int i = 0; i < length; ++i) {
        attributeOffsets[i] = size;
        size += attrsSize[i];
        attributeSizes[i] = attrsSize[i];
    }

    rowSize = size;
    if (capacity > 0) {
        reserve(capacity);
    }
}

cRowHeapTable::cRowHeapTable(const TableSchema * schema) {
    cols = schema->attrs_count;
    rowSize = schema->record_size;
    attributeOffsets = new unsigned int[cols];
    attributeSizes = new unsigned int[cols];
    std::memcpy(attributeOffsets, schema->attribute_offsets, cols * sizeof(unsigned int));
    std::memcpy(attributeSizes, schema->attr_sizes, cols * sizeof(unsigned int));
    this->schema = schema;
}


bool cRowHeapTable::Insert(char *rec) {
    if (rowCount > capacity) {
        return false;
    }
    auto rowPointer = getRowPointer(rowCount);
    std::memcpy(rowPointer, rec, rowSize);
    rowCount += 1;
    return true;
}

unsigned int cRowHeapTable::Select(unsigned int conditions[][2], std::size_t size) const {
    auto found = 0;
    for (int i = 0; i < rowCount; ++i) {
        auto row = getRowPointer(i);
        for (int j = 0; j < size; ++j) {
            auto col = conditions[j][0];
            auto col_value = conditions[j][1];

            if (col_value < 0) {
                continue;
            }

            uint8_t * data = (uint8_t *) row + attributeOffsets[col];
            if (*data != col_value) {
                goto continue_outer;
            }
        }

        found += 1;
        continue_outer:;
    }
    return found;
}

unsigned int cRowHeapTable::Select(const char * query) const {
    auto found = 0;
    // maybe detect if the inner loop can start from bigger number
    // to speed up everything a bit

    for (int i = 0; i < rowCount; ++i) {
        auto row = getRowPointer(i);
        for (int j = 0; j < cols; ++j) {
            auto col_value = query[j];
            if (col_value < 0) {
                continue;
            }
            uint8_t * data = (uint8_t *) row + attributeOffsets[j];
            if (*data != col_value) {
                goto continue_outer;
            }
        }
        found += 1;
        continue_outer:;
    }
    return found;
}

unsigned int cRowHeapTable::SelectWithIndex(unsigned int conditions[][2], std::size_t size) const {
    return bitmapIndex->Select(conditions, size);
}

unsigned int cRowHeapTable::SelectWithIndex(const char * query) const {
    bool canUseHashStats = true;
    for (int i = 0; i < cols; ++i) {
        if (query[i] < 0 && schema->attr_max_values[i] > 0) {
            canUseHashStats = false;
            break;
        }
    }

    if (canUseHashStats) {
        int count;
        statistics->Select(query, count);
        return count;
    }
    return bitmapIndex->Select(query);
}

void cRowHeapTable::generateRecord(
        char *rec,
        const int attr_sizes[],
        const int * attrs_max_value,
        int attr_count
) {
    auto offset = 0;
    for (int c = 0; c < attr_count; ++c) {
        if (attrs_max_value[c] == -1) {
            for (int j = 0; j < attr_sizes[c]; ++j) {
                rec[offset + j] = rand() % 256;
            }
        } else {
            for (int j = 0; j < attr_sizes[c]; ++j) {
                rec[offset + j] = rand() % attrs_max_value[c];
            }
        }
        offset += attr_sizes[c];
    }
}

bool cRowHeapTable::createBitmapIndex(const int * attrs_max_value, int attr_count) {
    bitmapIndex = new BitmapIndex(attrs_max_value, attr_count, capacity);
    int i = 0;
    for (; i < rowCount; ++i) {
        const char * row = getRowPointer(i);
        bitmapIndex->createRecord(row, attributeSizes);
    }
    printf("Done creating bitmapIndex: %d rows created\n", i);
    return true;
}

bool cRowHeapTable::createBitmapIndex() {
    if (schema == nullptr || bitmapIndex != nullptr) {
        return false;
    }
    return createBitmapIndex(schema->attr_max_values, cols);
}

bool cRowHeapTable::ReadFile(const char * filename) {
    int line_offset;
    constexpr int MAX_LEN = 1024;
    char line[MAX_LEN];
    int records = 0;
    std::ifstream data(filename);

    data.getline(line, MAX_LEN);
    sscanf(line, "RowCount:%d", &records);
    if (capacity <= 0) {
        reserve(records);
    }

    while (!data.eof()) {
        line_offset = 0;
        data.getline(line, MAX_LEN);
        // skip empty line
        if (line[0] == 0) {
            continue;
        }

        auto rowPointer = getRowPointer(rowCount);
        for (int i = 0; i < cols; ++i) {
            if (attributeSizes[i] > 1) {
                std::memcpy(rowPointer + attributeOffsets[i], line + line_offset, attributeSizes[i]);
            } else {
                // C way to convert char to int
                rowPointer[attributeOffsets[i]] = line[line_offset] - '0';
            }
            line_offset += attributeSizes[i] + 1; // add 1 to offset comma
        }
        rowCount += 1;
    }
    data.close();

    return false;
}

void cRowHeapTable::createHashTableIndex(const int *attr_order) {
    if (hashIndex != nullptr) {
        return;
    }


    int * index_offsets = new int[cols];
    auto index_attributes = 0;
    for (int i = 0; i < cols; ++i) {
        if (attr_order[i] > 0) {
            index_offsets[attr_order[i] - 1] = attributeOffsets[i];
            index_attributes += 1;
        }
    }

    unsigned int keySize = 0;
    for (int i = 0; i < cols; ++i) {
        if (attributeSizes[i] == 1) {
            keySize += 1;
        }
    }

    hashIndex = new cHashTable<int>(rowCount, NODE_SIZE, keySize, schema->attr_max_values, cols);
    statistics = new cHashTable<int>(rowCount, NODE_SIZE, keySize, schema->attr_max_values, cols);
    char * key = new char[keySize];
    memset(key, 0, keySize);
    for (int i = 0; i < rowCount; ++i) {
        auto p = getRowPointer(i);
        for (int j = 0; j < index_attributes; ++j) {
            key[j] = *(p + index_offsets[j]);
        }
        hashIndex->Add(key, i);
        statistics->IncrementData(key);
        if (i % 100000 == 0) {
            printf("%d / %d\n", i, rowCount);
        }
    }

    delete [] key;
    delete [] index_offsets;
}

bool cRowHeapTable::get(int rowId, char * data) const {
    if (rowId > rowCount) {
        return false;
    }
    auto p = getRowPointer(rowId);
    std::memcpy(data, p, rowSize);
    return true;
}

bool cRowHeapTable::Find(char * key, Cursor<int> & cursor) const {
    int dataCount = 0;
    auto wasFound = statistics->Find(key, dataCount);
    if (wasFound && dataCount <= 50) {
        return hashIndex->Find(key, cursor);
    }

    return false;
}

#endif //BITMAPINDEX_CROWHEAPTABLE_H
