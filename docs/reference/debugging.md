# Debugging

Logging, test suites, and diagnostic surfaces for the editor. Build invocations and CI details are in [build.md](./build.md), the executable inventory in [commands.md](./commands.md), and Lua-specific debugging in [scripting.md](./scripting.md).

---

## Logging

The logger lives in `MikanCoreApp` (`src/Libraries/MikanCoreApp/Public/Logger.h`). Levels are `LogSeverityLevel` trace/debug/info/warning/error/fatal. Log with the `MIKAN_LOG_<LEVEL>("Scope::function") << ...` stream macros; use the `MIKAN_MT_LOG_*` variants only off the main thread (they take a mutex). Each process calls `log_init(LoggerSettings)` once; settings choose the minimum level, an optional log file, an optional callback, and whether a Win32 GUI process allocates a console (`enable_console`).

Every line goes to stdout (stderr for error and above) and, if configured, to the log file. Log files are opened with a relative path, so they land in the process working directory:

- `Mikan.exe`: `MikanXR.log`, minimum level debug, no console (`App::startup`); the same lines feed the editor's log panel through `AutomationLogBuffer::logCallback`.
- `MikanCmd.exe`: `MikanCmd.log`, minimum level debug (`CmdApp::exec`).

Both executables parse `-flag` and `-key=value` command-line arguments (`hasCommandLineFlag` / `getCommandLineStringArg`).

---

## Test suites

Two suites, both required by CI (see [build.md](./build.md)):

**`build\bin\MikanCmd.exe -runTests`** runs the headless editor tests. `CmdApp::exec` (`src/Editor/AppCore/CmdApp.cpp`) builds the Refureku type registry, then `runTests()` runs `run_all_editor_unit_tests()`:

- `run_tracker_pose_calibrator_unit_tests`: `src/Editor/Calibration/Test/TrackerPoseCalibratorTests.cpp`
- `run_client_api_property_schema_tests`: `src/Editor/Server/Test/ClientApiPropertySchemaTests.cpp` (the schema guard that keeps client-API values structs, property descriptors, and `getPropertyValue` consistent; see [wire-protocol.md](./wire-protocol.md))
- `run_depth_mesh_generator_unit_tests`: `src/Editor/Calibration/Test/DepthMeshGeneratorTests.cpp` (the depth shift solver including its negative-z pole case, discontinuity culling, and OBJ winding; see [depth-proxy-mesh.md](./depth-proxy-mesh.md))
- `run_dmx_universe_rle_tests`: `src/Editor/Server/Test/DMXUniverseRLETests.cpp`
- `run_light_environment_persistence_tests`: `src/Editor/Server/Test/LightEnvironmentPersistenceTests.cpp`
- `run_localization_unit_tests`: `src/Editor/Localization/Test/LocalizationTests.cpp` (key parity against English, printf specifier parity, window-title uniqueness, and glyph coverage against the baked font ranges; see [standards.md](./standards.md))

Exit code is nonzero on any failure. Results also go to `MikanCmd.log` next to the working directory.

The localization module reads `resources/localization` relative to the working directory, so run `MikanCmd.exe` from the repo root (as the commands in [commands.md](./commands.md) do) or the tables will not be found.

`Mikan.exe` no longer allocates a console window: its log is the editor's Log panel (View menu), backed by the same `AutomationLogBuffer` ring the `log tail` automation command reads. `MikanXR.log` still receives every line. A callback passed to `log_init` is an additional sink rather than a replacement, so the standard streams and the log file stay connected alongside it.

