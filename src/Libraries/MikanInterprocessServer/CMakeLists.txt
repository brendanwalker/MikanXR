## Mikan Interprocess Server DLL
## =============================

MESSAGE(STATUS "Generating Mikan Interprocess Client DLL Project")

file(GLOB MIKAN_INTERPROCESS_SERVER_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_INTERPROCESS_SERVER_PRIVATE_SRC
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_INTERPROCESS_SERVER_SRC
	${MIKAN_INTERPROCESS_SERVER_HEADER}
	${MIKAN_INTERPROCESS_SERVER_PRIVATE_SRC}
)

source_group("Public" FILES ${MIKAN_INTERPROCESS_SERVER_HEADER})
source_group("Private" FILES ${MIKAN_INTERPROCESS_SERVER_PRIVATE_SRC})

list(APPEND MIKAN_INTERPROCESS_SERVER_INCL_DIRS
	${CMAKE_CURRENT_LIST_DIR}/Public
	${CMAKE_CURRENT_LIST_DIR}/Private
)

list(APPEND MIKAN_INTERPROCESS_SERVER_REQ_LIBS
	MikanUtility
	MikanCoreApp
)

add_library(MikanInterprocessServer SHARED ${MIKAN_INTERPROCESS_SERVER_SRC})
target_include_directories(MikanInterprocessServer PRIVATE ${MIKAN_INTERPROCESS_SERVER_INCL_DIRS})
target_compile_definitions(MikanInterprocessServer PRIVATE MIKAN_INTERPROCESS_SERVER_EXPORTS)
target_link_libraries(MikanInterprocessServer PRIVATE ${MIKAN_INTERPROCESS_SERVER_REQ_LIBS})
set_target_properties(MikanInterprocessServer PROPERTIES FOLDER MikanLibraries)
set_target_properties(MikanInterprocessServer PROPERTIES PUBLIC_HEADER "${MIKAN_INTERPROCESS_SERVER_HEADER}")

# Add dependencies
add_dependencies(MikanInterprocessServer MikanCoreApp)
add_dependencies(MikanInterprocessServer MikanUtility)

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS MikanInterprocessServer
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      PUBLIC_HEADER DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/include)
ELSE() #Linux/Darwin
ENDIF()