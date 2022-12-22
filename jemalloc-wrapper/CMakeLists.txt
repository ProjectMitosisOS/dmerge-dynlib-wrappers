project(dynlib-jemalloc-wrapper)
ADD_DEFINITIONS(-std=c++14)
cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_FLAGS "-O2")
include(deps.cmake)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
find_library(jemalloc_lib NAMES jemalloc PATHS ./deps/jemalloc PATH_SUFFIXES lib
        NO_DEFAULT_PATH)

if (NOT jemalloc_lib)
    set(jemalloc_lib "")
endif ()

add_library(malloc_wrapper SHARED malloc.cc)


add_executable(main main.cc)
target_link_libraries(malloc_wrapper ${jemalloc_lib} dl)
target_link_libraries(main malloc_wrapper dl)


