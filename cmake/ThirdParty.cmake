include(FindPackageHandleStandardArgs)

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

# RMLUI
if (WIN32) 
  set (RMLUI_INCLUDE_DIR ${ROOT_DIR}/deps/RML/RmlUi-4.4/Include)
  set (RMLUI_BINARIES_DIR ${ROOT_DIR}/deps/RML/RmlUi-4.4/Build/Debug)
  set (FREETYPE_SHARED_LIBRARY ${ROOT_DIR}/deps/freetype-windows-binaries-2.10.4/win64/freetype.dll)
  list (APPEND RMLUI_LIBRARIES 
    ${RMLUI_BINARIES_DIR}/RmlCore.lib
    ${RMLUI_BINARIES_DIR}/RmlDebugger.lib
    ${RMLUI_BINARIES_DIR}/RmlLua.lib)
else()
  #TODO
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

# Boost interprocess
set (BOOST_INTERPROCESS_INCLUDE_DIRS ${ROOT_DIR}/deps/boost_1_78_0)

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

# SWIG
find_package(SWIG 4.1.1 COMPONENTS csharp)