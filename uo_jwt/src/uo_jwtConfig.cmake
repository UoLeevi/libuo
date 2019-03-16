include(CMakeFindDependencyMacro)

if(WIN32)
    set(OPENSSL_ROOT_DIR "/Program files/OpenSSL")
endif()
find_dependency(OpenSSL)

find_dependency(uo_base64 0.5.0)
find_dependency(uo_json 0.5.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_jwtTargets.cmake")