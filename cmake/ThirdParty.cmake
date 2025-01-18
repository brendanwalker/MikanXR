include(FindPackageHandleStandardArgs)
include(FetchContent)

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

if (POLICY CMP0122)
	cmake_policy(SET CMP0122 NEW)
endif()

if (POLICY CMP0146)
	cmake_policy(SET CMP0146 OLD)
endif()

# When not using MSVC, we recommend using system-wide libraries
# (installed via homebrew on Mac or apt-get in Linux/Ubuntu)
# In MSVC, the InitialSetup batch script downloads pre-build binary packages for OpenCV and SDL

# Required core libraries on various platforms
set(MIKAN_EXTRA_LIBS "")
if (WIN32) 
  list(APPEND MIKAN_EXTRA_LIBS opengl32 mfplat mf mfuuid shlwapi winmm wsock32 ws2_32)
elseif (APPLE)
  find_library(cocoa_library Cocoa)
  find_library(opengl_library OpenGL)
  find_library(corevideo_library CoreVideo)
  find_library(iokit_library IOKit)
  list(APPEND MIKAN_EXTRA_LIBS ${cocoa_library} ${opengl_library} ${corevideo_library} ${iokit_library})
  list(APPEND NNGUI_EXTRA_SOURCE darwin.mm)
elseif(CMAKE_SYSTEM MATCHES "Linux")
  list(APPEND MIKAN_EXTRA_LIBS GL Xxf86vm Xrandr Xinerama Xcursor Xi X11 pthread dl rt)
endif()

# Configuru
set (CONFIGURU_INCLUDE_DIR ${ROOT_DIR}/thirdparty/Configuru)

FetchContent_Declare(
    dylib
    GIT_REPOSITORY "https://github.com/martin-olivier/dylib"
    GIT_TAG        "v2.2.1"
)
FetchContent_MakeAvailable(dylib)

# OpenCV
# Override by adding "-DOpenCV_DIR=C:\path\to\opencv\build" to your cmake command
find_package(OpenCV REQUIRED)

# OpenVR
# Override by adding -DOPENVR_ROOT_DIR=... -DOPENVR_HEADERS_ROOT_DIR=...
find_package(OpenVR REQUIRED)

# SDL
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2TTF REQUIRED)

# GL Extension Wrangler (GLEW)
if (WIN32) 
  set (GLEW_INCLUDE_DIRS ${ROOT_DIR}/deps/glew-2.2.0/include)
  set (GLEW_LIBRARIES ${ROOT_DIR}/deps/glew-2.2.0/lib/Release/x64/glew32.lib)
  set (GLEW_SHARED_LIBRARIES ${ROOT_DIR}/deps/glew-2.2.0/bin/Release/x64/glew32.dll)
else()
  find_package(GLEW REQUIRED)
endif()

# Freetype
if (WIN32) 
  set (FREETYPE_INCLUDE_DIRS ${ROOT_DIR}/deps/freetype-windows-binaries-2.10.4/include)
  set (FREETYPE_LIBRARIES ${ROOT_DIR}/deps/freetype-windows-binaries-2.10.4/win64/freetype.lib)
  set (FREETYPE_SHARED_LIBRARY ${ROOT_DIR}/deps/freetype-windows-binaries-2.10.4/win64/freetype.dll)
else()
  #TODO
endif()

# RMLUI
set (RMLUI_DIR ${ROOT_DIR}/thirdparty/RmlUI)
set (RMLUI_INCLUDE_DIR ${RMLUI_DIR}/Include)

# Refureku
set (RFK_DIR ${ROOT_DIR}/deps/rfk)
set (RFK_INCLUDE_DIR ${RFK_DIR}/Include)
set (RFK_LIB_DIR ${RFK_DIR}/Lib)
set (RFK_BIN_DIR ${RFK_DIR}/Bin)
set (RFK_GENERATED_ROOT_DIR ${ROOT_DIR}/build/RfkGenerated)
if (WIN32) 
	set (RFK_LIBRARIES ${RFK_LIB_DIR}/Refureku.lib)
	set (RFK_GENERATOR_EXE ${RFK_BIN_DIR}/RefurekuGenerator.exe)
	list(APPEND RFK_SHARED_LIBRARIES
		 ${RFK_BIN_DIR}/Refureku.dll
	)	
