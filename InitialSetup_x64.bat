@echo off
setlocal

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
7z e sdl2.zip -obuild/debug -y -r -spf
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
7z e sdl2-devel.zip -y -r -spf
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
7z e sdl2img.zip -obuild/debug -y -r -spf
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
7z e sdl2img-devel.zip -y -r -spf
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
7z e sdl2ttf.zip -obuild/debug -y -r -spf
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
7z e sdl2ttf-devel.zip -y -r -spf        
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
7z e glew-2.2.0-win32.zip -y -r -spf
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
7z e SPOUT.zip -y -r -spf
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping SPOUT.zip"
  goto failure
)

echo "Downloading FFMpeg-Shared"
curl -L https://github.com/GyanD/codexffmpeg/releases/download/4.4.1/ffmpeg-4.4.1-full_build-shared.7z --output ffmpeg-4.4.1-full_build-shared.7z
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading ffmpeg-4.4.1-full_build-shared.7z"
  goto failure
)
7z e ffmpeg-4.4.1-full_build-shared.7z -y -r -spf
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping ffmpeg-4.4.1-full_build-shared.7z"
  goto failure
)

echo "Downloading gstreamer"
curl -L https://gstreamer.freedesktop.org/data/pkg/windows/1.24.10/msvc/gstreamer-1.0-devel-msvc-x86_64-1.24.10.msi --output gstreamer-1.0-devel-msvc-x86_64-1.24.10.msi
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading gstreamer-1.0-devel-msvc-x86_64-1.24.10.msi"
  goto failure
)
msiexec /i gstreamer-1.0-devel-msvc-x86_64-1.24.10.msi /qb
IF %ERRORLEVEL% NEQ 0 (
  echo "Error installing gstreamer"
  goto failure
)

echo "Downloading easy_profiler..."
curl -L https://github.com/yse/easy_profiler/releases/download/v2.1.0/easy_profiler-v2.1.0-msvc15-win64.zip --output easy_profiler-v2.1.0-msvc15-win64.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error easy_profiler-v2.1.0-msvc15-win64.zip"
  goto failure
)
7z e easy_profiler-v2.1.0-msvc15-win64.zip -y -r -spf -oeasy_profiler
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping easy_profiler-v2.1.0-msvc15-win64.zip"
  goto failure
)

:: Download pre-compiled FreeType libraries
echo "Downloading FreeType Binaries..."
curl -L https://github.com/ubawurinna/freetype-windows-binaries/archive/refs/tags/v2.10.4.zip  --output freetype-windows-binaries-2.10.4.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error FreeType.zip"
  goto failure
)
7z e freetype-windows-binaries-2.10.4.zip -y -r -spf
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping FreeType.zip"
  goto failure
)

:: Download pre-compiled Refureku libraries
echo "Downloading Refureku..."
curl -L https://github.com/MikanXR/Refureku/releases/download/v2.2.1/rfk_v2.2.1_windows.zip --output rfk_v2.2.1_windows.zip
IF %ERRORLEVEL% NEQ 0 (
  echo "Error downloading rfk_v2.2.1_windows.zip"
  goto failure
)
7z e rfk_v2.2.1_windows.zip -y -r -spf -orfk
IF %ERRORLEVEL% NEQ 0 (
  echo "Error unzipping rfk_v2.2.1_windows.zip"
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