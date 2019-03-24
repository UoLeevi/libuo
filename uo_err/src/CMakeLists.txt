cmake_minimum_required(VERSION 3.12)
project(uo_err VERSION 0.6.0)

add_library(uo_err
    uo_err.c)

target_compile_features(uo_err
    PUBLIC
        c_std_11)

target_include_directories(uo_err
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_err>
        $<INSTALL_INTERFACE:include/uo_err>)

add_library(uo::uo_err ALIAS uo_err)

install(TARGETS uo_err EXPORT uo_errTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_errTargets
    FILE uo_errTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_err)

install(DIRECTORY include/uo_err
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_errConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_errConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_errConfigVersion.cmake
    DESTINATION lib/cmake/uo_err)