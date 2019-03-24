cmake_minimum_required(VERSION 3.12)
project(uo_refcount VERSION 0.6.0)

if(NOT TARGET uo::uo_linklist)
    find_package(uo_linklist CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_linkpool)
    find_package(uo_linkpool CONFIG REQUIRED)
endif()

add_library(uo_refcount
    uo_refcount.c)

target_link_libraries(uo_refcount
    PUBLIC
        uo::uo_linkpool
        uo::uo_linklist)

target_compile_features(uo_refcount
    PUBLIC
        c_std_11)

target_include_directories(uo_refcount
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_refcount>
        $<INSTALL_INTERFACE:include/uo_refcount>)

add_library(uo::uo_refcount ALIAS uo_refcount)

install(TARGETS uo_refcount EXPORT uo_refcountTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_refcountTargets
    FILE uo_refcountTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_refcount)

install(DIRECTORY include/uo_refcount
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_refcountConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_refcountConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_refcountConfigVersion.cmake
    DESTINATION lib/cmake/uo_refcount)