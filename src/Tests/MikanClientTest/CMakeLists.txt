# Mikan Client Test App
file(GLOB MIKAN_TEST_SRC
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
    "${ROOT_DIR}/src/AppCore/Logger.h"
    "${ROOT_DIR}/src/AppCore/Logger.cpp"
    "${ROOT_DIR}/src/Renderer/GlBufferBlock.h"
    "${ROOT_DIR}/src/Renderer/GlBufferBlock.cpp"
    "${ROOT_DIR}/src/Renderer/GlCommon.h"
    "${ROOT_DIR}/src/Renderer/GlCommon.cpp"
    "${ROOT_DIR}/src/Renderer/GlProgram.h"
    "${ROOT_DIR}/src/Renderer/GlProgram.cpp"
    "${ROOT_DIR}/src/Renderer/GlProgramConstants.h"
    "${ROOT_DIR}/src/Renderer/GlProgramConstants.cpp"
    "${ROOT_DIR}/src/Renderer/GlShaderVar.h"
    "${ROOT_DIR}/src/Renderer/GlShaderVar.cpp"
    "${ROOT_DIR}/src/Renderer/GlTexture.h"
    "${ROOT_DIR}/src/Renderer/GlTexture.cpp"
    "${ROOT_DIR}/src/Renderer/GlUniformBlock.h"
    "${ROOT_DIR}/src/Renderer/GlUniformBlock.cpp"
    "${ROOT_DIR}/src/Renderer/GlVertexConstants.h"
    "${ROOT_DIR}/src/Renderer/GlVertexConstants.cpp"
    "${ROOT_DIR}/src/Renderer/GlVertexDefinition.h"
    "${ROOT_DIR}/src/Renderer/GlVertexDefinition.cpp"
    "${ROOT_DIR}/src/Utility/StringUtils.h"
    "${ROOT_DIR}/src/Utility/StringUtils.cpp"
)

list(APPEND MIKAN_TEST_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}
  ${ROOT_DIR}/src/AppCore/
  ${ROOT_DIR}/src/Client/Public
  ${ROOT_DIR}/src/Client/Public/Core
  ${ROOT_DIR}/src/Renderer/
  ${ROOT_DIR}/src/Utility/
  ${ROOT_DIR}/thirdparty/glm/
  ${GLEW_INCLUDE_DIRS}
  ${NLOHMANN_JSON_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIR}
  ${STB_INCLUDE_DIRS})

list(APPEND MIKAN_TEST_REQ_LIBS
  ${SDL2_LIBRARY}
  ${MIKAN_EXTRA_LIBS}
  ${GLEW_LIBRARIES}
  ${FFMPEG_LIBRARIES}
  Mikan_CAPI)

add_executable(Mikan_Test ${MIKAN_TEST_SRC})
target_include_directories(Mikan_Test PUBLIC ${MIKAN_TEST_INCL_DIRS})
target_link_libraries(Mikan_Test ${MIKAN_TEST_REQ_LIBS})
SET_TARGET_PROPERTIES(Mikan_Test PROPERTIES FOLDER Test)

# Runtime dependencies
set(SDL2_DLL ${ROOT_DIR}/deps/SDL2-2.0.10/lib/x64/SDL2.dll)

# Post build - copy runtime dependencies to binary build folder (for debugging)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  set_property(TARGET Mikan_Test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Mikan_Test>")

  add_custom_command(
    TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${SPOUT2_SHARED_LIBRARIES}" $<TARGET_FILE_DIR:Mikan_Test>)
  add_custom_command(
    TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${SDL2_DLL} $<TARGET_FILE_DIR:Mikan_Test>)
  add_custom_command(
    TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${GLEW_SHARED_LIBRARIES} $<TARGET_FILE_DIR:Mikan_Test>)
  add_custom_command(TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${ROOT_DIR}/resources/textures $<TARGET_FILE_DIR:Mikan_Test>/resources/textures)   
  add_custom_command(
    TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:Mikan_CAPI>/Mikan_CAPI.dll
          $<TARGET_FILE_DIR:Mikan_Test>)
ELSE() #Linux/Darwin
ENDIF()

# Install
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS Mikan_Test
      CONFIGURATIONS Debug
      RUNTIME DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}/lib)
  install(TARGETS Mikan_Test
      CONFIGURATIONS Release
      RUNTIME DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}/lib)
ELSE() #Linux/Darwin
ENDIF()