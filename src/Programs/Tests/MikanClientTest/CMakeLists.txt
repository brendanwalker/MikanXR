# Mikan Client Test App
file(GLOB MIKAN_TEST_SRC
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

list(APPEND MIKAN_TEST_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}
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

list(APPEND MIKAN_TEST_REQ_LIBS
  ${SDL2_LIBRARY}
  ${MIKAN_EXTRA_LIBS}
  ${GLEW_LIBRARIES}
  ${FFMPEG_LIBRARIES}
  MikanClientCore
  MikanClientAPI
  MikanCoreApp
  MikanRenderer
  MikanSerialization
  MikanUtility)

add_executable(Mikan_Test ${MIKAN_TEST_SRC})
target_include_directories(Mikan_Test PUBLIC ${MIKAN_TEST_INCL_DIRS})
target_link_libraries(Mikan_Test ${MIKAN_TEST_REQ_LIBS})
SET_TARGET_PROPERTIES(Mikan_Test PROPERTIES FOLDER Test)

# Runtime dependencies
set(SDL2_DLL ${ROOT_DIR}/deps/SDL2-2.0.10/lib/x64/SDL2.dll)
set(EASY_PROFILER_FOLDER ${ROOT_DIR}/deps/easy_profiler/bin)

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
          $<TARGET_FILE_DIR:MikanClientCore>/MikanClientCore.dll
          $<TARGET_FILE_DIR:Mikan_Test>)
  add_custom_command(
    TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanClientAPI>/MikanClientAPI.dll
          $<TARGET_FILE_DIR:Mikan_Test>)
  add_custom_command(
    TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanCoreApp>/MikanCoreApp.dll
          $<TARGET_FILE_DIR:Mikan_Test>)
  add_custom_command(
    TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanRenderer>/MikanRenderer.dll
          $<TARGET_FILE_DIR:Mikan_Test>)
  add_custom_command(
    TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanSerialization>/MikanSerialization.dll
          $<TARGET_FILE_DIR:Mikan_Test>)
  add_custom_command(
    TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanSharedTexture>/MikanSharedTexture.dll
          $<TARGET_FILE_DIR:Mikan_Test>)
  add_custom_command(
    TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanUtility>/MikanUtility.dll
          $<TARGET_FILE_DIR:Mikan_Test>)
  add_custom_command(
    TARGET Mikan_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${EASY_PROFILER_FOLDER}/easy_profiler.dll"
    $<TARGET_FILE_DIR:Mikan_Test>)
  add_custom_command(
	TARGET Mikan_Test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${RFK_SHARED_LIBRARIES}"
	$<TARGET_FILE_DIR:Mikan_Test>)
ELSE() #Linux/Darwin
ENDIF()

# Install
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS Mikan_Test
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib)
ELSE() #Linux/Darwin
ENDIF()