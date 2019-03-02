cmake_minimum_required(VERSION 3.12)
project(uo_linklist VERSION 0.2.0)

add_library(uo_linklist INTERFACE)

target_compile_features(uo_linklist
    INTERFACE
        c_std_11)

target_include_directories(uo_linklist
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_linklist>
        $<INSTALL_INTERFACE:include/uo_linklist>)

add_library(uo::uo_linklist ALIAS uo_linklist)

install(TARGETS uo_linklist EXPORT uo_linklistTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_linklistTargets
    FILE uo_linklistTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_linklist)

install(DIRECTORY include/uo_linklist
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_linklistConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_linklistConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_linklistConfigVersion.cmake
    DESTINATION lib/cmake/uo_linklist)
