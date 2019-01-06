cmake_minimum_required(VERSION 3.12)
project(uo_sock VERSION 0.1.0)

add_library(uo_sock
    uo_sock.c)

if(WIN32)
    target_compile_definitions(uo_sock PUBLIC _WIN32_WINNT=0x0601)
    target_link_libraries(uo_sock PUBLIC ws2_32)
endif()

target_compile_features(uo_sock
    PUBLIC
        c_std_11)

target_include_directories(uo_sock
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_sock>
        $<INSTALL_INTERFACE:include/uo_sock>)

add_library(uo::uo_sock ALIAS uo_sock)

install(TARGETS uo_sock EXPORT uo_sockTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_sockTargets
    FILE uo_sockTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_sock)

install(DIRECTORY include/uo_sock
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_sockConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_sockConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_sockConfigVersion.cmake
    DESTINATION lib/cmake/uo_sock)