include(CMakeFindDependencyMacro)

find_dependency(uo_refcount 0.6.0)
find_dependency(uo_stack 0.6.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_refstackTargets.cmake")
