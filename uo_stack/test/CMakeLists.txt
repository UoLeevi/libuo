cmake_minimum_required(VERSION 3.12)
project(uo_stack_test VERSION 0.1.0)

enable_testing()

if(NOT TARGET uo::uo_stack)
    find_package(uo_stack CONFIG REQUIRED)
endif()

add_executable(uo_stack_test
    uo_stack_test.c)

target_link_libraries(uo_stack_test
    PRIVATE
        uo::uo_stack)

target_compile_features(uo_stack_test
    PUBLIC
        c_std_11)

add_test(NAME uo_stack_test
    COMMAND uo_stack_test)
