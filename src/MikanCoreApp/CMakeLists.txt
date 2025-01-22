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
	MikanUtilityLib
)

add_library(MikanCoreAppDll SHARED ${MIKAN_CORE_APP_SRC})
target_include_directories(MikanCoreAppDll PRIVATE ${MIKAN_CORE_APP_INCL_DIRS})
target_compile_definitions(MikanCoreAppDll PRIVATE MIKAN_COREAPP_EXPORTS)
target_link_libraries(MikanCoreAppDll PRIVATE ${MIKAN_CORE_APP_REQ_LIBS})
set_target_properties(MikanCoreAppDll PROPERTIES FOLDER MikanLibraries)
set_target_properties(MikanCoreAppDll PROPERTIES PUBLIC_HEADER "${MIKAN_CORE_APP_HEADER}")

add_library(MikanCoreAppLib STATIC ${MIKAN_CORE_APP_SRC})
target_include_directories(MikanCoreAppLib PRIVATE ${MIKAN_CORE_APP_INCL_DIRS})
target_link_libraries(MikanCoreAppLib PRIVATE ${MIKAN_CORE_APP_REQ_LIBS})
set_target_properties(MikanCoreAppLib PROPERTIES FOLDER MikanLibraries)
set_target_properties(MikanCoreAppLib PROPERTIES PUBLIC_HEADER "${MIKAN_CORE_APP_HEADER}")