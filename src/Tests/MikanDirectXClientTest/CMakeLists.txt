cmake_minimum_required(VERSION 3.0)

# Mikan Client Test App
file(GLOB MIKAN_DX_TEST_SRC
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
    "${ROOT_DIR}/src/AppCore/Logger.h"
    "${ROOT_DIR}/src/AppCore/Logger.cpp"
)

list(APPEND MIKAN_DX_TEST_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}
  ${ROOT_DIR}/src/AppCore/
  ${ROOT_DIR}/src/Client/
  ${STB_INCLUDE_DIRS})

list(APPEND MIKAN_DX_TEST_REQ_LIBS
  d3d11 
  d3dcompiler 
  dxguid 
  comctl32
  ${MIKAN_EXTRA_LIBS}
  Mikan_CAPI)

add_executable(MikanDX_Test WIN32 ${MIKAN_DX_TEST_SRC})
target_include_directories(MikanDX_Test PUBLIC ${MIKAN_DX_TEST_INCL_DIRS})
target_link_libraries(MikanDX_Test ${MIKAN_DX_TEST_REQ_LIBS})

# Post build - copy runtime dependencies to binary build folder (for debugging)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  set_property(TARGET MikanDX_Test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:MikanDX_Test>")

  add_custom_command(
    TARGET MikanDX_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${SPOUT2_SHARED_LIBRARIES}" $<TARGET_FILE_DIR:MikanDX_Test>)
  add_custom_command(TARGET MikanDX_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_LIST_DIR}/shader.fxh 
      $<TARGET_FILE_DIR:MikanDX_Test>)
  add_custom_command(
    TARGET MikanDX_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:Mikan_CAPI>/Mikan_CAPI.dll
          $<TARGET_FILE_DIR:MikanDX_Test>)
ELSE() #Linux/Darwin
ENDIF()

# Install
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS MikanDX_Test
      CONFIGURATIONS Debug
      RUNTIME DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}/lib)
  install(TARGETS MikanDX_Test
      CONFIGURATIONS Release
      RUNTIME DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}/lib)
      
  install(FILES 
      "${CMAKE_CURRENT_LIST_DIR}/shader.fxh"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES 
      "${CMAKE_CURRENT_LIST_DIR}/shader.fxh"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})
ELSE() #Linux/Darwin
ENDIF()