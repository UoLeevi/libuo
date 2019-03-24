cmake_minimum_required(VERSION 3.12)
project(uo_util VERSION 0.6.0)

add_library(uo_util INTERFACE)

target_compile_features(uo_util
    INTERFACE
        c_std_11)

target_include_directories(uo_util
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_util>
        $<INSTALL_INTERFACE:include/uo_util>)

add_library(uo::uo_util ALIAS uo_util)

install(TARGETS uo_util EXPORT uo_utilTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_utilTargets
    FILE uo_utilTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_util)

install(DIRECTORY include/uo_util
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_utilConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_utilConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_utilConfigVersion.cmake
    DESTINATION lib/cmake/uo_util)
