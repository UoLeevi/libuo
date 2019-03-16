include(CMakeFindDependencyMacro)

find_dependency(uo_macro 0.5.0)
find_dependency(uo_stack 0.5.0)
find_dependency(uo_linklist 0.5.0)
find_dependency(uo_queue 0.5.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_linkpoolTargets.cmake")
