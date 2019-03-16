cmake_minimum_required(VERSION 3.12)
project(uo_cb_test VERSION 0.5.0)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

enable_testing()

if(NOT TARGET uo::uo_cb)
    find_package(uo_cb CONFIG REQUIRED)
endif()

add_executable(uo_cb_test
    uo_cb_test.c)

target_link_libraries(uo_cb_test
    PRIVATE
        uo::uo_cb
        Threads::Threads)

add_test(NAME uo_cb_test
    COMMAND uo_cb_test)
