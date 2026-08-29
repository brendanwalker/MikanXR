# Plan

The living plan: what is in flight now, what comes next, and the open questions. Completed items are removed rather than checked off. Resolved questions are removed. Deferred work enters here at the moment of deferral.

## Now

- [ ] UI modernization (`ui_update` branch): the MikanTrack UI treatment has landed (ImGui v1.92.9-docking, SDL2 2.30.10, the `MkGuiTheme` palette and Mochiy Pop One font, localized keys in en/ja including the descriptor-generated panel labels, and a dockable shell with a File/View menu bar and an in-app Log panel). One consequence to keep in mind: the `resources/localization` CDN manifest now lists the JSON tables, so old shipped builds fetching the deleted CSVs fall back to their bundled strings.
- [ ] Decide whether the editor wants multi-viewport docking (`ImGuiConfigFlags_ViewportsEnable`), which would let a floating panel become a real OS window on a second monitor. Deliberately left off: MikanXR runs four independent ImGui contexts over one shared GL context, and the SDL2 backend's viewport bookkeeping is the least-tested path in that setup.
- [ ] Localize the USB video setting slider labels. `GuiPanel_USBVideoSourceComponent` builds them at runtime from `k_videoSettingPropertyPrefixes` ("brightness" to "Brightness") inside a custom renderer, so they sit outside the descriptor label mechanism and still read English in every language.
- [ ] ARKit video source (`iphone` branch): pose-in-RTP streaming and the hardware/software two-tier decode landed through Phase 7; verify a live iPhone session end to end and confirm an old saved project referencing removed texture-source enum names still loads (relies on `VideoTextureNode`'s `FindEnumValue` fallback).
- [ ] ARKit video source: the once-per-keyframe receive stall originates on the phone's UDP send, not in the editor. Raw packet capture on the wire shows zero inter-packet gaps over 100ms but 35 gaps over 50ms at 0.95/s with a 60ms median, keyframe aligned, while the phone's encode timing is even (keyframe 18.6ms against 16.2ms for P frames, and zero gaps over 100ms on the debug channel's separate TCP transport). So the phone pauses transmitting for ~60ms around each keyframe and the decoder amplifies that into a ~110ms output gap. Next step is phone side: instrument when `StreamingSession` hands each frame's packets to `UDPStreamer` versus when the encoder produced them, which separates a send-queue backlog behind the large IDR from something in `NWConnection`'s pacing.
	Ruled out by measurement, all reproducing in a standalone `gst-launch` pipeline with no editor involved: appsink queue depth, the redundant NV12 conversion pass, caps renegotiation, decoder re-initialisation, `num-output-surfaces`, `max-display-delay`, keyframe size (6x bitrate change leaves the stall rate and p99 identical), and the jitter buffer, which is unchanged at 50/200/500ms and with the element removed entirely. Note the standalone runs report ~334ms stalls against Mikan's ~110ms because `videodecoder:5` logging inflates them; treat 110ms as the real figure and the standalone numbers as comparative only.
- [ ] Node editor link UX niceties deferred from the migration: Ctrl+drag to detach an existing link off a pin and rewire it, and a pin right-click menu with a disconnect item (`ed::ShowPinContextMenu` plus the existing `NodePin::editorRenderContextMenu` hook). Dragging a new link from a connected input pin already rewires it, since input pins keep a single connection.

## Next


## Later

- [ ] Scene lighting: decide whether a single global probe suffices or region-of-interest probes near the character are needed. `LightEnvironmentComponent` already carries a world position, so this needs no wire-format change. Answerable only from the end-to-end look in Unreal.
- [ ] Scene lighting: calibrate the exposure scalar automatically instead of by hand per shoot.
- [ ] Depth proxy mesh: refine residual metric scale against tracked anchors or the floor plane, so a capture needs no ArUco marker in frame.
- [ ] Depth proxy mesh: occlusion-grade silhouette edges. The current proxy is a shadow catcher; it cannot occlude a character walking behind real furniture.
- [ ] Stereo calibration: `MikanStereoIntrinsics` and the rectification math exist but no stereo calibration AppStage does.
- [ ] Video recording: `eSupportedCodec` in `CompositorConstants.h` is defined but nothing records the composited output to a file.
- [ ] Remove the empty `src/Libraries/ARKitReceiver` and `src/Libraries/MikanARKitReceiver` scaffolding directories.
- [ ] Stop checking generated bindings into git (`bindings/csharp/CMakeLists.txt` carries the TODO).
- [ ] Wire the unrecordable property notifications into descriptors so they reach undo and client events: the `EditorObjectSystem` settings names and `depth_mesh_scale_correction` (the standing gaps listed in `docs/reference/transactions.md`).

## Open questions

- Is the chessboard calibration pattern mode meant to be reachable? `CalibrationPatternFinder_Chessboard` exists but no flow instantiates it; the calibrators hardcode ChArUco.
