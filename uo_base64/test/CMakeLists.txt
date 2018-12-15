cmake_minimum_required(VERSION 3.7)
project(uo_base64_test VERSION 0.1.0)

enable_testing()
if(NOT TARGET uo::uo_base64)
    find_package(uo_base64 CONFIG REQUIRED)
endif()

add_executable(uo_base64_test
    uo_base64_test.c)

target_link_libraries(uo_base64_test
    PRIVATE
        uo::uo_base64)

target_compile_features(uo_base64_test
    PUBLIC
        c_std_11)

add_test(NAME uo_base64_test
    COMMAND uo_base64_test)
