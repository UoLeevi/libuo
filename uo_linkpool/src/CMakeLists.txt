cmake_minimum_required(VERSION 3.12)
project(uo_linkpool VERSION 0.5.0)

if(NOT TARGET uo::uo_macro)
    find_package(uo_macro CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_stack)
    find_package(uo_stack CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_linklist)
    find_package(uo_linklist CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_queue)
    find_package(uo_queue CONFIG REQUIRED)
endif()

add_library(uo_linkpool INTERFACE)

target_link_libraries(uo_linkpool
    INTERFACE
        uo::uo_macro
        uo::uo_stack
        uo::uo_linklist
        uo::uo_queue)

target_compile_features(uo_linkpool
    INTERFACE
        c_std_11)

target_include_directories(uo_linkpool
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_linkpool>
        $<INSTALL_INTERFACE:include/uo_linkpool>)

add_library(uo::uo_linkpool ALIAS uo_linkpool)

install(TARGETS uo_linkpool EXPORT uo_linkpoolTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_linkpoolTargets
    FILE uo_linkpoolTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_linkpool)

install(DIRECTORY include/uo_linkpool
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_linkpoolConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_linkpoolConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_linkpoolConfigVersion.cmake
    DESTINATION lib/cmake/uo_linkpool)