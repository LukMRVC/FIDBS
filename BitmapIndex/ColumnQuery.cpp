//
// Created by lukas on 13.04.22.
//

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <iomanip>
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
    auto query_set = QuerySet::getFromFile(query_file, schema->attrs_count, true);

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
    std::ofstream output("./row_count_results.txt");
    auto rowSelectCountDuration = timeit([&rowHeapTable, &output, &query_set] {
        for (int i = 0; i < query_set->query_count; ++i) {
            auto query = query_set->get_query(i);
            auto count = rowHeapTable.Select(query + 1);
            output << count << "\n";
        }
    });
    output.close();
    printf("Row COUNT(*) duration: %.4f s\n", rowSelectCountDuration);

    rowHeapTable.createBitmapIndex();

    output.open("./index_count_results.txt");
    auto bitmapIndexCountDuration = timeit([&rowHeapTable, &output, &query_set] {
        for (int i = 0; i < query_set->query_count; ++i) {
            auto query = query_set->get_query(i);
            auto count = rowHeapTable.SelectWithIndex(query + 1);
            output << count << "\n";
        }
    });
    output.close();
    printf("Index COUNT(*) duration: %.4f \n", bitmapIndexCountDuration);

    output.open("./row_avg_results.txt");
    output << std::fixed << std::setprecision(6);
    auto rowAvgDuration = timeit([&rowHeapTable, &output, &query_set] {
        for (int i = 0; i < query_set->query_count; ++i) {
            auto query = query_set->get_query(i);
            auto avg = rowHeapTable.SelectAvg(query);
            output << avg << "\n";
//            printf("RowTable AVG(a%d): %.6f \n", query[0], avg);
        }
    });
    printf("RowTable AVG duration: %.4f s \n", rowAvgDuration);
    output.close();

    output.open("./col_avg_results.txt");
    auto colAvgDuration = timeit([&columnTable, &output, &query_set] {
        for (int i = 0; i < query_set->query_count; ++i) {
            auto query = query_set->get_query(i);
            auto average = columnTable.SelectAvg((const int8_t *)query);
            output << average << "\n";
            //            printf("ColTable AVG(a%d): %.6f \n", col, average);
        }
    });
    output.close();
    printf("ColTable AVG duration: %.4f s \n", colAvgDuration);

    return 0;
}
