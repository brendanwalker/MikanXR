## Mikan Renderer DLL
## ==============

MESSAGE(STATUS "Generating Mikan Renderer DLL Project")

file(GLOB MIKAN_RENDERER_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_RENDERER_PRIVATE_SRC
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_RENDERER_SRC
	${MIKAN_RENDERER_HEADER}
	${MIKAN_RENDERER_PRIVATE_SRC}
)

source_group("Public" FILES ${MIKAN_RENDERER_HEADER})
source_group("Private" FILES ${MIKAN_RENDERER_PRIVATE_SRC})

list(APPEND MIKAN_RENDERER_INCL_DIRS
	${CMAKE_CURRENT_LIST_DIR}/Public
	${CMAKE_CURRENT_LIST_DIR}/Private
	${ROOT_DIR}/thirdparty/glm/
	${GLEW_INCLUDE_DIRS}
)

list(APPEND MIKAN_RENDERER_REQ_LIBS 
  ${GLEW_LIBRARIES})

add_library(MikanRenderer SHARED ${MIKAN_RENDERER_SRC})
target_link_libraries(MikanRenderer ${MIKAN_RENDERER_REQ_LIBS})
target_include_directories(MikanRenderer PRIVATE ${MIKAN_RENDERER_INCL_DIRS})
target_compile_definitions(MikanRenderer PRIVATE MIKAN_RENDERER_EXPORTS)
set_target_properties(MikanRenderer PROPERTIES FOLDER MikanLibraries)
set_target_properties(MikanRenderer PROPERTIES PUBLIC_HEADER "${MIKAN_RENDERER_HEADER}")

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS MikanRenderer
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      PUBLIC_HEADER DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/include)
ELSE() #Linux/Darwin
ENDIF()