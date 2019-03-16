cmake_minimum_required(VERSION 3.12)
project(uo_base64 VERSION 0.5.0)

add_library(uo_base64
    uo_base64.c)

target_compile_features(uo_base64
    PUBLIC
        c_std_11)

target_include_directories(uo_base64
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_base64>
        $<INSTALL_INTERFACE:include/uo_base64>)

add_library(uo::uo_base64 ALIAS uo_base64)

install(TARGETS uo_base64 EXPORT uo_base64Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_base64Targets
    FILE uo_base64Targets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_base64)

install(DIRECTORY include/uo_base64
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_base64ConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_base64Config.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_base64ConfigVersion.cmake
    DESTINATION lib/cmake/uo_base64)