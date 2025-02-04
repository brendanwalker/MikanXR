## Mikan Interprocess Client DLL
## =============================

MESSAGE(STATUS "Generating Mikan Interprocess Client DLL Project")

file(GLOB MIKAN_INTERPROCESS_CLIENT_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_INTERPROCESS_CLIENT_PRIVATE_SRC
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_INTERPROCESS_CLIENT_SRC
	${MIKAN_INTERPROCESS_CLIENT_HEADER}
	${MIKAN_INTERPROCESS_CLIENT_PRIVATE_SRC}
)

source_group("Public" FILES ${MIKAN_INTERPROCESS_CLIENT_HEADER})
source_group("Private" FILES ${MIKAN_INTERPROCESS_CLIENT_PRIVATE_SRC})

list(APPEND MIKAN_INTERPROCESS_CLIENT_INCL_DIRS
	${CMAKE_CURRENT_LIST_DIR}/Public
	${CMAKE_CURRENT_LIST_DIR}/Private
)

list(APPEND MIKAN_INTERPROCESS_CLIENT_REQ_LIBS
	MikanUtility
	MikanCoreApp
)

add_library(MikanInterprocessClient SHARED ${MIKAN_INTERPROCESS_CLIENT_SRC})
target_include_directories(MikanInterprocessClient PRIVATE ${MIKAN_INTERPROCESS_CLIENT_INCL_DIRS})
target_compile_definitions(MikanInterprocessClient PRIVATE MIKAN_INTERPROCESS_CLIENT_EXPORTS)
target_link_libraries(MikanInterprocessClient PRIVATE ${MIKAN_INTERPROCESS_CLIENT_REQ_LIBS})
set_target_properties(MikanInterprocessClient PROPERTIES FOLDER MikanLibraries)
set_target_properties(MikanInterprocessClient PROPERTIES PUBLIC_HEADER "${MIKAN_INTERPROCESS_CLIENT_HEADER}")

# Add dependencies
add_dependencies(MikanInterprocessClient MikanCoreApp)
add_dependencies(MikanInterprocessClient MikanUtility)

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS MikanInterprocessClient
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      PUBLIC_HEADER DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/include)
ELSE() #Linux/Darwin
ENDIF()