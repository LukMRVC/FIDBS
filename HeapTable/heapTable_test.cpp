#include <cstdio>
#include <chrono>
#include <iostream>
#include <cstdlib>

#include "DataStructures/Tables/cHeapTable.h"
#include "DataStructures/Tables/Table.h"
#include "DataStructures/Tables/cRecordHeapTable.h"
#include "DataStructures/Tables/Record.h"

#define TKey int
#define TData int

using namespace std;
using namespace std::chrono;



int main() {
    int const RowCount = 10000000;

//    Table<TKey, TData> *table = new cRecordHeapTable<TKey, TData>(RowCount);
    Table<TKey, TData> *table = new cHeapTable<TKey, TData>(RowCount);

    TKey key;
    TData data;

    // start insert
    auto t1 = high_resolution_clock::now();

    for (int i = 0; i < RowCount; i++) {
        key = data = i;
        if (!table->Add(key, data)) {
            printf("Critical Error: Record %d insertion failed!\n", i);
        }
//        if (i % 10000 == 0) {
//            printf("#Record inserted: %d   \n", i);
//        }
    }

    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Records insertion done. Time: %.2fs, Throughput: %.2f op/s.\n", time_span.count(),
           (float) RowCount / time_span.count());

    // start scan
    t1 = high_resolution_clock::now();

    for (int i = 0; i < RowCount; i++) {
        bool ret = table->Get(i, key, data);
        if (!ret || key != i || data != i) {
            printf("Critical Error: Record %d not found!\n", i);
        }
//        if (i % 10000 == 0) {
//            printf("#Records retrieved: %d   \n", i);
//        }
    }

    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Table scan done. Time: %.2fs, Throughput: %.2f op/s.\n", time_span.count(),
           (float) RowCount / time_span.count());

    const int findRowCount = 8;
    int keys[] = {0, 100000, 1000000, 2500000, 5000000, 8000000, 10000000, 10000001};
    // start sequential table scan
    t1 = high_resolution_clock::now();

    for (int i = 0; i < findRowCount; i++) {
        key = keys[i];
        bool ret = table->Find(key, data);
        if (!ret || data != key) {
            printf("Critical Error: Record %d not found!\n", i);
        }
//        if (i % 10000 == 0) {
//            printf("#Records retrieved: %d   \n", i);
//        }
    }

    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Table sequential scan done. Time: %.2fs, Throughput: %.2f op/s.\n", time_span.count(),
           (float) findRowCount / time_span.count());


    delete table;

    return 0;
}
