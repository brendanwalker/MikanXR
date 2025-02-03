## Mikan Utility Static LIB
## ========================

MESSAGE(STATUS "Generating Mikan Utility Static LIB Project")

file(GLOB MIKAN_UTILITY_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_UTILITY_PRIVATE_SRC
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_UTILITY_SRC
	${MIKAN_UTILITY_HEADER}
	${MIKAN_UTILITY_PRIVATE_SRC}
)

source_group("Public" FILES ${MIKAN_UTILITY_HEADER})
source_group("Private" FILES ${MIKAN_UTILITY_PRIVATE_SRC})

list(APPEND MIKAN_UTILITY_INCL_DIRS
	${CMAKE_CURRENT_LIST_DIR}/Public
	${CMAKE_CURRENT_LIST_DIR}/Private
)

add_library(MikanUtilityLib STATIC ${MIKAN_UTILITY_SRC})
target_include_directories(MikanUtilityLib PRIVATE ${MIKAN_UTILITY_INCL_DIRS})
set_target_properties(MikanUtilityLib PROPERTIES FOLDER MikanLibraries)
set_target_properties(MikanUtilityLib PROPERTIES PUBLIC_HEADER "${MIKAN_UTILITY_HEADER}")
