# Video Sources

How camera video gets into and out of the editor: the ECS video-source components, the device-plugin DLL model, frame delivery and texture upload, intrinsics and tracking-pose association, and the shared-texture/Spout paths that cross the process boundary. See [objects.md](./objects.md) for the ECS object model, [calibration.md](./calibration.md) for how intrinsics are computed, [compositor.md](./compositor.md) for how the frames are consumed, and [wire-protocol.md](./wire-protocol.md) for the client-facing request/event types.

---

## Core abstraction

There is no standalone `VideoSourceManager`/`VideoSourceView` layer; video sources are ECS components. `VideoSourceComponent` (`src/Editor/ECS/VideoSource/VideoSourceComponent.h`) is the abstract base, paired with a persisted `VideoSourceDefinition` (mirroring, frame queue size, and the `MikanVideoSourceIntrinsics` calibration blob). Two concrete subclasses exist on `main`, each owned by a matching object system:

- `USBVideoSourceComponent` / `USBVideoSourceSystem`: local USB cameras (device path, video mode, per-mode camera settings).
- `NetworkVideoSourceComponent` / `NetworkVideoSourceSystem`: RTMP/RTSP network streams (protocol, address, port, path).

The `iphone` branch adds a third, `ARKitVideoSourceComponent` / `ARKitVideoSourceSystem` for iPhone ARKit streaming (a base UDP port; video RTP on `base_port+0`). Everything in this doc marked as ARKit lives on that branch and is not built on `main`; it is kept here because the branch is in flight (see [plan.md](../plan.md)).

The components talk to hardware through device interfaces defined in `src/Libraries/MikanCoreApp/Public`: `IVideoDevice` (settings, colorimetry) with per-transport extensions `IUsbVideoDevice` and `INetworkVideoDevice` (plus `IARKitVideoDevice` on the branch), each with a listener interface (`IUsbVideoDeviceListener` etc.) and a manager (`IUsbVideoDeviceManager`, ...). The editor side implements the listener; the plugin side implements the device.

`VideoSourceComponent` exposes the editor-facing surface: `openVideoSource()`/`closeVideoSource()`, `startVideoStream()`/`stopVideoStream()`, `getCameraIntrinsics()`/`setCameraIntrinsics()`, `getProjectionMatrix()`, `getVideoColorimetry()`, and `set/getVideoSetting()` (`eVideoSettingType`: exposure, gain, focus, ...). It is also an `IPropertyInterface`/`IFunctionInterface` participant, so clients can drive it remotely (reflected values structs in `src/Libraries/MikanClientAPI/Public/MikanVideoSourceTypes.h`; server routing in `src/Editor/Server/VideoSourceRequestHandler.cpp`).

---

## Plugin model

Video backends are separate DLLs under `src/Plugins`, loaded on demand by name. The contract lives in `MikanCoreApp`:

- `IMikanModule` (`startup()`/`shutdown()`) plus two exported C functions per plugin, `AllocatePluginModule` and `FreePluginModule` (`IMikanModule.h`).
- Per-domain module interfaces: `IUsbVideoDeviceModule::createUsbVideoDeviceManager()`, `INetworkVideoDeviceModule`, `IVRDeviceModule::createTrackingRuntime()`, and `IARKitVideoDeviceModule` on the `iphone` branch.
- `MikanModuleManager` (`MikanModuleManager.cpp`) caches loaded modules; `MikanModule::load()` uses `dylib` to open the DLL from the executable directory (`PathUtils::getModulePath()`) and resolve the two exports.

