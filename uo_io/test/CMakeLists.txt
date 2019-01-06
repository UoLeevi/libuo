cmake_minimum_required(VERSION 3.7)
project(uo_io_test VERSION 0.2.0)

enable_testing()
if(NOT TARGET uo::uo_io)
    find_package(uo_io CONFIG REQUIRED)
endif()

add_executable(uo_io_test
    uo_io_test.c)

target_link_libraries(uo_io_test
    PRIVATE
        uo::uo_io)

target_compile_features(uo_io_test
    PUBLIC
        c_std_11)

add_test(NAME uo_io_test
    COMMAND uo_io_test)
