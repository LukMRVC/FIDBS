#include <iostream>
#include "cRowHeapTable.h"
#include <string>
#include <chrono>
#include <ctime>

inline double getThroughput(int opsCount, double period, int unit) {
    return  (( (double)opsCount / (double)unit )) / period;
}

int main(int args_count, char *args[]) {
    constexpr int records = 10000000;
    constexpr int colSize = 5;

    unsigned char testout = 0;

    const int cols[colSize] = {10, 10, 1, 1, 1};

    int max_col_values[] = { -1, -1, 2, 6, 5 };
    cRowHeapTable heapTable(cols, colSize, records);

    auto rowSize = 0;
    for (int c = 0; c < colSize; ++c) {
        rowSize += cols[c];
    }

    srand(time(nullptr));
    char *rec = new char[rowSize];
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < records; ++i) {
        cRowHeapTable::generateRecord(rec, cols, max_col_values, colSize);
        heapTable.Insert(rec);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    auto throughput = getThroughput(records, time_span, 1000 * 1000);
    printf("INSERTION %.2fs, %.2f mil. op/s \n", time_span, throughput);

    t1 = std::chrono::high_resolution_clock::now();
    heapTable.createBitmapIndex(max_col_values, colSize);
    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    throughput = getThroughput(records, time_span, 1000 * 1000);

    printf("BITMAP INDEX CREATING: %.2fs, %.2f mil. op/s \n", time_span, throughput);
    // SELECTION

    constexpr int selectionsCount = 10;
    unsigned int selectionConditions[selectionsCount][3][2] = {
    {
                {2, 0},
                {3, 3},
                {4, 3},
        },
        {
                {2, 1},
                {3, 5},
                {4, 2},
        },
        {
                {2, 0},
                {3, 5},
                {4, 4},
        },
        {
                {2, 1},
                {3, 5},
                {4, 4},
        },
        {
                {2, 0},
                {3, 2},
                {4, 1},
        },
        {
                {2, 0},
                {3, 1},
                {4, 2},
        },
        {
                {2, 1},
                {3, 1},
                {4, 0},
        },
        {
                {2, 0},
                {3, 1},
                {4, 1},
        },
        {
                {2, 1},
                {3, 4},
                {4, 2},
        },
        {
                {2, 0},
                {3, 2},
                {4, 3},
        },

    };

    t1 = std::chrono::high_resolution_clock::now();
    auto totalRecordsFounds = 0;
    for (int i = 0; i < selectionsCount; ++i) {
        auto recordsFound = heapTable.Select(selectionConditions[i], 3);
        printf("SELECTION %d RECORDS FOUND \n", recordsFound);
        totalRecordsFounds += recordsFound;
    }
    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    throughput = getThroughput(selectionsCount, time_span, 1);
    printf("SELECTION %d RECORDS FOUND %.2fs, %.2f op/s \n", totalRecordsFounds, time_span, throughput);

    // SELECT WITH INDEX

    t1 = std::chrono::high_resolution_clock::now();
    totalRecordsFounds = 0;
    for (int i = 0; i < selectionsCount; ++i) {
        auto recordsFound = heapTable.SelectWithIndex(selectionConditions[i], 3);
        printf("SELECTION WITH INDEX %d RECORDS FOUND \n", recordsFound);
        totalRecordsFounds += recordsFound;
    }
    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    throughput = getThroughput(selectionsCount, time_span, 1);
    printf("SELECTION WITH INDEX %d RECORDS FOUND %.2fs, %.2f op/s \n", totalRecordsFounds, time_span, throughput);


    delete [] rec;
    rec = nullptr;
    return 0;
}
