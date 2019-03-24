include(CMakeFindDependencyMacro)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_dependency(Threads)

if(WIN32)
    set(OPENSSL_ROOT_DIR "/Program files/OpenSSL")
endif()
find_dependency(OpenSSL)

find_dependency(uo_cb 0.6.0)
find_dependency(uo_err 0.6.0)
find_dependency(uo_mem 0.6.0)
find_dependency(uo_queue 0.6.0)
find_dependency(uo_sock 0.6.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_httpcTargets.cmake")
