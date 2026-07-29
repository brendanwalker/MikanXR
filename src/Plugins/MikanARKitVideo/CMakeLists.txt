## Mikan ARKit Video Plugin DLL
## ============================

MESSAGE(STATUS "Generating Mikan ARKit Video DLL Project")

file(GLOB MIKAN_ARKIT_VIDEO_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_ARKIT_VIDEO_PRIVATE_SRC
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

# Non-recursive GLOB above doesn't reach Private/Cuda/ - globbed separately. Note
# NV12ConversionKernel.cu itself is NOT in this list / not a source of this target -
# it's compiled to PTX by a custom command below and loaded at runtime via
# cuModuleLoadData, never linked into any host translation unit.
file(GLOB MIKAN_ARKIT_VIDEO_CUDA_SRC
	"${CMAKE_CURRENT_LIST_DIR}/Private/Cuda/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/Private/Cuda/*.cpp"
)

file(GLOB MIKAN_ARKIT_VIDEO_SRC
	${MIKAN_ARKIT_VIDEO_HEADER}
	${MIKAN_ARKIT_VIDEO_PRIVATE_SRC}
	${MIKAN_ARKIT_VIDEO_CUDA_SRC}
)

source_group("Public" FILES ${MIKAN_ARKIT_VIDEO_HEADER})
source_group("Private" FILES ${MIKAN_ARKIT_VIDEO_PRIVATE_SRC})
source_group("Private\\Cuda" FILES ${MIKAN_ARKIT_VIDEO_CUDA_SRC})

# NV12ConversionKernel.cu -> NV12ConversionKernel.ptx (ticket E3) - compiled once
# here and reused by unit_test_suite_cpp too (see UnitTests/CMakeLists.txt) via the
# same output path; loaded at runtime via cuModuleLoadData, never linked into any
# host translation unit.
set(MIKAN_ARKIT_NV12_KERNEL_CU "${CMAKE_CURRENT_LIST_DIR}/Private/Cuda/NV12ConversionKernel.cu")
set(MIKAN_ARKIT_NV12_KERNEL_PTX "${CMAKE_CURRENT_BINARY_DIR}/NV12ConversionKernel.ptx" CACHE INTERNAL "")
add_custom_command(
	OUTPUT "${MIKAN_ARKIT_NV12_KERNEL_PTX}"
	COMMAND "${CUDA_NVCC_EXECUTABLE}" --ptx "${MIKAN_ARKIT_NV12_KERNEL_CU}" -o "${MIKAN_ARKIT_NV12_KERNEL_PTX}"
	DEPENDS "${MIKAN_ARKIT_NV12_KERNEL_CU}"
	COMMENT "Compiling NV12ConversionKernel.cu to PTX"
	VERBATIM)
add_custom_target(MikanARKitVideoNV12KernelPTX DEPENDS "${MIKAN_ARKIT_NV12_KERNEL_PTX}")
set_target_properties(MikanARKitVideoNV12KernelPTX PROPERTIES FOLDER Plugins)

list(APPEND MIKAN_ARKIT_VIDEO_INCL_DIRS
	${CMAKE_CURRENT_LIST_DIR}/Public
	${CMAKE_CURRENT_LIST_DIR}/Private
	${MIKAN_LIBRARIES_DIR}/MikanCoreApp/Public
	${MIKAN_LIBRARIES_DIR}/MikanRenderer/Public
	${MIKAN_LIBRARIES_DIR}/MikanUtility/Public
	${GSTREAMER_INCLUDE_DIR}
	${GSTREAMER_RTP_INCLUDE_DIR}
	${GSTREAMER_CUDA_INCLUDE_DIR}
	${CUDA_TOOLKIT_INCLUDE_DIR}
	${GLIB2_INCLUDE_DIR}
	${GLEW_INCLUDE_DIRS}
)

list(APPEND MIKAN_ARKIT_VIDEO_REQ_LIBS
	${GLIB2_LIBRARIES}
	${GOBJECT_LIBRARIES}
	${GSTREAMER_LIBRARY}
	${GSTREAMER_APP_LIBRARY}
	${GSTREAMER_BASE_LIBRARY}
	${GSTREAMER_VIDEO_LIBRARY}
	${GSTREAMER_RTP_LIBRARY}
	${GSTREAMER_CUDA_LIBRARY}
	${CUDA_DRIVER_LIBRARY}
	${GLEW_LIBRARIES}
	MikanCoreApp
	MikanRenderer
	MikanUtility)

add_library(MikanARKitVideo SHARED ${MIKAN_ARKIT_VIDEO_SRC})
target_include_directories(MikanARKitVideo PRIVATE ${MIKAN_ARKIT_VIDEO_INCL_DIRS})
target_link_libraries(MikanARKitVideo PRIVATE ${MIKAN_ARKIT_VIDEO_REQ_LIBS})
add_dependencies(MikanARKitVideo MikanARKitVideoNV12KernelPTX)
target_compile_definitions(MikanARKitVideo PRIVATE NV12_KERNEL_PTX_PATH="${MIKAN_ARKIT_NV12_KERNEL_PTX}")
set_target_properties(MikanARKitVideo PROPERTIES FOLDER Plugins)
set_target_properties(MikanARKitVideo PROPERTIES PUBLIC_HEADER "${MIKAN_ARKIT_VIDEO_HEADER}")
set_target_properties(MikanARKitVideo PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(MikanARKitVideo PRIVATE MIKAN_ARKIT_VIDEO_EXPORTS)

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	install(TARGETS MikanARKitVideo
		RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
		LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
		ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
		PUBLIC_HEADER DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/include)
ELSE() #Linux/Darwin
ENDIF()
