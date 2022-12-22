project(dynlib-jemalloc-wrapper)
ADD_DEFINITIONS(-std=c++14)
cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_FLAGS "-O2")
include(deps.cmake)
build_jemalloc()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_library(malloc_wrapper SHARED malloc.cc)


add_executable(main main.cc)
target_link_libraries(malloc_wrapper ${JEMALLOC_STATIC_LIBRARY} pthread dl)
target_link_libraries(main malloc_wrapper dl)


