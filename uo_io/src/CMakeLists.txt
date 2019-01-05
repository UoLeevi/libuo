cmake_minimum_required(VERSION 3.7)
project(uo_io VERSION 0.2.0)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(NOT TARGET uo::uo_err)
    find_package(uo_err CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_cb)
    find_package(uo_cb CONFIG REQUIRED)
endif()

add_library(uo_io
    uo_io.c)

target_link_libraries(uo_io
    PUBLIC
        uo::uo_cb
        Threads::Threads
    PRIVATE
        uo::uo_err)

target_compile_features(uo_io
    PUBLIC
        c_std_11)

target_include_directories(uo_io
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_io>
        $<INSTALL_INTERFACE:include/uo_io>)

add_library(uo::uo_io ALIAS uo_io)

install(TARGETS uo_io EXPORT uo_ioTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_ioTargets
    FILE uo_ioTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_io)

install(DIRECTORY include/uo_io
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_ioConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_ioConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_ioConfigVersion.cmake
    DESTINATION lib/cmake/uo_io)
