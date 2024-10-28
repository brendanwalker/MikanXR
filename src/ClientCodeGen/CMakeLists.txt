# Mikan Client Code Generator
file(GLOB CLIENT_CODEGEN_SRC
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
    "${ROOT_DIR}/src/AppCore/Logger.h"
    "${ROOT_DIR}/src/AppCore/Logger.cpp"
    "${ROOT_DIR}/src/Utility/StringUtils.h"
    "${ROOT_DIR}/src/Utility/StringUtils.cpp"
)

list(APPEND CLIENT_CODEGEN_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}
  ${ROOT_DIR}/src/AppCore/
  ${ROOT_DIR}/src/ClientAPI/Public
  ${ROOT_DIR}/src/ClientCore/Public
  ${ROOT_DIR}/src/Serialization/Public
  ${ROOT_DIR}/src/Utility/
  ${RFK_GENERATED_ROOT_DIR}/Serialization
  ${RFK_GENERATED_ROOT_DIR}/MikanClientAPI
  ${NLOHMANN_JSON_INCLUDE_DIR}
  ${RFK_INCLUDE_DIR})

list(APPEND CLIENT_CODEGEN_REQ_LIBS
  ${RFK_LIBRARIES}
  MikanCore
  MikanAPI
  MikanSerialization)

add_executable(MikanClientCodeGen ${CLIENT_CODEGEN_SRC})
target_include_directories(MikanClientCodeGen PUBLIC ${CLIENT_CODEGEN_INCL_DIRS})
target_link_libraries(MikanClientCodeGen ${CLIENT_CODEGEN_REQ_LIBS})
target_compile_definitions(MikanClientCodeGen PRIVATE ENABLE_SERIALIZATION_REFLECTION)
target_compile_definitions(MikanClientCodeGen PRIVATE ENABLE_MIKANAPI_REFLECTION)
set_target_properties(MikanClientCodeGen PROPERTIES FOLDER ClientAPI)

# Post build - copy runtime dependencies to binary build folder (for debugging)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  set_property(TARGET MikanClientCodeGen PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:MikanClientCodeGen>")

  add_custom_command(
    TARGET MikanClientCodeGen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanCore>/MikanCore.dll
          $<TARGET_FILE_DIR:MikanClientCodeGen>)
  add_custom_command(
    TARGET MikanClientCodeGen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanAPI>/MikanAPI.dll
          $<TARGET_FILE_DIR:MikanClientCodeGen>)
  add_custom_command(
    TARGET MikanClientCodeGen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanSerialization>/MikanSerialization.dll
          $<TARGET_FILE_DIR:MikanClientCodeGen>)
  add_custom_command(
	TARGET MikanClientCodeGen POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${RFK_SHARED_LIBRARIES}"
	$<TARGET_FILE_DIR:MikanClientCodeGen>)
ELSE() #Linux/Darwin
ENDIF()

# Install
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS MikanClientCodeGen
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib)
ELSE() #Linux/Darwin
ENDIF()