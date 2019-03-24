cmake_minimum_required(VERSION 3.12)
project(uo_refstack VERSION 0.6.0)

if(NOT TARGET uo::uo_refcount)
    find_package(uo_refcount CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_stack)
    find_package(uo_stack CONFIG REQUIRED)
endif()

add_library(uo_refstack INTERFACE)

target_link_libraries(uo_refstack
    INTERFACE
        uo::uo_refcount
        uo::uo_stack)

target_compile_features(uo_refstack
    INTERFACE
        c_std_11)

target_include_directories(uo_refstack
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_refstack>
        $<INSTALL_INTERFACE:include/uo_refstack>)

add_library(uo::uo_refstack ALIAS uo_refstack)

install(TARGETS uo_refstack EXPORT uo_refstackTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_refstackTargets
    FILE uo_refstackTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_refstack)

install(DIRECTORY include/uo_refstack
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_refstackConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_refstackConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_refstackConfigVersion.cmake
    DESTINATION lib/cmake/uo_refstack)