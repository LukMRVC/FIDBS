//
// Created by lukas on 13.04.22.
//

#include "cColumnStoreTable.h"
#include <vector>
#include <stdexcept>
#include <functional>
#include <limits>

bool cColumnStoreTable::reserve(uint32_t max_capacity) {
    if (mData != nullptr || schema == nullptr || recordSize == 0 || column_offsets == nullptr) return false;

    uint32_t offset_sum = 0;
    for (int i = 0; i < schema->attrs_count; ++i) {
        column_offsets[i] = offset_sum;
        offset_sum += max_capacity * schema->attr_sizes[i];
    }

    capacity = max_capacity;
    mData = new uint8_t [capacity * recordSize];
    return true;
}

cColumnStoreTable::cColumnStoreTable(const TableSchema *table_schema, uint32_t max_capacity) {
    schema = table_schema;
    column_offsets = new uint32_t[schema->attrs_count];
    column_offsets[0] = 0;
    for (int i = 0; i < schema->attrs_count; ++i) {
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

double cColumnStoreTable::SelectAvg(const int8_t *query) const {
    if (column_offsets == nullptr) {
        throw std::runtime_error("Column offsets were not set!");
    }

    size_t col_avg = query[0];
    auto found_rows = 0;
    auto pointer = get_col_pointer(col_avg);
    auto averages = std::vector<float>();
    // get attribute size in bytes
    auto attr_size = schema->attr_sizes[col_avg];
    std::function<float(const uint8_t *)> converter;
    if (schema->data_types[col_avg] == 'F') {
        converter = convert_float;
    } else {
        throw std::runtime_error("Calculating average on non-float data");
    }

    bool is_constrained = false;
    size_t constrained_col = 0;
    for (int i = 1; i < schema->attrs_count + 1; ++i) {
        if (query[i] >= 0) {
            constrained_col = i;
            is_constrained = true;
            break;
        }
    }

    auto doubleMax = std::numeric_limits<float>::max();
    float converted = 0;
    float avg = 0;
    if (!is_constrained) {
        found_rows = recordCount;
        for (int i = 0; i < recordCount; ++i) {
            converted = converter(pointer);
            if (avg + converted > doubleMax) {
                averages.emplace_back(avg);
                avg = 0;
            }
            avg += converted;
            pointer += attr_size;
        }
        averages.emplace_back(avg);
    } else {
        // constrained col is + 1 offsetted because of query, which as +1 attribute
        pointer = get_col_pointer(constrained_col - 1);
        for (int i = 0; i < recordCount; ++i) {
            if (pointer[i] == query[constrained_col]) {
                for (int j = constrained_col + 1; j < schema->attrs_count + 1; ++j) {
                    if (query[j] >= 0) {
                        pointer = get_col_pointer(j - 1) + i * schema->attr_sizes[j - 1];
                        if (*((uint8_t *) pointer) != query[j]) {
                            goto get_next;
                        }
                    }
                }
                converted = converter(get_col_pointer(col_avg) + i * attr_size);
                if (avg + converted > doubleMax) {
                    averages.emplace_back(avg);
                    avg = 0;
                }
                avg += converted;
                found_rows += 1;
                get_next:
                pointer = get_col_pointer(constrained_col - 1);
            }
        }
    }

    if (averages.size() > 1) {
        float totalAvg = averages[0];
        for (int i = 1; i < averages.size() - 1; ++i) {
            auto n = i + 1;
            totalAvg = ((n - 1) * totalAvg + averages[n - 1]) / n;
        }

        return totalAvg;
    }

    return avg / (float)found_rows;
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

    int32_t load_int;
    float load_float;
    int32_t bytes_read;

    while (!data.eof()) {
        line_offset = 0;
        data.getline(line, MAX_LEN);
        // skip empty line
        if (line[0] == 0) {
            continue;
        }

        for (int i = 0; i < schema->attrs_count; ++i) {
            auto colPointer = get_col_pointer(i) + (schema->attr_sizes[i] * recordCount);
            if (schema->data_types[i] == 'C') {
                std::memcpy(colPointer, line + line_offset, schema->attr_sizes[i]);
                bytes_read = schema->attr_sizes[i] + 1; // add one for semicolon
            } else if (schema->attr_sizes[i] == 1) {
                *(uint8_t *)colPointer = line[line_offset] - '0';
                bytes_read = 2; // byte value plus a semilocon
            } else if (schema->data_types[i] == 'I') {
                sscanf(line + line_offset, "%d;%n", &load_int, &bytes_read);
                *(int *)colPointer = load_int;
            } else {
                sscanf(line + line_offset, "%f;%n", &load_float, &bytes_read);
                *(float *)colPointer = load_float;
            }
            line_offset += bytes_read;
        }
        recordCount += 1;
    }
    data.close();

    return true;
}

//float cColumnStoreTable::convert_int_32t(const uint8_t * pointer) {
//    return *(int32_t *)pointer;
//}

float cColumnStoreTable::convert_float(const uint8_t * pointer) {
    return *(float *)pointer;
}

