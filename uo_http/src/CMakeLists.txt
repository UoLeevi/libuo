cmake_minimum_required(VERSION 3.7)
project(uo_http VERSION 0.1.0)

if(NOT TARGET uo::uo_tcp)
    find_package(uo_tcp CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_mem)
    find_package(uo_mem CONFIG REQUIRED)
endif()

add_library(uo_http
    uo_http_header.c
    uo_http_request.c
    uo_http_response.c
    uo_http_server.c
    uo_http.c)

target_link_libraries(uo_http
    PRIVATE
        uo::uo_tcp
        uo::uo_mem)

target_compile_features(uo_http
    PUBLIC
        c_std_11)

target_include_directories(uo_http
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_http>
        $<INSTALL_INTERFACE:include/uo_http>)

add_library(uo::uo_http ALIAS uo_http)

install(TARGETS uo_http EXPORT uo_httpTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_httpTargets
    FILE uo_httpTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_http)

install(DIRECTORY include/uo_http
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_httpConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_httpConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_httpConfigVersion.cmake
    DESTINATION lib/cmake/uo_http)