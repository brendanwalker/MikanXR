## Mikan Core DLL
## =======================

MESSAGE(STATUS "Generating Mikan Core DLL Project")

file(GLOB MIKAN_CLIENT_CORE_PRIVATE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_CLIENT_CORE_INTERPROCESS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/../Interprocess/InterprocessMessages.h"
    "${CMAKE_CURRENT_LIST_DIR}/../Interprocess/InterprocessMessages.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../Interprocess/InterprocessRenderTargetShared.h"
    "${CMAKE_CURRENT_LIST_DIR}/../Interprocess/InterprocessRenderTargetShared.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../Interprocess/InterprocessRenderTargetWriter.h"
    "${CMAKE_CURRENT_LIST_DIR}/../Interprocess/InterprocessRenderTargetWriter.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../Interprocess/WebsocketInterprocessMessageClient.h"
    "${CMAKE_CURRENT_LIST_DIR}/../Interprocess/WebsocketInterprocessMessageClient.cpp"
)

file(GLOB MIKAN_CLIENT_CORE_SPOUT2_DX_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Private/SpoutDX/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Private/SpoutDX/*.h"
)

file(GLOB MIKAN_CLIENT_CORE_PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_CLIENT_CORE_LIBRARY_SRC
    ${MIKAN_CLIENT_CORE_PUBLIC}
    ${MIKAN_CLIENT_CORE_PRIVATE_SRC}
    ${MIKAN_CLIENT_CORE_INTERPROCESS_SRC}
    ${MIKAN_CLIENT_CORE_SPOUT2_DX_SRC}
)

# Remove problematic files from the unity builds (header file inclusion ordering issues)
set_source_files_properties(${MIKAN_CLIENT_CORE_SPOUT2_DX_SRC} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/Private/MikanClient.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/../Interprocess/InterprocessRenderTargetWriter.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/../Interprocess/WebsocketInterprocessMessageClient.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

source_group("Public" FILES ${MIKAN_CLIENT_CORE_PUBLIC})
source_group("Private" FILES ${MIKAN_CLIENT_CORE_PRIVATE_SRC})
source_group("Private\\Interprocess" FILES ${MIKAN_CLIENT_CORE_INTERPROCESS_SRC})
source_group("Private\\SpoutDX" FILES ${MIKAN_CLIENT_CORE_SPOUT2_DX_SRC})
source_group("Private\\Utility" FILES ${MIKAN_CLIENT_CORE_UTILITY_SRC})

list(APPEND MIKAN_CLIENT_CORE_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/../AppCore
  ${CMAKE_CURRENT_LIST_DIR}/Public
  ${CMAKE_CURRENT_LIST_DIR}/Private
  ${CMAKE_CURRENT_LIST_DIR}/Private/SpoutDX
  ${CMAKE_CURRENT_LIST_DIR}/../Serialization/Public
  ${CMAKE_CURRENT_LIST_DIR}/../Interprocess
  ${CMAKE_CURRENT_LIST_DIR}/../MikanCoreApp/Public
  ${CMAKE_CURRENT_LIST_DIR}/../MikanUtility/Public
  ${RFK_GENERATED_ROOT_DIR}/MikanCore
  ${RFK_GENERATED_ROOT_DIR}/Serialization
  ${NLOHMANN_JSON_INCLUDE_DIR}
  ${RFK_INCLUDE_DIR}
  ${SPOUT2_INCLUDE_DIRS}
  ${IXWEBSOCKET_INCLUDE_DIR}
  ${LOCKFREEQUEUE_INCLUDE_DIR})

list(APPEND MIKAN_CLIENT_REQ_LIBS
  ${SPOUT2_LIBRARIES}
  ${MIKAN_EXTRA_LIBS}
  ${RFK_LIBRARIES}
  ixwebsocket
  MikanCoreAppDll
  MikanSerialization
  MikanUtilityLib)

add_library(MikanCore SHARED ${MIKAN_CLIENT_CORE_LIBRARY_SRC})
target_include_directories(MikanCore PRIVATE ${MIKAN_CLIENT_CORE_INCL_DIRS})
target_link_libraries(MikanCore PRIVATE ${MIKAN_CLIENT_REQ_LIBS})
set_target_properties(MikanCore PROPERTIES FOLDER ClientAPI)
set_target_properties(MikanCore PROPERTIES PUBLIC_HEADER "${MIKAN_CLIENT_CORE_PUBLIC}")
set_target_properties(MikanCore PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(MikanCore PROPERTIES C_VISIBILITY_PRESET hidden)
target_compile_definitions(MikanCore PRIVATE MIKAN_CORE_EXPORTS)
target_compile_definitions(MikanCore PRIVATE ENABLE_MIKANCORE_REFLECTION)
target_compile_definitions(MikanCore PRIVATE ENABLE_SERIALIZATION_REFLECTION)
target_compile_definitions(MikanCore PRIVATE ENABLE_SPOUT_DX)
target_compile_definitions(MikanCore PRIVATE JSON_DISABLE_ENUM_SERIALIZATION=1)

# Make sure MikanSerialization is build before MikanCore
add_dependencies(MikanCore MikanSerialization)

# Create the command to run RefurekuGenerator
add_custom_target(MikanCoreReflection
					WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
					COMMAND "${RFK_GENERATOR_EXE}" "${CMAKE_CURRENT_LIST_DIR}/RefurekuSettings.toml") 
set_target_properties(MikanCoreReflection PROPERTIES FOLDER ClientAPI)

# Run the RefurekuGenerator BEFORE building the project to refresh generated files
add_dependencies(MikanCore MikanCoreReflection)

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
    
  install(TARGETS MikanCore
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      PUBLIC_HEADER DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/include)
	  
ELSE() #Linux/Darwin
ENDIF()