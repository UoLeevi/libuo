include(CMakeFindDependencyMacro)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_dependency(Threads)

find_dependency(uo_finstack 0.5.0)
find_dependency(uo_tcp 0.5.0)
find_dependency(uo_mem 0.5.0)
find_dependency(uo_buf 0.5.0)
find_dependency(uo_cb 0.5.0)
find_dependency(uo_hashtbl 0.5.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_httpTargets.cmake")
