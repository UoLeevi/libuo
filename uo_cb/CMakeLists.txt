cmake_minimum_required(VERSION 3.7)
project(uo_cb)

set(CMAKE_C_FLAGS "-Wall -O2")
set(CMAKE_C_FLAGS_RELEASE "-Wall -O2")
set(CMAKE_C_FLAGS_DEBUG "-Wall -g")

set(CMAKE_PREFIX_PATH "..")

configure_file(src/uo_cb.h
    "${CMAKE_SOURCE_DIR}/include/uo_cb.h" COPYONLY)

add_library("${CMAKE_PROJECT_NAME}"
    src/uo_cb.c)

target_include_directories("${CMAKE_PROJECT_NAME}"
    PRIVATE
        src
        "${CMAKE_PREFIX_PATH}/uo_queue/include")

find_library(LIBUO_QUEUE 
    NAMES uo_queue
    HINTS "${CMAKE_PREFIX_PATH}/uo_queue/lib")

target_link_libraries("${CMAKE_PROJECT_NAME}"
    PRIVATE
        "${LIBUO_QUEUE}")

set_target_properties("${CMAKE_PROJECT_NAME}"
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY            "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_SOURCE_DIR}/lib")
