## Mikan DataTypes
## ==================

MESSAGE(STATUS "Generating Mikan DataTypes Project")

file(GLOB MIKAN_DATATYPES_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

source_group("Public" FILES ${MIKAN_DATATYPES_HEADER})

list(APPEND MIKAN_DATATYPES_INCL_DIRS
	${CMAKE_CURRENT_LIST_DIR}/Public
)