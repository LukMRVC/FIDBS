//
// Created by lukas on 06.04.22.
//

#include "cRowHeapTable.h"
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <vector>
#include <limits>

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

bool cRowHeapTable::canUseHashIndex(const char * query) const {
    for (int i = 0; i < cols; ++i) {
        if (query[i] < 0 && schema->attr_max_values[i] > 0) {
            return false;
        }
    }
    return true;
}

unsigned int cRowHeapTable::SelectWithIndex(const char * query) const {
    if (hashIndex != nullptr && statistics != nullptr) {
        bool canUseHashStats = canUseHashIndex(query);

        if (canUseHashStats) {
            int count = 0;
            statistics->Select(query, count);
            return count;
        }
    }

    if (bitmapIndex == nullptr) {
        return -1;
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
    return true;
}

bool cRowHeapTable::createBitmapIndex() {
    if (schema == nullptr || bitmapIndex != nullptr) {
        return false;
    }
    return createBitmapIndex(schema->attr_max_values, cols);
}

bool cRowHeapTable::ReadFile(const char * filename, bool with_data_types) {
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
        int load_int;
        float load_float;
        int bytes_read;

        auto rowPointer = getRowPointer(rowCount);
        if (with_data_types) {
            for (int i = 0; i < cols; ++i) {
                if (schema->data_types[i] == 'C') {
                    std::memcpy(rowPointer, line + line_offset, schema->attr_sizes[i]);
                    bytes_read = schema->attr_sizes[i] + 1;
                } else if (schema->attr_sizes[i] == 1) {
                    *(uint8_t *) rowPointer = line[line_offset] - '0';
                    bytes_read = 2;
                } else if (schema->data_types[i] == 'I') {
                    sscanf(line + line_offset, "%d;%n", &load_int, &bytes_read);
                    *(int *) rowPointer = load_int;
                } else {
                    sscanf(line + line_offset, "%f;%n", &load_float, &bytes_read);
                    *(float *) rowPointer = load_float;
                }
                line_offset += bytes_read; // add 1 to offset comma
            }
        } else {
            for (int i = 0; i < cols; ++i) {
                if (attributeSizes[i] > 1) {
                    std::memcpy(rowPointer + attributeOffsets[i], line + line_offset, attributeSizes[i]);
                } else {
                    // C way to convert char to int
                    rowPointer[attributeOffsets[i]] = line[line_offset] - '0';
                }
                line_offset += attributeSizes[i] + 1; // add 1 to offset comma
            }
        }
        rowCount += 1;
    }
    data.close();

    return true;
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

bool cRowHeapTable::Find(const char * query, Cursor<int> & cursor) const {
    if (hashIndex == nullptr || !canUseHashIndex(query)) {
        return false;
    }
    return hashIndex->Select(query, cursor);
}

float cRowHeapTable::SelectAvg(const char * query) const {
    auto found = 0;
    size_t col_avg = query[0];
    auto averages = std::vector<float>();
    if (schema->data_types[col_avg] != 'F') {
        throw std::runtime_error("Calculating average on non-float data");
    }

    float avg = 0 ;
    float converted = 0;
    auto float_max = std::numeric_limits<float>::max();

    bool is_constrained = false;
    size_t constrained_col = 0;
    for (int i = 1; i < schema->attrs_count + 1; ++i) {
        if (query[i] >= 0) {
            constrained_col = i;
            is_constrained = true;
            break;
        }
    }

    if (!is_constrained) {
        found = rowCount;
        for (int i = 0; i < rowCount; ++i) {
            auto row = getRowPointer(i);
            converted = *((float *) (row + attributeOffsets[col_avg]));
            if (avg + converted > float_max) {
                averages.emplace_back(avg);
                avg = 0;
            }
            avg += converted;
        }
        averages.emplace_back(avg);
    } else {
        for (int i = 0; i < rowCount; ++i) {
            auto row = getRowPointer(i);
            for (int j = 0; j < cols; ++j) {
                auto col_value = query[j + 1];
                if (col_value < 0) {
                    continue;
                }
                uint8_t * data = (uint8_t *) row + attributeOffsets[j];
                if (*data != col_value) {
                    goto continue_outer;
                }
            }
            found += 1;
            converted = *((float *) (row + attributeOffsets[col_avg]));
            if (avg + converted > float_max) {
                averages.emplace_back(avg);
                avg = 0;
            }
            avg += converted;
            continue_outer:;
        }
        averages.emplace_back(avg);
    }

    if (averages.size() > 1) {
        float totalAvg = averages[0];
        for (int i = 1; i < averages.size() - 1; ++i) {
            auto n = i + 1;
            totalAvg = ((n - 1) * totalAvg + averages[n - 1]) / n;
        }

        return totalAvg;
    }

    return avg / (float)found;
}
