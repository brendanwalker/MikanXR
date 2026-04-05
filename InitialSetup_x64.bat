@echo off
setlocal

set UNZIP_EXE=%~dp0/tools/7zip/7za.exe

::Clean up the old build folder
IF EXIST build (
del /f /s /q build > nul
rmdir /s /q build
)

::Clean up the old deps folder
IF EXIST deps (
del /f /s /q deps > nul
rmdir /s /q deps
)

:: Fetch dependencies in the "deps" folders
mkdir deps
pushd deps

:: Download and unzip the prebuilt libs
echo "Downloading SDL2..."
curl https://www.libsdl.org/release/SDL2-2.0.10-win32-x64.zip --output sdl2.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading SDL2-2.0.10-win32-x64.zip"
  goto failure
)
%UNZIP_EXE% e sdl2.zip -obuild/debug -y -r -spf
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping SDL2-2.0.10-win32-x64.zip"
  goto failure
)

echo "Downloading SDL2-devel..."
curl https://www.libsdl.org/release/SDL2-devel-2.0.10-VC.zip --output sdl2-devel.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading SDL2-devel-2.0.10-VC.zip"
  goto failure
)
%UNZIP_EXE% e sdl2-devel.zip -y -r -spf
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping SDL2-devel-2.0.10-VC.zip"
  goto failure
)

echo "Downloading SDL2-image..."
curl https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5-win32-x64.zip --output sdl2img.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading SDL2_image-2.0.5-win32-x64.zip"
  goto failure
)
%UNZIP_EXE% e sdl2img.zip -obuild/debug -y -r -spf
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping SDL2_image-2.0.5-win32-x64.zip"
  goto failure
)

echo "Downloading SDL2-image..."
curl https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.5-VC.zip --output sdl2img-devel.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading SDL2_image-devel-2.0.5-VC.zip"
  goto failure
)
%UNZIP_EXE% e sdl2img-devel.zip -y -r -spf
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping SDL2_image-devel-2.0.5-VC.zip"
  goto failure
)

echo "Downloading SDL2-ttf..."
curl https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.15-win32-x64.zip --output sdl2ttf.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading SDL2_ttf-2.0.15-win32-x64.zip"
  goto failure
)
%UNZIP_EXE% e sdl2ttf.zip -obuild/debug -y -r -spf
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping SDL2_ttf-2.0.15-win32-x64.zip"
  goto failure
)

echo "Downloading SDL2-ttf-devel..."
curl https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.15-VC.zip --output sdl2ttf-devel.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading SDL2_ttf-devel-2.0.15-VC.zip"
  goto failure
)
%UNZIP_EXE% e sdl2ttf-devel.zip -y -r -spf        
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping SDL2_ttf-devel-2.0.15-VC.zip"
  goto failure
)

echo "Downloading OpenCV..."
curl -L https://github.com/opencv/opencv/releases/download/4.10.0/opencv-4.10.0-windows.exe > opencv-4.10.0-windows.exe
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading opencv-4.10.0-windows.exe"
  goto failure
)
opencv-4.10.0-windows.exe -o"." -y
IF %ERRORLEVEL% NEQ 0 (
  echo "Error running self extracting zip opencv-4.10.0-windows.exe"
  goto failure
)

echo "Downloading glew..."
curl -L https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip --output glew-2.2.0-win32.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading glew-2.2.0-win32.zip"
  goto failure
)
%UNZIP_EXE% e glew-2.2.0-win32.zip -y -r -spf
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping glew-2.2.0-win32.zip"
  goto failure
)

echo "Downloading Spout2"
curl -L https://github.com/leadedge/Spout2/archive/refs/tags/2.007h.zip --output SPOUT.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading SPOUT.zip"
  goto failure
)
%UNZIP_EXE% e SPOUT.zip -y -r -spf
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping SPOUT.zip"
  goto failure
)

echo "Downloading gstreamer-runtime installer"
curl -L https://gstreamer.freedesktop.org/data/pkg/windows/1.26.10/mingw/gstreamer-1.0-mingw-x86_64-1.26.10.msi --output gstreamer-1.0-mingw-x86_64-1.26.10.msi
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading gstreamer-1.0-mingw-x86_64-1.26.10.msi"
  goto failure
)
msiexec /i gstreamer-1.0-mingw-x86_64-1.26.10.msi /qb
IF %ERRORLEVEL% NEQ 0 (
  echo "Error installing gstreamer-runtime installer"
  goto failure
)

echo "Downloading gstreamer-devel installer"

curl -L https://gstreamer.freedesktop.org/data/pkg/windows/1.26.10/mingw/gstreamer-1.0-devel-mingw-x86_64-1.26.10.msi --output gstreamer-1.0-devel-mingw-x86_64-1.26.10.msi
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading gstreamer-1.0-devel-mingw-x86_64-1.26.10.msi"
  goto failure
)
msiexec /i gstreamer-1.0-devel-mingw-x86_64-1.26.10.msi /qb
IF %ERRORLEVEL% NEQ 0 (
  echo "Error installing gstreamer-devel installer"
  goto failure
)

echo "Downloading easy_profiler..."
curl -L https://github.com/yse/easy_profiler/releases/download/v2.1.0/easy_profiler-v2.1.0-msvc15-win64.zip --output easy_profiler-v2.1.0-msvc15-win64.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error easy_profiler-v2.1.0-msvc15-win64.zip"
  goto failure
)
%UNZIP_EXE% e easy_profiler-v2.1.0-msvc15-win64.zip -y -r -spf -oeasy_profiler
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping easy_profiler-v2.1.0-msvc15-win64.zip"
  goto failure
)

:: Download pre-compiled Refureku libraries
echo "Downloading Refureku..."
curl -L https://github.com/MikanXR/Refureku/releases/download/v2.2.1/rfk_v2.2.1_windows.zip --output rfk_v2.2.1_windows.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading rfk_v2.2.1_windows.zip"
  goto failure
)
%UNZIP_EXE% e rfk_v2.2.1_windows.zip -y -r -spf -orfk
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping rfk_v2.2.1_windows.zip"
  goto failure
)

:: Download pre-compiled libharu library (PDF generator)
echo "Downloading libharu..."
curl -L https://github.com/MikanXR/libharu/releases/download/2.4.5/libharu-2.4.5-static.zip --output libharu-2.4.5-static.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading libharu-2.4.5-static.zip"
  goto failure
)
%UNZIP_EXE% e libharu-2.4.5-static.zip -y -r -spf -olibharu-2.4.5-static
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping libharu-2.4.5-static.zip"
  goto failure
)

echo "Downloading CEF (Chromium Embedded Framework)..."
curl -L https://cef-builds.spotifycdn.com/cef_binary_145.0.27+g4ddda2e+chromium-145.0.7632.117_windows64.tar.bz2 --output cef_binary_windows64.tar.bz2
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading cef_binary_windows64.tar.bz2"
  goto failure
)
%UNZIP_EXE% x cef_binary_windows64.tar.bz2 -so | %UNZIP_EXE% x -aoa -si -ttar -ocef
IF %ERRORLEVEL% NEQ 0 (
  echo "Error extracting cef_binary_windows64.tar.bz2"
  goto failure
)

:: NuGet tool used to fetch c# packages
echo "Downloading nuget..."
curl -L https://dist.nuget.org/win-x86-commandline/latest/nuget.exe --output nuget.exe

:: Exit back out of the deps folder
popd

EXIT /B 0

:failure
pause
EXIT /B 1