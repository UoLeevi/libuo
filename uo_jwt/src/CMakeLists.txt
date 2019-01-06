cmake_minimum_required(VERSION 3.12)
project(uo_jwt VERSION 0.1.0)

find_package(OpenSSL MODULE REQUIRED)

if(NOT TARGET uo::uo_base64)
    find_package(uo_base64 CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_json)
    find_package(uo_json CONFIG REQUIRED)
endif()

add_library(uo_jwt
    uo_jwt.c)

target_link_libraries(uo_jwt
    PUBLIC
        uo::uo_json
    PRIVATE
        uo::uo_base64
        OpenSSL::Crypto)

target_compile_features(uo_jwt
    PUBLIC
        c_std_11)

target_include_directories(uo_jwt
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uo_jwt>
        $<INSTALL_INTERFACE:include/uo_jwt>)

add_library(uo::uo_jwt ALIAS uo_jwt)

install(TARGETS uo_jwt EXPORT uo_jwtTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT uo_jwtTargets
    FILE uo_jwtTargets.cmake
    NAMESPACE uo::
    DESTINATION lib/cmake/uo_jwt)

install(DIRECTORY include/uo_jwt
    DESTINATION include)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(uo_jwtConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES uo_jwtConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/uo_jwtConfigVersion.cmake
    DESTINATION lib/cmake/uo_jwt)