cmake_minimum_required(VERSION 3.12)
project(uo_cb_benchmark VERSION 0.3)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

enable_testing()

if(NOT TARGET uo::uo_cb)
    find_package(uo_cb CONFIG REQUIRED)
endif()

add_executable(uo_cb_benchmark
    uo_cb_benchmark.c)

target_link_libraries(uo_cb_benchmark
    PRIVATE
        uo::uo_cb
        Threads::Threads)

add_test(NAME uo_cb_benchmark
    COMMAND uo_cb_benchmark)
