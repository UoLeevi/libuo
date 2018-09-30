cmake_minimum_required (VERSION 3.10)
project(uo_httpc)

set(CMAKE_PREFIX_PATH "..")

add_library("${CMAKE_PROJECT_NAME}"
    STATIC
        src/uo_httpc.c
        src/uo_http_res.c)

target_include_directories("${CMAKE_PROJECT_NAME}"
    PRIVATE
        src
        ../uo_cb/include
        ../uo_sock/include)

find_library(UOLIB_CB 
    NAMES uo_cb
    HINTS "${CMAKE_PREFIX_PATH}/uo_cb/lib")

find_library(UOLIB_SOCK
    NAMES uo_sock
    HINTS "${CMAKE_PREFIX_PATH}/uo_sock/lib")

target_link_libraries("${CMAKE_PROJECT_NAME}"
    PRIVATE
        "${UOLIB_CB}"
        "${UOLIB_SOCK}")

set_target_properties("${CMAKE_PROJECT_NAME}"
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY            "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_SOURCE_DIR}/lib")

configure_file(src/uo_httpc.h
    "${CMAKE_SOURCE_DIR}/include/uo_httpc.h" COPYONLY)
configure_file(src/uo_http_res.h
    "${CMAKE_SOURCE_DIR}/include/uo_http_res.h" COPYONLY)
