cmake_minimum_required(VERSION 3.12)
project(uo_linkpool_test VERSION 0.5.0)

enable_testing()

if(NOT TARGET uo::uo_linkpool)
    find_package(uo_linkpool CONFIG REQUIRED)
endif()

add_executable(uo_linkpool_test
    uo_linkpool_test.c)

target_link_libraries(uo_linkpool_test
    PRIVATE
        uo::uo_linkpool)

target_compile_features(uo_linkpool_test
    PUBLIC
        c_std_11)

add_test(NAME uo_linkpool_test
    COMMAND uo_linkpool_test)
