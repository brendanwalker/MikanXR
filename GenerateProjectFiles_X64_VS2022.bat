@echo on

::pushd %~dp0\bindings
::call RebuildCSharpBindings.bat
::popd 
::IF %ERRORLEVEL% NEQ 0 (
::  echo "Error generating C# bindings"
::  goto failure
::)

IF NOT EXIST build mkdir build
pushd build

echo "Rebuilding Mikan x64 Project files..."
set DEPS_ROOT_PATH=%~dp0/deps
set THIRDPARTY_ROOT_PATH=%~dp0\thirdparty

cmake .. -G "Visual Studio 17 2022" -A x64 ^
-DOpenCV_DIR=%DEPS_ROOT_PATH%\opencv_cuda ^
-DOPENVR_ROOT_DIR=%THIRDPARTY_ROOT_PATH%\openvr ^
-DOPENVR_HEADERS_ROOT_DIR=%THIRDPARTY_ROOT_PATH%openvr\include ^
-DSDL2_LIBRARY="%DEPS_ROOT_PATH%\SDL2-2.0.10\lib\x64\sdl2.lib" ^
-DSDL2_INCLUDE_DIR="%DEPS_ROOT_PATH%\sdl2-2.0.10\include" ^
-DSDL2TTF_LIBRARY="%DEPS_ROOT_PATH%\SDL2_ttf-2.0.15\lib\x64\sdl2_ttf.lib" ^
-DSDL2TTF_INCLUDE_DIR="%DEPS_ROOT_PATH%\SDL2_ttf-2.0.15\include" ^
-DSDL2_IMAGE_LIBRARY="%DEPS_ROOT_PATH%\SDL2_image-2.0.5\lib\x64\sdl2_image.lib" ^
-DSDL2_IMAGE_INCLUDE_DIR="%DEPS_ROOT_PATH%\SDL2_image-2.0.5\include" ^
-DFFMPEG_ROOT="%DEPS_ROOT_PATH%\ffmpeg-4.4.1-full_build-shared" ^
-DCMAKE_PREFIX_PATH="%DEPS_ROOT_PATH%\easy_profiler\lib\cmake\easy_profiler" ^
-DSWIG_EXECUTABLE=%DEPS_ROOT_PATH%/swigwin-4.1.1/swig.exe ^
-DCMAKE_UNITY_BUILD=ON

IF %ERRORLEVEL% NEQ 0 (
  echo "Error generating Mikan 64-bit project files"
  goto failure
)

popd
EXIT \B 0

:failure
pause
EXIT \B 1