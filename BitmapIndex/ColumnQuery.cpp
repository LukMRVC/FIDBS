//
// Created by lukas on 13.04.22.
//

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
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
    auto query_set = QuerySet::getFromFile(query_file, schema->attrs_count);

    cColumnStoreTable columnTable(schema);
    auto dataLoadDuration = timeit([&data_file, &columnTable]() {
        columnTable.ReadFile(data_file);
    });

    auto throughput = getThroughput(columnTable.getRecordCount(), dataLoadDuration, 1000000);
    std::cout << "Column data load duration: " << dataLoadDuration << "s, " << throughput << "m op/s" << std::endl;

    cRowHeapTable rowHeapTable(schema);
    dataLoadDuration = timeit([&data_file, &rowHeapTable]() {
        rowHeapTable.ReadFile(data_file, true);
    });
    throughput = getThroughput(rowHeapTable.getRowCount(), dataLoadDuration, 1000000);
    std::cout << "Row data load duration: " << dataLoadDuration << "s, " << throughput << "m op/s" << std::endl;

    auto bitmapCreationDuration = timeit([&rowHeapTable] {
        if (!rowHeapTable.createBitmapIndex()) {
            throw std::runtime_error("Failed to create bitmap index");
        }
    });
    throughput = getThroughput(rowHeapTable.getRowCount(), bitmapCreationDuration, 1000000);
    std::cout << "BitmapIndex load duration: " << bitmapCreationDuration << "s, " << throughput << "m op/s" << std::endl;


    double constrained_duration = 0;
    double unconstrained_duration = 0;
    std::ofstream avg_output("./column_avg_results.txt");
    avg_output << std::fixed << std::setprecision(6);
    auto constrained_queries = 0;

    for (int i = 0; i < query_set->query_count; ++i) {
        auto query = query_set->get_query(i);
        auto is_constrained = columnTable.isQueryConstrained((const int8_t *) query) >= 0;
        auto duration = timeit([&columnTable, &query, &avg_output] {
            double average = columnTable.SelectAvg((const int8_t *) query);
            avg_output << average << "\n";
        });

        if (is_constrained) {
            constrained_duration += duration;
            constrained_queries += 1;
        } else {
            unconstrained_duration += duration;
        }
    }
    int unconstrained_queries = query_set->query_count - constrained_queries;
    printf("CONSTRAINED query count: %d \n", constrained_queries);
    printf("UNCONSTRAINED query count: %d \n", unconstrained_queries);

    printf("ColTable AVG CONSTRAINED duration: %.6f s, %.1f op/s.\n", constrained_duration,
           getThroughput(constrained_queries, constrained_duration, 1));
    printf("ColTable AVG UNCONSTRAINED duration: %.6f s, %.1f op/s.\n", unconstrained_duration,
           getThroughput(unconstrained_queries, unconstrained_duration, 1));
    avg_output.close();


    constrained_duration = 0;
    unconstrained_duration = 0;
    avg_output.open("./row_avg_results.txt");
    avg_output << std::fixed << std::setprecision(6);

    for (int i = 0; i < query_set->query_count; ++i) {
        auto query = query_set->get_query(i);
        auto is_constrained = rowHeapTable.isQueryConstrained(query);

        auto duration = timeit([&rowHeapTable, &query, &avg_output] {
            double average = rowHeapTable.SelectAvg(query);
            avg_output << average << "\n";
        });

        if (is_constrained) {
            constrained_duration += duration;
        } else {
            unconstrained_duration += duration;
        }
    }
    avg_output.close();
    printf("RowTable AVG CONSTRAINED duration: %.6f s, %.1f op/s.\n", constrained_duration,
           getThroughput(constrained_queries, constrained_duration, 1));
    printf("RowTable AVG UNCONSTRAINED duration: %.6f s, %.1f op/s.\n", unconstrained_duration,
           getThroughput(unconstrained_queries, unconstrained_duration, 1));


    constrained_duration = 0;
    unconstrained_duration = 0;
    avg_output.open("./index_avg_results.txt");
    avg_output << std::fixed << std::setprecision(6);

    for (int i = 0; i < query_set->query_count; ++i) {
        auto query = query_set->get_query(i);
        auto is_constrained = rowHeapTable.isQueryConstrained(query);

        auto duration = timeit([&rowHeapTable, &query, &avg_output] {
            double average = rowHeapTable.SelectAvgWithIndex(query);
            avg_output << average << "\n";
        });

        if (is_constrained) {
            constrained_duration += duration;
        } else {
            unconstrained_duration += duration;
        }
    }
    avg_output.close();
    printf("BitmapIndex AVG CONSTRAINED duration: %.6f s, %.1f op/s.\n", constrained_duration,
           getThroughput(constrained_queries, constrained_duration, 1));
    printf("BitmapIndex AVG UNCONSTRAINED duration: %.6f s, %.1f op/s.\n", unconstrained_duration,
           getThroughput(unconstrained_queries, unconstrained_duration, 1));

    return 0;
}
