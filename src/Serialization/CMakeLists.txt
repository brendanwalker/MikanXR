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
 
add_library(MikanSerialization STATIC ${MIKAN_SERIALIZATION_PRIVATE_SRC})
target_include_directories(MikanSerialization PRIVATE ${MIKAN_SERIALIZATION_INCL_DIRS})
set_target_properties(MikanSerialization PROPERTIES FOLDER Serialization)
set_target_properties(MikanSerialization PROPERTIES PUBLIC_HEADER "${MIKAN_SERIALIZATION_PUBLIC_SRC}")
set_target_properties(MikanSerialization PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(MikanSerialization PROPERTIES C_VISIBILITY_PRESET hidden)
target_compile_definitions(MikanSerialization PRIVATE ENABLE_SERIALIZATION_REFLECTION)
target_compile_definitions(MikanSerialization PRIVATE JSON_DISABLE_ENUM_SERIALIZATION=1)
target_sources(MikanSerialization PRIVATE ${MIKAN_SERIALIZATION_PRIVATE_SRC})
target_sources(MikanSerialization PUBLIC ${MIKAN_SERIALIZATION_PUBLIC_SRC})
source_group("Private" FILES ${MIKAN_SERIALIZATION_PRIVATE_SRC})
source_group("Public" FILES ${MIKAN_SERIALIZATION_PUBLIC_SRC})

# Create the command to run RefurekuGenerator
add_custom_target(MikanSerializationReflection
					WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
					COMMAND "${RFK_GENERATOR_EXE}" "${CMAKE_CURRENT_LIST_DIR}/RefurekuSettings.toml") 
set_target_properties(MikanSerializationReflection PROPERTIES FOLDER Serialization)

# Run the RefurekuGenerator BEFORE building the project to refresh generated files
add_dependencies(MikanSerialization MikanSerializationReflection)