#include <stdio.h>
#include <chrono>
#include <iostream>

#include "DataStructures/Tables/cHeapTable.h"
#include "DataStructures/Tables/cHashTable.h"
#include "DataStructures/cMemory.h"

#define TKey int
#define TData int

using namespace std;
using namespace std::chrono;

float GetThroughput(int opsCount, float period, int unit = 10e6);
void heapTableTest(const int rowCount);
float hashTableTest(const int rowCount, bool recursive = false);

float add_recursive_throughput = 0;
float add_non_recursive_throughput = 0;

float find_recursive_throughput = 0;
float find_non_recursive_throughput = 0;

int main()
{
	const int RowCount = 10000000;
	heapTableTest(RowCount);
	printf("\n");


    printf("---------------- RECURSIVE CALLS ----------------\n");
    for (int i = 0; i < 10; ++i) {
        hashTableTest(RowCount, true);
        hashTableTest(RowCount, false);
    }
    printf("\n");
    printf("\n");
    printf("Average throughput of RECURSIVE ADD: %.2f mil. ops/s.\n", add_recursive_throughput / 10);
    printf("Average throughput of NON_RECURSIVE ADD: %.2f mil. ops/s.\n", add_non_recursive_throughput / 10);

    printf("Average throughput of RECURSIVE FIND: %.2f mil. ops/s.\n", find_recursive_throughput / 10);
    printf("Average throughput of NON_RECURSIVE FIND: %.2f mil. ops/s.\n", find_non_recursive_throughput / 10);
    return 0;
}

float GetThroughput(int opsCount, float period, int unit) {
	return (((float)opsCount / (float)unit)) / period;
}

void heapTableTest(const int rowCount)
{
	auto *heapTable = new cHeapTable<TKey, TData>(rowCount);

	TKey key;
	TData data;

	// start insert heap table
	auto t1 = high_resolution_clock::now();


	for (int i = 0; i < rowCount; i++)
	{
		key = data = i;
		if (!heapTable->Add(key, data)) {
			printf("Critical Error: Record %d insertion failed!\n", i);
		}
		if (i % 10000 == 0) {
			printf("#Record inserted: %d   \r", i);
		}
	}

	auto t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	printf("Records insertion recursion done, HeapTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

	// start scan heap table
	t1 = high_resolution_clock::now();

	for (int i = 0; i < rowCount; i++)
	{
		bool ret = heapTable->Get(i, key, data);
		if (!ret || key != i || data != i) {
			printf("Critical Error: Record %d not found!\n", i);
		}
		if (i % 10000 == 0)
		{
			printf("#Records retrieved: %d   \r", i);
		}
	}

	t2 = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	printf("Table scan (Get) done, HeapTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

	// start scan heap table, Find is invoked 7x
	const int findRowCount = 7;
	int keys[] = { 0, 100000, 1000000, 2500000, 5000000, 8000000, 10000001 };
	t1 = high_resolution_clock::now();

	for (int i = 0; i < findRowCount; i++) {
		bool ret = heapTable->Find(keys[i], data);
		if (ret != true || keys[i] != data) {
			printf("cHeapTable::Find, Key: %d is not found.\n", keys[i]);
		}
	}

	t2 = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	printf("Table Find done, %dx, HeapTable. Time: %.6fs, Throughput: %.2f op/s.\n", findRowCount, time_span.count(), GetThroughput(findRowCount, time_span.count(), 1));

	delete heapTable;
}

float hashTableTest(const int rowCount, bool recursive)
{
    auto *memory = new cMemory((rowCount + 1) * sizeof (cHashTableNode<TKey, TData>));
	auto *hashTable = new cHashTable<TKey, TData>(rowCount / 20, memory);

	TKey key;
	TData data;

	// start insert hash table
	auto t1 = high_resolution_clock::now();

	for (int i = 0; i < rowCount; i++) {
		key = data = i;
		if (!hashTable->Add(key, data, recursive)) {
			printf("Critical Error: Record %d insertion failed!\n", i);
		}

		// for testing only
		/*for (int j = 0; j <= i; j++) {
		bool ret = hashTable->Find(j, data);
		if (!ret || data != j) {
		printf("Critical Error: Record %d not found!\n", i);
		return 0;
		}
		}*/

//		if (i % 10000 == 0) {
//			printf("#Record inserted: %d   \n", i);
//		}
	}

	auto t2 = high_resolution_clock::now();
	auto time_span = duration_cast<duration<double>>(t2 - t1);
    auto throughput = GetThroughput(rowCount, time_span.count());
    printf("Records insertion done, HashTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n",
           time_span.count(), throughput);
    if (recursive) {
        add_recursive_throughput += throughput;
    } else {
        add_non_recursive_throughput += throughput;
    }

	// start scan hash table
	t1 = high_resolution_clock::now();

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

	t2 = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
    throughput = GetThroughput(rowCount, time_span.count());
	printf("Table scan done, HashTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), throughput);
    if (recursive) {
        find_recursive_throughput += throughput;
    } else {
        find_non_recursive_throughput += throughput;
    }
	hashTable->PrintStat();

	delete hashTable;
    return throughput;
}