cmake_minimum_required(VERSION 3.12)
project(uo_mem VERSION 0.6.0)

if(NOT TARGET uo::uo_macro)
    find_package(uo_macro CONFIG REQUIRED)
endif()

add_library(uo_mem INTERFACE)

target_compile_features(uo_mem
    INTERFACE
        c_std_11)

target_link_libraries(uo_mem
    INTERFACE
        uo::uo_macro)

target_include_directories(uo_mem
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_mem>
        $<INSTALL_INTERFACE:include/uo_mem>)

add_library(uo::uo_mem ALIAS uo_mem)

install(TARGETS uo_mem EXPORT uo_memTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_memTargets
    FILE uo_memTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_mem)

install(DIRECTORY include/uo_mem
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_memConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_memConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_memConfigVersion.cmake
    DESTINATION lib/cmake/uo_mem)