set(MIKAN_EDITOR_DIR "${CMAKE_CURRENT_LIST_DIR}/Editor")
set(MIKAN_LIBRARIES_DIR "${CMAKE_CURRENT_LIST_DIR}/Libraries")
set(MIKAN_PLUGINS_DIR "${CMAKE_CURRENT_LIST_DIR}/Plugins")
set(MIKAN_PROGRAMS_DIR "${CMAKE_CURRENT_LIST_DIR}/Programs")

MESSAGE(STATUS "Stepping into Libraries")
add_subdirectory(Libraries)

MESSAGE(STATUS "Stepping into Plugins")
add_subdirectory(Plugins)

MESSAGE(STATUS "Stepping into Programs")
add_subdirectory(Programs)

MESSAGE(STATUS "Stepping into Editor")
add_subdirectory(Editor)