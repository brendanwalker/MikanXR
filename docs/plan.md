# Plan

The living plan: what is in flight now, what comes next, and the open questions. Completed items are removed rather than checked off. Resolved questions are removed. Deferred work enters here at the moment of deferral.

## Now

- [ ] ARKit video source (`iphone` branch): pose-in-RTP streaming and the hardware/software two-tier decode landed through Phase 7; verify a live iPhone session end to end and confirm an old saved project referencing removed texture-source enum names still loads (relies on `VideoTextureNode`'s `FindEnumValue` fallback).

## Next

- [ ] Root-cause the in-process `nvh264dec`/`d3d11h264dec` plugin loading gap (see `docs/reference/debugging.md`) so the ARKit hardware decode tier works inside `Mikan.exe`.

## Later

- [ ] Stereo calibration: `MikanStereoIntrinsics` and the rectification math exist but no stereo calibration AppStage does.
- [ ] Video recording: `eSupportedCodec` in `CompositorConstants.h` is defined but nothing records the composited output to a file.
- [ ] Remove the empty `src/Libraries/ARKitReceiver` and `src/Libraries/MikanARKitReceiver` scaffolding directories.
- [ ] Stop checking generated bindings into git (`bindings/csharp/CMakeLists.txt` carries the TODO).

## Open questions

- Is the chessboard calibration pattern mode meant to be reachable? `CalibrationPatternFinder_Chessboard` exists but no flow instantiates it; the calibrators hardcode ChArUco.
