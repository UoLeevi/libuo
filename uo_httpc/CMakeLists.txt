cmake_minimum_required(VERSION 3.7)
project(uo_httpc)

set(CMAKE_PREFIX_PATH "..")

add_library("${CMAKE_PROJECT_NAME}"
    src/uo_httpc.c
    src/uo_http_res.c)

find_package(OpenSSL)

target_include_directories("${CMAKE_PROJECT_NAME}"
    PRIVATE
        src
        ../uo_cb/include
        ../uo_err/include
        ../uo_mem/include
        ../uo_sock/include
        "${OPENSSL_INCLUDE_DIR}")

find_library(UOLIB_CB 
    NAMES uo_cb
    HINTS "${CMAKE_PREFIX_PATH}/uo_cb/lib")

find_library(UOLIB_ERR 
    NAMES uo_err
    HINTS "${CMAKE_PREFIX_PATH}/uo_err/lib")

find_library(UOLIB_SOCK
    NAMES uo_sock
    HINTS "${CMAKE_PREFIX_PATH}/uo_sock/lib")

target_link_libraries("${CMAKE_PROJECT_NAME}"
    PRIVATE
        "${UOLIB_CB}"
        "${UOLIB_SOCK}"
        "${UOLIB_ERR}"
        "${OPENSSL_SSL_LIBRARY}")

set_target_properties("${CMAKE_PROJECT_NAME}"
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY            "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_SOURCE_DIR}/lib")

configure_file(src/uo_httpc.h
    "${CMAKE_SOURCE_DIR}/include/uo_httpc.h" COPYONLY)
configure_file(src/uo_http_res.h
    "${CMAKE_SOURCE_DIR}/include/uo_http_res.h" COPYONLY)
