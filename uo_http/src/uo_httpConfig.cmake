include(CMakeFindDependencyMacro)

find_dependency(uo_tcp 0.1.0)
find_dependency(uo_path 0.1.0)
find_dependency(uo_mem 0.1.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_httpTargets.cmake")
