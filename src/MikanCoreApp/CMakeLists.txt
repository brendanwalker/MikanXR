## Mikan Core App DLL
## ==================

MESSAGE(STATUS "Generating Mikan Core App DLL Project")

file(GLOB MIKAN_CORE_APP_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_CORE_APP_PRIVATE_SRC
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_CORE_APP_SRC
	${MIKAN_CORE_APP_HEADER}
	${MIKAN_CORE_APP_PRIVATE_SRC}
)

source_group("Public" FILES ${MIKAN_CORE_APP_HEADER})
source_group("Private" FILES ${MIKAN_CORE_APP_PRIVATE_SRC})

list(APPEND MIKAN_CORE_APP_INCL_DIRS
	${CMAKE_CURRENT_LIST_DIR}/Public
	${CMAKE_CURRENT_LIST_DIR}/Private
)

list(APPEND MIKAN_CORE_APP_REQ_LIBS
	MikanUtility
)

add_library(MikanCoreApp SHARED ${MIKAN_CORE_APP_SRC})
target_include_directories(MikanCoreApp PRIVATE ${MIKAN_CORE_APP_INCL_DIRS})
target_link_libraries(MikanCoreApp PRIVATE ${MIKAN_CORE_APP_REQ_LIBS})
set_target_properties(MikanCoreApp PROPERTIES FOLDER MikanLibraries)
set_target_properties(MikanCoreApp PROPERTIES PUBLIC_HEADER "${MIKAN_CORE_APP_HEADER}")
