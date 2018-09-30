cmake_minimum_required (VERSION 3.10)
project(uo_stack)

configure_file(src/uo_stack.h
    "${CMAKE_SOURCE_DIR}/include/uo_stack.h" COPYONLY)

add_library("${CMAKE_PROJECT_NAME}"
    SHARED
        src/uo_stack.c)

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
