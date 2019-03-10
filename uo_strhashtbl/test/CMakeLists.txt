cmake_minimum_required(VERSION 3.12)
project(uo_strhashtbl_test VERSION 0.3.0)

enable_testing()
if(NOT TARGET uo::uo_strhashtbl)
    find_package(uo_strhashtbl CONFIG REQUIRED)
endif()

add_executable(uo_strhashtbl_test
    uo_strhashtbl_test.c)

target_link_libraries(uo_strhashtbl_test
    PRIVATE
        uo::uo_strhashtbl)

target_compile_features(uo_strhashtbl_test
    PUBLIC
        c_std_11)

add_test(NAME uo_strhashtbl_test
    COMMAND uo_strhashtbl_test)

