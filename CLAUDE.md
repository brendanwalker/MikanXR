# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

MikanXR is a Windows tool for mixed-reality camera calibration and video compositing. It's a C++ editor/server application (`Mikan.exe` / `MikanCmd.exe`) that exposes a websocket RPC API consumed by client applications (game engines, etc.) via generated C++/C#/TypeScript bindings.

## Build

Windows only (Win10/11), MSVC/Visual Studio 2022, CMake >= 3.15.

First-time setup (downloads prebuilt deps into `deps/`, ~large download):
```
InitialSetup_x64.bat
```

Generate VS2022 solution (`build/Mikan.sln`):
```
GenerateProjectFiles_X64_VS2022.bat
```

CI instead configures with the Ninja generator (for sccache) and `-DMIKAN_WITH_GSTREAMER=OFF -DCMAKE_UNITY_BUILD=ON` — see `.github/workflows/*.yml` for the exact invocation if reproducing a CI build locally.

Key CMake targets:
- `Mikan` — the editor GUI app
- `MikanCmd` — headless/console entry point, also used to run the interprocess/API test suite
- `unit_test_suite_cpp` — C++ unit tests (Programs/Tests/UnitTests)
- `FormatFix` / `FormatCheck` — clang-format wrapper targets (see Formatting below)

The project is split into `src/` (add_subdirectory: `Libraries`, `Plugins`, `Programs`, `Editor`), `bindings/` (csharp, typescript — generated client bindings), and `thirdparty/` (git submodules).

## Tests

```
build\bin\MikanCmd.exe -runTests       # interprocess/client-API integration tests (see CmdApp.cpp)
build\bin\unit_test_suite_cpp.exe      # C++ unit tests
```

Both must pass in CI (`.github/workflows/*.yml`, `build` job). `MikanCmd.exe -runTests` writes results to `MikanCmd.log` in addition to stdout — check that file if a run fails with no console output (e.g. a crash).

## Formatting

Only `src/` is formatted with clang-format (`thirdparty/` is left untouched). Use **clang-format 19.1.x** to match CI — a different major version reformats differently and CI will reject it. VS2022 bundles a compatible copy at `VC\Tools\Llvm\bin\clang-format.exe`; otherwise `pip install clang-format==19.1.5`.

```
cmake --build build --target FormatFix      # reformat in place
cmake --build build --target FormatCheck    # check only (what CI runs)
```

CI also runs a standalone `format-check` job on Linux that needs no build tree — it just runs `cmake -P cmake/RunClangFormat.cmake -- --check`.

The repo-wide reformat commit is excluded from `git blame` via `.git-blame-ignore-revs`; run `git config blame.ignoreRevsFile .git-blame-ignore-revs` once locally to benefit from that.

## Architecture

### Process model

`Mikan`/`MikanCmd` is a single editor/server process. It hosts a websocket server (`src/Editor/Interprocess/WebsocketInterprocessMessageServer`) that speaks the Mikan RPC protocol. External client applications connect over that websocket and drive the editor (query/set scene state, subscribe to video frames, etc.) using the generated client API rather than linking directly into the editor process. Video frames are handed to clients out-of-band via shared texture memory (`SharedTextureReader`, `MikanSharedTexture` library), not over the websocket.

### `src/Editor/Server` — RPC surface

`MikanServer` owns the websocket server and routes incoming requests to a set of per-domain `IServerRequestHandler` implementations (`CameraRequestHandler`, `StageRequestHandler`, `StencilRequestHandler`, `VideoSourceRequestHandler`, `MarkerRequestHandler`, `LightRequestHandler`, `PropertyRequestHandler`, `ScriptRequestHandler`, `ShapeRequestHandler`, `TextureSourceRequestHandler`, `FunctionRequestHandler`, ...). Adding a new remote-controllable capability generally means adding a handler here plus the corresponding request/response types in `MikanClientAPI` (see below), not modifying `MikanServer` itself. `RemoteControlManager` / `IRemoteControllable` is the interface objects implement to become remotely queryable/settable (backs the client "properties" system).

### `src/Libraries/MikanClientAPI` and `MikanClientCore` — the wire contract

These two libraries define the public request/response/event structs and enums that cross the process boundary. They are annotated for **Refureku** (C++ static reflection, invoked at build time) so that `src/Programs/ClientCodeGen` can walk the reflected struct/enum metadata and emit equivalent C# and TypeScript types into `bindings/csharp/Generated` and `bindings/typescript`. **Changing a struct in `MikanClientAPI`/`MikanClientCore` changes the wire protocol and the generated bindings — do not hand-edit files under `bindings/*/Generated` or `bindings/typescript/types`, regenerate them.** There's a schema-guard integration test (`App::runTests`, exercised via `MikanCmd -runTests`) that checks the values-struct ↔ descriptor ↔ `getPropertyValue` contract stays consistent — keep new client-facing properties wired through all three.

`bindings/csharp` only builds under the Visual Studio generator (C# isn't supported under Ninja), so it's skipped in CI and when configuring with Ninja.

### `src/Libraries/MikanSerialization`

Wire/config (de)serialization layer. `Serialization::String` is a `const char*`-only type for cross-CRT safety across the DLL/EXE boundary — don't widen it to `std::string` at that interface, and watch for the `const char*`→`bool` implicit-conversion trap when writing `to_binary` overloads (easy to accidentally bind to the wrong overload).

### `src/Editor` layout

- `AppCore` — app/process entry points (`EntryPoint.cpp` for `Mikan`, `CmdEntryPoint.cpp`/`CmdApp` for `MikanCmd`) and top-level app state machine (`AppStages`)
- `ECS` — entity/component scene representation used by the editor (stages, cameras, stencils, anchors, lights, etc.)
- `Calibration` — camera/mixed-reality calibration algorithms (built on `OpenCV`)
- `NodeEditors` / `Scripting` — visual scripting / node graph editor, backed by Lua (`LuaBridge3`)
- `Renderer` — editor viewport rendering (built on `MikanRenderer`)
- `Interprocess`, `Server` — see above
- `Project`, `Config`, `Asset` — persisted project/scene/asset state (Configuru-backed config files)

### `src/Plugins`

Optional video-source backends compiled as separate plugin modules: `MikanWMFVideo` (Windows Media Foundation), `MikanGStreamerVideo` (off by default in CI via `MIKAN_WITH_GSTREAMER=OFF`), `MikanSteamVR` (OpenVR tracking integration).

### Unity build gotcha

The default build uses `CMAKE_UNITY_BUILD=ON`. This means translation units from unrelated files get concatenated, so a transitively-included `windows.h` can rewrite a same-named method (e.g. `GetObject` → `GetObjectA`) and produce a confusing `LNK2019` far from the actual conflict. Avoid method names that collide with Win32 macros (`GetObject`, `SendMessage`, `CreateWindow`, etc.).
