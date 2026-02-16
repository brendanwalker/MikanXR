# Mikan Client Test App
file(GLOB MIKAN_CLIENT_TEST_APP_SRC
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

# Shared Source
file(GLOB MIKAN_CLIENT_TEST_SHARED_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Shared/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Shared/*.cpp"
)
source_group("Shared" FILES ${MIKAN_CLIENT_TEST_SHARED_SRC})

# DirectX Source
file(GLOB MIKAN_CLIENT_TEST_DX_SRC
    "${CMAKE_CURRENT_LIST_DIR}/DirectX/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/DirectX/*.cpp"
)
source_group("DirectX" FILES ${MIKAN_CLIENT_TEST_DX_SRC})

# OpenGL Source
file(GLOB MIKAN_CLIENT_TEST_GL_SRC
    "${CMAKE_CURRENT_LIST_DIR}/OpenGL/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/OpenGL/*.cpp"
)
source_group("OpenGL" FILES ${MIKAN_CLIENT_TEST_GL_SRC})


set(MIKAN_CLIENT_TEST_SRC
    ${MIKAN_CLIENT_TEST_APP_SRC}
    ${MIKAN_CLIENT_TEST_SHARED_SRC}
    ${MIKAN_CLIENT_TEST_DX_SRC}
    ${MIKAN_CLIENT_TEST_GL_SRC}
)

list(APPEND MIKAN_CLIENT_TEST_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/DirectX
  ${CMAKE_CURRENT_LIST_DIR}/OpenGL
  ${CMAKE_CURRENT_LIST_DIR}/Shared
  ${MIKAN_LIBRARIES_DIR}/MikanClientAPI/Public
  ${MIKAN_LIBRARIES_DIR}/MikanClientCore/Public
  ${MIKAN_LIBRARIES_DIR}/MikanCoreApp/Public
  ${MIKAN_LIBRARIES_DIR}/MikanRenderer/Public
  ${MIKAN_LIBRARIES_DIR}/MikanSerialization/Public
  ${MIKAN_LIBRARIES_DIR}/MikanUtility/Public
  ${ROOT_DIR}/thirdparty/glm/
  ${GLEW_INCLUDE_DIRS}
  ${NLOHMANN_JSON_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIR}
  ${STB_INCLUDE_DIRS}
  ${RFK_INCLUDE_DIR})

# SDL2main is needed for Windows to provide the main() entry point
set(SDL2MAIN_LIBRARY ${ROOT_DIR}/deps/SDL2-2.0.10/lib/x64/SDL2main.lib)

list(APPEND MIKAN_CLIENT_TEST_REQ_LIBS
  d3d11
  d3dcompiler
  dxguid
  comctl32
  ${SDL2MAIN_LIBRARY}
  ${SDL2_LIBRARY}
  ${MIKAN_EXTRA_LIBS}
  ${GLEW_LIBRARIES}
  MikanClientCore
  MikanClientAPI
  MikanCoreApp
  MikanRenderer
  MikanSerialization
  MikanUtility)

add_executable(MikanClientTestCPP ${MIKAN_CLIENT_TEST_SRC})
target_include_directories(MikanClientTestCPP PUBLIC ${MIKAN_CLIENT_TEST_INCL_DIRS})
target_link_libraries(MikanClientTestCPP ${MIKAN_CLIENT_TEST_REQ_LIBS})
SET_TARGET_PROPERTIES(MikanClientTestCPP PROPERTIES FOLDER Test WIN32_EXECUTABLE FALSE)

# Runtime dependencies
set(SDL2_DLL ${ROOT_DIR}/deps/SDL2-2.0.10/lib/x64/SDL2.dll)
set(EASY_PROFILER_FOLDER ${ROOT_DIR}/deps/easy_profiler/bin)

# Post build - copy runtime dependencies to binary build folder (for debugging)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  set_property(TARGET MikanClientTestCPP PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:MikanClientTestCPP>")

  add_custom_command(
    TARGET MikanClientTestCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${SPOUT2_SHARED_LIBRARIES}" $<TARGET_FILE_DIR:MikanClientTestCPP>)
  add_custom_command(
    TARGET MikanClientTestCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${SDL2_DLL} $<TARGET_FILE_DIR:MikanClientTestCPP>)
  add_custom_command(
    TARGET MikanClientTestCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${GLEW_SHARED_LIBRARIES} $<TARGET_FILE_DIR:MikanClientTestCPP>)
  add_custom_command(
    TARGET MikanClientTestCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanClientCore>/MikanClientCore.dll
          $<TARGET_FILE_DIR:MikanClientTestCPP>)
  add_custom_command(
    TARGET MikanClientTestCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanClientAPI>/MikanClientAPI.dll
          $<TARGET_FILE_DIR:MikanClientTestCPP>)
  add_custom_command(
    TARGET MikanClientTestCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanCoreApp>/MikanCoreApp.dll
          $<TARGET_FILE_DIR:MikanClientTestCPP>)
  add_custom_command(
    TARGET MikanClientTestCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanRenderer>/MikanRenderer.dll
          $<TARGET_FILE_DIR:MikanClientTestCPP>)
  add_custom_command(
    TARGET MikanClientTestCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanSerialization>/MikanSerialization.dll
          $<TARGET_FILE_DIR:MikanClientTestCPP>)
  add_custom_command(
    TARGET MikanClientTestCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanSharedTexture>/MikanSharedTexture.dll
          $<TARGET_FILE_DIR:MikanClientTestCPP>)
  add_custom_command(
    TARGET MikanClientTestCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanUtility>/MikanUtility.dll
          $<TARGET_FILE_DIR:MikanClientTestCPP>)
  add_custom_command(
    TARGET MikanClientTestCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${EASY_PROFILER_FOLDER}/easy_profiler.dll"
    $<TARGET_FILE_DIR:MikanClientTestCPP>)
  add_custom_command(
	TARGET MikanClientTestCPP POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${RFK_SHARED_LIBRARIES}"
	$<TARGET_FILE_DIR:MikanClientTestCPP>)
ELSE() #Linux/Darwin
ENDIF()

# Install
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS MikanClientTestCPP
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib)
ELSE() #Linux/Darwin
ENDIF()