cmake_minimum_required(VERSION 3.7)
project(uo_queue VERSION 0.1.0)

add_library(uo_queue
    uo_queue.c)

target_compile_features(uo_queue
    PUBLIC
        c_std_11)

target_include_directories(uo_queue
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_queue>
        $<INSTALL_INTERFACE:include/uo_queue>)

add_library(uo::uo_queue ALIAS uo_queue)

install(TARGETS uo_queue EXPORT uo_queueTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_queueTargets
    FILE uo_queueTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_queue)

install(DIRECTORY include/uo_queue
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_queueConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_queueConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_queueConfigVersion.cmake
    DESTINATION lib/cmake/uo_queue)