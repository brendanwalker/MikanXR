## Mikan Libraries
## =======================

# Reflection based serialization library
add_subdirectory(Serialization)

# Mikan Client Core C API
add_subdirectory(ClientCore)

# Mikan Client C++ API
add_subdirectory(ClientAPI)

# Mikan Client Code Gen
add_subdirectory(ClientCodeGen)

## Mikan App
## =======================
MESSAGE(STATUS "Generating Mikan App Project")

# Dependencies
set(MIKAN_INCL_DIRS)
set(MIKAN_REQ_LIBS)

list(APPEND MIKAN_REQ_LIBS ${PLATFORM_LIBS})

file(GLOB MIKAN_APP_CORE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppCore/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppCore/*.h"
)
source_group("AppCore" FILES ${MIKAN_APP_CORE_SRC})

# AppStages 
## Root
file(GLOB MIKAN_APP_STAGES_ROOT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/*.h"
)
source_group("AppStages" FILES ${MIKAN_APP_STAGES_ROOT_SRC})
## AlignmentCalibrationSettings
file(GLOB MIKAN_APP_STAGES_ALIGNMENT_CALIBRATION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/AlignmentCalibration/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/AlignmentCalibration/*.h"
)
source_group("AppStages\\AlignmentCalibration" FILES ${MIKAN_APP_STAGES_ALIGNMENT_CALIBRATION_SRC})
## AnchorTriangulation
file(GLOB MIKAN_APP_STAGES_ANCHOR_CALIBRATION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/AnchorTriangulation/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/AnchorTriangulation/*.h"
)
source_group("AppStages\\AnchorTriangulation" FILES ${MIKAN_APP_STAGES_ANCHOR_CALIBRATION_SRC})
## CameraSettings
file(GLOB MIKAN_APP_STAGES_CAMERA_SETTINGS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/CameraSettings/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/CameraSettings/*.h"
)
source_group("AppStages\\CameraSettings" FILES ${MIKAN_APP_STAGES_CAMERA_SETTINGS_SRC})
## Compositor
file(GLOB MIKAN_APP_STAGES_COMPOSITOR_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/Compositor/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/Compositor/*.h"
)
source_group("AppStages\\Compositor" FILES ${MIKAN_APP_STAGES_COMPOSITOR_SRC})
## CTOffsetCalibration
file(GLOB MIKAN_APP_STAGES_CTOFFSET_CALIBRATION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/CTOffsetCalibration/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/CTOffsetCalibration/*.h"
)
source_group("AppStages\\CTOffsetCalibration" FILES ${MIKAN_APP_STAGES_CTOFFSET_CALIBRATION_SRC})
## DepthMeshCapture
file(GLOB MIKAN_APP_STAGES_DEPTH_MESH_CAPTURE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/DepthMeshCapture/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/DepthMeshCapture/*.h"
)
source_group("AppStages\\DepthMeshCapture" FILES ${MIKAN_APP_STAGES_DEPTH_MESH_CAPTURE_SRC})
## FileBrowser
file(GLOB MIKAN_APP_STAGES_FILE_BROWSER_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/FileBrowser/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/FileBrowser/*.h"
)
source_group("AppStages\\FileBrowser" FILES ${MIKAN_APP_STAGES_FILE_BROWSER_SRC})
## MainMenu
file(GLOB MIKAN_APP_STAGES_MAIN_MENU_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/MainMenu/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/MainMenu/*.h"
)
source_group("AppStages\\MainMenu" FILES ${MIKAN_APP_STAGES_MAIN_MENU_SRC})
## ModalConfirm
file(GLOB MIKAN_APP_STAGES_MODAL_CONFIRM_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/ModalConfirm/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/ModalConfirm/*.h"
)
source_group("AppStages\\ModalConfirm" FILES ${MIKAN_APP_STAGES_MODAL_CONFIRM_SRC})
## MonoLensCalibration
file(GLOB MIKAN_APP_STAGES_MONO_LENS_CALIBRATION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/MonoLensCalibration/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/MonoLensCalibration/*.h"
)
source_group("AppStages\\MonoLensCalibration" FILES ${MIKAN_APP_STAGES_MONO_LENS_CALIBRATION_SRC})
## Shared
file(GLOB MIKAN_APP_STAGES_SHARED_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/Shared/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/Shared/*.h"
)
source_group("AppStages\\Shared" FILES ${MIKAN_APP_STAGES_SHARED_SRC})
## SpatialAnchors
file(GLOB MIKAN_APP_STAGES_SPATIAL_ANCHORS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/SpatialAnchors/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/SpatialAnchors/*.h"
)
source_group("AppStages\\SpatialAnchors" FILES ${MIKAN_APP_STAGES_SPATIAL_ANCHORS_SRC})
## StencilAlignment
file(GLOB MIKAN_APP_STAGES_STENCIL_ALIGNMENT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/StencilAlignment/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/StencilAlignment/*.h"
)
source_group("AppStages\\StencilAlignment" FILES ${MIKAN_APP_STAGES_STENCIL_ALIGNMENT_SRC})
## VRDeviceSettings
file(GLOB MIKAN_APP_STAGES_VR_DEVICE_SETTINGS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/VRDeviceSettings/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/VRDeviceSettings/*.h"
)
source_group("AppStages\\VRDeviceSettings" FILES ${MIKAN_APP_STAGES_VR_DEVICE_SETTINGS_SRC})
## VRTrackingRecenter
file(GLOB MIKAN_APP_STAGES_VR_TRACKING_RECENTER_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/VRTrackingRecenter/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/VRTrackingRecenter/*.h"
)
source_group("AppStages\\VRTrackingRecenter" FILES ${MIKAN_APP_STAGES_VR_TRACKING_RECENTER_SRC})

## Mikan All AppStage Source
set(MIKAN_APP_STAGES_SRC
    ${MIKAN_APP_STAGES_ROOT_SRC}
    ${MIKAN_APP_STAGES_ALIGNMENT_CALIBRATION_SRC}
    ${MIKAN_APP_STAGES_ANCHOR_CALIBRATION_SRC}
    ${MIKAN_APP_STAGES_CAMERA_SETTINGS_SRC}
    ${MIKAN_APP_STAGES_COMPOSITOR_SRC}
    ${MIKAN_APP_STAGES_CTOFFSET_CALIBRATION_SRC}
	${MIKAN_APP_STAGES_DEPTH_MESH_CAPTURE_SRC}
    ${MIKAN_APP_STAGES_FILE_BROWSER_SRC}
    ${MIKAN_APP_STAGES_MAIN_MENU_SRC}
    ${MIKAN_APP_STAGES_MODAL_CONFIRM_SRC}
    ${MIKAN_APP_STAGES_MONO_LENS_CALIBRATION_SRC}
    ${MIKAN_APP_STAGES_SHARED_SRC}
    ${MIKAN_APP_STAGES_SPATIAL_ANCHORS_SRC}
	${MIKAN_APP_STAGES_STENCIL_ALIGNMENT_SRC}
    ${MIKAN_APP_STAGES_VR_DEVICE_SETTINGS_SRC}
	${MIKAN_APP_STAGES_VR_TRACKING_RECENTER_SRC}
)

# Asset
file(GLOB MIKAN_ASSET_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Asset/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Asset/*.h"
)
source_group("Asset" FILES ${MIKAN_ASSET_SRC})
## Asset/Import
file(GLOB MIKAN_ASSET_IMPORT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Asset/Import/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Asset/Import/*.h"
)
source_group("Asset\\Import" FILES ${MIKAN_ASSET_IMPORT_SRC})
## Asset/Export
file(GLOB MIKAN_ASSET_EXPORT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Asset/Export/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Asset/Export/*.h"
)
source_group("Asset\\Export" FILES ${MIKAN_ASSET_EXPORT_SRC})

# Calibration
file(GLOB MIKAN_CALIBRATION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Calibration/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Calibration/*.h"
)
source_group("Calibration" FILES ${MIKAN_CALIBRATION_SRC})

# Components
file(GLOB MIKAN_COMPONENTS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Components/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Components/*.h"
)
source_group("Components" FILES ${MIKAN_COMPONENTS_SRC})

# Config
file(GLOB MIKAN_CONFIG_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Config/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Config/*.h"
)
source_group("Config" FILES ${MIKAN_CONFIG_SRC})

# Device
## Device Enumerator
file(GLOB MIKAN_DEVICE_ENUM_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Device/Enumerator/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Device/Enumerator/*.h"
)
source_group("Device\\Enumerator" FILES ${MIKAN_DEVICE_ENUM_SRC})

## Device Interface
file(GLOB MIKAN_DEVICE_INTERFACE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Device/Interface/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Device/Interface/*.h"
)
source_group("Device\\Interface" FILES ${MIKAN_DEVICE_INTERFACE_SRC})

## Device Manager
file(GLOB MIKAN_DEVICE_MANAGER_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Device/Manager/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Device/Manager/*.h"
)
source_group("Device\\Manager" FILES ${MIKAN_DEVICE_MANAGER_SRC})

## Device View
file(GLOB MIKAN_DEVICE_VIEW_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Device/View/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Device/View/*.h"
)
source_group("Device\\View" FILES ${MIKAN_DEVICE_VIEW_SRC})

# ImGUI's source files are included in directly, rather than linking against a lib
source_group("ImGui" FILES ${IMGUI_SOURCE})

# ImNodes's source files are included in directly, rather than linking against a lib
source_group("ImNodes" FILES ${IMNODES_SOURCE})

# Interprocess
file(GLOB MIKAN_INTERPROCESS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessMessages.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessMessages.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/WebsocketInterprocessMessageServer.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/WebsocketInterprocessMessageServer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetShared.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetShared.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetReader.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetReader.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetWriter.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetWriter.cpp"
)
source_group("Interprocess" FILES ${MIKAN_INTERPROCESS_SRC})

# Localization
file(GLOB MIKAN_LOCALIZATION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Localization/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Localization/*.h"
)
source_group("Localization" FILES ${MIKAN_LOCALIZATION_SRC})

# Math
file(GLOB MIKAN_MATH_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Math/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Math/*.h"
)
source_group("Math" FILES ${MIKAN_MATH_SRC})

# Object
file(GLOB MIKAN_OBJECT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Object/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Object/*.h"
)
source_group("Object" FILES ${MIKAN_OBJECT_SRC})

# OpenCV
file(GLOB MIKAN_OPENCV_SRC
    "${CMAKE_CURRENT_LIST_DIR}/OpenCV/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/OpenCV/*.h"
)
source_group("OpenCV" FILES ${MIKAN_OPENCV_SRC})

# Node Editor
## Root
file(GLOB MIKAN_NODE_EDITORS_ROOT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/*.h"
)
source_group("NodeEditors" FILES ${MIKAN_NODE_EDITORS_ROOT_SRC})
## DataSources
file(GLOB MIKAN_NODE_EDITORS_DATASOURCES_SRC
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/DataSources/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/DataSources/*.h"
)
source_group("NodeEditors\\DataSources" FILES ${MIKAN_NODE_EDITORS_DATASOURCES_SRC})
## Graphs
file(GLOB MIKAN_NODE_EDITORS_GRAPHS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/Graphs/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/Graphs/*.h"
)
source_group("NodeEditors\\Graphs" FILES ${MIKAN_NODE_EDITORS_GRAPHS_SRC})
## Nodes
file(GLOB MIKAN_NODE_EDITORS_NODES_SRC
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/Nodes/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/Nodes/*.h"
)
source_group("NodeEditors\\Nodes" FILES ${MIKAN_NODE_EDITORS_NODES_SRC})
## Pins
file(GLOB MIKAN_NODE_EDITORS_PINS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/Pins/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/Pins/*.h"
)
source_group("NodeEditors\\Pins" FILES ${MIKAN_NODE_EDITORS_PINS_SRC})
## Properties
file(GLOB MIKAN_NODE_EDITORS_PROPERTIES_SRC
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/Properties/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/Properties/*.h"
)
source_group("NodeEditors\\Properties" FILES ${MIKAN_NODE_EDITORS_PROPERTIES_SRC})

## Windows
file(GLOB MIKAN_NODE_EDITORS_WINDOWS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/Windows/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/NodeEditors/Windows/*.h"
)
source_group("NodeEditors\\Windows" FILES ${MIKAN_NODE_EDITORS_WINDOWS_SRC})
## Node Editor Full Source
set(MIKAN_NODE_EDITORS_SRC
    ${MIKAN_NODE_EDITORS_ROOT_SRC}
	${MIKAN_NODE_EDITORS_DATASOURCES_SRC}
    ${MIKAN_NODE_EDITORS_GRAPHS_SRC}
    ${MIKAN_NODE_EDITORS_NODES_SRC}
    ${MIKAN_NODE_EDITORS_PINS_SRC}
    ${MIKAN_NODE_EDITORS_PROPERTIES_SRC}
    ${MIKAN_NODE_EDITORS_WINDOWS_SRC}
)

# Renderer
file(GLOB MIKAN_RENDERER_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Renderer/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Renderer/*.h"
)
source_group("Renderer" FILES ${MIKAN_RENDERER_SRC})

# RmlUI
file(GLOB MIKAN_RMLUI_SRC
    "${CMAKE_CURRENT_LIST_DIR}/RmlUi/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/RmlUi/*.h"
)
source_group("RmlUI" FILES ${MIKAN_RMLUI_SRC})

# Scene
file(GLOB MIKAN_SCENE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Scene/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Scene/*.h"
)
source_group("Scene" FILES ${MIKAN_SCENE_SRC})

# Scripting
file(GLOB MIKAN_SCRIPTING_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Scripting/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Scripting/*.h"
)
source_group("Scripting" FILES ${MIKAN_SCRIPTING_SRC})

# Scripting
file(GLOB MIKAN_SDL_SRC
    "${CMAKE_CURRENT_LIST_DIR}/SDL/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SDL/*.h"
)
source_group("SDL" FILES ${MIKAN_SDL_SRC})

# Scripting
file(GLOB MIKAN_SYSTEM_SRC
    "${CMAKE_CURRENT_LIST_DIR}/System/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/System/*.h"
)
source_group("System" FILES ${MIKAN_SYSTEM_SRC})

# TinyFileDialogs's source files are included in directly, rather than linking against a lib
source_group("TinyFileDialogs" FILES ${TINYFILEDIALOGS_SOURCE})

# Utilities
file(GLOB MIKAN_UTILS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Utility/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Utility/*.h"
)
source_group("Utility" FILES ${MIKAN_UTILS_SRC})

# Video
file(GLOB MIKAN_VIDEO_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Video/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Video/*.h"
)
source_group("Video" FILES ${MIKAN_VIDEO_SRC})

## Video OpenCV
file(GLOB MIKAN_VIDEO_OPENCV_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Video/OpenCV/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Video/OpenCV/*.h"
)
source_group("Video\\OpenCV" FILES ${MIKAN_VIDEO_OPENCV_SRC})

## Video WMF
if (WIN32)
file(GLOB MIKAN_VIDEO_WMF_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Video/WMF/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Video/WMF/*.h"
)
source_group("Video\\WMF" FILES ${MIKAN_VIDEO_WMF_SRC})
else()
set(MIKAN_VIDEO_WMF_SRC)
endif()

# VRTracker
file(GLOB MIKAN_VRTRACKER_SRC
    "${CMAKE_CURRENT_LIST_DIR}/VRTracker/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/VRTracker/*.h"
)
source_group("VRTracker" FILES ${MIKAN_VRTRACKER_SRC})

# VRTracker SteamVR
file(GLOB MIKAN_VRTRACKER_STEAMVR_SRC
    "${CMAKE_CURRENT_LIST_DIR}/VRTracker/SteamVR/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/VRTracker/SteamVR/*.h"
)
source_group("VRTracker\\SteamVR" FILES ${MIKAN_VRTRACKER_STEAMVR_SRC})


# Mikan Executable
set(MIKAN_SRC
    ${MIKAN_APP_CORE_SRC}
    ${MIKAN_APP_STAGES_SRC}
    ${MIKAN_ASSET_SRC}
	${MIKAN_ASSET_IMPORT_SRC}
	${MIKAN_ASSET_EXPORT_SRC}
    ${MIKAN_CALIBRATION_SRC}
    ${MIKAN_COMPONENTS_SRC}
    ${MIKAN_CONFIG_SRC}
    ${MIKAN_DEVICE_ENUM_SRC}
    ${MIKAN_DEVICE_INTERFACE_SRC}
    ${MIKAN_DEVICE_MANAGER_SRC}
    ${MIKAN_DEVICE_VIEW_SRC}
    ${MIKAN_INTERPROCESS_SRC}
    ${MIKAN_LOCALIZATION_SRC}
    ${MIKAN_MATH_SRC}
	${MIKAN_NODE_EDITORS_SRC}
    ${MIKAN_OBJECT_SRC}
	${MIKAN_OPENCV_SRC}
    ${MIKAN_RENDERER_SRC}
    ${MIKAN_RMLUI_SRC}
    ${MIKAN_SCENE_SRC}
    ${MIKAN_SCRIPTING_SRC}
    ${MIKAN_SDL_SRC}
    ${MIKAN_SYSTEM_SRC}
    ${MIKAN_UTILS_SRC}
    ${MIKAN_VIDEO_SRC}
    ${MIKAN_VIDEO_OPENCV_SRC}
    ${MIKAN_VIDEO_WMF_SRC}
    ${MIKAN_VRTRACKER_SRC}
    ${MIKAN_VRTRACKER_STEAMVR_SRC}
    ${IMGUI_SOURCE}
	${IMNODES_SOURCE}
	${TINYFILEDIALOGS_SOURCE}	
)

list(APPEND MIKAN_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/AppCore
  ${CMAKE_CURRENT_LIST_DIR}/AppStages
  ${CMAKE_CURRENT_LIST_DIR}/Asset
  ${CMAKE_CURRENT_LIST_DIR}/Asset/Import
  ${CMAKE_CURRENT_LIST_DIR}/Asset/Export
  ${CMAKE_CURRENT_LIST_DIR}/Calibration
  ${CMAKE_CURRENT_LIST_DIR}/Components
  ${CMAKE_CURRENT_LIST_DIR}/Config
  ${CMAKE_CURRENT_LIST_DIR}/ClientAPI/Public
  ${CMAKE_CURRENT_LIST_DIR}/ClientCore/Public
  ${CMAKE_CURRENT_LIST_DIR}/Device/Enumerator
  ${CMAKE_CURRENT_LIST_DIR}/Device/Interface
  ${CMAKE_CURRENT_LIST_DIR}/Device/Manager
  ${CMAKE_CURRENT_LIST_DIR}/Device/View
  ${CMAKE_CURRENT_LIST_DIR}/Generated
  ${CMAKE_CURRENT_LIST_DIR}/Interprocess
  ${CMAKE_CURRENT_LIST_DIR}/Localization
  ${CMAKE_CURRENT_LIST_DIR}/Math
  ${CMAKE_CURRENT_LIST_DIR}/NodeEditors
  ${CMAKE_CURRENT_LIST_DIR}/Object
  ${CMAKE_CURRENT_LIST_DIR}/OpenCV
  ${CMAKE_CURRENT_LIST_DIR}/Renderer
  ${CMAKE_CURRENT_LIST_DIR}/RmlUi
  ${CMAKE_CURRENT_LIST_DIR}/Scene
  ${CMAKE_CURRENT_LIST_DIR}/Scripting
  ${CMAKE_CURRENT_LIST_DIR}/Serialization/Public/
  ${CMAKE_CURRENT_LIST_DIR}/SDL
  ${CMAKE_CURRENT_LIST_DIR}/System
  ${CMAKE_CURRENT_LIST_DIR}/Utility
  ${CMAKE_CURRENT_LIST_DIR}/Video
  ${CMAKE_CURRENT_LIST_DIR}/Video/OpenCV
  ${CMAKE_CURRENT_LIST_DIR}/Video/WMF
  ${CMAKE_CURRENT_LIST_DIR}/VRTracker
  ${CMAKE_CURRENT_LIST_DIR}/VRTracker/SteamVR
  ${RFK_GENERATED_ROOT_DIR}/MikanCore
  ${RFK_GENERATED_ROOT_DIR}/MikanAPI
  ${RFK_GENERATED_ROOT_DIR}/Serialization
  ${IMGUI_DIR}
  ${IMNODES_DIR}
  ${IXWEBSOCKET_INCLUDE_DIR}
  ${LOCKFREEQUEUE_INCLUDE_DIR}
  ${RFK_INCLUDE_DIR}
  ${RMLUI_INCLUDE_DIR}
  ${ROOT_DIR}/thirdparty/glm/
  ${CONFIGURU_INCLUDE_DIR}
  ${NLOHMANN_JSON_INCLUDE_DIR}  
  ${OpenCV_INCLUDE_DIR} 
  ${OPENVR_INCLUDE_DIR} 
  ${SDL2_INCLUDE_DIR} 
  ${SDL2_IMAGE_INCLUDE_DIR}
  ${SDL2TTF_INCLUDE_DIR}
  ${STB_INCLUDE_DIRS}
  ${TINYFILEDIALOGS_DIR}
  ${GLEW_INCLUDE_DIRS}
  ${FastCSV_INCLUDE_DIRS}
  ${FFMPEG_INCLUDE_DIRS}
  ${SPOUT2_INCLUDE_DIRS}
  ${LUA_INCLUDE_DIRS}
  ${LUA_BRIDGE_INCLUDE_DIRS}
  ${FAST_OBJ_LOADER_INCLUDE_DIRS})

list(APPEND MIKAN_REQ_LIBS 
  ${OpenCV_LIBS}
  ${OPENVR_LIBRARY}
  ${SDL2_LIBRARY}
  ${SDL2_IMAGE_LIBRARY}
  ${SDL2TTF_LIBRARY}
  ${MIKAN_EXTRA_LIBS}
  ${GLEW_LIBRARIES}
  ${SPOUT2_LIBRARIES}
  ${FFMPEG_avcodec_LIBRARY}
  ${FFMPEG_avdevice_LIBRARY}
  ${FFMPEG_avfilter_LIBRARY}
  ${FFMPEG_avformat_LIBRARY}
  ${FFMPEG_avutil_LIBRARY}
  ${FFMPEG_swscale_LIBRARY}
  ${FREETYPE_LIBRARIES}
  ${LUA_LIBRARIES}
  ${RFK_LIBRARIES}
  easy_profiler
  fast_obj_lib
  ixwebsocket
  MikanCore
  MikanAPI
  MikanSerialization
  RmlCore
  RmlDebugger)

# Remove problematic files from the unity builds (header file inclusion ordering issues)
set_source_files_properties(${IMGUI_SOURCE} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${IMNODES_SOURCE} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/Device/Enumerator/OpenCVCameraEnumerator.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetReader.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetWriter.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/Interprocess/WebsocketInterprocessMessageClient.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/Math/MathGLM.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
set(APP_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/winapp_resources.rc")
add_executable(Mikan WIN32 ${MIKAN_SRC} ${APP_RESOURCE_WINDOWS})
ELSE() #Linux/Darwin
add_executable(Mikan ${MIKAN_SRC})
ENDIF()

target_include_directories(Mikan PUBLIC ${MIKAN_INCL_DIRS})
target_link_libraries(Mikan ${MIKAN_REQ_LIBS})
target_compile_definitions(Mikan PRIVATE ENABLE_SERIALIZATION_REFLECTION)
target_compile_definitions(Mikan PRIVATE ENABLE_MIKANCORE_REFLECTION)
target_compile_definitions(Mikan PRIVATE ENABLE_MIKANAPI_REFLECTION)
target_compile_definitions(Mikan PRIVATE ENABLE_GL_PROGRAM_CONFIG)
target_compile_definitions(Mikan PRIVATE JSON_DISABLE_ENUM_SERIALIZATION=1)

# Runtime dependencies
set(FFMPEG_DLL_FOLDER ${ROOT_DIR}/deps/ffmpeg-4.4.1-full_build-shared/bin)
set(OPENCV_VIDEOIO_DLL ${OpenCV_DIR}/x64/vc16/bin/opencv_videoio_ffmpeg4100_64.dll)
set(OPENVR_DLL ${OPENVR_ROOT_DIR}/bin/${OPENVR_PLATFORM}/openvr_api.dll)
set(SDL2_DLL ${ROOT_DIR}/deps/SDL2-2.0.10/lib/x64/SDL2.dll)
set(SDL2_IMAGE_DLL_FOLDER ${ROOT_DIR}/deps/SDL2_image-2.0.5/lib/x64)
set(SDL2_TTF_DLL_FOLDER ${ROOT_DIR}/deps/SDL2_ttf-2.0.15/lib/x64)
set(EASY_PROFILER_FOLDER ${ROOT_DIR}/deps/easy_profiler/bin)

# Post build - copy runtime dependencies to binary build folder (for debugging)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  set_property(TARGET Mikan PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "../..")

  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanCore>/MikanCore.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanAPI>/MikanAPI.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanSerialization>/MikanSerialization.dll
          $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE_DIR:MikanSerialization>/MikanSerialization.pdb
          $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${OPENCV_VIDEOIO_DLL} $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${OpenCV_DIR}/x64/vc16/bin/opencv_world4100d.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${OpenCV_DIR}/x64/vc16/bin/opencv_world4100.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${FFMPEG_DLL_FOLDER}/avcodec-58.dll"
    "${FFMPEG_DLL_FOLDER}/avdevice-58.dll"
    "${FFMPEG_DLL_FOLDER}/avfilter-7.dll"
    "${FFMPEG_DLL_FOLDER}/avformat-58.dll"
    "${FFMPEG_DLL_FOLDER}/avutil-56.dll"
    "${FFMPEG_DLL_FOLDER}/swresample-3.dll"
    "${FFMPEG_DLL_FOLDER}/swscale-5.dll"
    $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${OPENVR_DLL} $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${SDL2_DLL} $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${SDL2_IMAGE_DLL_FOLDER}/libjpeg-9.dll" 
    "${SDL2_IMAGE_DLL_FOLDER}/libpng16-16.dll"
    "${SDL2_IMAGE_DLL_FOLDER}/libtiff-5.dll"
    "${SDL2_IMAGE_DLL_FOLDER}/libwebp-7.dll"
    "${SDL2_IMAGE_DLL_FOLDER}/SDL2_image.dll"
    "${SDL2_IMAGE_DLL_FOLDER}/zlib1.dll"
    $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${SDL2_TTF_DLL_FOLDER}/libfreetype-6.dll"
    "${SDL2_TTF_DLL_FOLDER}/SDL2_ttf.dll"
    $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${GLEW_SHARED_LIBRARIES} $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${SPOUT2_SHARED_LIBRARIES}" 
    $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${EASY_PROFILER_FOLDER}/easy_profiler.dll"
    $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${LUA_SHARED_LIBRARIES}" 
    $<TARGET_FILE_DIR:Mikan>)	
  add_custom_command(
	TARGET Mikan POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${RFK_SHARED_LIBRARIES}"
	$<TARGET_FILE_DIR:Mikan>)	
	
ELSE() #Linux/Darwin
ENDIF()

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS Mikan
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib)

  install(FILES 
      "${FFMPEG_DLL_FOLDER}/avcodec-58.dll"
      "${FFMPEG_DLL_FOLDER}/avcodec-58.dll"
      "${FFMPEG_DLL_FOLDER}/avdevice-58.dll"
      "${FFMPEG_DLL_FOLDER}/avfilter-7.dll"
      "${FFMPEG_DLL_FOLDER}/avformat-58.dll"
      "${FFMPEG_DLL_FOLDER}/avutil-56.dll"
      "${FFMPEG_DLL_FOLDER}/swresample-3.dll"
      "${FFMPEG_DLL_FOLDER}/swscale-5.dll"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})

  install(FILES "${OPENCV_VIDEOIO_DLL}"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})

  install(FILES 
      "${OpenCV_DIR}/x64/vc16/bin/opencv_world4100d.dll"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})
  install(FILES "${OpenCV_DIR}/x64/vc16/bin/opencv_world4100d.dll"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})
      
  install(FILES "${OPENVR_DLL}"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})
  
  install(FILES "${SDL2_DLL}"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})

  install(FILES "${SPOUT2_SHARED_LIBRARIES}"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})
      
  install(FILES "${LUA_SHARED_LIBRARIES}"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})
	  
  install(FILES "${RFK_SHARED_LIBRARIES}"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})	  
      
  install(FILES 
      "${SDL2_IMAGE_DLL_FOLDER}/libjpeg-9.dll" 
      "${SDL2_IMAGE_DLL_FOLDER}/libpng16-16.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/libtiff-5.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/libwebp-7.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/SDL2_image.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/zlib1.dll"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})

  install(FILES 
      "${SDL2_TTF_DLL_FOLDER}/libfreetype-6.dll"
      "${SDL2_TTF_DLL_FOLDER}/SDL2_ttf.dll"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})
      
  install(FILES 
      "${GLEW_SHARED_LIBRARIES}"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})

  install(FILES 
      "${EASY_PROFILER_FOLDER}/easy_profiler.dll"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})

  install(DIRECTORY ${ROOT_DIR}/resources
    DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
    FILES_MATCHING PATTERN "*.jpg" PATTERN "*.png" PATTERN "*.ttf" PATTERN "*.json" PATTERN "*.csv" PATTERN "*.lua" PATTERN "*.rml" PATTERN "*.rcss" PATTERN "*.onnx")
    
ELSE() #Linux/Darwin
ENDIF()

MESSAGE(STATUS "Stepping into tests")
add_subdirectory(Tests)