There is no directory scan: each object system asks for a hardcoded module name. `USBVideoSourceSystem` loads `"MikanWMFVideo"`, `NetworkVideoSourceSystem` loads `"MikanGStreamerVideo"`, `VRObjectSystem` builds `"Mikan" + runtime name` (currently yielding `"MikanSteamVR"`), and on the `iphone` branch `ARKitVideoDeviceManagerLoader` loads `"MikanARKitVideo"`. Module load and manager startup run on a detached worker thread (see `USBVideoSourceSystem::initUsbVideoDeviceManagerOnThread`, which also enters COM MTA for WMF enumeration); the system polls a future each tick and retries any components left in `isPendingOpen()` once the manager is ready. In CMake, `MikanGStreamerVideo` is gated behind `MIKAN_WITH_GSTREAMER` (off in CI) and so is `MikanARKitVideo` on the branch; `MikanSteamVR` and `MikanWMFVideo` always build (`src/Plugins/CMakeLists.txt`).

---

## Backends

- `MikanWMFVideo`: Windows Media Foundation USB capture. `MikanWMFVideoDeviceManager` enumerates devices (`WMFDeviceList`, `WMFDeviceInfo`) and watches hotplug via `DeviceHotplugNotifier`. `WMFVideoFrameProcessor` is an `IMFSourceReaderCallback`; `OnReadSample` decodes/forwards samples to `MikanWMFVideoDevice`, which raises `notifyVideoFrameReceived` with a `UsbVideoFrameBuffer` (formats `RGB24`, `NV12`, `YUY2`, with per-section stride info). Video modes carry `UsbVideoModeProperties` including `VideoColorimetry` (color matrix + transfer function enums in `IVideoDevice.h`).

- `MikanGStreamerVideo`: network streams. `MikanGStreamerVideoDevice` implements `INetworkVideoDevice` for `eNetworkVideoProtocol::RTMP`/`RTSP` URLs built from `NetworkVideoConnectionSettings`; open/close are async (`WorkerThread`, futures) and `update()` polls the pipeline. Camera settings are not applicable and stubbed.

- `MikanARKitVideo` (`iphone` branch only): iPhone ARKit over the network. `MikanARKitVideoDevice` builds a GStreamer RTP receive pipeline (`udpsrc`/`rtpjitterbuffer`/`rtph264depay`/`h264parse`) on `base_port+0`. Camera pose and frame sequence ride inside the video RTP stream's own header extension: `ARKitRTPHeaderExtension` parses them and attaches an `ARKitFrameSeqMeta` to each decoded buffer, so `update()` can emit an `ARKitVideoFrameBundle` (frameSeq, timestamp, optional pose, optional decoded pixels) with pose exactly coupled to its frame. Decode is two-tier: a hardware pipeline (`nvh264dec`, decoded frames stay in CUDA device memory and reach GL via CUDA-GL interop) is tried first, falling back to a software pipeline (`openh264dec`, packed BGR in system memory) when the NVIDIA path fails to build. The wire format is defined in `ARKitWireProtocol.h`.
GStreamer/CUDA caveats baked into the ARKit backend, learned empirically:

- `MikanARKitVideoDevice` owns its own `CUcontext` (lazily created) and re-asserts it with `cuCtxSetCurrent` at the start of every CUDA-touching `update()` tick. A successful `gst_buffer_map(..., GST_MAP_CUDA)` does not leave nvcodec's internal context current on the calling thread, so relying on "whatever context is current" fails with `CUDA_ERROR_INVALID_CONTEXT`. Unified virtual addressing makes the mapped `CUdeviceptr` usable from the device's own context.

- After a fatal CUDA fault (e.g. illegal address reported at synchronize), the context is unrecoverable: make no further Driver API calls against it, including cleanup, and tear the whole session down. A fault can also poison later fresh `cuCtxCreate` calls in the same process, so full process restart is the only fully reliable recovery.

- GStreamer bus messages are polled explicitly per tick (`gst_bus_pop_filtered` in `update()`); `gst_bus_add_watch` is inert in this codebase because nothing pumps a GLib main loop.

