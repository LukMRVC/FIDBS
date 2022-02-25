cmake_minimum_required(VERSION 3.21)
project(HeapTable)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -m64)
set(CMAKE_GENERATOR_PLATFORM x64)

list(
        APPEND
        COMMON_CPP_FILES
        DataStructures/Tables/cHeapTable.h
        DataStructures/Tables/Record.h
        DataStructures/Tables/cRecordHeapTable.h
        DataStructures/Tables/Table.h
        DataStructures/Tables/cHashTable.h
        DataStructures/Tables/cHashTable.h
        DataStructures/cHashTableNode.h
        DataStructures/cMemory.h
)

add_executable(HeapTable
        heapTable_test.cpp
        ${COMMON_CPP_FILES}
        )

add_executable(HashTable
        hashTable_test.cpp
        ${COMMON_CPP_FILES}
        DataStructures/Tables/cRowHeapTable.h
        DataStructures/Indexes/cBitmapIndex.h
        )


add_executable(MainTest

        ${COMMON_CPP_FILES}
        main_test.cpp)
