//
// Created by lukas on 06.04.22.
//
#include "TableSchema.h"

TableSchema::TableSchema(unsigned int atrs, unsigned int * sizes, int * max_values) {
    attrs_count = atrs;
    attr_sizes = new unsigned int[attrs_count];
    std::memcpy(attr_sizes, sizes, attrs_count * sizeof (unsigned int));
    attr_max_values = new int[attrs_count];
    std::memcpy(attr_max_values, max_values, attrs_count * sizeof (int));
    calculate_offsets();
}

TableSchema::~TableSchema() {
    if (attr_sizes != nullptr) {
        delete [] attr_sizes;
        attr_sizes = nullptr;
    }
    if (attribute_offsets != nullptr) {
        delete [] attribute_offsets;
        attribute_offsets = nullptr;
    }
    if (attr_max_values != nullptr) {
        delete [] attr_max_values;
        attr_max_values = nullptr;
    }
    attrs_count = -1;
}

TableSchema *TableSchema::getFromFile(const char *filename) {
    constexpr int MAX_LEN = 1024;
    char line[MAX_LEN];
    std::ifstream schema(filename);
    schema.getline(line, MAX_LEN);
    int attrs_count;
    sscanf(line, "AttrCount:%d", &attrs_count);
    auto * const table_schema = new TableSchema();
    table_schema->attrs_count = attrs_count;
    table_schema->attr_sizes = new unsigned int [attrs_count];
    // attribute sizes
    schema.getline(line, MAX_LEN);
    char values[512];
    sscanf(line, "AttrSize:%s", values);
    char * tok = nullptr;
    tok = std::strtok(values, ",");
    int i = 0;
    while (tok != NULL) {
        table_schema->attr_sizes[i] = std::atoi(tok);
        tok = std::strtok(NULL, ",");
        i++;
    }

    // max values
    table_schema->attr_max_values = new int [attrs_count];
    schema.getline(line, MAX_LEN);
    sscanf(line, "AttrValueCount:%s", values);
    tok = std::strtok(values, ",");
    i = 0;
    while (tok != NULL) {
        table_schema->attr_max_values[i] = std::atoi(tok);
        tok = std::strtok(NULL, ",");
        i++;
    }
    table_schema->calculate_offsets();
    return table_schema;
}

void TableSchema::calculate_offsets() {
    if (attrs_count <= 0 || attribute_offsets != nullptr) {
        return;
    }

    attribute_offsets = new unsigned int [attrs_count];
    int offset = 0;
    for (int i = 0; i < attrs_count; ++i) {
        attribute_offsets[i] = offset;
        offset += attr_sizes[i];
    }
    record_size = offset;
}

TableSchema::TableSchema() = default;
