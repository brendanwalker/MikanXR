@echo on

IF NOT EXIST build mkdir build
pushd build

echo "Rebuilding Mikan x64 Project files..."
set DEPS_ROOT_PATH=%~dp0deps
set THIRDPARTY_ROOT_PATH=%~dp0thirdparty
set DIST_ROOT_PATH=%~dp0dist/Win64

cmake .. -G "Visual Studio 17 2022" -A x64 ^
-DCMAKE_INSTALL_PREFIX=%DIST_ROOT_PATH% ^
-DCEF_ROOT="%DEPS_ROOT_PATH%/cef/cef_binary_145.0.27+g4ddda2e+chromium-145.0.7632.117_windows64" ^
-DOpenCV_DIR=%DEPS_ROOT_PATH%\opencv\build ^
-DOPENVR_ROOT_DIR=%THIRDPARTY_ROOT_PATH%\openvr ^
-DOPENVR_HEADERS_ROOT_DIR=%THIRDPARTY_ROOT_PATH%openvr\include ^
-DSDL2_LIBRARY="%DEPS_ROOT_PATH%\SDL2-2.30.10\lib\x64\sdl2.lib" ^
-DSDL2_INCLUDE_DIR="%DEPS_ROOT_PATH%\SDL2-2.30.10\include" ^
-DSDL2TTF_LIBRARY="%DEPS_ROOT_PATH%\SDL2_ttf-2.24.0\lib\x64\sdl2_ttf.lib" ^
-DSDL2TTF_INCLUDE_DIR="%DEPS_ROOT_PATH%\SDL2_ttf-2.24.0\include" ^
-DSDL2_IMAGE_LIBRARY="%DEPS_ROOT_PATH%\SDL2_image-2.8.8\lib\x64\sdl2_image.lib" ^
-DSDL2_IMAGE_INCLUDE_DIR="%DEPS_ROOT_PATH%\SDL2_image-2.8.8\include" ^
-DCMAKE_PREFIX_PATH="%DEPS_ROOT_PATH%\easy_profiler\lib\cmake\easy_profiler" ^
-DNUGET_PATH="%DEPS_ROOT_PATH%" ^
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