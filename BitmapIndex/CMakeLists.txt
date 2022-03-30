cmake_minimum_required(VERSION 3.16)
project(BitmapIndex)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

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
add_executable(BitmapQuery BitmapQuery.cpp ${CPP_FILES} TableSchema.h QuerySet.h BitmapIndex.cpp)
add_executable(HashQuery HashQuery.cpp ${CPP_FILES} TableSchema.h QuerySet.h BitmapIndex.cpp)
