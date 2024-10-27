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

file(GLOB MIKAN_CLIENT_API_UTILITY_SRC
    "${CMAKE_CURRENT_LIST_DIR}/../AppCore/Logger.h"
    "${CMAKE_CURRENT_LIST_DIR}/../AppCore/Logger.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/../Utility/JsonUtils.h"
)

file(GLOB MIKAN_CLIENT_API_SRC
	${MIKAN_CLIENT_API_HEADER}
	${MIKAN_CLIENT_API_PRIVATE_SRC}
	${MIKAN_CLIENT_API_UTILITY_SRC}
)

source_group("Public" FILES ${MIKAN_CLIENT_API_HEADER})
source_group("Private\\API" FILES ${MIKAN_CLIENT_API_PRIVATE_SRC})
source_group("Private\\Utility" FILES ${MIKAN_CLIENT_API_UTILITY_SRC})

list(APPEND MIKAN_CLIENT_API_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/Public
  ${CMAKE_CURRENT_LIST_DIR}/Private
  ${CMAKE_CURRENT_LIST_DIR}/../AppCore
  ${CMAKE_CURRENT_LIST_DIR}/../ClientCore/Public/
  ${CMAKE_CURRENT_LIST_DIR}/../Serialization/Public/
  ${CMAKE_CURRENT_LIST_DIR}/../Utility
  ${RFK_GENERATED_ROOT_DIR}/MikanCore
  ${RFK_GENERATED_ROOT_DIR}/MikanAPI
  ${RFK_GENERATED_ROOT_DIR}/Serialization
  ${NLOHMANN_JSON_INCLUDE_DIR}
  ${RFK_INCLUDE_DIR})

list(APPEND MIKAN_CLIENT_API_REQ_LIBS
  ${RFK_LIBRARIES}
  MikanCore
  MikanSerialization)
    
add_library(MikanAPI SHARED ${MIKAN_CLIENT_API_SRC})
target_include_directories(MikanAPI PRIVATE ${MIKAN_CLIENT_API_INCL_DIRS})
target_link_libraries(MikanAPI PRIVATE ${MIKAN_CLIENT_API_REQ_LIBS})
set_target_properties(MikanAPI PROPERTIES FOLDER ClientAPI)
set_target_properties(MikanAPI PROPERTIES PUBLIC_HEADER "${MIKAN_CLIENT_API_HEADER}")
set_target_properties(MikanAPI PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(MikanAPI PRIVATE MIKAN_API_EXPORTS)
target_compile_definitions(MikanAPI PRIVATE ENABLE_MIKANAPI_REFLECTION)
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