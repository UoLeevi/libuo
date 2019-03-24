cmake_minimum_required(VERSION 3.12)
project(uo_refstack_test VERSION 0.6.0)

enable_testing()

if(NOT TARGET uo::uo_refstack)
    find_package(uo_refstack CONFIG REQUIRED)
endif()

add_executable(uo_refstack_test
    uo_refstack_test.c)

target_link_libraries(uo_refstack_test
    PRIVATE
        uo::uo_refstack)

target_compile_features(uo_refstack_test
    PUBLIC
        c_std_11)

add_test(NAME uo_refstack_test
    COMMAND uo_refstack_test)
