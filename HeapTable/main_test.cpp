//
// Created by lukas on 25.02.22.
//

#include <cstdio>
#include <chrono>
#include <iostream>

#include "DataStructures/Tables/cHeapTable.h"
#include "DataStructures/Tables/cHashTable.h"
#include "DataStructures/Tables/cRecordHeapTable.h"
#include "DataStructures/cMemory.h"
#include "tests/memory_access_test.h"
#include "tests/heap_table_test.h"
#include "tests/hash_table_test.h"


#define TKey int
#define TData int

using namespace std;
using namespace std::chrono;


int main() {
#ifdef DEBUG
    printf("Debug mode \n");
#endif
    AbstractHeapTable<TKey, TData> *heapTable = nullptr;
    cHashTable<TKey, TData> *hashTable = nullptr;
    std::cout << "--------- MEMORY ACCESS TEST ---------" << std::endl;
    test_memory_access();

    int const RowCount = 10000000;

    std::cout << "--------- HEAP TABLE TEST ---------" << std::endl;
    heapTable = new cHeapTable<TKey, TData>(RowCount);
    test_heap_table(heapTable, RowCount);
    delete heapTable;


    std::cout << "--------- RECORD HEAP TABLE TEST ---------" << std::endl;
    heapTable = new cRecordHeapTable<TKey, TData>(RowCount);
    test_heap_table(heapTable, RowCount);
    delete heapTable;

    std::cout << "--------- RECORD HEAP TABLE TEST ---------" << std::endl;
    heapTable = new cRecordHeapTable<TKey, TData>(RowCount);
    test_heap_table(heapTable, RowCount);
    delete heapTable;
    heapTable = nullptr;

    const int avgSlots[2] = { 2, 20 };
    const bool withMemory[2] = { false, true };
    const bool recursions[2] = { true, false };

    for (int avgSlot : avgSlots) {
        for (bool isWithMemory : withMemory) {
            cMemory * memory = nullptr;


            for (bool withRecursion : recursions) {
                if (isWithMemory) {
                    memory = new cMemory((RowCount + 1) * sizeof (cHashTableNode<TKey, TData>));
                }

                auto memoryStr = isWithMemory ? " WITH MEMORY, " : " WITHOUT MEMORY, ";
                auto recursionStr = withRecursion ? "RECURSIVE " : "NON-RECURSIVE ";

                std::cout << "--------- HASH TABLE TEST - " << memoryStr << recursionStr << avgSlot
                << " SLOTS ---------" << std::endl;
                hashTable = new cHashTable<TKey, TData>(RowCount / avgSlot, memory);
                test_hash_table(hashTable, RowCount, withRecursion);
                delete hashTable;
                hashTable = nullptr;

                if (memory != nullptr) {
                    delete memory;
                    memory = nullptr;
                }
            }
        }
    }

    return 0;
}
