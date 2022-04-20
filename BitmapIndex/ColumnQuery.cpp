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
//    auto query_set = QuerySet::getFromFile(query_file, schema->attrs_count);

    cColumnStoreTable columnTable(schema);
    auto dataLoadDuration = timeit([&data_file, &columnTable]() {
        columnTable.ReadFile(data_file);
    });

    std::cout << "Data load duration: " << dataLoadDuration << "s" << std::endl;

    char query[21];
    query[0] = 5;
    for (int i = 1; i < 21; ++i) {
        query[i] = -1;
    }

    auto avgDuration = timeit([&columnTable, &query] {
        double average = columnTable.SelectAvg((const uint8_t *)query);
        printf("Col 0 AVG: %.3f \n", average);
    });
    std::cout << "AVG(a0) duration: " << avgDuration << "s" << std::endl;

//    query[0] = 0;
//    auto avg6Duration = timeit([&columnTable, &query] {
//        double average = columnTable.SelectAvg((const uint8_t *)query);
//        printf("Col 6 AVG: %.3f \n", average);
//    });
//    std::cout << "AVG(a6) duration: " << avg6Duration << "s" << std::endl;

    /*for (int i = 0; i < query_set->query_count; ++i) {
        auto query = query_set->get_query(i);

        double average = columnTable.SelectAvg((const uint8_t *)query);
        printf("%.2f\n", average);
    }*/

    return 0;
}
