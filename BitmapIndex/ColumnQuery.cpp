//
// Created by lukas on 13.04.22.
//

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include "TableSchema.h"
#include "QuerySet.h"
#include "cColumnStoreTable.h"
#include "cRowHeapTable.h"

inline double getThroughput(int opsCount, double period, int unit) {
    return (((double) opsCount / (double) unit)) / period;
}

template<class F, class ... Args>
double timeit(F func, Args &&...args) {
    auto start = std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}

int main(int args_count, char *args[]) {
    if (args_count < 4) {
        std::cerr << "Program must be called with 3 args: [schema file] [data file] [query file]" << std::endl;
        return -1;
    }
    char const *const schema_file = args[1];
    char const *const data_file = args[2];
    char const *const query_file = args[3];

    const TableSchema *schema = TableSchema::getFromFile(schema_file, true);
    auto query_set = QuerySet::getFromFile(query_file, schema->attrs_count);

    cColumnStoreTable columnTable(schema);
    auto dataLoadDuration = timeit([&data_file, &columnTable]() {
        columnTable.ReadFile(data_file);
    });
    std::cout << "Column data load duration: " << dataLoadDuration << "s" << std::endl;

    cRowHeapTable rowHeapTable(schema);
    dataLoadDuration = timeit([&data_file, &rowHeapTable]() {
        rowHeapTable.ReadFile(data_file, true);
    });
    std::cout << "Row data load duration: " << dataLoadDuration << "s" << std::endl;

    auto rowAvgDuration = timeit([&rowHeapTable, &query_set] {
        for (int i = 0; i < query_set->query_count; ++i) {
            auto query = query_set->get_query(i);
            auto avg = rowHeapTable.SelectAvg(query);
            printf("RowTable AVG(a%d): %.6f \n", query[0], avg);
        }
    });
    printf("RowTable AVG duration: %.4f \n", rowAvgDuration);

    auto colAvgDuration = timeit([&columnTable, &query_set] {
        for (int i = 0; i < query_set->query_count; ++i) {
            auto query = query_set->get_query(i);
            auto col = query[0];
            double average = columnTable.SelectAvg((const int8_t *)query);
            printf("ColTable AVG(a%d): %.6f \n", col, average);
        }
    });

    printf("ColTable AVG duration: %.4f \n", colAvgDuration);



    return 0;
}
