cmake_minimum_required(VERSION 3.12)
project(uo_prog VERSION 0.5.0)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(uo_prog
    uo_prog.c)

target_link_libraries(uo_prog
    PRIVATE
        Threads::Threads)

target_compile_features(uo_prog
    PUBLIC
        c_std_11)

target_include_directories(uo_prog
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_prog>
        $<INSTALL_INTERFACE:include/uo_prog>)

add_library(uo::uo_prog ALIAS uo_prog)

install(TARGETS uo_prog EXPORT uo_progTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_progTargets
    FILE uo_progTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_prog)

install(DIRECTORY include/uo_prog
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_progConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_progConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_progConfigVersion.cmake
    DESTINATION lib/cmake/uo_prog)