**`build\bin\unit_test_suite_cpp.exe`** runs the C++ unit tests, entry point `src/Programs/Tests/UnitTests/unit_test_suite.cpp`. Modules: math utility, GLM math, serialization, mikan API, spherical harmonic lighting, and ONNX session. The `unit_test.h` macros print a `PASSED`/`FAILED` line per test and per module. The spherical harmonic module is the round-trip guard on the lighting solve described in [scene-lighting.md](./scene-lighting.md): it validates the fit against synthetic data with known ground truth and asserts the l=2 ridge actually shrinks that band. Neither ML module touches a checkpoint or requires a GPU, which is what lets both suites run unchanged on a CI runner: the ONNX module probes for DirectML and reports the result without failing when it is absent, and otherwise only checks that a missing model file fails cleanly instead of throwing. On the `iphone` branch this suite also carries the ARKit wire-protocol, CUDA-GL-interop, video-device, and video-source-system modules; the CUDA-GL interop module deliberately runs early there because it creates its own CUDA context.

---

## When a run crashes with no output

`CmdApp::exec` sets stdout unbuffered (`setvbuf(stdout, nullptr, _IONBF, 0)`) precisely because a piped stdout (CI) is fully buffered and a hard crash would otherwise discard everything: the last printed line is the crash site. Also check `MikanCmd.log`, since test results are written there in addition to stdout.

If a process dies instantly with exit status `0xC0000135` (`STATUS_DLL_NOT_FOUND`), a runtime DLL is missing next to the executable. Known project gotcha: a locally installed GStreamer on `PATH` can satisfy DLL loads that CI cannot, so a missing post-build DLL copy passes locally and only fails in CI. Verify the DLL copy steps rather than the code.

Second known gotcha: with the default `CMAKE_UNITY_BUILD=ON`, a transitively included `windows.h` in a jumbo TU can rewrite a method name that collides with a Win32 macro (`GetObject` → `GetObjectA`, likewise `SendMessage`, `CreateWindow`, ...), producing an `LNK2019` far from the real conflict. Avoid such method names; see [build.md](./build.md).

---

## Stall watchdogs and the delta clamp

`App::tick` clamps the per-frame delta to 0.1 s (`deltaSeconds = fminf(now - m_lastFrameTimestamp, 0.1f)` in `src/Editor/AppCore/App.cpp`), and that clamped value propagates down the entire update chain (`MainWindow` to `ProjectManager` to systems to device managers). A watchdog or timeout that accumulates `deltaSeconds` therefore badly under-counts across a freeze: after a debugger breakpoint resumes, the delta is still at most 0.1 s, so an N-second timeout takes another N real seconds to trip instead of firing immediately. Stall detection must store a `std::chrono::steady_clock::time_point` and compare against `steady_clock::now()` (which keeps advancing while threads are frozen), never `+= deltaSeconds`. `MikanGStreamerVideoDevice`'s frame-stall watchdog is the reference implementation of the correct pattern.

---

## In-process GStreamer decode failures

`Failed to load plugin ... The specified procedure could not be found` when loading `nvh264dec`/`d3d11h264dec` is a PATH problem, not a D3D11/CUDA conflict. That message is `ERROR_PROC_NOT_FOUND`: a dependency did load, but was missing an export the plugin needed.

`libgstnvcodec.dll` links the MinGW runtime (`libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`) alongside the GStreamer libraries. Git for Windows ships its own copies of all three in `C:\Program Files\Git\mingw64\bin`, built from a different GCC. That directory is not on the machine PATH, but Git Bash prepends it to the PATH of anything launched from a Git Bash shell. The loader then resolves the plugin's imports against Git's copies and the load fails. The software decoder `openh264dec` survives it, which is what makes the failure look decoder specific.

So launching `Mikan.exe` from Git Bash silently forces the software decode tier, while the same binary launched from Explorer, Visual Studio, or `cmd` gets hardware decode. Verified both ways: with the machine PATH the ARKit device logs `Decoded CUDA video frame` and never emits the fallback message.

