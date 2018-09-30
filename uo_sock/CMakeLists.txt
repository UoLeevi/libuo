cmake_minimum_required (VERSION 3.10)
project(uo_sock)

configure_file(src/uo_sock.h
    "${CMAKE_SOURCE_DIR}/include/uo_sock.h" COPYONLY)

add_library("${CMAKE_PROJECT_NAME}"
    SHARED
        src/uo_sock.c)

target_include_directories("${CMAKE_PROJECT_NAME}"
    PRIVATE
        src
    INTERFACE
        include)

if(WIN32)
    target_link_libraries("${CMAKE_PROJECT_NAME}"
        PUBLIC
            ws2_32)
endif()

set_target_properties("${CMAKE_PROJECT_NAME}"
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY            "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_SOURCE_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_SOURCE_DIR}/lib")

