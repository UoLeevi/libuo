cmake_minimum_required(VERSION 3.7)
project(uo_json VERSION 0.1.0)

add_library(uo_json
    uo_json.c)

target_compile_features(uo_json
    PUBLIC
        c_std_11)

target_include_directories(uo_json
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_json>
        $<INSTALL_INTERFACE:include/uo_json>)

add_library(uo::uo_json ALIAS uo_json)

install(TARGETS uo_json EXPORT uo_jsonTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_jsonTargets
    FILE uo_jsonTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_json)

install(DIRECTORY include/uo_json
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_jsonConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_jsonConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_jsonConfigVersion.cmake
    DESTINATION lib/cmake/uo_json)