include(CMakeFindDependencyMacro)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_dependency(Threads)

find_dependency(uo_stack 0.2.0)
find_dependency(uo_linklist 0.3.0)
find_dependency(uo_linkpool 0.1.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_cbTargets.cmake")
