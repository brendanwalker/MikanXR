# Repository Layout

Annotated map of the MikanXR directory hierarchy: where source, generated code, dependencies, and build output live. See [build.md](./build.md) for how these pieces are compiled and [commands.md](./commands.md) for the commands that drive them. The architectural view of the same code is in [modules.md](./modules.md).

---

## Root

```
MikanXR/
├── CMakeLists.txt                      # top-level CMake: includes cmake/*.cmake, adds src/, bindings/, thirdparty/
├── InitialSetup_x64.bat                # first-time setup: wipes build/ and deps/, downloads prebuilt deps into deps/
├── GenerateProjectFiles_X64_VS2022.bat # configures build/ with the "Visual Studio 17 2022" generator
├── CLAUDE.md / README.md / CONTRIBUTING.md / LICENSE
├── .github/workflows/build-and-test.yml # CI: Linux format-check job + Windows Ninja build/test job
├── .gitmodules                         # submodule list (all under thirdparty/)
├── cmake/                              # CMake modules: Environment, Version, ThirdParty, Installer, ClangFormat, Find*.cmake
├── src/                                # all first-party C++ source (the only tree clang-format touches)
├── bindings/                           # generated client bindings for C# and TypeScript
├── thirdparty/                         # git submodules + a few vendored libs, built from source or used header-only
├── deps/                               # prebuilt binary dependencies downloaded by InitialSetup_x64.bat (not in git)
├── build/                              # CMake binary dir (build/Mikan.sln, object files, built exes; not in git)
├── dist/                               # install prefix (dist/Win64) written by the INSTALL target; version.txt
├── resources/                          # runtime assets: calibration patterns, config, dnn models, fonts, icons, gui_styles, localization, lua-definitions
├── templates/                          # installer_win64.iss.in (Inno Setup script template, filled in by cmake/Installer.cmake)
├── tools/                              # checked-in helper tools: 7zip/7za.exe (used by InitialSetup), Spout2
└── docs/                               # documentation, including this reference set
```

- `deps/` holds prebuilt downloads (SDL2, OpenCV, GLEW, Spout2, easy_profiler, Refureku, libharu, CEF, nuget.exe). It is recreated from scratch by `InitialSetup_x64.bat`; never edit it by hand. GStreamer is the exception: its MSIs install system-wide rather than into `deps/`.
- `thirdparty/` holds git submodules (see `.gitmodules`: `openvr`, `glm`, `Configuru`, `stb`, `fast-cpp-csv-parser`, `LuaBridge3`, `imgui`, `imnodes`, `fast_obj`, `IXWebSocket`, `readerwriterqueue`, `nlohmann_json`) plus vendored non-submodule dirs (`lua` prebuilt binaries, `lrdb`, `tinyfiledialogs`). `thirdparty/CMakeLists.txt` builds `fast_obj_lib`, `ixwebsocket`, and CEF's `libcef_dll_wrapper`; the rest are consumed header-only or as source lists from `cmake/ThirdParty.cmake`.
- `build/` layout depends on the generator: the VS generator puts executables in per-target per-config folders (e.g. `build\src\Editor\Release\Mikan.exe`), while CI flattens everything to `build\bin` via `CMAKE_RUNTIME_OUTPUT_DIRECTORY`. Refureku reflection codegen output lands in `build/RfkGenerated/<Library>`.

---

## src/

`src/CMakeLists.txt` steps into `Libraries`, `Plugins`, `Programs`, `Editor` in that order.

### src/Editor

The editor/server application itself. All editor code compiles once into the `MikanEditor` OBJECT library; the `Mikan` (GUI, `AppCore/EntryPoint.cpp`) and `MikanCmd` (console, `AppCore/CmdEntryPoint.cpp`) executables both link it.

