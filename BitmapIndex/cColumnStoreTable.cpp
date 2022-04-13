//
// Created by lukas on 13.04.22.
//

#include "cColumnStoreTable.h"
#include <vector>

bool cColumnStoreTable::reserve(uint32_t max_capacity) {
    if (mData != nullptr || schema == nullptr || recordSize == 0) return false;

    capacity = max_capacity;
    mData = new uint8_t [capacity * recordSize];
    return true;
}

cColumnStoreTable::cColumnStoreTable(const TableSchema *table_schema, uint32_t max_capacity) {
    schema = table_schema;
    column_offsets = new uint32_t[schema->attrs_count];
    column_offsets[0] = 0;
    for (int i = 0; i < schema->attrs_count; ++i) {
        // TODO: Is this the correct way to get the column offset?
        column_offsets[i] = i * recordSize;
        if (schema->data_types[i] == 'C') {
            recordSize += schema->attr_sizes[i];
        } else if (schema->data_types[i] == 'F' || schema->data_types[i] == 'I') {
            recordSize += 4;
        } else {
            // must be a byte value
            recordSize += 1;
        }
    }

    if (max_capacity > 0) {
        capacity = max_capacity;
        reserve(capacity);
    }
}

cColumnStoreTable::~cColumnStoreTable() {
    if (mData != nullptr) {
        delete [] mData;
        mData = nullptr;
    }

    if (column_offsets != nullptr) {
        delete [] column_offsets;
        column_offsets = nullptr;
    }
}

double cColumnStoreTable::SelectAvg(const uint8_t *query) const {
    size_t col_avg = query[0];
    auto pointer = get_col_pointer(col_avg);
    auto averages = std::vector<double>();
    const int avg_count_max = 1000000;
    averages.reserve( capacity / avg_count_max + 2);
    int iteration = 0;
    // get attribute size in bytes
    auto attr_size = 0;
    if (schema->data_types[col_avg] == 'C') {
        attr_size = schema->attr_sizes[col_avg];
    } else if (schema->data_types[col_avg] == 'F' || schema->data_types[col_avg] == 'I') {
        attr_size = 4;
    } else {
        // must be a byte value
        attr_size = 1;
    }

    do {
        double avg = 0;
        auto loopStop = recordCount - iteration * avg_count_max > avg_count_max ? avg_count_max : recordCount - iteration * avg_count_max;
        for (int i = 0; i < loopStop; ++i) {
            avg += (*(double *)pointer);
            pointer +=  attr_size;
        }
        averages.emplace_back(avg);
        iteration += 1;
    } while (iteration * avg_count_max < recordCount);

    double totalAvg = averages[0];
    for (int i = 1; i < averages.size() - 1; ++i) {
        auto n = i + 1;
        totalAvg = ((n - 1) * totalAvg + averages[n - 1]) / n;
    }

    return totalAvg;
}
