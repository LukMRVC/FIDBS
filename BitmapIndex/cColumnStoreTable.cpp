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
        recordSize += schema->attr_sizes[i];
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
    auto attr_size = schema->attr_sizes[col_avg];

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

bool cColumnStoreTable::ReadFile(const char *filename) {
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

        for (int i = 0; i < schema->attrs_count; ++i) {
            auto colPointer = get_col_pointer(i) + (schema->attr_sizes[i] * recordCount);
            if (schema->attr_sizes[i] > 1) {
                std::memcpy(colPointer, line + line_offset, schema->attr_sizes[i]);
            } else {
                *colPointer = line[line_offset];
            }
            line_offset += schema->attr_sizes[i];
        }
        recordCount += 1;
    }
    data.close();

    return true;
}

