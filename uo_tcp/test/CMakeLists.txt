cmake_minimum_required(VERSION 3.12)
project(uo_tcp_test VERSION 0.3)

enable_testing()

if(NOT TARGET uo::uo_tcp)
    find_package(uo_tcp CONFIG REQUIRED)
endif()

add_executable(uo_tcp_test
    uo_tcp_test.c)

target_link_libraries(uo_tcp_test
    PRIVATE
        uo::uo_tcp)

add_test(NAME uo_tcp.unit_tests
    COMMAND uo_tcp_test)
