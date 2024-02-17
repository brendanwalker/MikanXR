# Mikan Client Test App
file(GLOB UNIT_TEST_SRC
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
    "${ROOT_DIR}/src/Math/MathGLM.h"
    "${ROOT_DIR}/src/Math/MathUtility.h"
    "${ROOT_DIR}/src/Math/MathGLM.cpp"    
    "${ROOT_DIR}/src/Math/MathUtility.cpp"
    "${ROOT_DIR}/src/Math/Transform.h"
    "${ROOT_DIR}/src/Math/Transform.cpp"
)

list(APPEND UNIT_TEST_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}
  ${ROOT_DIR}/src/AppCore/
  ${ROOT_DIR}/src/Client/
  ${ROOT_DIR}/src/Math/
  ${ROOT_DIR}/thirdparty/glm/)

add_executable(unit_test_suite ${UNIT_TEST_SRC})
target_include_directories(unit_test_suite PUBLIC ${UNIT_TEST_INCL_DIRS})
SET_TARGET_PROPERTIES(unit_test_suite PROPERTIES FOLDER Test)

# Install
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS unit_test_suite
      CONFIGURATIONS Debug
      RUNTIME DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}/lib)
  install(TARGETS unit_test_suite
      CONFIGURATIONS Release
      RUNTIME DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}/lib)
ELSE() #Linux/Darwin
ENDIF()