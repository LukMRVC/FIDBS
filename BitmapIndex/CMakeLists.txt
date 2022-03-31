cmake_minimum_required(VERSION 3.16)
project(BitmapIndex)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

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
add_executable(BitmapQuery BitmapQuery.cpp ${CPP_FILES} TableSchema.h QuerySet.h BitmapIndex.cpp)
add_executable(HashQuery HashQuery.cpp ${CPP_FILES} TableSchema.h QuerySet.h BitmapIndex.cpp)
