//
// Created by lukas on 13.04.22.
//

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include "TableSchema.h"
#include "QuerySet.h"
#include "cRowHeapTable.h"
#include <thread>

const int thread_count = 2;
QuerySet * query_set;
unsigned int *results;
cRowHeapTable *rowHeapTable;

void SelectCountThread(int threadId) {
    for (int i = threadId; i < query_set->query_count; i += thread_count) {
        auto query = query_set->get_query(i);
        auto found = rowHeapTable->Select(query);
        results[i] = found;
    }
}

void SelectCountParallel() {
    std::thread threads[thread_count - 1];
    for (int i = 0; i < thread_count - 1; ++i) {
        threads[i] = std::thread(SelectCountThread, i + 1);
    }
    SelectCountThread(0);

    for (int i = 0; i < thread_count - 1; ++i) {
        threads[i].join();
    }
}

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

    const TableSchema *schema = TableSchema::getFromFile(schema_file, false);
    query_set = QuerySet::getFromFile(query_file, schema->attrs_count, false);

    rowHeapTable = new cRowHeapTable(schema);
    auto dataLoadDuration = timeit([&data_file] {
        rowHeapTable->ReadFile(data_file, false);
    });
    auto throughput = getThroughput(rowHeapTable->getRowCount(), dataLoadDuration, 1000000);
    std::cout << "Row data load duration: " << dataLoadDuration << "s, " << throughput << "m op/s" << std::endl;

    results = new unsigned int[query_set->query_count];

    auto selectDuration = timeit([] {
        SelectCountParallel();
    });

    std::ofstream output("./parallel_select.txt");

    for (int i = 0; i < query_set->query_count; ++i) {
        output << results[i] << std::endl;
    }

    output.close();

    delete [] results;

    throughput = getThroughput(rowHeapTable->getRowCount(), selectDuration, 1);
    std::cout << "Parallel Select duration: " << selectDuration << "s, " << throughput << " op/s" << std::endl;

    return 0;
}