- `AppCore` — entry points, top-level app object, `Version.h` (source of the project version string)
- `AppStages` — top-level app state machine (menus, calibration flows, compositor stage)
- `Asset`, `Project`, `Config` — persisted project/scene/asset state (Configuru-backed config files)
- `Calibration` — camera/mixed-reality calibration algorithms (OpenCV-based), see [calibration.md](./calibration.md)
- `ECS` — entity/component scene representation (see [objects.md](./objects.md))
- `Delegates`, `Events` — callback/event plumbing
- `Interprocess` — websocket/http message servers, shared-texture reader
- `Server` — `MikanServer` and per-domain RPC request handlers (see [wire-protocol.md](./wire-protocol.md))
- `Localization`, `Math`, `OpenCV` — support code
- `NodeEditors`, `Scripting` — node graph editors and Lua scripting (see [scripting.md](./scripting.md))
- `Renderer` — editor viewport and compositor rendering (see [compositor.md](./compositor.md))

### src/Libraries

Each is a shared DLL target of the same name:

- `MikanClientAPI` — public request/response/event structs of the client API; Refureku-annotated wire contract
- `MikanClientCore` — low-level client core: C API, connection/transport types
- `MikanCoreApp` — module/device interfaces shared between the editor and plugins (`IMikanModule`, video/VR device interfaces)
- `MikanDMX` — DMX lighting control (`IDMXManager`, E1.31)
- `MikanGUI` — ImGui wrapper layer (`MkGui*` scoped helpers, styles)
- `MikanMath` — math utilities and transforms (GLM-based)
- `MikanRenderer` — OpenGL rendering abstraction (`IMk*` interfaces: shaders, meshes, framebuffers, scenes)
- `MikanSerialization` — reflection-driven JSON/binary (de)serialization used by the wire protocol and configs
- `MikanSharedTexture` — shared texture memory for out-of-band video frame delivery to clients
- `MikanUtility` — general utilities (strings, paths, JSON, logging helpers)
- `MikanWindow` — SDL window / GL context / font management layer

`ARKitReceiver` and `MikanARKitReceiver` directories exist but are empty and not referenced by any `CMakeLists.txt`.

### src/Plugins

Optional backends, each built as a separate `SHARED` DLL and copied next to the executables post-build:

- `MikanWMFVideo` — USB webcam video source via Windows Media Foundation
- `MikanGStreamerVideo` — GStreamer-based network video source (built only when `MIKAN_WITH_GSTREAMER=ON`)
- `MikanARKitVideo` — iOS ARKit network video source (RTP video + pose, CUDA-GL interop); also gated behind `MIKAN_WITH_GSTREAMER`
- `MikanSteamVR` — SteamVR/OpenVR tracking device integration

### src/Programs

- `ClientCodeGen` — the `MikanClientCodeGen` executable; walks Refureku reflection metadata and emits C#/TypeScript bindings
- `Tests/UnitTests` — `unit_test_suite_cpp` C++ unit test executable
- `Tests/MikanClientTestCPP` — C++ client API test app
- `Tests/MikanClientTestCSharp` — C# client test app (Visual Studio generator only)

---

## bindings/

Client-side binding packages consumed by external applications. Generated portions are emitted by `MikanClientCodeGen`; do not hand-edit them, regenerate instead (see [wire-protocol.md](./wire-protocol.md)).

- `bindings/csharp` — C# client library. Hand-written core (`MikanAPI.cs`, `MikanRequestManager.cs`, ...) plus `Generated/` (generated, driven by `CSharpCodeGenConfig.json`). Only builds under a Visual Studio generator.
- `bindings/typescript` — TypeScript client package (`package.json`, `tsconfig.json`). Hand-written core at the root and `Serialization/`; `types/` is generated (driven by `TypeScriptCodeGenConfig.json`); `dist/` is compiled JS output.

---

## Build outputs and generated trees (never hand-edit)

- `build/` — everything under it, including `build/RfkGenerated` (Refureku reflection output)
- `deps/` — recreated by `InitialSetup_x64.bat`
- `dist/` — populated by the `INSTALL` target and the installer
- `bindings/csharp/Generated`, `bindings/typescript/types`, `bindings/typescript/dist`
