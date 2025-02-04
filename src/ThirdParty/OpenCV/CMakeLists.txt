## Mikan OpenCV LIB
## =================

MESSAGE(STATUS "Generating Mikan OpenCV Lib Project")

file(GLOB MIKAN_OPENCV_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_OPENCV_PRIVATE_SRC
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_OPENCV_SRC
	${MIKAN_OPENCV_HEADER}
	${MIKAN_OPENCV_PRIVATE_SRC}
)

source_group("Public" FILES ${MIKAN_OPENCV_HEADER})
source_group("Private" FILES ${MIKAN_OPENCV_PRIVATE_SRC})

list(APPEND MIKAN_OPENCV_INCL_DIRS
	${CMAKE_CURRENT_LIST_DIR}/Public
	${CMAKE_CURRENT_LIST_DIR}/Private
)

add_library(MikanOpenCV STATIC ${MIKAN_OPENCV_SRC})
target_include_directories(MikanOpenCV PRIVATE ${MIKAN_OPENCV_INCL_DIRS})
set_target_properties(MikanOpenCV PROPERTIES FOLDER ThirdParty)
set_target_properties(MikanOpenCV PROPERTIES PUBLIC_HEADER "${MIKAN_OPENCV_HEADER}")