- `MikanSteamVR`: not a video backend. `MikanSteamVRModule` implements `IVRDeviceModule` and creates a `MikanSteamVRManager` tracking runtime. It provides `IVRDevice` objects (HMD/tracker/controller poses via `getDevicePose` with frame-delay support, serial/role properties, render meshes via `SteamVRRenderModelResource`, and named attachment sockets). `VRObjectSystem` consumes it to drive `VRDeviceComponent` poses, which video sources use for camera tracking (below).

---

## Open and streaming lifecycle

`openVideoSource()` binds the component to a plugin device (by device path for USB, by URL or port for the network transports); if the async manager is not up yet the component stays `isPendingOpen()` and the system retries. Status is reported via `eVideoOpeningStatus` (the network transports open asynchronously) and `eVideoStreamingStatus` (`stopped`/`pendingStart`/`started`/`failed`).

Streaming is refcounted by consumers: `VideoSourceComponent::startVideoStream(VideoFrameDistortionView*)` adds the view to a mutex-guarded `m_activeViews` set (max 32) and calls the subclass `startVideoStreamInternal()` on the first subscriber; `stopVideoStream()` stops the device when the last view unsubscribes. `OnOpened`/`OnClosed`/`OnStarted`/`OnStopped`/`OnFrameSizeChanged`/`OnIntrinsicsChanged` multicast delegates notify the rest of the editor.

---

## Frame delivery into the editor

`VideoFrameDistortionView` (`src/Editor/Calibration/VideoFrameDistortionView.h`) is the per-consumer frame sink and texture pipeline. Device receive threads call `VideoSourceComponent::writeVideoFrame()` (or `writeStereoVideoFrameSection()`), which fans out to every active view's BGR source buffer under a mutex and bumps an atomic write index. On the main thread, `readAndProcessVideoFrame()` detects a new index, converts/undistorts, and uploads into a circular queue of GL textures (`VideoFrameQueueEntry`, queue size from `VideoSourceDefinition::getVideoFrameQueueSize()`); `getVideoTexture(frameIndex)` serves delayed frames so video can be latency-matched against tracking (`CameraDefinition::getTrackingFrameDelay()`). Processing runs in one of two modes (`eVideoFrameProcessorMode`): `CALIBRATION` uses `CVVideoFrameProcessor` (CPU `cv::remap` undistortion plus grayscale buffers for pattern detection) and `COMPOSITOR` uses `GLVideoFrameProcessor` (GPU shader undistortion driven by a distortion-map texture).

GPU-direct sources bypass the CPU buffer entirely: `VideoSourceComponent::getDirectColorTexture()` / `processDirectVideoFrame()` / `getDirectFrameIndex()` default to null/no-op and no source on `main` overrides them. On the `iphone` branch `ARKitVideoSourceComponent` does, and its NV12 luma/chroma GL textures (exposed by the plugin as raw GL ids) are wrapped in `IMkExternalTexture`s and converted to RGBA by a fullscreen shader pass once per tick.

---

## Intrinsics, settings, and persistence

Camera calibration data is a `MikanVideoSourceIntrinsics` (reflected wire type, `MikanVideoSourceTypes.h`): a polymorphic pointer to `MikanMonoIntrinsics` or `MikanStereoIntrinsics`, both deriving `MikanBaseIntrinsics` (pixel dimensions, hfov/vfov, znear/zfar). `MikanMonoIntrinsics` carries `MikanDistortionCoefficients` (k1-k6 rational radial + p1/p2 tangential, per the OpenCV model) plus distorted and undistorted camera matrices. It lives on `VideoSourceDefinition` and persists with the rest of the component definition into the project file (Configuru JSON, `*.mikanproj`, saved by `ProjectManager`/`ProjectConfig`). `setCameraIntrinsics()` triggers `recomputeCameraProjectionMatrix()`, so `VideoSourceComponent::getProjectionMatrix()` always reflects current calibration. USB sources additionally persist per-video-mode `eVideoSettingType` values in `USBVideoSourceDefinition`'s `m_videoSettingsMap`.

