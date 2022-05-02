//
// Created by lukas on 02.03.22.
//

#ifndef BITMAPINDEX_CROWHEAPTABLE_H
#define BITMAPINDEX_CROWHEAPTABLE_H

#define NODE_SIZE 4096

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

    static int loadBytes(char * into, char * from, unsigned int max_len);
    static int loadFloat(char * into, char * from, unsigned int max_len);
    static int loadInt(char * into, char * from, unsigned int max_len);
    static int loadByte(char * into, char * from, unsigned int max_len);

public:
    bool canUseHashIndex(const char * query) const;
    cHashTable<int> * hashIndex = nullptr;

    [[nodiscard]] const BitmapIndex * getBitmapIndex() const {
        return bitmapIndex;
    }

    cRowHeapTable(const int attrsSize[], unsigned int length, unsigned int capacity = 0);
    cRowHeapTable(const TableSchema*);
    ~cRowHeapTable();

    bool Insert(char * rec);
    bool ReadFile(const char * filename, bool with_data_types = false);
    unsigned int Select(unsigned int conditions[][2], std::size_t size) const;
    unsigned int Select(const char *) const;
    float SelectAvg(const char *) const;
    float SelectAvgWithIndex(const char *) const;
    unsigned int SelectWithIndex(unsigned int conditions[][2], std::size_t size) const;
    unsigned int SelectWithIndex(const char *) const;
    bool Find(const char *, Cursor<int> &) const;
    bool isQueryConstrained(const char *) const;

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


#endif //BITMAPINDEX_CROWHEAPTABLE_H
