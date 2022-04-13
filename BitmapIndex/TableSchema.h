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
};


#endif //BITMAPINDEX_TABLESCHEMA_H
