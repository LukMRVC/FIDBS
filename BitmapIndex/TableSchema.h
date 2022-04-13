//
// Created by lukas on 16.03.22.
//

#ifndef BITMAPINDEX_TABLESCHEMA_H
#define BITMAPINDEX_TABLESCHEMA_H

#include <cstring>
#include <fstream>

class TableSchema {
public:
    unsigned int attrs_count = -1;
    unsigned int * attr_sizes = nullptr;
    char * data_types = nullptr;
    int * attr_max_values = nullptr;
    unsigned int * attribute_offsets = nullptr;
    unsigned int record_size = -1;

    void calculate_offsets();

    TableSchema();
    TableSchema(unsigned int, unsigned int *, int *);
    ~TableSchema();

    static TableSchema* getFromFile(const char * filename, bool with_data_types = false);

    uint32_t get_attr_size(char dataType, uint32_t = 0) const;
};


#endif //BITMAPINDEX_TABLESCHEMA_H
