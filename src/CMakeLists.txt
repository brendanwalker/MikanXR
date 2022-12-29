cmake_minimum_required(VERSION 3.0)

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
## CalibrationPatternSetings
file(GLOB MIKAN_APP_STAGES_CALIBRATION_PATTERN_SETTINGS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/CalibrationPatternSettings/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/CalibrationPatternSettings/*.h"
)
source_group("AppStages\\CalibrationPatternSettings" FILES ${MIKAN_APP_STAGES_CALIBRATION_PATTERN_SETTINGS_SRC})
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
## VRDeviceSettings
file(GLOB MIKAN_APP_STAGES_VR_DEVICE_SETTINGS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/VRDeviceSettings/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/AppStages/VRDeviceSettings/*.h"
)
source_group("AppStages\\VRDeviceSettings" FILES ${MIKAN_APP_STAGES_VR_DEVICE_SETTINGS_SRC})
## Mikan All AppStage Source
set(MIKAN_APP_STAGES_SRC
    ${MIKAN_APP_STAGES_ROOT_SRC}
    ${MIKAN_APP_STAGES_ALIGNMENT_CALIBRATION_SRC}
    ${MIKAN_APP_STAGES_CALIBRATION_PATTERN_SETTINGS_SRC}
    ${MIKAN_APP_STAGES_CAMERA_SETTINGS_SRC}
    ${MIKAN_APP_STAGES_COMPOSITOR_SRC}
    ${MIKAN_APP_STAGES_FILE_BROWSER_SRC}
    ${MIKAN_APP_STAGES_MAIN_MENU_SRC}
    ${MIKAN_APP_STAGES_MONO_LENS_CALIBRATION_SRC}
    ${MIKAN_APP_STAGES_SHARED_SRC}
    ${MIKAN_APP_STAGES_SPATIAL_ANCHORS_SRC}
    ${MIKAN_APP_STAGES_VR_DEVICE_SETTINGS_SRC}
)

# Calibration
file(GLOB MIKAN_CALIBRATION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Calibration/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Calibration/*.h"
)
source_group("Calibration" FILES ${MIKAN_CALIBRATION_SRC})

# Client
## Client-Unity
file(GLOB MIKAN_CLIENT_UNITY_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Client/Unity/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Client/Unity/*.h"
)
source_group("Client\\Unity" FILES ${MIKAN_CLIENT_UNITY_SRC})
## Client Spout
file(GLOB MIKAN_CLIENT_SPOUT2_DX_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Client/SpoutDX/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Client/SpoutDX/*.h"
)
source_group("Client\\SpoutDX" FILES ${MIKAN_CLIENT_SPOUT2_DX_SRC})
## Client Types
file(GLOB MIKAN_CLIENT_TYPES_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Client/MikanClientTypes.h"
    "${CMAKE_CURRENT_LIST_DIR}/Client/MikanMathTypes.h"
)
source_group("Client" FILES ${MIKAN_CLIENT_TYPES_SRC})

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

# Interprocess
file(GLOB MIKAN_INTERPROCESS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessMessages.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessMessages.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetShared.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetShared.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetReader.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetReader.cpp"
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

# Renderer
file(GLOB MIKAN_RENDERER_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Renderer/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Renderer/*.h"
)
source_group("Renderer" FILES ${MIKAN_RENDERER_SRC})

# Renderer
file(GLOB MIKAN_RMLUI_SRC
    "${CMAKE_CURRENT_LIST_DIR}/RmlUi/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/RmlUi/*.h"
)
source_group("RmlUI" FILES ${MIKAN_RMLUI_SRC})

# Scripting
file(GLOB MIKAN_SCRIPTING_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Scripting/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Scripting/*.h"
)
source_group("Scripting" FILES ${MIKAN_SCRIPTING_SRC})

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
    ${MIKAN_CALIBRATION_SRC}
    ${MIKAN_CLIENT_SRC}
    ${MIKAN_CLIENT_TYPES_SRC}
    ${MIKAN_CONFIG_SRC}
    ${MIKAN_DEVICE_ENUM_SRC}
    ${MIKAN_DEVICE_INTERFACE_SRC}
    ${MIKAN_DEVICE_MANAGER_SRC}
    ${MIKAN_DEVICE_VIEW_SRC}
    ${MIKAN_INTERPROCESS_SRC}
    ${MIKAN_LOCALIZATION_SRC}
    ${MIKAN_MATH_SRC}
    ${MIKAN_RENDERER_SRC}
    ${MIKAN_RMLUI_SRC}
    ${MIKAN_SCRIPTING_SRC}
    ${MIKAN_UTILS_SRC}
    ${MIKAN_VIDEO_SRC}
    ${MIKAN_VIDEO_OPENCV_SRC}
    ${MIKAN_VIDEO_WMF_SRC}
    ${MIKAN_VRTRACKER_SRC}
    ${MIKAN_VRTRACKER_STEAMVR_SRC}
    ${IMGUI_SOURCE}
)

list(APPEND MIKAN_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/AppCore
  ${CMAKE_CURRENT_LIST_DIR}/AppStages
  ${CMAKE_CURRENT_LIST_DIR}/Calibration
  ${CMAKE_CURRENT_LIST_DIR}/Config
  ${CMAKE_CURRENT_LIST_DIR}/Client
  ${CMAKE_CURRENT_LIST_DIR}/Device/Enumerator
  ${CMAKE_CURRENT_LIST_DIR}/Device/Interface
  ${CMAKE_CURRENT_LIST_DIR}/Device/Manager
  ${CMAKE_CURRENT_LIST_DIR}/Device/View
  ${CMAKE_CURRENT_LIST_DIR}/Interprocess
  ${CMAKE_CURRENT_LIST_DIR}/Localization
  ${CMAKE_CURRENT_LIST_DIR}/Math
  ${CMAKE_CURRENT_LIST_DIR}/Renderer
  ${CMAKE_CURRENT_LIST_DIR}/RmlUi
  ${CMAKE_CURRENT_LIST_DIR}/Scripting
  ${CMAKE_CURRENT_LIST_DIR}/Utility
  ${CMAKE_CURRENT_LIST_DIR}/Video
  ${CMAKE_CURRENT_LIST_DIR}/Video/OpenCV
  ${CMAKE_CURRENT_LIST_DIR}/Video/WMF
  ${CMAKE_CURRENT_LIST_DIR}/VRTracker
  ${CMAKE_CURRENT_LIST_DIR}/VRTracker/SteamVR
  ${IMGUI_DIR}
  ${RMLUI_INCLUDE_DIR}
  ${ROOT_DIR}/thirdparty/imgui-filebrowser
  ${ROOT_DIR}/thirdparty/glm/
  ${ROOT_DIR}/thirdparty/Configuru/
  ${OpenCV_INCLUDE_DIR} 
  ${OPENVR_INCLUDE_DIR} 
  ${SDL2_INCLUDE_DIR} 
  ${SDL2_IMAGE_INCLUDE_DIR}
  ${SDL2TTF_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIRS}
  ${FastCSV_INCLUDE_DIRS}
  ${FFMPEG_INCLUDE_DIRS}
  ${SPOUT2_INCLUDE_DIRS}
  ${BOOST_INTERPROCESS_INCLUDE_DIRS}
  ${OBJ_LOADER_INCLUDE_DIRS}
  ${LUA_INCLUDE_DIRS}
  ${LUA_BRIDGE_INCLUDE_DIRS})

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
  ${LUA_LIBRARIES}
  ${RMLUI_LIBRARIES}
  easy_profiler)

set_source_files_properties(${IMGUI_SOURCE} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
set(APP_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/winapp_resources.rc")
add_executable(Mikan WIN32 ${MIKAN_SRC} ${APP_RESOURCE_WINDOWS})
ELSE() #Linux/Darwin
add_executable(Mikan ${MIKAN_SRC})
ENDIF()

target_include_directories(Mikan PUBLIC ${MIKAN_INCL_DIRS})
target_link_libraries(Mikan ${MIKAN_REQ_LIBS})

# Mikan C-API shared library
file(GLOB MIKAN_CLIENT_LIBRARY_SRC
    "${CMAKE_CURRENT_LIST_DIR}/AppCore/Logger.h"
    "${CMAKE_CURRENT_LIST_DIR}/AppCore/Logger.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Client/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Client/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessMessages.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessMessages.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetShared.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetShared.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetWriter.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetWriter.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Utility/RandomUtils.h"
    "${CMAKE_CURRENT_LIST_DIR}/Utility/RandomUtils.cpp"
    ${MIKAN_CLIENT_UNITY_SRC}
    ${MIKAN_CLIENT_SPOUT2_DX_SRC}
)

file(GLOB MIKAN_INTERPROCESS_WRITER_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetWriter.h"
    "${CMAKE_CURRENT_LIST_DIR}/Interprocess/InterprocessRenderTargetWriter.cpp"
)
source_group("Interprocess" FILES ${MIKAN_INTERPROCESS_WRITER_SRC})

list(APPEND MIKAN_CLIENT_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/AppCore
  ${CMAKE_CURRENT_LIST_DIR}/Client
  ${CMAKE_CURRENT_LIST_DIR}/Client/SpoutDX
  ${CMAKE_CURRENT_LIST_DIR}/Interprocess
  ${CMAKE_CURRENT_LIST_DIR}/Utility
  ${SPOUT2_INCLUDE_DIRS}
  ${BOOST_INTERPROCESS_INCLUDE_DIRS})

list(APPEND MIKAN_CLIENT_REQ_LIBS
  ${SPOUT2_LIBRARIES}
  ${MIKAN_EXTRA_LIBS})
  
set_source_files_properties(${MIKAN_CLIENT_SPOUT2_DX_SRC} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

add_library(Mikan_CAPI SHARED ${MIKAN_CLIENT_LIBRARY_SRC})
target_include_directories(Mikan_CAPI PRIVATE ${MIKAN_CLIENT_INCL_DIRS})
target_link_libraries(Mikan_CAPI PRIVATE ${MIKAN_CLIENT_REQ_LIBS})
set_target_properties(Mikan_CAPI PROPERTIES PUBLIC_HEADER "Client/MikanClientTypes.h;Client/MikanMathTypes.h;Client/MikanClient_CAPI.h;Client/MikanClient_export.h")
set_target_properties(Mikan_CAPI PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(Mikan_CAPI PROPERTIES C_VISIBILITY_PRESET hidden)
target_compile_definitions(Mikan_CAPI PRIVATE MIKAN_CLIENT_CAPI_EXPORTS)

# Runtime dependencies
set(FFMPEG_DLL_FOLDER ${ROOT_DIR}/deps/ffmpeg-4.4.1-full_build-shared/bin)
set(OPENCV_VIDEOIO_DLL ${OpenCV_DIR}/bin/opencv_videoio_ffmpeg455_64.dll)
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
    COMMAND ${CMAKE_COMMAND} -E copy ${OPENCV_VIDEOIO_DLL} $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${OpenCV_DIR}/x64/vc15/bin/opencv_world455d.dll $<TARGET_FILE_DIR:Mikan>)
  add_custom_command(
    TARGET Mikan POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${OpenCV_DIR}/x64/vc15/bin/opencv_world455.dll $<TARGET_FILE_DIR:Mikan>)
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
    "${RMLUI_BINARIES_DIR}/RmlCore.dll" 
    "${RMLUI_BINARIES_DIR}/RmlDebugger.dll" 
    "${RMLUI_BINARIES_DIR}/RmlLua.dll"
    "${FREETYPE_SHARED_LIBRARY}"
    $<TARGET_FILE_DIR:Mikan>)
ELSE() #Linux/Darwin
ENDIF()

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS Mikan
      CONFIGURATIONS Debug
      RUNTIME DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}/lib)
  install(TARGETS Mikan
      CONFIGURATIONS Release
      RUNTIME DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}/lib)

  install(FILES 
      "${FFMPEG_DLL_FOLDER}/avcodec-58.dll"
      "${FFMPEG_DLL_FOLDER}/avcodec-58.dll"
      "${FFMPEG_DLL_FOLDER}/avdevice-58.dll"
      "${FFMPEG_DLL_FOLDER}/avfilter-7.dll"
      "${FFMPEG_DLL_FOLDER}/avformat-58.dll"
      "${FFMPEG_DLL_FOLDER}/avutil-56.dll"
      "${FFMPEG_DLL_FOLDER}/swresample-3.dll"
      "${FFMPEG_DLL_FOLDER}/swscale-5.dll"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES 
      "${FFMPEG_DLL_FOLDER}/avcodec-58.dll"
      "${FFMPEG_DLL_FOLDER}/avcodec-58.dll"
      "${FFMPEG_DLL_FOLDER}/avdevice-58.dll"
      "${FFMPEG_DLL_FOLDER}/avfilter-7.dll"
      "${FFMPEG_DLL_FOLDER}/avformat-58.dll"
      "${FFMPEG_DLL_FOLDER}/avutil-56.dll"
      "${FFMPEG_DLL_FOLDER}/swresample-3.dll"
      "${FFMPEG_DLL_FOLDER}/swscale-5.dll"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})

  install(FILES "${OPENCV_VIDEOIO_DLL}"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES "${OPENCV_VIDEOIO_DLL}"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})

  install(FILES 
      "${OpenCV_DIR}/x64/vc15/bin/opencv_world455d.dll"
      "${OpenCV_DIR}/x64/vc15/bin/opencv_world455d.pdb"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES "${OpenCV_DIR}/x64/vc15/bin/opencv_world455.dll"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})
      
  install(FILES "${OPENVR_DLL}"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES "${OPENVR_DLL}"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})
  
  install(FILES "${SDL2_DLL}"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES "${SDL2_DLL}"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})

  install(FILES "${SPOUT2_SHARED_LIBRARIES}"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES "${SPOUT2_SHARED_LIBRARIES}"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})
      
  install(FILES "${LUA_SHARED_LIBRARIES}"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES "${LUA_SHARED_LIBRARIES}"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})
      
  install(FILES 
      "${RMLUI_BINARIES_DIR}/RmlCore.dll" 
      "${RMLUI_BINARIES_DIR}/RmlDebugger.dll" 
      "${RMLUI_BINARIES_DIR}/RmlLua.dll"
      "${FREETYPE_SHARED_LIBRARY}"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES 
      "${RMLUI_BINARIES_DIR}/RmlCore.dll" 
      "${RMLUI_BINARIES_DIR}/RmlDebugger.dll" 
      "${RMLUI_BINARIES_DIR}/RmlLua.dll"
      "${FREETYPE_SHARED_LIBRARY}"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})
      
  install(FILES 
      "${SDL2_IMAGE_DLL_FOLDER}/libjpeg-9.dll" 
      "${SDL2_IMAGE_DLL_FOLDER}/libpng16-16.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/libtiff-5.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/libwebp-7.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/SDL2_image.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/zlib1.dll"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES 
      "${SDL2_IMAGE_DLL_FOLDER}/libjpeg-9.dll" 
      "${SDL2_IMAGE_DLL_FOLDER}/libpng16-16.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/libtiff-5.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/libwebp-7.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/SDL2_image.dll"
      "${SDL2_IMAGE_DLL_FOLDER}/zlib1.dll"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})

  install(FILES 
      "${SDL2_TTF_DLL_FOLDER}/libfreetype-6.dll"
      "${SDL2_TTF_DLL_FOLDER}/SDL2_ttf.dll"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES 
      "${SDL2_TTF_DLL_FOLDER}/libfreetype-6.dll"
      "${SDL2_TTF_DLL_FOLDER}/SDL2_ttf.dll"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})
      
  install(FILES 
      "${GLEW_SHARED_LIBRARIES}"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES 
      "${GLEW_SHARED_LIBRARIES}"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})

  install(FILES 
      "${EASY_PROFILER_FOLDER}/easy_profiler.dll"
      CONFIGURATIONS Debug
      DESTINATION ${MIKAN_DEBUG_INSTALL_PATH})
  install(FILES 
      "${EASY_PROFILER_FOLDER}/easy_profiler.dll"
      CONFIGURATIONS Release
      DESTINATION ${MIKAN_RELEASE_INSTALL_PATH})

  install(DIRECTORY ${ROOT_DIR}/resources
    CONFIGURATIONS Debug
    DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}
    FILES_MATCHING PATTERN "*.jpg" PATTERN "*.png" PATTERN "*.ttf" PATTERN "*.json" PATTERN "*.csv" PATTERN "*.lua" PATTERN "*.rml" PATTERN "*.rcss")
  install(DIRECTORY ${ROOT_DIR}/resources
    CONFIGURATIONS Release
    DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}
    FILES_MATCHING PATTERN "*.jpg" PATTERN "*.png" PATTERN "*.ttf" PATTERN "*.json" PATTERN "*.csv" PATTERN "*.lua" PATTERN "*.rml" PATTERN "*.rcss")
    
  install(TARGETS Mikan_CAPI
      CONFIGURATIONS Debug
      RUNTIME DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}/lib
      PUBLIC_HEADER DESTINATION ${MIKAN_DEBUG_INSTALL_PATH}/include)
  install(TARGETS Mikan_CAPI
      CONFIGURATIONS Release
      RUNTIME DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}/lib
      PUBLIC_HEADER DESTINATION ${MIKAN_RELEASE_INSTALL_PATH}/include)

ELSE() #Linux/Darwin
ENDIF()

MESSAGE(STATUS "Stepping into tests")
add_subdirectory(Tests)
