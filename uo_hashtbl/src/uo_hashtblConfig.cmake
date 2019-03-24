include(CMakeFindDependencyMacro)

find_dependency(uo_util 0.6.0)
find_dependency(uo_linklist 0.6.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_hashtblTargets.cmake")
