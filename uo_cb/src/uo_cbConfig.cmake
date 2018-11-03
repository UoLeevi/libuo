include(CMakeFindDependencyMacro)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_dependency(Threads)

find_dependency(uo_queue 0.1.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_cbTargets.cmake")
