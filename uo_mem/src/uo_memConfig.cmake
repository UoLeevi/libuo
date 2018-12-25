include(CMakeFindDependencyMacro)

find_dependency(uo_macro 0.1.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_memTargets.cmake")