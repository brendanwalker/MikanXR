# Mikan Client Test App
file(GLOB UNIT_TEST_SRC
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

# MikanARKitVideo plugin sources under test don't have their own CMake target yet
# (that lands with the plugin scaffolding), so pull the ones with unit test coverage
# in directly.
list(APPEND UNIT_TEST_SRC
  ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/RVLCodec.h
  ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/RVLCodec.cpp
  ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/UdpReceiveSocket.h
  ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/UdpReceiveSocket.cpp
)

list(APPEND UNIT_TEST_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}
  ${RFK_INCLUDE_DIR}
  ${NLOHMANN_JSON_INCLUDE_DIR}
  ${RFK_GENERATED_ROOT_DIR}/UnitTests
  ${RFK_GENERATED_ROOT_DIR}/MikanSerialization
  ${RFK_GENERATED_ROOT_DIR}/MikanClientCore
  ${RFK_GENERATED_ROOT_DIR}/MikanClientAPI
  ${MIKAN_LIBRARIES_DIR}/MikanClientCore/Public
  ${MIKAN_LIBRARIES_DIR}/MikanClientAPI/Public
  ${MIKAN_LIBRARIES_DIR}/MikanCoreApp/Public
  ${MIKAN_LIBRARIES_DIR}/MikanMath/Public
  ${MIKAN_LIBRARIES_DIR}/MikanSerialization/Public
  ${MIKAN_LIBRARIES_DIR}/MikanUtility/Public
  ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private
  ${ROOT_DIR}/thirdparty/glm/
)

list(APPEND UNIT_TEST_LIBS 
  ${RFK_LIBRARIES}
  MikanClientCore
  MikanClientAPI
  MikanCoreApp
  MikanMath
  MikanSerialization
  MikanUtility
)

add_executable(unit_test_suite_cpp ${UNIT_TEST_SRC})
target_include_directories(unit_test_suite_cpp PUBLIC ${UNIT_TEST_INCL_DIRS})
target_link_libraries(unit_test_suite_cpp ${UNIT_TEST_LIBS})
target_compile_definitions(unit_test_suite_cpp PRIVATE ENABLE_SERIALIZATION_REFLECTION)
target_compile_definitions(unit_test_suite_cpp PRIVATE JSON_DISABLE_ENUM_SERIALIZATION=1)
target_compile_definitions(unit_test_suite_cpp PRIVATE ENABLE_MIKANCORE_REFLECTION)
target_compile_definitions(unit_test_suite_cpp PRIVATE ENABLE_MIKANAPI_REFLECTION)
SET_TARGET_PROPERTIES(unit_test_suite_cpp PROPERTIES FOLDER Test)

# Create the command to run RefurekuGenerator
add_custom_target(unit_test_suite_reflection
					WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
					COMMAND "${RFK_GENERATOR_EXE}" "${CMAKE_CURRENT_LIST_DIR}/RefurekuSettings.toml") 
set_target_properties(unit_test_suite_reflection PROPERTIES FOLDER Test)

# Run the RefurekuGenerator BEFORE building the project to refresh generated files
add_dependencies(unit_test_suite_cpp unit_test_suite_reflection)
add_dependencies(unit_test_suite_cpp MikanClientCore)
add_dependencies(unit_test_suite_cpp MikanClientAPI)
add_dependencies(unit_test_suite_cpp MikanCoreApp)
add_dependencies(unit_test_suite_cpp MikanMath)
add_dependencies(unit_test_suite_cpp MikanSerialization)
add_dependencies(unit_test_suite_cpp MikanSharedTexture)
add_dependencies(unit_test_suite_cpp MikanUtility)

# Post build - copy runtime dependencies to binary build folder (for debugging)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  add_custom_command(
    TARGET unit_test_suite_cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
	$<TARGET_FILE_DIR:MikanClientAPI>/MikanClientAPI.dll 
	$<TARGET_FILE_DIR:unit_test_suite_cpp>)
  add_custom_command(
    TARGET unit_test_suite_cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
	$<TARGET_FILE_DIR:MikanClientCore>/MikanClientCore.dll 
	$<TARGET_FILE_DIR:unit_test_suite_cpp>)
  add_custom_command(
	TARGET unit_test_suite_cpp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	$<TARGET_FILE_DIR:MikanCoreApp>/MikanCoreApp.dll
	$<TARGET_FILE_DIR:unit_test_suite_cpp>)
  add_custom_command(
	TARGET unit_test_suite_cpp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	$<TARGET_FILE_DIR:MikanSharedTexture>/MikanSharedTexture.dll
	$<TARGET_FILE_DIR:unit_test_suite_cpp>)
  add_custom_command(
    TARGET unit_test_suite_cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${GLEW_SHARED_LIBRARIES} $<TARGET_FILE_DIR:unit_test_suite_cpp>)
  add_custom_command(
	TARGET unit_test_suite_cpp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	$<TARGET_FILE_DIR:MikanMath>/MikanMath.dll
	$<TARGET_FILE_DIR:unit_test_suite_cpp>)
  add_custom_command(
	TARGET unit_test_suite_cpp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	$<TARGET_FILE_DIR:MikanSerialization>/MikanSerialization.dll
	$<TARGET_FILE_DIR:unit_test_suite_cpp>)
  add_custom_command(
	TARGET unit_test_suite_cpp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	$<TARGET_FILE_DIR:MikanUtility>/MikanUtility.dll
	$<TARGET_FILE_DIR:unit_test_suite_cpp>)
  add_custom_command(
    TARGET unit_test_suite_cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${SPOUT2_SHARED_LIBRARIES}" 
    $<TARGET_FILE_DIR:unit_test_suite_cpp>)
  add_custom_command(
	TARGET unit_test_suite_cpp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${RFK_SHARED_LIBRARIES}"
	$<TARGET_FILE_DIR:unit_test_suite_cpp>)
ELSE() #Linux/Darwin
ENDIF()

# Install
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  install(TARGETS unit_test_suite_cpp
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib)
ELSE() #Linux/Darwin
ENDIF()