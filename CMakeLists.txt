cmake_minimum_required(VERSION 3.21)
project(HeapTable)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -m64)
set(CMAKE_GENERATOR_PLATFORM x64)

add_executable(HeapTable
        cHeapTable.h
        heapTable_test.cpp
        Record.h
        cRecordHeapTable.h
        Table.h
        cHashTable.h
        cHashTable.h
        cHashTableNode.h
        cMemory.h
        )
