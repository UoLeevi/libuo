cmake_minimum_required(VERSION 3.12)
project(uo_util_test VERSION 0.5.0)

enable_testing()
if(NOT TARGET uo::uo_util)
    find_package(uo_util CONFIG REQUIRED)
endif()

add_executable(uo_util_test
    uo_util_test.c)

target_link_libraries(uo_util_test
    PRIVATE
        uo::uo_util)

target_compile_features(uo_util_test
    PUBLIC
        c_std_11)

add_test(NAME uo_util_test
    COMMAND uo_util_test)
