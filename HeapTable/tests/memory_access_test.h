//
// Created by lukas on 25.02.22.
//

#ifndef HEAPTABLE_MEMORYACCESS_TEST
#define HEAPTABLE_MEMORYACCESS_TEST

#include <cstdlib>
#include <chrono>
#include <iostream>


void test_memory_access() {
    constexpr int len = 1024 * 1024 * 1024;
    std::cout << "Num. of accesses" << len << std::endl;

    int *data = new int[len];
    long sum = 0;
    srand(990578457);
    auto time_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < len; ++i) {
        data[i] = rand() % len;
        sum += data[i];
    }
    auto time_end = std::chrono::high_resolution_clock::now();
    auto time_proc = time_end - time_start;
    auto timeMilis = std::chrono::duration_cast<std::chrono::milliseconds>(time_proc).count();
    auto timeDouble = std::chrono::duration_cast<std::chrono::duration<double>>(time_proc).count();
    std::cout << "Serial memory access took: " << timeMilis << "ms" << std::endl;

    time_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < len; ++i) {
        int j = rand() % len;
        data[j] = 1;
        sum += data[j];
    }
    time_end = std::chrono::high_resolution_clock::now();
    time_proc = time_end - time_start;
    timeMilis = std::chrono::duration_cast<std::chrono::milliseconds>(time_proc).count();
    std::cout << "Random memory access took: " << timeMilis << "ms" << std::endl;
    delete [] data;
}

#endif