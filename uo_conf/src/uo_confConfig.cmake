include(CMakeFindDependencyMacro)

find_dependency(uo_err 0.1.0)
find_dependency(uo_strhashtbl 0.1.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_confTargets.cmake")
