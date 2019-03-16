cmake_minimum_required(VERSION 3.12)
project(uo_linklist_test VERSION 0.5.0)

enable_testing()
if(NOT TARGET uo::uo_linklist)
    find_package(uo_linklist CONFIG REQUIRED)
endif()


add_executable(uo_linklist_test
    uo_linklist_test.c)

target_link_libraries(uo_linklist_test
    PRIVATE
        uo::uo_linklist)

target_compile_features(uo_linklist_test
    PUBLIC
        c_std_11)

add_test(NAME uo_linklist_test
    COMMAND uo_linklist_test)

