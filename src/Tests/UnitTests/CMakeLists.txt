# Mikan Client Test App
file(GLOB UNIT_TEST_SRC
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
    "${ROOT_DIR}/src/AppCore/Logger.h"
    "${ROOT_DIR}/src/AppCore/Logger.cpp"
    "${ROOT_DIR}/src/Math/MathGLM.h"
    "${ROOT_DIR}/src/Math/MathUtility.h"
    "${ROOT_DIR}/src/Math/MathGLM.cpp"    
    "${ROOT_DIR}/src/Math/MathUtility.cpp"
    "${ROOT_DIR}/src/Math/Transform.h"
    "${ROOT_DIR}/src/Math/Transform.cpp"
)

list(APPEND UNIT_TEST_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}
  ${RFK_INCLUDE_DIR}
  ${NLOHMANN_JSON_INCLUDE_DIR}
  ${RFK_GENERATED_ROOT_DIR}/UnitTests
  ${RFK_GENERATED_ROOT_DIR}/Serialization
  ${ROOT_DIR}/src/AppCore/
  ${ROOT_DIR}/src/Client/
  ${ROOT_DIR}/src/Math/
  ${ROOT_DIR}/src/Serialization/Public/
  ${ROOT_DIR}/src/Utility/
  ${ROOT_DIR}/thirdparty/glm/
)

list(APPEND UNIT_TEST_LIBS 
  ${RFK_LIBRARIES}
  MikanSerialization
)

add_executable(unit_test_suite ${UNIT_TEST_SRC})
target_include_directories(unit_test_suite PUBLIC ${UNIT_TEST_INCL_DIRS})
target_link_libraries(unit_test_suite ${UNIT_TEST_LIBS})
target_compile_definitions(unit_test_suite PRIVATE ENABLE_SERIALIZATION_REFLECTION)
target_compile_definitions(unit_test_suite PRIVATE JSON_DISABLE_ENUM_SERIALIZATION=1)
SET_TARGET_PROPERTIES(unit_test_suite PROPERTIES FOLDER Test)

# Create the command to run RefurekuGenerator
add_custom_target(unit_test_suite_reflection
					WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
					COMMAND "${RFK_GENERATOR_EXE}" "${CMAKE_CURRENT_LIST_DIR}/RefurekuSettings.toml") 
set_target_properties(unit_test_suite_reflection PROPERTIES FOLDER Test)

# Run the RefurekuGenerator BEFORE building the project to refresh generated files
add_dependencies(unit_test_suite unit_test_suite_reflection)

# Post build - copy runtime dependencies to binary build folder (for debugging)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  add_custom_command(
	TARGET unit_test_suite POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	$<TARGET_FILE_DIR:MikanSerialization>/MikanSerialization.dll
	$<TARGET_FILE_DIR:unit_test_suite>)
  add_custom_command(
	TARGET unit_test_suite POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${RFK_SHARED_LIBRARIES}"
	$<TARGET_FILE_DIR:unit_test_suite>)
ELSE() #Linux/Darwin
ENDIF()

# Install
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")   
  install(TARGETS unit_test_suite
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib)
ELSE() #Linux/Darwin
ENDIF()