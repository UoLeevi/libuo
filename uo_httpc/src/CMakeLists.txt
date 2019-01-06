cmake_minimum_required(VERSION 3.12)
project(uo_httpc VERSION 0.1.0)

if(WIN32)
    set(OPENSSL_ROOT_DIR "/Program files/OpenSSL")
endif()
find_package(OpenSSL MODULE REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(NOT TARGET uo::uo_cb)
    find_package(uo_cb CONFIG REQUIRED)
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

add_library(uo_httpc
    uo_http_res.c
    uo_httpc.c)

target_link_libraries(uo_httpc
    PUBLIC
        uo::uo_cb
        uo::uo_sock
    PRIVATE
        uo::uo_err
        uo::uo_mem
        uo::uo_queue
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads)

if(WIN32)
    target_link_libraries(uo_httpc PUBLIC crypt32)
endif()

target_compile_features(uo_httpc
    PUBLIC
        c_std_11)

target_include_directories(uo_httpc
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_httpc>
        $<INSTALL_INTERFACE:include/uo_httpc>)

add_library(uo::uo_httpc ALIAS uo_httpc)

install(TARGETS uo_httpc EXPORT uo_httpcTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_httpcTargets
    FILE uo_httpcTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_httpc)

install(DIRECTORY include/uo_httpc
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_httpcConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_httpcConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_httpcConfigVersion.cmake
    DESTINATION lib/cmake/uo_httpc)
