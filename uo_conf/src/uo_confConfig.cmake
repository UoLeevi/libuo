include(CMakeFindDependencyMacro)

find_dependency(uo_err 0.5.0)
find_dependency(uo_hashtbl 0.5.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_confTargets.cmake")
