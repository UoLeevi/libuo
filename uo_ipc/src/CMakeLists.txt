cmake_minimum_required(VERSION 3.12)
project(uo_ipc VERSION 0.1.0)

if(NOT TARGET uo::uo_buf)
    find_package(uo_buf CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_tcp)
    find_package(uo_tcp CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_cb)
    find_package(uo_cb CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_err)
    find_package(uo_err CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_mem)
    find_package(uo_mem CONFIG REQUIRED)
endif()

add_library(uo_ipc
    uo_ipc.c
    uo_ipcmsg.c
    uo_ipcs.c)

target_link_libraries(uo_ipc
    PUBLIC
        uo::uo_cb
        uo::uo_buf
    PRIVATE
        uo::uo_tcp
        uo::uo_err
        uo::uo_mem)

target_compile_features(uo_ipc
    PUBLIC
        c_std_11)

target_include_directories(uo_ipc
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_ipc>
        $<INSTALL_INTERFACE:include/uo_ipc>)

add_library(uo::uo_ipc ALIAS uo_ipc)

install(TARGETS uo_ipc EXPORT uo_ipcTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_ipcTargets
    FILE uo_ipcTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_ipc)

install(DIRECTORY include/uo_ipc
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_ipcConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_ipcConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_ipcConfigVersion.cmake
    DESTINATION lib/cmake/uo_ipc)
