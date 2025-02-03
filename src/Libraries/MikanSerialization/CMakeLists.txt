## Mikan Serialization LIB
## =======================

MESSAGE(STATUS "Generating Mikan Serialization Lib Project")

file(GLOB MIKAN_SERIALIZATION_PRIVATE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"		
)

file(GLOB MIKAN_SERIALIZATION_PUBLIC_SRC
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

list(APPEND MIKAN_SERIALIZATION_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/Private
  ${CMAKE_CURRENT_LIST_DIR}/Public
  ${RFK_GENERATED_ROOT_DIR}/Serialization
  ${RFK_GENERATED_ROOT_DIR}/MikanClientAPI
  ${NLOHMANN_JSON_INCLUDE_DIR}
  ${RFK_INCLUDE_DIR})

list(APPEND MIKAN_SERIALIZATION_REQ_LIBS
  ${RFK_LIBRARIES})
 
add_library(MikanSerializationLib STATIC ${MIKAN_SERIALIZATION_PRIVATE_SRC})
target_include_directories(MikanSerializationLib PRIVATE ${MIKAN_SERIALIZATION_INCL_DIRS})
target_link_libraries(MikanSerializationLib PRIVATE ${MIKAN_SERIALIZATION_REQ_LIBS})
set_target_properties(MikanSerializationLib PROPERTIES FOLDER Serialization)
set_target_properties(MikanSerializationLib PROPERTIES PUBLIC_HEADER "${MIKAN_SERIALIZATION_PUBLIC_SRC}")
set_target_properties(MikanSerializationLib PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(MikanSerializationLib PROPERTIES C_VISIBILITY_PRESET hidden)
target_compile_definitions(MikanSerializationLib PRIVATE ENABLE_SERIALIZATION_REFLECTION)
target_compile_definitions(MikanSerializationLib PRIVATE JSON_DISABLE_ENUM_SERIALIZATION=1)
target_sources(MikanSerializationLib PRIVATE ${MIKAN_SERIALIZATION_PRIVATE_SRC})
target_sources(MikanSerializationLib PUBLIC ${MIKAN_SERIALIZATION_PUBLIC_SRC})
source_group("Private" FILES ${MIKAN_SERIALIZATION_PRIVATE_SRC})
source_group("Public" FILES ${MIKAN_SERIALIZATION_PUBLIC_SRC})

# Create the command to run RefurekuGenerator
add_custom_target(MikanSerializationLibReflection
					WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
					COMMAND "${RFK_GENERATOR_EXE}" "${CMAKE_CURRENT_LIST_DIR}/RefurekuSettings.toml") 
set_target_properties(MikanSerializationLibReflection PROPERTIES FOLDER Serialization)

# Run the RefurekuGenerator BEFORE building the project to refresh generated files
add_dependencies(MikanSerializationLib MikanSerializationLibReflection)

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
    	  
  install(TARGETS MikanSerializationLib
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      PUBLIC_HEADER DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/include)	  

ELSE() #Linux/Darwin
ENDIF()