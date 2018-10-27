cmake_minimum_required(VERSION 3.7)
project(uo_hashtbl)

set(CMAKE_C_FLAGS "-Wall -O2")
set(CMAKE_C_FLAGS_RELEASE "-Wall -O2")
set(CMAKE_C_FLAGS_DEBUG "-Wall -g")

configure_file(src/uo_hashtbl.h
    "${CMAKE_SOURCE_DIR}/include/uo_hashtbl.h" COPYONLY)

add_library("${CMAKE_PROJECT_NAME}" 
    src/uo_hashtbl.c)

target_include_directories("${CMAKE_PROJECT_NAME}"
    PRIVATE
        src
    INTERFACE
        include)

set_target_properties("${CMAKE_PROJECT_NAME}"
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY            "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_SOURCE_DIR}/lib")
