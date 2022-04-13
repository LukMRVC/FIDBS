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

void PrintRecord(char * record, const TableSchema * schema) {
    for (int i = 0; i < schema->attrs_count; ++i) {
        if (schema->attr_sizes[i] > 1) {
            for (int j = 0; j < schema->attr_sizes[i]; ++j) {
                printf("%c", (record + schema->attribute_offsets[i])[j]);
            }
            printf(";");
        } else {
            printf("%d;", *((uint8_t *) (record + schema->attribute_offsets[i])));
        }
    }
    printf("\n");
}

void PrintRecord(char * record, const TableSchema * schema, std::ofstream & outstream) {
    char outputBuffer[256];
    memset(outputBuffer, 0, 256);
    auto outputOffset = 0;
    for (int i = 0; i < schema->attrs_count; ++i) {
        if (schema->attr_sizes[i] > 1) {
            for (int j = 0; j < schema->attr_sizes[i]; ++j) {
                sprintf(outputBuffer + outputOffset, "%c", (record + schema->attribute_offsets[i])[j]);
                outputOffset += 1;
            }
            sprintf(outputBuffer + outputOffset, ";");
            outputOffset += 1;
        } else {
            sprintf(outputBuffer + outputOffset, "%d;", *((uint8_t *) (record + schema->attribute_offsets[i])));
            outputOffset += 2;
        }
    }
    outstream << outputBuffer << "\n";
}

int main(int args_count, char *args[]) {
    if (args_count < 4) {
        std::cerr << "Program must be called with 3 args: [schema file] [data file] [query file]" << std::endl;
        return -1;
    }
    const int max_select_rows = 50;
    constexpr size_t outputbufsize = 256 * 1024;
    char buf[outputbufsize];
    char select_buf[outputbufsize];

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

    std::cout << "Running queries without bitmapIndex" << std::endl;
    // Select COUNT(*) NO INDEX
    std::ofstream output("./query_results.txt");
    output.rdbuf()->pubsetbuf(buf, outputbufsize);
    auto selectDuration = timeit([&query_set, &heapTable, &output]() {
        for (int i = 0; i < query_set->query_count; ++i) {
            auto query = query_set->get_query(i);
            auto found = heapTable.Select(query);
            output << found << "\n";
        }
    });
    output.close();
    auto throughput = getThroughput(query_set->query_count, selectDuration, 1);
    std::cout << "NO INDEX COUNT(*) duration: " << selectDuration << "s " << throughput << " op/s." << std::endl;


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
    auto bitmapIndexCreationDuration = timeit([&heapTable] {
        heapTable.createBitmapIndex();
    });

    std::cout << "Created bitmap index in: " << bitmapIndexCreationDuration << "s" << std::endl;

    output.open("./query_bitmap_index_results.txt");
    auto indexSelectDuration = timeit([&query_set, &heapTable, &output]() {
        for (int i = 0; i < query_set->query_count; ++i) {
            auto query = query_set->get_query(i);
            auto found = heapTable.SelectWithIndex(query);
            output << found << "\n";
        }
    });
    output.close();
    throughput = getThroughput(query_set->query_count, indexSelectDuration, 1);
    std::cout << "BITMAP INDEX COUNT(*) duration: " << indexSelectDuration << "s " << throughput << " op/s."<< std::endl;


    auto hashIndexCreationDuration = timeit([&heapTable, &attr_pos] {
        heapTable.createHashTableIndex(attr_pos);
    });
    std::cout << "Created hashIndex in: " << hashIndexCreationDuration << "s" << std::endl;

    output.open("./hash_count_query_results.txt");
    std::ofstream select_output("./hash_select_query_results.txt");
    output.rdbuf()->pubsetbuf(buf, outputbufsize);
    select_output.rdbuf()->pubsetbuf(select_buf, outputbufsize);
    Cursor<int> cursor;
    int rowId;
    char *record = new char[schema->record_size + 1];
    record[schema->record_size] = 0;
    auto startTime = std::chrono::high_resolution_clock::now();
    double totalSelectDuration = 0;
    auto totalTimesUsedHashIndex = 0;
    for (int i = 0; i < query_set->query_count; ++i) {
        auto query = query_set->get_query(i);
        if (heapTable.canUseHashIndex(query)) {
            totalTimesUsedHashIndex += 1;
        }
        auto found = heapTable.SelectWithIndex(query);
        output << found << "\n";
        if (found <= max_select_rows) {
            auto selectDuration = timeit([&heapTable, &cursor, &rowId, &record, &query, &select_output, &schema] {
                if (heapTable.Find(query, cursor)) {
                    while (cursor.nextRecord(rowId)) {
                        heapTable.get(rowId, record);
                        PrintRecord(record, schema, select_output);
                    }
                }
            });
            totalSelectDuration += selectDuration;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto selectCountDuration = std::chrono::duration_cast<std::chrono::duration<double>>(end - startTime).count() - totalSelectDuration;
    std::cout << "COMPOUND INDEX COUNT(*) duration: " << selectCountDuration << "s" << std::endl;
    std::cout << "COMPOUND INDEX SELECT(*) duration: " << totalSelectDuration << "s" << std::endl;
    printf("Used HASH Index: %d / %d \n", totalTimesUsedHashIndex, query_set->query_count);

    output.close();
    select_output.close();

    delete [] attr_pos;
    delete [] key;
    delete [] record;

    return 0;
}