## Mikan API DLL
## =======================

MESSAGE(STATUS "Generating Mikan Object Library Project")

file(GLOB MIKAN_OBJECT_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_OBJECT_PRIVATE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

source_group("Public" FILES ${MIKAN_OBJECT_HEADER})
source_group("Private\\API" FILES ${MIKAN_OBJECT_PRIVATE_SRC})
source_group("Private\\Utility" FILES ${MIKAN_OBJECT_UTILITY_SRC})

list(APPEND MIKAN_OBJECT_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/Public
  ${CMAKE_CURRENT_LIST_DIR}/Private)

list(APPEND MIKAN_OBJECT_REQ_LIBS
  MikanSerialization
  MikanUtilityLib)

add_library(MikanAPI SHARED ${MIKAN_OBJECT_SRC})
target_include_directories(MikanAPI PRIVATE ${MIKAN_OBJECT_INCL_DIRS})
target_link_libraries(MikanAPI PRIVATE ${MIKAN_OBJECT_REQ_LIBS})
set_target_properties(MikanAPI PROPERTIES FOLDER ClientAPI)
set_target_properties(MikanAPI PROPERTIES PUBLIC_HEADER "${MIKAN_OBJECT_HEADER}")
set_target_properties(MikanAPI PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(MikanAPI PRIVATE MIKAN_API_EXPORTS)
target_compile_definitions(MikanAPI PRIVATE ENABLE_MIKANAPI_REFLECTION)
target_compile_definitions(MikanAPI PRIVATE ENABLE_MIKANCORE_REFLECTION)
target_compile_definitions(MikanAPI PRIVATE ENABLE_SERIALIZATION_REFLECTION)
target_compile_definitions(MikanAPI PRIVATE JSON_DISABLE_ENUM_SERIALIZATION=1)

# Make sure MikanSerialization is build before MikanAPI
add_dependencies(MikanAPI MikanSerialization)

# Create the command to run RefurekuGenerator
add_custom_target(MikanAPIReflection
					WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
					COMMAND "${RFK_GENERATOR_EXE}" "${CMAKE_CURRENT_LIST_DIR}/RefurekuSettings.toml") 
set_target_properties(MikanAPIReflection PROPERTIES FOLDER ClientAPI)

# Run the RefurekuGenerator BEFORE building the project to refresh generated files
add_dependencies(MikanAPI MikanAPIReflection)

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
    	  
  install(TARGETS MikanAPI
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      PUBLIC_HEADER DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/include)	  

ELSE() #Linux/Darwin
ENDIF()