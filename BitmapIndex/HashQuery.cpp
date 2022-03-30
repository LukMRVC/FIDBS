//
// Created by lukas on 24.03.22.
//

#include "cRowHeapTable.h"
#include "TableSchema.h"
#include "QuerySet.h"
#include "DataStructures/HashTable.h"
#include "DataStructures/Cursor.h"
#include <iostream>
#include <chrono>

template<class F, class ... Args>
double timeit(F func, Args &&...args) {
    auto start = std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}

inline double getThroughput(int opsCount, double period, int unit) {
    return (((double) opsCount / (double) unit)) / period;
}

int main(int args_count, char *args[]) {
    if (args_count < 4) {
        std::cerr << "Program must be called with 3 args: [schema file] [data file] [query file]" << std::endl;
        return -1;
    }

    char const *const schema_file = args[1];
    char const *const data_file = args[2];
    char const *const query_file = args[3];
    const TableSchema *schema = TableSchema::getFromFile(schema_file);
    auto query_set = QuerySet::getFromFile(query_file, schema->attrs_count);

    // Create Row Heap Table
    cRowHeapTable heapTable(schema);
    auto dataLoadDuration = timeit([&data_file, &heapTable]() {
        heapTable.ReadFile(data_file);
    });

    std::cout << "Data load duration: " << dataLoadDuration << "s, loaded: " << heapTable.getRowCount() << " records." << std::endl;
    int * attr_pos = new int[schema->attrs_count];
    auto pos = 1;
    for (int i = 0; i < schema->attrs_count; ++i) {
        if (schema->attr_max_values[i] > 0) {
            attr_pos[i] = pos++;
        } else {
            attr_pos[i] = -1;
        }
    }
    char * key = new char [pos];
    heapTable.createBitmapIndex();
    heapTable.createHashTableIndex(attr_pos);

    Cursor<int> cursor;
    int rowId;
    char *record = new char[schema->record_size + 1];
    record[schema->record_size] = 0;
    for (int i = 0; i < query_set->query_count; ++i) {
        auto query = query_set->get_query(i);
        auto found = heapTable.SelectWithIndex(query);
        printf("%d\n", found);


//        if (heapTable.Find(key, cursor)) {
//            while (cursor.nextRecord(rowId)) {
//                heapTable.get(rowId, record);
//                for (int i = 0; i < schema->attrs_count; ++i) {
//                    if (schema->attr_sizes[i] > 1) {
//                        for (int j = 0; j < schema->attr_sizes[i]; ++j) {
//                            printf("%c", (record + schema->attribute_offsets[i])[j]);
//                        }
//                        printf(";");
//                    } else {
//                        printf("%d;", *((uint8_t *) (record + schema->attribute_offsets[i])));
//                    }
//                }
//                printf("\n");
//            }
//        }
//        continue_loop:;
    }

    delete [] attr_pos;
    delete [] key;
    delete [] record;

    return 0;
}