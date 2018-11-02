cmake_minimum_required(VERSION 3.7)
project(uo_tcpserv VERSION 0.1.0)

if(NOT TARGET uo::uo_cb)
    find_package(uo_cb CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_err)
    find_package(uo_err CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_queue)
    find_package(uo_queue CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_sock)
    find_package(uo_sock CONFIG REQUIRED)
endif()

add_library(uo_tcpserv
    uo_tcpserv.c
    uo_tcpserv_conf.c)

target_link_libraries(uo_tcpserv
    PUBLIC
        uo::uo_cb
    PRIVATE
        uo::uo_err
        uo::uo_queue
        uo::uo_sock)

target_compile_features(uo_tcpserv
    PUBLIC
        c_std_11)

target_include_directories(uo_tcpserv
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_tcpserv>
        $<INSTALL_INTERFACE:include/uo_tcpserv>)

add_library(uo::uo_tcpserv ALIAS uo_tcpserv)

install(TARGETS uo_tcpserv EXPORT uo_tcpservTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_tcpservTargets
    FILE uo_tcpservTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_tcpserv)

install(DIRECTORY include/uo_tcpserv
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_tcpservConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_tcpservConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_tcpservConfigVersion.cmake
    DESTINATION lib/cmake/uo_tcpserv)