endif()

# Lua
if (WIN32) 
  set (LUA_INCLUDE_DIRS ${ROOT_DIR}/thirdparty/lua/include)
  set (LUA_LIBRARIES ${ROOT_DIR}/thirdparty/lua/lua54.lib)
  set (LUA_SHARED_LIBRARIES ${ROOT_DIR}/thirdparty/lua/lua54.dll)
else()
  find_package(LUA REQUIRED)
endif()

# LuaBridge3
set (LUA_BRIDGE_INCLUDE_DIRS ${ROOT_DIR}/thirdparty/LuaBridge3/Source)

# fast-cpp-csv-parser
set (FastCSV_INCLUDE_DIRS ${ROOT_DIR}/thirdparty/fast-cpp-csv-parser)

# ImGUI
set(IMGUI_DIR ${ROOT_DIR}/thirdparty/imgui)
set(IMGUI_SOURCE "")
list(APPEND IMGUI_SOURCE
     ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
     ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
     ${IMGUI_DIR}/imgui.cpp
     ${IMGUI_DIR}/imgui_draw.cpp
     ${IMGUI_DIR}/imgui_tables.cpp
     ${IMGUI_DIR}/imgui_widgets.cpp
     ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

# ImNodes
set(IMNODES_DIR ${ROOT_DIR}/thirdparty/imnodes)
set(IMNODES_SOURCE "")
list(APPEND IMNODES_SOURCE
     ${IMNODES_DIR}/imnodes.cpp
)

# IXWebSocket
option(USE_ZLIB "Add ZLib support" FALSE)
option(IXWEBSOCKET_INSTALL "Install IXWebSocket" FALSE)
set (IXWEBSOCKET_DIR ${ROOT_DIR}/thirdparty/IXWebSocket/)
set (IXWEBSOCKET_INCLUDE_DIR ${IXWEBSOCKET_DIR})

# GStreamer
find_package(GStreamer REQUIRED COMPONENTS base)
find_package(GStreamerPluginsBase COMPONENTS app)
find_package(GStreamerPluginsBase COMPONENTS video)
find_package(GLIB2 REQUIRED)
find_package(GObject REQUIRED)
set (GSTREAMER_BIN_DIR ${GSTREAMER_ROOT}/bin)

# Nlohmann JSON
set (NLOHMANN_JSON_INCLUDE_DIR ${ROOT_DIR}/thirdparty/nlohmann_json/include)

# readerwriterqueue
set (LOCKFREEQUEUE_INCLUDE_DIR ${ROOT_DIR}/thirdparty/readerwriterqueue)

# stb
set (STB_INCLUDE_DIRS ${ROOT_DIR}/thirdparty/stb)

# tinyfiledialogs
set(TINYFILEDIALOGS_DIR ${ROOT_DIR}/thirdparty/tinyfiledialogs)
set(TINYFILEDIALOGS_SOURCE "${TINYFILEDIALOGS_DIR}/tinyfiledialogs.c")

# fast_obj loader
set (FAST_OBJ_LOADER_INCLUDE_DIRS ${ROOT_DIR}/thirdparty/fast_obj)

# ffmpeg
find_package(FFMPEG REQUIRED)

# easy_profiler
find_package(easy_profiler REQUIRED)

# Spout2
if (WIN32)
  set (SPOUT2_SDK_DIR ${ROOT_DIR}/deps/Spout2-2.007h/SPOUTSDK/SpoutLibrary/Binaries/x64)
  list (APPEND SPOUT2_INCLUDE_DIRS 
    ${ROOT_DIR}/deps/Spout2-2.007h/SPOUTSDK
    ${ROOT_DIR}/deps/Spout2-2.007h/SPOUTSDK/SpoutLibrary/Binaries/x64)
  set (SPOUT2_LIBRARIES ${SPOUT2_SDK_DIR}/SpoutLibrary.lib)
  set (SPOUT2_SHARED_LIBRARIES ${SPOUT2_SDK_DIR}/SpoutLibrary.dll)
endif()