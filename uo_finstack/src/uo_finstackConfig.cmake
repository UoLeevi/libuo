include(CMakeFindDependencyMacro)

find_dependency(uo_stack 0.2.0)

include("${CMAKE_CURRENT_LIST_DIR}/uo_finstackTargets.cmake")
