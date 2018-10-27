cmake_minimum_required(VERSION 3.7)
project(uo_tcpserv)

set(CMAKE_C_FLAGS "-Wall -O2")
set(CMAKE_C_FLAGS_RELEASE "-Wall -O2")
set(CMAKE_C_FLAGS_DEBUG "-Wall -g")

set(CMAKE_PREFIX_PATH "..")

add_library("${CMAKE_PROJECT_NAME}"
    src/uo_tcpserv.c
    src/uo_tcpserv_conf.c)

find_package(OpenSSL)

target_include_directories("${CMAKE_PROJECT_NAME}"
    PRIVATE
        src
        ../uo_cb/include
        ../uo_queue/include
        ../uo_err/include
        ../uo_mem/include
        ../uo_sock/include)

find_library(UOLIB_CB 
    NAMES uo_cb
    HINTS "${CMAKE_PREFIX_PATH}/uo_cb/lib")

find_library(UOLIB_QUEUE 
    NAMES uo_queue
    HINTS "${CMAKE_PREFIX_PATH}/uo_queue/lib")

find_library(UOLIB_ERR 
    NAMES uo_err
    HINTS "${CMAKE_PREFIX_PATH}/uo_err/lib")

find_library(UOLIB_SOCK
    NAMES uo_sock
    HINTS "${CMAKE_PREFIX_PATH}/uo_sock/lib")

target_link_libraries("${CMAKE_PROJECT_NAME}"
    PRIVATE
        "${UOLIB_CB}"
        "${UOLIB_QUEUE}"
        "${UOLIB_SOCK}"
        "${UOLIB_ERR}")

set_target_properties("${CMAKE_PROJECT_NAME}"
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY            "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_SOURCE_DIR}/lib")

configure_file(src/uo_tcpserv.h
    "${CMAKE_SOURCE_DIR}/include/uo_tcpserv.h" COPYONLY)

configure_file(conf/uo_tcpserv.conf
    "${CMAKE_SOURCE_DIR}/lib/uo_tcpserv.conf" COPYONLY)
configure_file(keys/uo_tcpserv-key.txt
    "${CMAKE_SOURCE_DIR}/lib/uo_tcpserv-key.txt" COPYONLY)
