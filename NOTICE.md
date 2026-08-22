# Third-party and borrowed code

This file records code copied in from sibling projects and binary dependencies
fetched by `InitialSetup_x64.bat`. Libraries vendored under `thirdparty/` are
git submodules that carry their own upstream license files; they are not
re-inventoried here.

## Code copied from MikanTrack

MikanTrack (https://github.com/BrendanWalker/MikanTrack, same author) originally
copied a large amount of source *from* this project — see its `NOTICE.md`. The
following flowed back the other way.

| File(s) here | MikanTrack origin |
|---|---|
| `src/Libraries/MikanOnnx/Public/OnnxSession.h`, `Private/OnnxSession.cpp` | `src/Vision/OnnxSession.{h,cpp}` (ONNX Runtime env name changed to `MikanXR`; `isDirectMLAvailable()` added) |

## Binary dependencies fetched by InitialSetup_x64.bat

Only the entries relevant to in-process ML inference are listed; the rest of
`deps/` predates this file.

- **ONNX Runtime (DirectML)** 1.20.1 — MIT — NuGet package extracted to `deps/onnxruntime`
- **DirectML** 1.15.4 — Microsoft binary license — NuGet package extracted to `deps/directml`.
  The package ships every architecture (~350MB); `InitialSetup_x64.bat` keeps only
  `bin/x64-win` and deletes the rest.
