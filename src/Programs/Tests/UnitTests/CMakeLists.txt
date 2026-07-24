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
  ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/ARKitDepthReceiver.h
  ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/ARKitDepthReceiver.cpp
  ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/ARKitPoseReceiver.h
  ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/ARKitPoseReceiver.cpp
  ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/ARKitFrameCorrelator.h
  ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/ARKitFrameCorrelator.cpp
  ${MIKAN_EDITOR_DIR}/ECS/VideoSource/ARKitVideoSourceSystem.h
  ${MIKAN_EDITOR_DIR}/ECS/VideoSource/ARKitVideoSourceSystem.cpp
)

# NOTE: ARKitRTPHeaderExtension.h/.cpp (ticket C3) is deliberately NOT pulled in
# here like the ARKit sources above. It registers named GObject/GstMeta types
# (G_DEFINE_TYPE, gst_meta_api_type_register) - GLib's type registry is process-
# global and shared across DLL boundaries, so compiling this file into both this
# executable AND MikanARKitVideo.dll (which also globs it - see that plugin's
# CMakeLists.txt) causes a duplicate-name type registration failure as soon as
# both copies' registration code runs in the same process (confirmed empirically:
# "cannot register existing type 'ARKitRTPHeaderExtension'" once the DLL is
# loaded by arkit_video_source_system's tests). It's only exercised indirectly,
# through the already-loaded plugin DLL, same as MikanARKitVideoDevice.cpp itself.

# JBUKernel.h/.cpp (ticket D1) and CudaGLInterop.h/.cpp (ticket D4) are plain CUDA
# Driver API C++ - no GObject/GstMeta registration, so (unlike
# ARKitRTPHeaderExtension.cpp above) it's safe to dual-compile into both this
# executable and MikanARKitVideo.dll, matching RVLCodec.cpp etc. Needs the CUDA
# Toolkit headers/driver lib, which - like GStreamer's own CUDA support - are only
# available when MIKAN_WITH_GSTREAMER is ON (see ThirdParty.cmake).
# CudaGLInterop.cpp additionally needs MikanRenderer (IMkTexture) and GLEW
# (GLuint/GL_TEXTURE_2D types, required before cudaGL.h) - compiling it here
# proves it builds cleanly against the real headers/libs on every run, even
# though its actual CUDA-GL registration/mapping behavior needs a live GL context
# this headless console test executable doesn't have (see
# arkit_jbu_kernel_unit_tests.cpp's own notes on what is/isn't covered here).
if(MIKAN_WITH_GSTREAMER)
  list(APPEND UNIT_TEST_SRC
    ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/Cuda/JBUKernel.h
    ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/Cuda/JBUKernel.cpp
    ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/Cuda/CudaGLInterop.h
    ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/Cuda/CudaGLInterop.cpp
  )

  list(APPEND UNIT_TEST_INCL_DIRS
    ${CUDA_TOOLKIT_INCLUDE_DIR}
    ${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/Cuda
    ${MIKAN_LIBRARIES_DIR}/MikanRenderer/Public
    ${GLEW_INCLUDE_DIRS}
  )

  list(APPEND UNIT_TEST_LIBS
    ${CUDA_DRIVER_LIBRARY}
    ${GLEW_LIBRARIES}
    MikanRenderer
  )
endif()

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
  ${MIKAN_EDITOR_DIR}/ECS/VideoSource
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
if(MIKAN_WITH_GSTREAMER)
  target_compile_definitions(unit_test_suite_cpp PRIVATE JBU_KERNEL_PTX_PATH="${MIKAN_ARKIT_JBU_KERNEL_PTX}")
endif()
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
if(MIKAN_WITH_GSTREAMER)
  add_dependencies(unit_test_suite_cpp MikanARKitVideo)
  add_dependencies(unit_test_suite_cpp MikanARKitVideoJBUKernelPTX)
endif()

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
  if(MIKAN_WITH_GSTREAMER)
    add_custom_command(
	  TARGET unit_test_suite_cpp POST_BUILD
	  COMMAND ${CMAKE_COMMAND} -E copy
	  $<TARGET_FILE_DIR:MikanARKitVideo>/MikanARKitVideo.dll
	  $<TARGET_FILE_DIR:unit_test_suite_cpp>)
    # MikanARKitVideo.dll now links MikanRenderer (ticket D4, CudaGLInterop's
    # IMkTexture usage) - needed alongside it for the DLL load to resolve, even
    # though unit_test_suite_cpp itself never links MikanRenderer directly.
    add_custom_command(
	  TARGET unit_test_suite_cpp POST_BUILD
	  COMMAND ${CMAKE_COMMAND} -E copy
	  $<TARGET_FILE_DIR:MikanRenderer>/MikanRenderer.dll
	  $<TARGET_FILE_DIR:unit_test_suite_cpp>)
  endif()
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