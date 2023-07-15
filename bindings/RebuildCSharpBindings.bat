@echo off
if exist csharp\ (
	del /Q csharp\*.cs > nul
	del /Q csharp\*.c > nul
) else (
	mkdir csharp
)
..\deps\swigwin-4.1.1\swig.exe -csharp -namespace Mikan -outdir csharp -dllimport MikanClient_swig_csharp -I../src/Client -o csharp/MikanClientCSHARP_wrap.c MikanClient.i