include(CMakeFindDependencyMacro)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_dependency(Threads)

find_dependency(uo_err 0.1.0)
find_dependency(uo_cb 0.3.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_ioTargets.cmake")
