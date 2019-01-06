cmake_minimum_required(VERSION 3.12)
project(uo_hashtbl VERSION 0.1.0)

add_library(uo_hashtbl
    uo_hashtbl.c)

target_compile_features(uo_hashtbl
    PUBLIC
        c_std_11)

target_include_directories(uo_hashtbl
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_hashtbl>
        $<INSTALL_INTERFACE:include/uo_hashtbl>)

add_library(uo::uo_hashtbl ALIAS uo_hashtbl)

install(TARGETS uo_hashtbl EXPORT uo_hashtblTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_hashtblTargets
    FILE uo_hashtblTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_hashtbl)

install(DIRECTORY include/uo_hashtbl
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_hashtblConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_hashtblConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_hashtblConfigVersion.cmake
    DESTINATION lib/cmake/uo_hashtbl)