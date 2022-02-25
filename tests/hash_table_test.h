//
// Created by lukas on 25.02.22.
//

#ifndef HEAPTABLE_HASH_TABLE_TEST_H
#define HEAPTABLE_HASH_TABLE_TEST_H

#include "../DataStructures/Tables/cHashTable.h"
#define TKey int
#define TData int

void test_hash_table(cHashTable<TKey, TData> *hashTable, const int rowCount, bool recursive) {
    TKey key;
    TData data;

    // INSERT
    auto t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < rowCount; i++) {
        key = data = i;
        if (!hashTable->Add(key, data, recursive)) {
            printf("Critical Error: Record %d insertion failed!\n", i);
        }
#ifdef DEBUG
        for (int j = 0; j <= i; j++) {
        bool ret = hashTable->Find(j, data, recursive);
        if (!ret || data != j) {
        printf("Critical Error: Record %d not found!\n", i);
        return 0;
        }
        }

		if (i % 10000 == 0) {
			printf("#Record inserted: %d   \n", i);
		}
#endif
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    auto throughput = GetThroughput(rowCount, time_span, 1000 * 1000);
    printf("INSERTION: %.2fs, : %.2f mil. op/s.\n", time_span, throughput);



    // HASH TABLE SCAN
    t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < rowCount; i++)
    {
        bool ret = hashTable->Find(i, data, recursive);
        if (!ret || data != i) {
            printf("Critical Error: Record %d not found!\n", i);
        }
//		if (i % 10000 == 0)
//		{
//			printf("#Records retrieved: %d   \n", i);
//		}
    }

    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    throughput = GetThroughput(rowCount, time_span, 1000 * 1000);
    printf("SCAN: %.2fs, : %.2f mil. op/s.\n", time_span, throughput);
    hashTable->PrintStat();
    printf("\n");
}

#endif //HEAPTABLE_HASH_TABLE_TEST_H
