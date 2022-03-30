cmake_minimum_required(VERSION 3.21)
project(BitmapIndex)

set(CMAKE_CXX_STANDARD 17)

list(
        APPEND
        CPP_FILES
        cRowHeapTable.h
        BitmapIndex.h
        BitString.h
        DataStructures/HashTableNode.h
        DataStructures/Memory.h
        DataStructures/HashTable.h
        DataStructures/Cursor.h
)

add_executable(BitmapIndex main.cpp ${CPP_FILES})
add_executable(BitmapQuery BitmapQuery.cpp ${CPP_FILES} TableSchema.h QuerySet.h)
add_executable(HashQuery HashQuery.cpp ${CPP_FILES} TableSchema.h QuerySet.h)
