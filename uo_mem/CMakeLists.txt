cmake_minimum_required(VERSION 3.7)
project(uo_mem)

set(CMAKE_C_FLAGS "-Wall -O2")
set(CMAKE_C_FLAGS_RELEASE "-Wall -O2")
set(CMAKE_C_FLAGS_DEBUG "-Wall -g")

configure_file(src/uo_mem.h
    "${CMAKE_SOURCE_DIR}/include/uo_mem.h" COPYONLY)
