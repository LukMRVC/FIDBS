//
// Created by lukas on 13.04.22.
//

#ifndef BITMAPINDEX_CCOLUMNSTORETABLE_H
#define BITMAPINDEX_CCOLUMNSTORETABLE_H

#include <cstdint>
#include "TableSchema.h"

class cColumnStoreTable {
private:
    uint32_t recordCount = 0;
    uint32_t capacity = 0;
    uint32_t recordSize = 0;
    uint8_t * mData = nullptr;
    uint32_t * column_offsets = nullptr;
    const TableSchema * schema = nullptr;
    [[nodiscard]] uint8_t * get_col_pointer(size_t column) const {
        return mData + column_offsets[column];
    }


public:
    explicit cColumnStoreTable(const TableSchema *, uint32_t = 0);
    ~cColumnStoreTable();
    bool reserve(uint32_t capacity);

    double SelectAvg(const uint8_t * query) const;
};

#endif //BITMAPINDEX_CCOLUMNSTORETABLE_H
