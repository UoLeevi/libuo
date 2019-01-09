cmake_minimum_required(VERSION 3.12)
project(uo_cb VERSION 0.3)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(uo_cb
    uo_cb.c
    uo_cb_queue.c
    uo_cb_thrd_pool.c)

target_link_libraries(uo_cb
    PUBLIC
        Threads::Threads
    PRIVATE
        m)

target_compile_features(uo_cb
    PUBLIC
        c_std_11)

target_include_directories(uo_cb
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_cb>
        $<INSTALL_INTERFACE:include/uo_cb>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

add_library(uo::uo_cb ALIAS uo_cb)

install(TARGETS uo_cb EXPORT uo_cbTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_cbTargets
    FILE uo_cbTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_cb)

install(DIRECTORY include/uo_cb
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_cbConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_cbConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_cbConfigVersion.cmake
    DESTINATION lib/cmake/uo_cb)
