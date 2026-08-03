# Command Cheat Sheet

Handy commands for working in the MikanXR repo, all run from the repo root unless noted. Background on the build system is in [build.md](./build.md); directory context is in [layout.md](./layout.md).

---

## One-time setup

```
InitialSetup_x64.bat
```

Downloads prebuilt dependencies into `deps/` (large download) and installs GStreamer system-wide via MSI. Warning: it deletes any existing `build/` and `deps/` first. Set `SKIP_GSTREAMER=1` in the environment to skip the GStreamer MSIs (then configure with `-DMIKAN_WITH_GSTREAMER=OFF`).

```
git submodule update --init --recursive
git config blame.ignoreRevsFile .git-blame-ignore-revs
```

The second command (once per clone) makes `git blame` skip the repo-wide clang-format commit listed in `.git-blame-ignore-revs`.

---

## Generate project files

```
GenerateProjectFiles_X64_VS2022.bat
```

Configures `build/` with the `Visual Studio 17 2022` generator and produces `build/Mikan.sln`. Rerun after `InitialSetup_x64.bat` (which wipes `build/`).

---

## Build

Via CMake (works for any generator; add `--config Release` under the VS generator):

```
cmake --build build --target Mikan --config Release --parallel
cmake --build build --target MikanCmd --config Release --parallel
cmake --build build --target unit_test_suite_cpp --config Release --parallel
```

Or open `build\Mikan.sln` in Visual Studio 2022 and build there.

Building `MikanCmd` also builds `Mikan` (dependency) and copies all runtime DLLs and plugin DLLs into the shared output folder.

---

## Run tests

Under the local VS generator, executables land in per-config folders:

```
build\src\Editor\Release\MikanCmd.exe -runTests
build\src\Programs\Tests\UnitTests\Release\unit_test_suite_cpp.exe
```

In CI (and any configure with `-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=...\build\bin`) both live in `build\bin\`:

```
build\bin\MikanCmd.exe -runTests
build\bin\unit_test_suite_cpp.exe
```

- `MikanCmd.exe -runTests` runs the interprocess/client-API integration tests and writes details to `MikanCmd.log` in the working directory in addition to stdout. Check that file if the run fails with no console output (e.g. a crash exit code like `0xC0000005`).
- Both suites must pass in CI (`.github/workflows/build-and-test.yml`).

---

## Formatting

Only `src/` is formatted; `thirdparty/` is never touched. Use clang-format 19.1.x to match CI (VS2022 bundles a compatible copy under `VC\Tools\Llvm\bin\clang-format.exe`, which the CMake scripts find automatically; otherwise `pip install clang-format==19.1.5`).

With a configured build tree:

```
cmake --build build --target FormatFix      # reformat in place
cmake --build build --target FormatCheck    # check only (what CI runs)
```

Standalone, no build tree needed (same script CI runs):

```
cmake -P cmake/RunClangFormat.cmake -- --check
cmake -P cmake/RunClangFormat.cmake -- --fix
```

Point at a specific binary with `cmake -DCLANG_FORMAT_EXE=path\to\clang-format -P cmake/RunClangFormat.cmake -- --check`.

---

## Client bindings codegen

Never hand-edit `bindings/csharp/Generated` or `bindings/typescript/types`; regenerate them instead (see [wire-protocol.md](./wire-protocol.md)):

```
cmake --build build --target MikanTypeScriptCodeGen --config Release   # regenerate bindings/typescript/types
cmake --build build --target MikanClientTypeScript --config Release    # codegen + npm install + npm run build
cmake --build build --target MikanCSharpCodeGen --config Release       # regenerate bindings/csharp/Generated (VS generator only)
```

All of these build and invoke `MikanClientCodeGen` with the matching `*CodeGenConfig.json`. The TypeScript targets require `npm` on PATH.

---

## Install and installer

```
cmake --build build --target INSTALL --config Release       # install exes/DLLs/bindings into dist\Win64
cmake --build build --target CREATE_INSTALLER               # build the Inno Setup installer (requires Inno Setup installed)
```

`CREATE_INSTALLER` only exists if the Inno Setup compiler was found at configure time (`cmake/Installer.cmake`); it compiles `templates/installer_win64.iss.in` with the version string filled in.

---

## Reproducing the CI build locally

CI uses Ninja (for sccache) with GStreamer off and a flattened output dir. From a VS2022 developer command prompt:

```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DMIKAN_WITH_GSTREAMER=OFF -DCMAKE_UNITY_BUILD=ON -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%CD%\build\bin ...
```

plus the same dependency path variables as `GenerateProjectFiles_X64_VS2022.bat`; see `.github/workflows/build-and-test.yml` for the exact full invocation. Note the C# bindings and C# client test are skipped under Ninja.
