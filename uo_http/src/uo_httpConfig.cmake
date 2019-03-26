include(CMakeFindDependencyMacro)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_dependency(Threads)

find_dependency(uo_refcount 0.6.0)
find_dependency(uo_refstack 0.6.0)
find_dependency(uo_tcp 0.6.0)
find_dependency(uo_mem 0.6.0)
find_dependency(uo_util 0.6.0)
find_dependency(uo_json 0.6.0)
find_dependency(uo_buf 0.6.0)
find_dependency(uo_cb 0.6.0)
find_dependency(uo_hashtbl 0.6.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_httpTargets.cmake")
