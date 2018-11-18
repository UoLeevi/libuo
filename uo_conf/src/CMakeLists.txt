cmake_minimum_required(VERSION 3.7)
project(uo_conf VERSION 0.1.0)

if(NOT TARGET uo::uo_hashtbl)
    find_package(uo_hashtbl CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_err)
    find_package(uo_err CONFIG REQUIRED)
endif()

add_library(uo_conf
    uo_conf.c)

target_link_libraries(uo_conf
    PRIVATE
        uo::uo_err
        uo::uo_hashtbl)

target_compile_features(uo_conf
    PUBLIC
        c_std_11)

target_include_directories(uo_conf
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_conf>
        $<INSTALL_INTERFACE:include/uo_conf>)

add_library(uo::uo_conf ALIAS uo_conf)

install(TARGETS uo_conf EXPORT uo_confTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_confTargets
    FILE uo_confTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_conf)

install(DIRECTORY include/uo_conf
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_confConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_confConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_confConfigVersion.cmake
    DESTINATION lib/cmake/uo_conf)