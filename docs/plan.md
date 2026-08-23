# Plan

The living plan: what is in flight now, what comes next, and the open questions. Completed items are removed rather than checked off. Resolved questions are removed. Deferred work enters here at the moment of deferral.

## Now

- [ ] ARKit video source (`iphone` branch): pose-in-RTP streaming and the hardware/software two-tier decode landed through Phase 7; verify a live iPhone session end to end and confirm an old saved project referencing removed texture-source enum names still loads (relies on `VideoTextureNode`'s `FindEnumValue` fallback).

## Next

- [ ] Root-cause the in-process `nvh264dec`/`d3d11h264dec` plugin loading gap (see `docs/reference/debugging.md`) so the ARKit hardware decode tier works inside `Mikan.exe`.

## Later

- [ ] Scene lighting: decide whether a single global probe suffices or region-of-interest probes near the character are needed. `LightEnvironmentComponent` already carries a world position, so this needs no wire-format change. Answerable only from the end-to-end look in Unreal.
- [ ] Scene lighting: calibrate the exposure scalar automatically instead of by hand per shoot.
- [ ] Depth proxy mesh: refine residual metric scale against tracked anchors or the floor plane, so a capture needs no ArUco marker in frame.
- [ ] Depth proxy mesh: occlusion-grade silhouette edges. The current proxy is a shadow catcher; it cannot occlude a character walking behind real furniture.
- [ ] Stereo calibration: `MikanStereoIntrinsics` and the rectification math exist but no stereo calibration AppStage does.
- [ ] Video recording: `eSupportedCodec` in `CompositorConstants.h` is defined but nothing records the composited output to a file.
- [ ] Remove the empty `src/Libraries/ARKitReceiver` and `src/Libraries/MikanARKitReceiver` scaffolding directories.
- [ ] Stop checking generated bindings into git (`bindings/csharp/CMakeLists.txt` carries the TODO).
- [ ] Editor transaction recording with undo/redo, surfaced through the automation server's reserved `history` namespace so a driven session can inspect the transaction log, roll back state, and replay interactions (`docs/reference/automation.md`).

## Open questions

- Is the chessboard calibration pattern mode meant to be reachable? `CalibrationPatternFinder_Chessboard` exists but no flow instantiates it; the calibrators hardcode ChArUco.
