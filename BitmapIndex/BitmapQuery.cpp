//
// Created by lukas on 16.03.22.
//

#include "cRowHeapTable.h"
#include "TableSchema.h"
#include "QuerySet.h"
#include <iostream>
#include <chrono>

template<class F, class ... Args>
double timeit(F func, Args && ...args) {
    auto start = std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}

inline double getThroughput(int opsCount, double period, int unit) {
    return  (( (double)opsCount / (double)unit )) / period;
}

/*
 * Args
 * schema file
 * data file
 * query file
 */
int main(int args_count, char *args[]) {
    if (args_count < 4) {
        std::cerr << "Program must be called with 3 args: [schema file] [data file] [query file]" << std::endl;
        return -1;
    }

    constexpr size_t outputbufsize = 256 * 1024;
    char buf[outputbufsize];

    char const * const schema_file = args[1];
    char const * const data_file = args[2];
    char const * const query_file = args[3];
    const TableSchema * schema = TableSchema::getFromFile(schema_file);

    // Create Row Heap Table
    cRowHeapTable heapTable(schema);
    auto dataLoadDuration = timeit([&data_file, &heapTable]() {
      heapTable.ReadFile(data_file);
    });

    std::cout << "Data load duration: " << dataLoadDuration << "s, loaded: " << heapTable.getRowCount() << " records." << std::endl;

    auto query_set = QuerySet::getFromFile(query_file, schema->attrs_count);

    std::cout << "Running queries without bitmapIndex" << std::endl;
    // Select
    std::ofstream output("./query_results.txt");
    output.rdbuf()->pubsetbuf(buf, outputbufsize);
//    auto selectDuration = timeit([&query_set, &heapTable, &output]() {
//        for (int i = 0; i < query_set->query_count; ++i) {
//            auto query = query_set->get_query(i);
//            auto found = heapTable.Select(query);
//            output << found << "\n";
//        }
//    });
    output.close();
//    auto throughput = getThroughput(query_set->query_count, selectDuration, 1);
//    std::cout << "Querying duration: " << selectDuration << "s " << throughput << " op/s." << std::endl;
    auto throughput = 0;
    auto indexCreateDuration = timeit([&heapTable]() {
        heapTable.createBitmapIndex();
    });
    std::cout << "Index creation: " << indexCreateDuration << "s" << std::endl;

    std::cout << "Running queries with bitmapIndex" << std::endl;
    output.open("./query_index_results.txt");
    auto indexSelectDuration = timeit([&query_set, &heapTable, &output]() {
        for (int i = 0; i < query_set->query_count; ++i) {
            auto query = query_set->get_query(i);
            auto found = heapTable.SelectWithIndex(query);
            output << found << "\n";
        }
    });

    output.close();
    throughput = getThroughput(query_set->query_count, indexSelectDuration, 1);
    std::cout << "Index querying duration: " << indexSelectDuration << "s " << throughput << " op/s."<< std::endl;

    auto indexSize = heapTable.getBitmapIndex()->getTotalByteSize();
    auto tableSize = heapTable.getTotalByteSize();

    std::cout << "Table byte size: " << tableSize << "B" << std::endl;
    std::cout << "Bitmap bitmapIndex byte size: " << indexSize << "B" << std::endl;

    delete schema;
    delete query_set;
    return 0;
}
