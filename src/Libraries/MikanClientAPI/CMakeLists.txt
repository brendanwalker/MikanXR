## Mikan API DLL
## =======================

MESSAGE(STATUS "Generating Mikan API DLL Project")

file(GLOB MIKAN_CLIENT_API_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_CLIENT_API_PRIVATE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_CLIENT_API_SRC
	${MIKAN_CLIENT_API_HEADER}
	${MIKAN_CLIENT_API_PRIVATE_SRC}
)

source_group("Public" FILES ${MIKAN_CLIENT_API_HEADER})
source_group("Private\\API" FILES ${MIKAN_CLIENT_API_PRIVATE_SRC})

list(APPEND MIKAN_CLIENT_API_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/Public
  ${CMAKE_CURRENT_LIST_DIR}/Private  
  ${MIKAN_LIBRARIES_DIR}/MikanClientCore/Public
  ${MIKAN_LIBRARIES_DIR}/MikanSerialization/Public
  ${MIKAN_LIBRARIES_DIR}/MikanCoreApp/Public
  ${MIKAN_LIBRARIES_DIR}/MikanUtility/Public
  ${RFK_GENERATED_ROOT_DIR}/MikanCore
  ${RFK_GENERATED_ROOT_DIR}/MikanClientApiDll
  ${RFK_GENERATED_ROOT_DIR}/Serialization
  ${RFK_INCLUDE_DIR}
  ${NLOHMANN_JSON_INCLUDE_DIR}
)

list(APPEND MIKAN_CLIENT_API_REQ_LIBS
  ${RFK_LIBRARIES}
  MikanClientCoreLib
  MikanCoreAppLib
  MikanSerializationLib
  MikanUtilityLib)

add_library(MikanClientApiDll SHARED ${MIKAN_CLIENT_API_SRC})
target_include_directories(MikanClientApiDll PRIVATE ${MIKAN_CLIENT_API_INCL_DIRS})
target_link_libraries(MikanClientApiDll PRIVATE ${MIKAN_CLIENT_API_REQ_LIBS})
set_target_properties(MikanClientApiDll PROPERTIES FOLDER ClientAPI)
set_target_properties(MikanClientApiDll PROPERTIES PUBLIC_HEADER "${MIKAN_CLIENT_API_HEADER}")
set_target_properties(MikanClientApiDll PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(MikanClientApiDll PRIVATE MIKAN_API_EXPORTS)
target_compile_definitions(MikanClientApiDll PRIVATE ENABLE_MikanClientApiDll_REFLECTION)
target_compile_definitions(MikanClientApiDll PRIVATE ENABLE_MIKANCORE_REFLECTION)
target_compile_definitions(MikanClientApiDll PRIVATE ENABLE_SERIALIZATION_REFLECTION)
target_compile_definitions(MikanClientApiDll PRIVATE JSON_DISABLE_ENUM_SERIALIZATION=1)

add_library(MikanClientApiLib STATIC ${MIKAN_CLIENT_API_SRC})
target_include_directories(MikanClientApiLib PRIVATE ${MIKAN_CLIENT_API_INCL_DIRS})
target_link_libraries(MikanClientApiLib PRIVATE ${MIKAN_CLIENT_API_REQ_LIBS})
set_target_properties(MikanClientApiLib PROPERTIES FOLDER ClientAPI)
set_target_properties(MikanClientApiLib PROPERTIES PUBLIC_HEADER "${MIKAN_CLIENT_API_HEADER}")
set_target_properties(MikanClientApiLib PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(MikanClientApiLib PRIVATE ENABLE_MikanClientApiDll_REFLECTION)
target_compile_definitions(MikanClientApiLib PRIVATE ENABLE_MIKANCORE_REFLECTION)
target_compile_definitions(MikanClientApiLib PRIVATE ENABLE_SERIALIZATION_REFLECTION)
target_compile_definitions(MikanClientApiLib PRIVATE JSON_DISABLE_ENUM_SERIALIZATION=1)

# Make sure MikanSerialization is build before MikanClientApi
add_dependencies(MikanClientApiDll MikanSerializationLib)
add_dependencies(MikanClientApiLib MikanSerializationLib)

# Create the command to run RefurekuGenerator
add_custom_target(MikanClientApiDllReflection
					WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
					COMMAND "${RFK_GENERATOR_EXE}" "${CMAKE_CURRENT_LIST_DIR}/RefurekuSettings.toml") 
set_target_properties(MikanClientApiDllReflection PROPERTIES FOLDER ClientAPI)

# Run the RefurekuGenerator BEFORE building the project to refresh generated files
add_dependencies(MikanClientApiDll MikanClientApiDllReflection)
add_dependencies(MikanClientApiLib MikanClientApiDllReflection)

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
    	  
  install(TARGETS MikanClientApiDll
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      PUBLIC_HEADER DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/include)	  

ELSE() #Linux/Darwin
ENDIF()