Before suspecting pipeline code, check how the process was launched, then run the identical pipeline string through a standalone `gst-launch-1.0.exe`. A failed hardware open also produces harmless `GStreamer-CRITICAL`/`GLib-GObject-CRITICAL` assertion spam (GStreamer's own cleanup on this failure mode); it does not crash the process.

---

## Spout logging

Spout keeps its own diagnostics, and Spout 2.007 exposes no log callback: `EnableSpoutLog()` allocates a console window titled "Spout Log" over the host process, and `EnableSpoutLogFile()` appends to a file. Both flags live in module globals, so one call configures every sender and receiver in that module. Spout logging is off by default everywhere.

The editor turns it on through `SpoutLogRelay` (`src/Editor/Interprocess/SpoutLogRelay.cpp`), owned by `App` and gated on the `spoutLogEnabled` setting in `AppSettingsConfig` (the "Relay Spout Logs" checkbox in the project Settings panel, off by default). Enabling it points Spout's file logging at `MikanSpout.log` in the working directory at verbose level, and `App::tick` tails the new bytes each frame into the logger under the `Spout` scope, so Spout's output lands in the log panel and `MikanXR.log` with no extra window. Level tags map `[warning]`/`[error]`/`[fatal]` onto the matching severity; `[notice]` and untagged verbose lines log as info.

Client processes (and the editor's own sender path inside `MikanSharedTexture`) have no settings file to read, so they gate on the `MIKAN_SPOUT_LOG` environment variable instead, applied once per process in `SharedTextureWriter.cpp`:

- unset, `0`, `off`: no Spout logging (default)
- `console`, `1`, `on`, `true`: Spout's own console window
- `file`: `MikanSpoutSender.log` in the Spout log folder (`%APPDATA%\Spout`)

The variable wins over the editor setting. When it is set, `SpoutLogRelay` logs one line saying so and stands down, so the two never fight over Spout's log destination.

---

## Profiling

The editor is instrumented with easy_profiler (`EASY_FUNCTION()` / `EASY_BLOCK()` throughout the tick, compositor, and node evaluation paths). `App::startup` calls `profiler::startListen()`, so a running `Mikan.exe` accepts connections from the easy_profiler GUI at any time. Launch with `-waitForProfiler` to block startup until a profiler client connects and starts capturing, which is useful for profiling initialization.

---

## Other diagnostic surfaces

- **ML capture runs.** `OnnxSession` logs each loaded model with the execution provider it actually got (`Loaded <path> (EP: DirectML|CPU)`) plus every input and output shape, so a session that fell back from DirectML to CPU is visible rather than just slow. `MoGeInference::run` additionally logs the inputs the metric recovery depended on (`fov_x=... shift=... metric_scale=...`); a wrong FOV reaching the model is the first suspect when a depth proxy comes out the wrong size, and that line settles it. See [scene-lighting.md](./scene-lighting.md) and [depth-proxy-mesh.md](./depth-proxy-mesh.md).

- **Node graph errors.** Failed compositor/shape graph evaluations accumulate `NodeEvaluationError` values on the owning component (`getLastNodeEvalErrors()`); `CompositorNodeEditorWindow` and `ShapeNodeEditorWindow` read and display them each frame.

- **Automation server.** A loopback TCP text command channel on port 21120 for driving and inspecting a running `Mikan.exe`: stage control, property read/write, function invokes, screenshots, Lua eval, and log tail. See [automation.md](./automation.md).

- **Lua debugger.** `LuaDebugServer` listens on TCP 21110 (LRDB protocol, VSCode `vscode-lrdb` extension). Attach from VSCode and set breakpoints in component `.lua` files; `lrdb_break()` forces a programmatic break. Details in [scripting.md](./scripting.md).

- **Lua errors.** Script failures are logged with full Lua tracebacks (`CommonScriptContext::checkLuaResult`), and the failing script's state is disposed rather than left half-broken.

- **Websocket server.** `WebsocketInterprocessMessageServer` logs listen errors and malformed/unroutable requests at warning level; there is no full per-message wire log.

- **GL state stack debug.** `App::tickWindows` has a static `bDebugPrintStack` flag that, when flipped in a debugger, makes `MkStateStack` print state push/pop activity for one frame.

- **Client-side logging.** Client apps get their own logger (`MikanClientLogger` inside `MikanClientCore`); the `LoggerSettings::log_callback` hook lets an embedding app route editor-side log lines into its own sink.
