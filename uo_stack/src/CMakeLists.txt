cmake_minimum_required(VERSION 3.12)
project(uo_stack VERSION 0.5.0)

add_library(uo_stack INTERFACE)

target_compile_features(uo_stack
    INTERFACE
        c_std_11)

target_include_directories(uo_stack
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_stack>
        $<INSTALL_INTERFACE:include/uo_stack>)

add_library(uo::uo_stack ALIAS uo_stack)

install(TARGETS uo_stack EXPORT uo_stackTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_stackTargets
    FILE uo_stackTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_stack)

install(DIRECTORY include/uo_stack
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_stackConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_stackConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_stackConfigVersion.cmake
    DESTINATION lib/cmake/uo_stack)
