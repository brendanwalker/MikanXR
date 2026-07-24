## JBU Tuning Tool
## ===============
## Standalone CLI dev tool (ticket D5) for visualizing MikanARKitVideo's Joint
## Bilateral Upsampling output at different radius/sigma parameters against real
## depth+RGB test data - not part of the automated unit_test_suite_cpp pass/fail
## suite (no assertions, produces image files for visual inspection), and not
## built without a working CUDA Toolkit since it links JBUKernel.cu/.cpp directly.
if(NOT MIKAN_WITH_GSTREAMER)
	return()
endif()

MESSAGE(STATUS "Generating JBU Tuning Tool Project")

file(GLOB JBU_TUNING_TOOL_SRC
	"${CMAKE_CURRENT_LIST_DIR}/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

# JBUKernel.h/.cpp don't have their own CMake library target (see
# MikanARKitVideo/CMakeLists.txt and UnitTests/CMakeLists.txt for the two other
# places that pull these same sources in directly) - JBUKernel.cu is NOT a source
# of this target either; it's compiled to PTX by MikanARKitVideo's own custom
# command (MIKAN_ARKIT_JBU_KERNEL_PTX, a CACHE INTERNAL path shared across all
# three consumers) and loaded at runtime via cuModuleLoadData.
list(APPEND JBU_TUNING_TOOL_SRC
	${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/Cuda/JBUKernel.h
	${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/Cuda/JBUKernel.cpp
)

list(APPEND JBU_TUNING_TOOL_INCL_DIRS
	${MIKAN_PLUGINS_DIR}/MikanARKitVideo/Private/Cuda
	${MIKAN_LIBRARIES_DIR}/MikanCoreApp/Public
	${MIKAN_LIBRARIES_DIR}/MikanUtility/Public
	${CUDA_TOOLKIT_INCLUDE_DIR}
	${STB_INCLUDE_DIRS}
)

list(APPEND JBU_TUNING_TOOL_REQ_LIBS
	${CUDA_DRIVER_LIBRARY}
	MikanCoreApp
	MikanUtility)

add_executable(JBUTuningTool ${JBU_TUNING_TOOL_SRC})
target_include_directories(JBUTuningTool PRIVATE ${JBU_TUNING_TOOL_INCL_DIRS})
target_link_libraries(JBUTuningTool PRIVATE ${JBU_TUNING_TOOL_REQ_LIBS})
add_dependencies(JBUTuningTool MikanARKitVideoJBUKernelPTX)
target_compile_definitions(JBUTuningTool PRIVATE JBU_KERNEL_PTX_PATH="${MIKAN_ARKIT_JBU_KERNEL_PTX}")
set_target_properties(JBUTuningTool PROPERTIES FOLDER Test WIN32_EXECUTABLE FALSE)

# Post build - copy runtime dependencies to binary build folder
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_custom_command(
	TARGET JBUTuningTool POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	$<TARGET_FILE_DIR:MikanCoreApp>/MikanCoreApp.dll
	$<TARGET_FILE_DIR:JBUTuningTool>)
  add_custom_command(
	TARGET JBUTuningTool POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	$<TARGET_FILE_DIR:MikanUtility>/MikanUtility.dll
	$<TARGET_FILE_DIR:JBUTuningTool>)
ELSE() #Linux/Darwin
ENDIF()
