cmake_minimum_required(VERSION 3.12)
project(uo_http VERSION 0.5.1)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(NOT TARGET uo::uo_finstack)
    find_package(uo_finstack CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_tcp)
    find_package(uo_tcp CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_mem)
    find_package(uo_mem CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_hashtbl)
    find_package(uo_hashtbl CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_buf)
    find_package(uo_buf CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_cb)
    find_package(uo_cb CONFIG REQUIRED)
endif()

add_library(uo_http
    uo_http_client.c
    uo_http_conn.c
    uo_http_file_server.c
    uo_http_msg.c
    uo_http_req_handler.c
    uo_http_server.c
    uo_http.c)

target_link_libraries(uo_http
    PUBLIC
        uo::uo_buf
        uo::uo_cb
        uo::uo_hashtbl
        uo::uo_finstack
    PRIVATE
        uo::uo_tcp
        uo::uo_mem
        Threads::Threads)

target_compile_features(uo_http
    PUBLIC
        c_std_11)

target_include_directories(uo_http
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_http>
        $<INSTALL_INTERFACE:include/uo_http>)

add_library(uo::uo_http ALIAS uo_http)

install(TARGETS uo_http EXPORT uo_httpTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_httpTargets
    FILE uo_httpTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_http)

install(DIRECTORY include/uo_http
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_httpConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_httpConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_httpConfigVersion.cmake
    DESTINATION lib/cmake/uo_http)
