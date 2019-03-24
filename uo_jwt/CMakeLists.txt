cmake_minimum_required(VERSION 3.12)
project(uo_jwt VERSION 0.6.0)

if(WIN32)
    set(OPENSSL_ROOT_DIR "/Program files/OpenSSL")
endif()

add_subdirectory(src)

enable_testing()
add_subdirectory(test)
