//
// Created by lukas on 25.02.22.
//

#ifndef HEAPTABLE_HEAP_TABLE_TEST_H
#define HEAPTABLE_HEAP_TABLE_TEST_H

#define TKey int
#define TData int

#include "../DataStructures/Tables/Table.h"
#include "utils.h"

void test_heap_table(AbstractHeapTable<TKey, TData> * table, const int RowCount) {
    TKey key;
    TData data;

    auto t1 = std::chrono::high_resolution_clock::now();

    // INSERTION
    for (int i = 0; i < RowCount; i++) {
        key = data = i;
        if (!table->Add(key, data)) {
            printf("Critical Error: Record %d insertion failed!\n", i);
        }
#ifdef DEBUG
        if (i % 10000 == 0) {
            printf("#Record inserted: %d   \n", i);
        }
#endif
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    printf("INSERTION: %.2fs, %.2f mil. op/s.\n", time_span,
           GetThroughput(RowCount, time_span, 1000 * 1000));



    // SCAN - GET
    t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < RowCount; i++) {
        bool ret = table->Get(i, key, data);
        if (!ret || key != i || data != i) {
            printf("Critical Error: Record %d not found!\n", i);
        }
#ifdef DEBUG
        if (i % 10000 == 0) {
            printf("#Records retrieved: %d   \n", i);
        }
#endif
    }

    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    printf("SCAN:  %.2fs, %.2f mil. op/s.\n", time_span,
           GetThroughput(RowCount, time_span, 1000 * 1000));


    // START SEQUENTIAL SCAN - FIND
    const int findRowCount = 8;
    int keys[] = {0, 100000, 1000000, 2500000, 5000000, 8000000, 10000000, 10000001};
    // start sequential table scan
    t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < findRowCount; i++) {
        key = keys[i];
        bool ret = table->Find(key, data);
        if (!ret || data != key) {
            printf("Critical Error: Record %d not found!\n", i);
        }
#ifdef DEBUG
        if (i % 10000 == 0) {
            printf("#Records retrieved: %d   \n", i);
        }
#endif
    }

    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    printf("SEQUENTIAL SCAN: %.2fs, %.2f op/s.\n", time_span,
           GetThroughput(findRowCount, time_span, 1));
}

#endif //HEAPTABLE_HEAP_TABLE_TEST_H
