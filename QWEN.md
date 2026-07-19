# QWEN.md — MikanXR

## What this is

MikanXR is a Windows C++20 application for **mixed-reality camera calibration and video compositing**. It runs as an editor/server process (`Mikan.exe` GUI, `MikanCmd.exe` headless) that exposes a **websocket RPC API**. Client applications (e.g. game engines) connect over that websocket and drive the editor — querying/setting scene state, subscribing to video frames — using generated C# or TypeScript client bindings. Video frames travel out-of-band via shared texture memory (`MikanSharedTexture`), not over the websocket.

## Platform

- **Windows 10/11 only** (MSVC / Visual Studio 2022, CMake ≥ 3.15)
- C++20, unity build by default (`CMAKE_UNITY_BUILD=ON`)

## Build

### First-time setup

```bat
InitialSetup_x64.bat          # downloads prebuilt deps into deps/ (~large download)
GenerateProjectFiles_X64_VS2022.bat   # generates build/Mikan.sln
```

### Key CMake targets

| Target | Description |
|---|---|
| `Mikan` | Editor GUI application |
| `MikanCmd` | Headless/console entry point + API test runner |
| `unit_test_suite_cpp` | C++ unit tests |
| `FormatFix` / `FormatCheck` | clang-format fix / check |

### Tests

```bat
build\bin\MikanCmd.exe -runTests          # interprocess / client-API integration tests → MikanCmd.log
build\bin\unit_test_suite_cpp.exe         # C++ unit tests
```

## Formatting

- Only `src/` is formatted. Use **clang-format 19.1.x** (bundled with VS2022).
```bat
cmake --build build --target FormatFix    # reformat
cmake --build build --target FormatCheck  # verify (what CI runs)
```

## Project structure

```
src/
  Editor/          App entry points, ECS scene, calibration, rendering, server/RPC, scripting, node editors
  Libraries/       Shared libs: MikanClientAPI, MikanClientCore, MikanSerialization, MikanRenderer, MikanSharedTexture, MikanGUI, MikanWindow, MikanCoreApp, MikanDMX, MikanMath, MikanUtility
  Plugins/         Optional video-source backends (WMF, GStreamer, SteamVR)
  Programs/        ClientCodeGen (binding codegen from Refureku reflection), Tests
bindings/
  csharp/          Generated C# client bindings (VS generator only)
  typescript/      Generated TypeScript client bindings
thirdparty/        Git submodules
cmake/             CMake modules: Environment, Version, ThirdParty, Installer, ClangFormat
```

## Architecture highlights

### RPC surface (`src/Editor/Server`)

`MikanServer` owns the websocket server and routes requests to per-domain `IServerRequestHandler` implementations (`CameraRequestHandler`, `StageRequestHandler`, `ScriptRequestHandler`, etc.). New remote capabilities = new handler + request/response types in `MikanClientAPI`.

### Wire contract (`MikanClientAPI` / `MikanClientCore`)

Request/response/event structs annotated with **Refureku** static reflection. `ClientCodeGen` walks these annotations and emits C#/TypeScript types into `bindings/*/Generated`. **Never hand-edit generated files** — regenerate instead.

### Serialization (`MikanSerialization`)

`Serialization::String` is `const char*`-only for cross-CRT safety. Watch for `const char* → bool` implicit-conversion traps in `to_binary` overloads.

### Unity build gotcha

Unrelated TUs are concatenated. A transitively-included `windows.h` can rewrite same-named methods (e.g. `GetObject → GetObjectA`) causing confusing `LNK2019`. Avoid Win32 macro-colliding names.

## CI

GitHub Actions (`.github/workflows/build-and-test.yml`) builds with Ninja generator, sccache, `-DMIKAN_WITH_GSTREAMER=OFF -DCMAKE_UNITY_BUILD=ON`, then runs `FormatCheck`, both test suites, and a standalone format-check job on Linux.
