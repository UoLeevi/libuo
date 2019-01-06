cmake_minimum_required(VERSION 3.12)
project(uo_json_test VERSION 0.1.0)

enable_testing()
if(NOT TARGET uo::uo_json)
    find_package(uo_json CONFIG REQUIRED)
endif()

add_executable(uo_json_encode_test
    uo_json_encode_test.c)

target_link_libraries(uo_json_encode_test
    PRIVATE
        uo::uo_json)

target_compile_features(uo_json_encode_test
    PUBLIC
        c_std_11)

add_test(NAME uo_json_encode_utf8_test
    COMMAND uo_json_encode_test utf8)

add_test(NAME uo_json_encode_number_test
    COMMAND uo_json_encode_test number)
