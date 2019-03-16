cmake_minimum_required(VERSION 3.12)
project(uo_finstack VERSION 0.5.0)

if(NOT TARGET uo::uo_stack)
    find_package(uo_stack CONFIG REQUIRED)
endif()

add_library(uo_finstack
    uo_finstack.c)

target_link_libraries(uo_finstack
    PRIVATE
        uo::uo_stack)

target_compile_features(uo_finstack
    PUBLIC
        c_std_11)

target_include_directories(uo_finstack
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_finstack>
        $<INSTALL_INTERFACE:include/uo_finstack>)

add_library(uo::uo_finstack ALIAS uo_finstack)

install(TARGETS uo_finstack EXPORT uo_finstackTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_finstackTargets
    FILE uo_finstackTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_finstack)

install(DIRECTORY include/uo_finstack
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_finstackConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_finstackConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_finstackConfigVersion.cmake
    DESTINATION lib/cmake/uo_finstack)