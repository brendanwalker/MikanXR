## Mikan Object DLL
## ================

MESSAGE(STATUS "Generating Mikan Object Library Project")

file(GLOB MIKAN_OBJECT_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_OBJECT_PRIVATE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_OBJECT_SRC
	${MIKAN_OBJECT_HEADER}
	${MIKAN_OBJECT_PRIVATE_SRC}
)

source_group("Public" FILES ${MIKAN_OBJECT_HEADER})
source_group("Private" FILES ${MIKAN_OBJECT_PRIVATE_SRC})

list(APPEND MIKAN_OBJECT_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/Public
  ${CMAKE_CURRENT_LIST_DIR}/Private)

list(APPEND MIKAN_OBJECT_REQ_LIBS
  MikanCoreApp
  MikanUtility)

add_library(MikanObject SHARED ${MIKAN_OBJECT_SRC})
target_include_directories(MikanObject PRIVATE ${MIKAN_OBJECT_INCL_DIRS})
target_link_libraries(MikanObject PRIVATE ${MIKAN_OBJECT_REQ_LIBS})
set_target_properties(MikanObject PROPERTIES FOLDER MikanLibraries)
set_target_properties(MikanObject PROPERTIES PUBLIC_HEADER "${MIKAN_OBJECT_HEADER}")
set_target_properties(MikanObject PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(MikanObject PRIVATE MIKAN_OBJECT_EXPORTS)

# Add dependencies
add_dependencies(MikanObject MikanCoreApp)
add_dependencies(MikanObject MikanUtility)

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS MikanObject
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      PUBLIC_HEADER DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/include)
ELSE() #Linux/Darwin
ENDIF()