The depth proxy mesh capture is the one consumer that depends on these intrinsics quantitatively rather than for rendering: it feeds `MikanMonoIntrinsics::hfov` to the model, and the recovered metric scale rides directly on that number. An uncalibrated or wrong FOV silently produces a proxy of the wrong size rather than an error. See [depth-proxy-mesh.md](./depth-proxy-mesh.md).

---

## Tracking pose association

A `CameraComponent` (`src/Editor/ECS/Camera/CameraComponent.h`) ties everything together via IDs on `CameraDefinition`: a `MikanVideoSourceID` (which video source feeds it), a `MikanTrackingMountID` (which physical tracker rig it sits on), and an aperture pose offset (tracker-to-lens transform produced by alignment calibration, see [calibration.md](./calibration.md)). `TrackingMountComponent` binds a VR device path plus an attachment socket name and produces a `VRDevicePoseView`; `CameraComponent::updateAperturePoseFromTrackingMount()` polls it each tick and composes the aperture offset to get the stage-space camera pose (`getStageSpaceAperturePose`, `getApertureViewMatrix`, `getApertureProjectionMatrix`).

The `iphone` branch's ARKit source instead implements `IFrameCoupledPoseProvider` (`src/Editor/ECS/Camera/IFrameCoupledPoseProvider.h`): pose and intrinsics arrive coupled to each video frame in the RTP header extension, and `getLatestFrameCoupledPose()` hands `CameraComponent` a camera-to-world transform plus a frame sequence number for correlation, with intrinsics applied only when they change meaningfully.

---

## ARKit debug side channel

Separate from the video path and independent of it: `ARKitDebugChannel` (`src\Editor\Interprocess\ARKitDebugChannel.h`) is a TCP line channel to the MikanARStreamer app, exposed as the automation server's `arkit` namespace ([automation.md](./automation.md)). The phone dials out, because the editor cannot learn the phone's address from a GStreamer `udpsrc`. Diagnostics the phone pushes are re-emitted through the editor's logger, which is what puts phone-side encode timing and editor-side receive timing on one ordered timeline. Commands travel the other way, with the phone owning the vocabulary.

It has no dependency on `ARKitVideoSourceComponent` or the plugin, and works whether or not video is streaming. It also binds every interface rather than loopback, so it is off unless enabled.

---

## Crossing the process boundary

Pixel data never travels over the websocket; only events and requests do (see [wire-protocol.md](./wire-protocol.md)).

- Editor to client: when the compositor processes a new video frame, `CompositorComponent` builds a `MikanCameraNewFrameEvent` via `CameraComponent::makeNewCameraFrameEvent()` (camera pose, frame index, dimensions) and `CameraRequestHandler::publishCameraNewFrameEvent()` broadcasts it as JSON. Clients use it to render their scene for that frame.

- Client to editor: clients allocate a shared render target through `MikanClientCore` using `createSharedTextureWriteAccessor` (`src/Libraries/MikanSharedTexture/Public/SharedTextureWriter.h`; color, optional depth and shadow buffers per `SharedTextureDescriptor`, D3D9/11/12, OpenGL, Metal, Vulkan client APIs). On a `PublishCameraRenderTargetTextures` request, `CameraRequestHandler::frameRenderedHandler` reads the textures through a per-client, per-camera `SharedTextureReadAccessor` (`src/Editor/Interprocess/SharedTextureReader.h`) and hands them to the compositor.

- Composited output: `CompositorComponent` can publish the final composited frame as a named Spout sender (`k_spoutEnableOutputNamePropertyId` / spout output name on `CompositorDefinition`), using the same `ISharedTextureWriteAccessor` API. The `MikanSharedTexture` library is itself implemented on Spout (`Private/SpoutGL`, `Private/SpoutDX`), so both the client render-target path and the Spout output path are Spout shared textures underneath.
