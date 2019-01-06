cmake_minimum_required(VERSION 3.12)
project(uo_macro VERSION 0.1.0)

add_library(uo_macro INTERFACE)

target_compile_features(uo_macro
    INTERFACE
        c_std_11)

target_include_directories(uo_macro
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_macro>
        $<INSTALL_INTERFACE:include/uo_macro>)

add_library(uo::uo_macro ALIAS uo_macro)

install(TARGETS uo_macro EXPORT uo_macroTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_macroTargets
    FILE uo_macroTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_macro)

install(DIRECTORY include/uo_macro
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_macroConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_macroConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_macroConfigVersion.cmake
    DESTINATION lib/cmake/uo_macro)