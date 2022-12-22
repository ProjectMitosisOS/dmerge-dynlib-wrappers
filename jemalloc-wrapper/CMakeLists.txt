project(dynlib-jemalloc-wrapper)
ADD_DEFINITIONS(-std=c++14)
cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_FLAGS "-O2")
include(deps.cmake)
# Building jemalloc (both for static and dynamic lib)
build_jemalloc()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_library(malloc_wrapper SHARED malloc.cc)


add_executable(main main.cc)
# !!!Important: The dynamic malloc wrapper lib should link with the jemalloc
#               under static lib format
target_link_libraries(malloc_wrapper ${JEMALLOC_STATIC_LIBRARY} pthread dl)


