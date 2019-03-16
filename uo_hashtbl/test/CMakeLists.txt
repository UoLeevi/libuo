cmake_minimum_required(VERSION 3.12)
project(uo_hashtbl_test VERSION 0.5.0)

enable_testing()
if(NOT TARGET uo::uo_hashtbl)
    find_package(uo_hashtbl CONFIG REQUIRED)
endif()

add_executable(uo_hashtbl_test
    uo_hashtbl_test.c)

target_link_libraries(uo_hashtbl_test
    PRIVATE
        uo::uo_hashtbl)

target_compile_features(uo_hashtbl_test
    PUBLIC
        c_std_11)

add_test(NAME uo_hashtbl_test
    COMMAND uo_hashtbl_test)

