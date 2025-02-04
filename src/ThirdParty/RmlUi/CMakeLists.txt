## Mikan RmlUi LIB
## =================

MESSAGE(STATUS "Generating Mikan RmlUi Lib Project")

file(GLOB MIKAN_RMLUI_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_RMLUI_PRIVATE_SRC
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_RMLUI_SRC
	${MIKAN_RMLUI_HEADER}
	${MIKAN_RMLUI_PRIVATE_SRC}
)

source_group("Public" FILES ${MIKAN_RMLUI_HEADER})
source_group("Private" FILES ${MIKAN_RMLUI_PRIVATE_SRC})

list(APPEND MIKAN_RMLUI_INCL_DIRS
	${CMAKE_CURRENT_LIST_DIR}/Public
	${CMAKE_CURRENT_LIST_DIR}/Private
)

add_library(MikanRmlUI STATIC ${MIKAN_RMLUI_SRC})
target_include_directories(MikanRmlUI PRIVATE ${MIKAN_RMLUI_INCL_DIRS})
set_target_properties(MikanRmlUI PROPERTIES FOLDER ThirdParty)
set_target_properties(MikanRmlUI PROPERTIES PUBLIC_HEADER "${MIKAN_RMLUI_HEADER}")