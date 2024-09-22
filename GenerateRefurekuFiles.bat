@echo off

echo "Rebuilding Refureku Reflection Code Generator..."
set RFK_GENERATOR_EXE=%~dp0/deps/rfk/Bin/RefurekuGenerator.exe

pushd src

%RFK_GENERATOR_EXE% RefurekuSettings.toml
IF %ERRORLEVEL% NEQ 0 (
  echo "Error generating Refureku Reflection Code files"
  goto failure
)

popd
EXIT \B 0

:failure
pause
EXIT \B 1