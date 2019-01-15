cmake_minimum_required(VERSION 3.12)
project(uo_tcp VERSION 0.3.0)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(NOT TARGET uo::uo_cb)
    find_package(uo_cb CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_io)
    find_package(uo_io CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_buf)
    find_package(uo_buf CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_err)
    find_package(uo_err CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_mem)
    find_package(uo_mem CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_queue)
    find_package(uo_queue CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_sock)
    find_package(uo_sock CONFIG REQUIRED)
endif()

add_library(uo_tcp
    uo_tcp_client.c
    uo_tcp_conn.c
    uo_tcp_server.c
    uo_tcp.c)

target_link_libraries(uo_tcp
    PUBLIC
        uo::uo_cb
        uo::uo_buf
    PRIVATE
        uo::uo_io
        uo::uo_err
        uo::uo_mem
        uo::uo_queue
        uo::uo_sock
        Threads::Threads)

if(WIN32)
    target_link_libraries(uo_tcp PUBLIC crypt32)
endif()

target_compile_features(uo_tcp
    PUBLIC
        c_std_11)

target_include_directories(uo_tcp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_tcp>
        $<INSTALL_INTERFACE:include/uo_tcp>)

add_library(uo::uo_tcp ALIAS uo_tcp)

install(TARGETS uo_tcp EXPORT uo_tcpTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_tcpTargets
    FILE uo_tcpTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_tcp)

install(DIRECTORY include/uo_tcp
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_tcpConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_tcpConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_tcpConfigVersion.cmake
    DESTINATION lib/cmake/uo_tcp)
