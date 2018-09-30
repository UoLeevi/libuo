cmake_minimum_required (VERSION 3.10)
project(uo_cb)

if(WIN32)
    set(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a" ".dll.a")
    set(CMAKE_PREFIX_PATH "C:/dev/lib")
endif()

configure_file(src/uo_cb.h
    "${CMAKE_SOURCE_DIR}/include/uo_cb.h" COPYONLY)

add_library("${CMAKE_PROJECT_NAME}"
    SHARED
        src/uo_cb.c)

target_include_directories("${CMAKE_PROJECT_NAME}"
    PRIVATE
        src
        "${CMAKE_PREFIX_PATH}/uo_queue/include"
        "${CMAKE_PREFIX_PATH}/uo_hashtbl/include")

find_library(LIBUO_QUEUE 
    NAMES uo_queue
    HINTS "${CMAKE_PREFIX_PATH}/uo_queue/lib")

find_library(LIBUO_HASHTBL 
    NAMES uo_hashtbl
    HINTS "${CMAKE_PREFIX_PATH}/uo_hashtbl/lib")

target_link_libraries("${CMAKE_PROJECT_NAME}"
    PRIVATE
        "C:/dev/lib/uo_queue/lib/libuo_queue.dll.a"
        "${LIBUO_HASHTBL}")

set_target_properties("${CMAKE_PROJECT_NAME}"
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY            "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_SOURCE_DIR}/lib")
