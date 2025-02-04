## Mikan SDL LIB
## =================

MESSAGE(STATUS "Generating Mikan SDL Lib Project")

file(GLOB MIKAN_SDL_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_SDL_PRIVATE_SRC
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_SDL_SRC
	${MIKAN_SDL_HEADER}
	${MIKAN_SDL_PRIVATE_SRC}
)

source_group("Public" FILES ${MIKAN_SDL_HEADER})
source_group("Private" FILES ${MIKAN_SDL_PRIVATE_SRC})

list(APPEND MIKAN_SDL_INCL_DIRS
	${CMAKE_CURRENT_LIST_DIR}/Public
	${CMAKE_CURRENT_LIST_DIR}/Private
)

add_library(MikanSDL STATIC ${MIKAN_SDL_SRC})
target_include_directories(MikanSDL PRIVATE ${MIKAN_SDL_INCL_DIRS})
set_target_properties(MikanSDL PROPERTIES FOLDER ThirdParty)
set_target_properties(MikanSDL PROPERTIES PUBLIC_HEADER "${MIKAN_SDL_HEADER}")