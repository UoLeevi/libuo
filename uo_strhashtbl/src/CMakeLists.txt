cmake_minimum_required(VERSION 3.12)
project(uo_strhashtbl VERSION 0.2.0)

if(NOT TARGET uo::uo_linklist)
    find_package(uo_linklist CONFIG REQUIRED)
endif()

add_library(uo_strhashtbl
    uo_strhashtbl.c)

target_link_libraries(uo_strhashtbl
    PUBLIC
        uo::uo_linklist)

target_compile_features(uo_strhashtbl
    PUBLIC
        c_std_11)

target_include_directories(uo_strhashtbl
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_strhashtbl>
        $<INSTALL_INTERFACE:include/uo_strhashtbl>)

add_library(uo::uo_strhashtbl ALIAS uo_strhashtbl)

install(TARGETS uo_strhashtbl EXPORT uo_strhashtblTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_strhashtblTargets
    FILE uo_strhashtblTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_strhashtbl)

install(DIRECTORY include/uo_strhashtbl
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_strhashtblConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_strhashtblConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_strhashtblConfigVersion.cmake
    DESTINATION lib/cmake/uo_strhashtbl)
