## Mikan Editor
## =======================
MESSAGE(STATUS "Generating Mikan Editor Project")

# Dependencies
set(MIKAN_INCL_DIRS)
set(MIKAN_REQ_LIBS)

list(APPEND MIKAN_REQ_LIBS ${PLATFORM_LIBS})

# AppCore 
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
## ModalSelectCamera
file(GLOB MIKAN_APP_STAGES_MODAL_SELECT_CAMERA_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/ModalSelectCamera/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/ModalSelectCamera/*.h"
)
source_group("AppStages\\ModalSelectCamera" FILES ${MIKAN_APP_STAGES_MODAL_SELECT_CAMERA_SRC})
## ModalSceneAddCompositor
file(GLOB MIKAN_APP_STAGES_MODAL_SCENE_ADD_COMPOSITOR_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/ModalSceneAddCompositor/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/ModalSceneAddCompositor/*.h"
)
source_group("AppStages\\ModalSceneAddCompositor" FILES ${MIKAN_APP_STAGES_MODAL_SCENE_ADD_COMPOSITOR_SRC})
## MonoLensCalibration
file(GLOB MIKAN_APP_STAGES_MONO_LENS_CALIBRATION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/MonoLensCalibration/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/MonoLensCalibration/*.h"
)
source_group("AppStages\\MonoLensCalibration" FILES ${MIKAN_APP_STAGES_MONO_LENS_CALIBRATION_SRC})
## Project
file(GLOB MIKAN_APP_STAGES_PROJECT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/Project/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/Project/*.h"
)
source_group("AppStages\\Project" FILES ${MIKAN_APP_STAGES_PROJECT_SRC})
## Shared
file(GLOB MIKAN_APP_STAGES_SHARED_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/Shared/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/Shared/*.h"
)
source_group("AppStages\\Shared" FILES ${MIKAN_APP_STAGES_SHARED_SRC})
## StencilAlignment
file(GLOB MIKAN_APP_STAGES_STENCIL_ALIGNMENT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/StencilAlignment/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/StencilAlignment/*.h"
)
source_group("AppStages\\StencilAlignment" FILES ${MIKAN_APP_STAGES_STENCIL_ALIGNMENT_SRC})
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
    ${MIKAN_APP_STAGES_MAIN_MENU_SRC}
    ${MIKAN_APP_STAGES_MODAL_CONFIRM_SRC}
    ${MIKAN_APP_STAGES_MODAL_SCENE_ADD_COMPOSITOR_SRC}
    ${MIKAN_APP_STAGES_MODAL_SELECT_CAMERA_SRC}
    ${MIKAN_APP_STAGES_MONO_LENS_CALIBRATION_SRC}
    ${MIKAN_APP_STAGES_PROJECT_SRC}
    ${MIKAN_APP_STAGES_SHARED_SRC}
    ${MIKAN_APP_STAGES_STENCIL_ALIGNMENT_SRC}
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

# Config
file(GLOB MIKAN_CONFIG_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Config/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Config/*.h"
)
source_group("Config" FILES ${MIKAN_CONFIG_SRC})

# Delegates
file(GLOB MIKAN_DELEGATES_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Delegates/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Delegates/*.h"
)
source_group("Delegates" FILES ${MIKAN_DELEGATES_SRC})

# ECS
file(GLOB MIKAN_ECS_ROOT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/*.h"
)
source_group("ECS" FILES ${MIKAN_ECS_ROOT_SRC})
## Anchor
file(GLOB MIKAN_ECS_ANCHOR_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Anchor/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Anchor/*.h"
)
source_group("ECS\\Anchor" FILES ${MIKAN_ECS_ANCHOR_SRC})
## Camera
file(GLOB MIKAN_ECS_CAMERA_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Camera/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Camera/*.h"
)
source_group("ECS\\Camera" FILES ${MIKAN_ECS_CAMERA_SRC})
## Collision
file(GLOB MIKAN_ECS_COLLISION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Collision/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Collision/*.h"
)
source_group("ECS\\Collision" FILES ${MIKAN_ECS_COLLISION_SRC})
## Compositor
file(GLOB MIKAN_ECS_COMPOSITOR_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Compositor/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Compositor/*.h"
)
source_group("ECS\\Compositor" FILES ${MIKAN_ECS_COMPOSITOR_SRC})
## Editor
file(GLOB MIKAN_ECS_EDITOR_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Editor/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Editor/*.h"
)
source_group("ECS\\Editor" FILES ${MIKAN_ECS_EDITOR_SRC})
## Marker
file(GLOB MIKAN_ECS_MARKER_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Marker/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Marker/*.h"
)
source_group("ECS\\Marker" FILES ${MIKAN_ECS_MARKER_SRC})
## Scene
file(GLOB MIKAN_ECS_SCENE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Scene/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Scene/*.h"
)
source_group("ECS\\Scene" FILES ${MIKAN_ECS_SCENE_SRC})
## Stage
file(GLOB MIKAN_ECS_STAGE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Stage/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Stage/*.h"
)
source_group("ECS\\Stage" FILES ${MIKAN_ECS_STAGE_SRC})
## Stencil
file(GLOB MIKAN_ECS_STENCIL_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Stencil/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/Stencil/*.h"
)
source_group("ECS\\Stencil" FILES ${MIKAN_ECS_STENCIL_SRC})
## TrackingMount
file(GLOB MIKAN_ECS_TRACKINGMOUNT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/TrackingMount/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/TrackingMount/*.h"
)
source_group("ECS\\TrackingMount" FILES ${MIKAN_ECS_TRACKINGMOUNT_SRC})
## TrackingVolume
file(GLOB MIKAN_ECS_TRACKINGVOLUME_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/TrackingVolume/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/TrackingVolume/*.h"
)
source_group("ECS\\TrackingVolume" FILES ${MIKAN_ECS_TRACKINGVOLUME_SRC})
## VRObject
file(GLOB MIKAN_ECS_VROBJECT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/VRObject/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/VRObject/*.h"
)
source_group("ECS\\VRObject" FILES ${MIKAN_ECS_VROBJECT_SRC})
## Video Source
file(GLOB MIKAN_ECS_VIDEO_SOURCE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/ECS/VideoSource/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ECS/VideoSource/*.h"
)
source_group("ECS\\VideoSource" FILES ${MIKAN_ECS_VIDEO_SOURCE_SRC})
## ECS Full Source
set(MIKAN_ECS_SRC
    ${MIKAN_ECS_ROOT_SRC}
    ${MIKAN_ECS_ANCHOR_SRC}
    ${MIKAN_ECS_CAMERA_SRC}
    ${MIKAN_ECS_COLLISION_SRC}
    ${MIKAN_ECS_COMPOSITOR_SRC}
    ${MIKAN_ECS_EDITOR_SRC}
    ${MIKAN_ECS_MARKER_SRC}
    ${MIKAN_ECS_SCENE_SRC}
    ${MIKAN_ECS_STAGE_SRC}
    ${MIKAN_ECS_STENCIL_SRC}
    ${MIKAN_ECS_TRACKINGMOUNT_SRC}
    ${MIKAN_ECS_TRACKINGVOLUME_SRC}
    ${MIKAN_ECS_VROBJECT_SRC}
    ${MIKAN_ECS_VIDEO_SOURCE_SRC}
)

# ImGUI's source files are included in directly, rather than linking against a lib
source_group("ImGui" FILES ${IMGUI_SOURCE})

# ImNodes's source files are included in directly, rather than linking against a lib
source_group("ImNodes" FILES ${IMNODES_SOURCE})

# Interprocess
file(GLOB MIKAN_INTERPROCESS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/*.h"
)
source_group("Interprocess" FILES ${MIKAN_INTERPROCESS_SRC})

# Localization
file(GLOB MIKAN_LOCALIZATION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Localization/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Localization/*.h"
)
source_group("Localization" FILES ${MIKAN_LOCALIZATION_SRC})

# Localization
file(GLOB MIKAN_MATH_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Math/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Math/*.h"
)
source_group("Math" FILES ${MIKAN_MATH_SRC})

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

# OpenCV
file(GLOB MIKAN_OPENCV_SRC
    "${CMAKE_CURRENT_LIST_DIR}/OpenCV/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/OpenCV/*.h"
)
source_group("OpenCV" FILES ${MIKAN_OPENCV_SRC})

# Project
file(GLOB MIKAN_PROJECT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Project/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Project/*.h"
)
source_group("Project" FILES ${MIKAN_PROJECT_SRC})

# Renderer
file(GLOB MIKAN_RENDERER_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Renderer/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Renderer/*.h"
)
source_group("Renderer" FILES ${MIKAN_RENDERER_SRC})

# RmlUI
file(GLOB MIKAN_RMLUI_SRC
    "${CMAKE_CURRENT_LIST_DIR}/RmlUI/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/RmlUI/*.h"
)
source_group("RmlUI" FILES ${MIKAN_RMLUI_SRC})

# Sources
## Client Sources
file(GLOB MIKAN_SOURCES_CLIENT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Sources/Client/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Sources/Client/*.h"
)
source_group("Sources\\Client" FILES ${MIKAN_SOURCES_CLIENT_SRC})

## Spout Sources
file(GLOB MIKAN_SOURCES_SPOUT_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Sources/Spout/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Sources/Spout/*.h"
)
source_group("Sources\\Spout" FILES ${MIKAN_SOURCES_SPOUT_SRC})

## Video Sources
file(GLOB MIKAN_SOURCES_VIDEO_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Sources/Video/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Sources/Video/*.h"
)
source_group("Sources\\Video" FILES ${MIKAN_SOURCES_VIDEO_SRC})

# Scripting
file(GLOB MIKAN_SCRIPTING_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Scripting/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Scripting/*.h"
)
source_group("Scripting" FILES ${MIKAN_SCRIPTING_SRC})

# SDL
file(GLOB MIKAN_SDL_SRC
    "${CMAKE_CURRENT_LIST_DIR}/SDL/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SDL/*.h"
)
source_group("SDL" FILES ${MIKAN_SDL_SRC})

# Server
file(GLOB MIKAN_SERVER_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Server/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Server/*.h"
)
source_group("Server" FILES ${MIKAN_SERVER_SRC})

# TinyFileDialogs's source files are included in directly, rather than linking against a lib
source_group("TinyFileDialogs" FILES ${TINYFILEDIALOGS_SOURCE})

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
    ${MIKAN_DELEGATES_SRC}
    ${MIKAN_ECS_SRC}
    ${MIKAN_INTERPROCESS_SRC}
    ${MIKAN_LOCALIZATION_SRC}
    ${MIKAN_MATH_SRC}
    ${MIKAN_NODE_EDITORS_SRC}
    ${MIKAN_OBJECT_SRC}
    ${MIKAN_OPENCV_SRC}
    ${MIKAN_PROJECT_SRC}
    ${MIKAN_RENDERER_SRC}
    ${MIKAN_RMLUI_SRC}
    ${MIKAN_SCRIPTING_SRC}
    ${MIKAN_SOURCES_CLIENT_SRC}
    ${MIKAN_SOURCES_SPOUT_SRC}
    ${MIKAN_SOURCES_VIDEO_SRC}
    ${MIKAN_SDL_SRC}
    ${MIKAN_SERVER_SRC}
    ${MIKAN_SYSTEM_SRC}
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
  ${CMAKE_CURRENT_LIST_DIR}/Config
  ${CMAKE_CURRENT_LIST_DIR}/Delegates
  ${CMAKE_CURRENT_LIST_DIR}/ECS
  ${CMAKE_CURRENT_LIST_DIR}/ECS/Anchor
  ${CMAKE_CURRENT_LIST_DIR}/ECS/Camera
  ${CMAKE_CURRENT_LIST_DIR}/ECS/Collision
  ${CMAKE_CURRENT_LIST_DIR}/ECS/Compositor
  ${CMAKE_CURRENT_LIST_DIR}/ECS/Editor
  ${CMAKE_CURRENT_LIST_DIR}/ECS/Marker
  ${CMAKE_CURRENT_LIST_DIR}/ECS/Scene
  ${CMAKE_CURRENT_LIST_DIR}/ECS/Stage
  ${CMAKE_CURRENT_LIST_DIR}/ECS/Stencil
  ${CMAKE_CURRENT_LIST_DIR}/ECS/TrackingMount
  ${CMAKE_CURRENT_LIST_DIR}/ECS/TrackingVolume
  ${CMAKE_CURRENT_LIST_DIR}/ECS/VRObject
  ${CMAKE_CURRENT_LIST_DIR}/ECS/VideoSource
  ${CMAKE_CURRENT_LIST_DIR}/Interprocess
  ${CMAKE_CURRENT_LIST_DIR}/Localization
  ${CMAKE_CURRENT_LIST_DIR}/Math
  ${CMAKE_CURRENT_LIST_DIR}/NodeEditors
  ${CMAKE_CURRENT_LIST_DIR}/OpenCV
  ${CMAKE_CURRENT_LIST_DIR}/Project
  ${CMAKE_CURRENT_LIST_DIR}/Renderer
  ${CMAKE_CURRENT_LIST_DIR}/RmlUI
  ${CMAKE_CURRENT_LIST_DIR}/SDL
  ${CMAKE_CURRENT_LIST_DIR}/Server
  ${CMAKE_CURRENT_LIST_DIR}/Scripting
  ${CMAKE_CURRENT_LIST_DIR}/Sources/Client
  ${CMAKE_CURRENT_LIST_DIR}/Sources/Spout
  ${CMAKE_CURRENT_LIST_DIR}/Sources/Video
  ${MIKAN_LIBRARIES_DIR}/MikanClientAPI/Public
  ${MIKAN_LIBRARIES_DIR}/MikanClientCore/Public
  ${MIKAN_LIBRARIES_DIR}/MikanCoreApp/Public
  ${MIKAN_LIBRARIES_DIR}/MikanMath/Public
  ${MIKAN_LIBRARIES_DIR}/MikanObject/Public
  ${MIKAN_LIBRARIES_DIR}/MikanRenderer/Public
  ${MIKAN_LIBRARIES_DIR}/MikanSerialization/Public
  ${MIKAN_LIBRARIES_DIR}/MikanSharedTexture/Public
  ${MIKAN_LIBRARIES_DIR}/MikanUtility/Public
  ${MIKAN_PLUGINS_DIR}/MikanGStreamer/Public
  ${CMAKE_CURRENT_LIST_DIR}/RmlUi/Public
  ${CMAKE_CURRENT_LIST_DIR}/SDL/Public
  ${RFK_GENERATED_ROOT_DIR}/MikanClientCore
  ${RFK_GENERATED_ROOT_DIR}/MikanClientAPI
  ${RFK_GENERATED_ROOT_DIR}/MikanSerialization
  ${IMGUI_DIR}
  ${IMNODES_DIR}
  ${IXWEBSOCKET_INCLUDE_DIR}
  ${LOCKFREEQUEUE_INCLUDE_DIR}
  ${LIBHARU_INCLUDE_DIRS}
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
  ${SPOUT2_INCLUDE_DIRS}
  ${LUA_INCLUDE_DIRS}
  ${LUA_BRIDGE_INCLUDE_DIRS}
  ${FAST_OBJ_LOADER_INCLUDE_DIRS}
  ${libharu_SOURCE_DIR}/include
  ${libharu_BINARY_DIR}/include)

list(APPEND MIKAN_REQ_LIBS
  ${OpenCV_LIBS}
  ${OPENVR_LIBRARY}
  ${SDL2_LIBRARY}
  ${SDL2_IMAGE_LIBRARY}
  ${SDL2TTF_LIBRARY}
  ${MIKAN_EXTRA_LIBS}
  ${GLEW_LIBRARIES}
  ${SPOUT2_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  ${LIBHARU_LIBRARIES}
  ${LUA_LIBRARIES}
  ${RFK_LIBRARIES}
  dylib
  easy_profiler
  fast_obj_lib
  ixwebsocket
  MikanClientAPI
  MikanClientCore
  MikanCoreApp
  MikanMath
  MikanRenderer
  MikanSerialization
  MikanSharedTexture
  MikanUtility
  RmlCore
  RmlDebugger)

# Remove problematic files from the unity builds (header file inclusion ordering issues)
set_source_files_properties(${IMGUI_SOURCE} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${IMNODES_SOURCE} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/Device/Enumerator/OpenCVCameraEnumerator.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/Interprocess/WebsocketInterprocessMessageServer.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/Interprocess/SharedTextureReader.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

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
set(SDL2_DLL ${ROOT_DIR}/deps/SDL2-2.0.10/lib/x64/SDL2.dll)
set(SDL2_IMAGE_DLL_FOLDER ${ROOT_DIR}/deps/SDL2_image-2.0.5/lib/x64)
set(SDL2_TTF_DLL_FOLDER ${ROOT_DIR}/deps/SDL2_ttf-2.0.15/lib/x64)
set(EASY_PROFILER_FOLDER ${ROOT_DIR}/deps/easy_profiler/bin)

# Post build - copy runtime dependencies to binary build folder (for debugging)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  set_property(TARGET Mikan PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "../../..")

  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanClientAPI>/MikanClientAPI.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanClientCore>/MikanClientCore.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanCoreApp>/MikanCoreApp.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanMath>/MikanMath.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanRenderer>/MikanRenderer.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanSharedTexture>/MikanSharedTexture.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanSerialization>/MikanSerialization.dll $<TARGET_FILE_DIR:Mikan>)
  set( MIKANSERIALIZATION_PDB_COPY_CMD 
    ${CMAKE_COMMAND} -E
    copy 
    "$<TARGET_FILE_DIR:MikanSerialization>/$<TARGET_FILE_BASE_NAME:MikanSerialization>.pdb"
    "$<TARGET_FILE_DIR:Mikan>")
  add_custom_command( TARGET Mikan POST_BUILD 
    COMMAND "$<$<CONFIG:Debug,RelWithDebInfo>:${MIKANSERIALIZATION_PDB_COPY_CMD}>"
    DEPENDS MikanSerialization
    COMMAND_EXPAND_LISTS)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanUtility>/MikanUtility.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${OpenCV_DIR}/x64/vc16/bin/opencv_world4100d.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${OpenCV_DIR}/x64/vc16/bin/opencv_world4100.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanGStreamerVideo>/MikanGStreamerVideo.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:MikanWMFVideo>/MikanWMFVideo.dll $<TARGET_FILE_DIR:Mikan>)
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
      "${OpenCV_DIR}/x64/vc16/bin/opencv_world4100d.dll"
      DESTINATION ${MIKAN_ARCH_INSTALL_PATH})
  install(FILES "${OpenCV_DIR}/x64/vc16/bin/opencv_world4100d.dll"
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