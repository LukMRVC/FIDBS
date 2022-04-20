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
    auto query_set = QuerySet::getFromFile(query_file, schema->attrs_count);

    cColumnStoreTable columnTable(schema);
    auto dataLoadDuration = timeit([&data_file, &columnTable]() {
        columnTable.ReadFile(data_file);
    });

    std::cout << "Data load duration: " << dataLoadDuration << "s" << std::endl;

    char sql_query[512];
    memset(sql_query, 0, 512);

    for (int i = 0; i < query_set->query_count; ++i) {
        auto query = query_set->get_query(i);
        auto col = query[0];
        auto offset = 0;
        offset = sprintf(sql_query, "SELECT AVG(a%d) FROM fidbs_col_table WHERE ", col + 1);

        for (int s = 1; s < schema->attrs_count + 1; ++s) {
            if (query[s] >= 0) {
                offset += sprintf(sql_query + offset, "a%d = %d AND ", s, query[s]);
            }
        }

        printf("%s\n", sql_query);

        auto avgDuration = timeit([&columnTable, col, &query] {
            double average = columnTable.SelectAvg((const int8_t *)query);
            printf("Col %d AVG: %.6f \n", col, average);
        });
        printf("AVG(a%d) duration: %.4f \n", col, avgDuration);
    }


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
