cmake_minimum_required(VERSION 3.12)
project(uo_http_test VERSION 0.6.0)

enable_testing()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(NOT TARGET uo::uo_http)
    find_package(uo_http CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_macro)
    find_package(uo_macro CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_util)
    find_package(uo_util CONFIG REQUIRED)
endif()


add_executable(uo_http_req_handler_test
    uo_http_req_handler_test.c)

target_link_libraries(uo_http_req_handler_test
    PRIVATE
        uo::uo_http
        uo::uo_util)

target_compile_features(uo_http_req_handler_test
    PUBLIC
        c_std_11)

add_test(NAME uo_http_req_handler_test
    COMMAND uo_http_req_handler_test)


add_executable(uo_http_test
    uo_http_test.c)

target_link_libraries(uo_http_test
    PRIVATE
        uo::uo_macro
        uo::uo_http
        uo::uo_util
        Threads::Threads)

target_compile_features(uo_http_test
    PUBLIC
        c_std_11)

add_test(NAME uo_http_test
    COMMAND uo_http_test)

add_custom_command(TARGET uo_http_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/test_content $<TARGET_FILE_DIR:uo_http_test>/test_content)
