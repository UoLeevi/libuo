cmake_minimum_required(VERSION 3.12)
project(uo_macro_test VERSION 0.6.0)

enable_testing()
if(NOT TARGET uo::uo_macro)
    find_package(uo_macro CONFIG REQUIRED)
endif()

add_executable(uo_macro_test
    uo_macro_test.c)

target_link_libraries(uo_macro_test
    PRIVATE
        uo::uo_macro)

target_compile_features(uo_macro_test
    PUBLIC
        c_std_11)

add_test(NAME uo_macro_test
    COMMAND uo_macro_test)
