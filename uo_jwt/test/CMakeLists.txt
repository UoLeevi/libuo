cmake_minimum_required(VERSION 3.12)
project(uo_jwt_test VERSION 0.6.0)

enable_testing()
if(NOT TARGET uo::uo_jwt)
    find_package(uo_jwt CONFIG REQUIRED)
endif()

add_executable(uo_jwt_test
    uo_jwt_test.c)

target_link_libraries(uo_jwt_test
    PRIVATE
        uo::uo_jwt)

target_compile_features(uo_jwt_test
    PUBLIC
        c_std_11)

add_test(NAME uo_jwt_test
    COMMAND uo_jwt_test)
