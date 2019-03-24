cmake_minimum_required(VERSION 3.12)
project(uo_buf VERSION 0.6.0)

if(NOT TARGET uo::uo_util)
    find_package(uo_util CONFIG REQUIRED)
endif()

add_library(uo_buf
    uo_buf.c)

target_link_libraries(uo_buf
    PRIVATE
        uo::uo_util)

target_compile_features(uo_buf
    PUBLIC
        c_std_11)

target_include_directories(uo_buf
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_buf>
        $<INSTALL_INTERFACE:include/uo_buf>)

add_library(uo::uo_buf ALIAS uo_buf)

install(TARGETS uo_buf EXPORT uo_bufTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_bufTargets
    FILE uo_bufTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_buf)

install(DIRECTORY include/uo_buf
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_bufConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_bufConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_bufConfigVersion.cmake
    DESTINATION lib/cmake/uo_buf)