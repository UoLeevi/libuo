cmake_minimum_required(VERSION 3.12)
project(uo_cb_test VERSION 0.2)

enable_testing()

if(NOT TARGET uo::uo_cb)
    find_package(uo_cb CONFIG REQUIRED)
endif()

add_executable(uo_cb_test
    uo_cb_test.c)

target_link_libraries(uo_cb_test
    PRIVATE
        uo::uo_cb)

add_test(NAME uo_cb.unit_tests
    COMMAND uo_cb_test)
