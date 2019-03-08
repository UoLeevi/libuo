cmake_minimum_required(VERSION 3.12)
project(uo_finstack_test VERSION 0.1.0)

enable_testing()

if(NOT TARGET uo::uo_finstack)
    find_package(uo_finstack CONFIG REQUIRED)
endif()

add_executable(uo_finstack_test
    uo_finstack_test.c)

target_link_libraries(uo_finstack_test
    PRIVATE
        uo::uo_finstack)

target_compile_features(uo_finstack_test
    PUBLIC
        c_std_11)

add_test(NAME uo_finstack_test
    COMMAND uo_finstack_test)
