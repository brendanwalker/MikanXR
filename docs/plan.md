# Plan

The living plan: what is in flight now, what comes next, and the open questions. Completed items are removed rather than checked off. Resolved questions are removed. Deferred work enters here at the moment of deferral.

## Now

- [ ] UI modernization (`ui_update` branch): the MikanTrack UI treatment has landed (ImGui v1.92.9-docking, SDL2 2.30.10, the `MkGuiTheme` palette and Mochiy Pop One font, localized keys in en/ja including the descriptor-generated panel labels, and a dockable shell with a File/View menu bar and an in-app Log panel). One consequence to keep in mind: the `resources/localization` CDN manifest now lists the JSON tables, so old shipped builds fetching the deleted CSVs fall back to their bundled strings.
- [ ] Decide whether the editor wants multi-viewport docking (`ImGuiConfigFlags_ViewportsEnable`), which would let a floating panel become a real OS window on a second monitor. Deliberately left off: MikanXR runs four independent ImGui contexts over one shared GL context, and the SDL2 backend's viewport bookkeeping is the least-tested path in that setup.
- [ ] Localize the USB video setting slider labels. `GuiPanel_USBVideoSourceComponent` builds them at runtime from `k_videoSettingPropertyPrefixes` ("brightness" to "Brightness") inside a custom renderer, so they sit outside the descriptor label mechanism and still read English in every language.
- [ ] ARKit video source (`iphone` branch): pose-in-RTP streaming and the hardware/software two-tier decode landed through Phase 7; verify a live iPhone session end to end and confirm an old saved project referencing removed texture-source enum names still loads (relies on `VideoTextureNode`'s `FindEnumValue` fallback).
- [ ] ARKit marker tracking origin, editor half. The phone can already be told where a marker sits (`arkit send setmarkerorigin <16 row-major floats>`, verified on the wire: a marker at (1,2,3) moves the reported camera translation to (-1,-2,-3)), and it renders markers from an OpenCV-verified `DICT_6X6_250` codebook for printing. What remains is the editor detecting the marker in a received ARKit frame and answering with its placement. Everything needed is already on the wire, since each RTP packet carries the frame's camera-to-world pose and its intrinsics alongside `frameSeq`, so `CalibrationPatternFinder_Aruco` plus `solvePnP` gives marker-to-world for that exact frame. Detection stays editor-side deliberately, to keep one ArUco implementation: the marker system's dictionary is configurable, and a second copy on the phone could silently drift out of sync with it.
	ARKit's own image tracking was tried first and does not work for this. `ARReferenceImage.validate` accepts a rendered marker, but detection never fires against a real printed sheet, tried both bare and with a white margin, while OpenCV reads that same marker instantly from the phone's own screenshot at an oblique angle. ARKit image tracking matches detailed photographs, and marker art is repetitive and nearly featureless by design. Do not re-attempt it.
	Measured side by side on one scene, a printed marker and a printed flower illustration in frame together, with `tools/aruco_pose_measure.py` for the OpenCV arm. ARKit found the flower in 0.06s and tracked it in all 2714 anchor updates, at 0.76/3.40/0.61mm jitter per axis and 6.77mm worst deviation. OpenCV read the marker in all 864 frames at 0.04/0.05/0.16mm and 0.55mm worst deviation, roughly twelve times tighter. ARKit never detected the marker at all. Two caveats on the gap: ARKit's figure is world-space so it carries world-tracking drift the camera-relative OpenCV figure does not, and the flower's printed width was assumed to be 100mm. The direction is not in doubt, the exact factor is. Note also that `validate` returned true for both targets, the one that tracked perfectly and the one that never tracked, so it predicts nothing.
	Also outstanding on the phone side: the launch popup offering print, scan or skip, and printing a marker at an exact physical size.
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
