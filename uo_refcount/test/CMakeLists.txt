cmake_minimum_required(VERSION 3.12)
project(uo_refcount_test VERSION 0.6.0)

enable_testing()

if(NOT TARGET uo::uo_refcount)
    find_package(uo_refcount CONFIG REQUIRED)
endif()

add_executable(uo_refcount_test
    uo_refcount_test.c)

target_link_libraries(uo_refcount_test
    PRIVATE
        uo::uo_refcount)

target_compile_features(uo_refcount_test
    PUBLIC
        c_std_11)

add_test(NAME uo_refcount_test
    COMMAND uo_refcount_test)
