I would like to draw up a plan for streaming video + depth data + camera pose from ARKit on an iPhone to MikanXR. 

There are two major parts to this: 
1. An iPhone app with basic settings UI that allows the user adjust video settings, and toggle depth streaming
2. Update MikanR to support a new "ARKit" VideoSource type that can be used along side the other 

The depth data on the iPhone is only 256x192 and needs to be upsampled when received on the Mikan side to match the resolution of the RGB video frame. I've already done some research into this using "Joint Bilateral Upsampling (JBU)" using CUDA. You can see that experiment here using PFM test data from a public depth data training set: 
D:\Github\git-BrendanWalker\CudaDepthUpsample

There are still some open questions about the best way to stream the video, camera pose and depth data to Mikan from the app. It sounds like the simpliest bet is probably going to be a H.264 encoded video stream. The depth data will be in it's own newwork stream since it's 16-bit depth data. It sounds like using lossless RVL compression is probably the best bet here since I want to avoid any flickering in the depth data (but I am open to suggestions). The camera pose data will be a seperate binary packet with with the frame timestamp (unless that is something we can add as a p[er frame payload in the RGB video stream?)

Where possible I want to try and leverage hardware encoding end decoding to minimize the overhead on the CPU. 

On the Mikan side I'm not sure if it makes more sense to leverage the GStreamer plugin for receiving the H.264 encoded video stream or if there is other code better suited to receive the video stream from the iPhone.

The existing GStreamer plugin in mikan can be found here:
D:\Github\git-BrendanWalker\MikanXR\src\Plugins\MikanGStreamerVideo

Let me know if you have any clarifying questions.

The goal here is two implementation plans:
1. Detailed implementation plan for the iPhone app
2. Detailed implementation plan for the Mikan changes




Generate a granular, step-by-step implementation plan that I can feed directly to a Qwen coder agent.

Please include:
1. Step-by-step logic.
2. Specific files to modify/create.
3 Edge cases to handle.
4. Verification steps/tests Qwen should run to ensure the code works.


# ARKit → MikanXR Streaming: Granular Implementation Tickets

## Context

MikanXR currently supports two video source types: USB (`MikanWMFVideo`, Windows Media Foundation) and Network (`MikanGStreamerVideo`, RTMP/RTSP pull via `decodebin`). Neither supports depth data, and camera pose is sourced independently from a SteamVR-style tracking-mount system sampled at composite time — not per-video-frame.

The goal is a third source type, **ARKit**, fed by a new iPhone app streaming:
1. RGB video (ARKit camera feed, H.264 hardware-encoded)
2. Depth (ARKit LiDAR `sceneDepth`, 256×192 Float32 meters, RVL-compressed)
3. Camera pose + intrinsics (frame-exact, from ARKit tracking)

to Mikan over the local network. Mikan upsamples the 256×192 depth to match RGB resolution using CUDA Joint Bilateral Upsampling (prototyped in `D:\Github\git-BrendanWalker\CudaDepthUpsample`), and uses the frame-exact pose to drive the camera directly, bypassing the existing composite-time tracking-mount sampling for this source type.

**Confirmed research findings driving this design:**
- Mikan's renderer is **OpenGL only** (`IMkTexture`/`GlTexture`) — CUDA interop uses `cudaGraphicsGLRegisterImage`, not D3D11.
- No existing depth-camera ingestion pipeline exists in Mikan — greenfield.
- `MikanGStreamerVideo` only builds RTMP/RTSP pull pipelines via generic `decodebin`; no UDP/RTP or hardware-decode precedent exists, but its open/close/watchdog/appsink-pull architecture (`MikanGStreamerVideoDevice.cpp`) is a solid template to copy from.
- The `CudaDepthUpsample` JBU kernel (`jbu_cuda.cu`) is portable (raw device pointers already) but its host wrapper is OpenCV-`GpuMat`-coupled, `float`-only, blocking (`cudaDeviceSynchronize()`), and calls `exit(-1)` on CUDA errors — all need rework.
- Camera pose today flows through `CameraComponent::makeNewCameraFrameEvent` (`CameraComponent.cpp:540-611`) sampling `VRDevicePoseView` at composite time. ARKit will instead feed pose directly, frame-exact.

**Design decisions locked in with the user:**
- Mikan-side decode targets **NVIDIA hardware** (`nvh264dec` via GStreamer's `nvcodec` plugin) outputting CUDA device memory directly — no non-NVIDIA fallback in v1.
- Sync uses an **RTP header extension** carrying `frameSeq` + capture timestamp, propagated through decode via `GstReferenceTimestampMeta`, matched exactly (integer key, not epsilon) against sidecar depth/pose packets.
- Pose is **frame-coupled** via a new interface, bypassing the VR-tracking-mount path for this source type.
- Full feature set as v1 (no phased MVP).

## How to use this document

Each ticket below is a self-contained unit of work sized for a single agent session: goal, files, step-by-step logic, edge cases, and verification. Work through tickets **in order within a track**; tracks A and B have no dependency on each other and can run in parallel. Track C depends on B (protocol) being stable; Track D depends on C (needs a decoded video frame to test against) but can be developed against synthetic/offline data first (the CUDA kernel itself only needs raw buffers, not a live pipeline). Track E depends on C+D existing. Track F is final integration and requires all prior tracks complete.

```
Track A (iPhone app)         Track B (Mikan protocol/plumbing)
      |                              |
      |                              v
      |                        Track C (Mikan GStreamer video pipeline)
      |                              |
      |                              v
      |                        Track D (Mikan CUDA JBU) -- can start early w/ offline test data
      |                              |
      +----------------------------->|
                                      v
                                Track E (ECS/GUI/Camera integration)
                                      |
                                      v
                                Track F (end-to-end verification)
```

---

## Wire Protocol Reference (both sides must match this exactly)

Direction: **iPhone pushes UDP to Mikan.** Mikan listens on `basePort+0` (video), `basePort+1` (depth), `basePort+2` (pose). One shared `frameSeq` (`uint32`, big-endian on the wire, monotonically increasing, one increment per captured+sent AR frame) correlates all three streams.

**Video (`basePort+0`)**: standard RTP/UDP, RFC 6184 H.264 payload. RTP header extension (RFC 5285 one-byte header, extension ID `1`), 12-byte payload, big-endian: `frameSeq(4B) | captureTimestampUs(8B)`. Attached to every RTP packet of the access unit.

**Depth (`basePort+1`)**: fragmented custom UDP packets. Fragment header (20 bytes, all fields big-endian):
```
uint16 magic      = 0xAD01
uint8  version     = 1
uint8  type        = 1 (DEPTH)
uint32 frameSeq
uint64 captureTimestampUs
uint16 fragIndex   (0-based)
uint16 fragCount
```
Followed by up to ~1200 bytes of payload. Full reassembled payload = `[uint32 rvlByteLength][RVL-compressed uint16 depth plane, 256x192, millimeters, 0=invalid][uint32 confByteLength][RLE-compressed confidence plane, 256x192, 2-bit values 0/1/2 packed as bytes pre-RLE]`.

**Pose (`basePort+2`)**: single UDP datagram, no fragmentation, big-endian:
```
uint16 magic      = 0xAD01
uint8  version     = 1
uint8  type        = 2 (POSE)
uint32 frameSeq
uint64 captureTimestampUs
float32 transform[16]   // row-major 4x4, camera-to-world
float32 fx, fy, cx, cy  // intrinsics
float32 imageWidth, imageHeight
```
Total: 16 + 64 + 16 + 8 = 104 bytes.

**RVL algorithm**: classic zero-run-length + delta bit-packing lossless 16-bit depth compression (the algorithm behind Microsoft's "Fast Lossless Depth Image Compression", also used by Azure Kinect). Both sides implement independently from the same spec below (Ticket A3 / B4) — no shared code between Swift and C++.

---

# Track A: iPhone App (Swift, iOS 16+, requires LiDAR device)

## A1 — Project scaffolding & capability gating
**Goal**: Create the Xcode project skeleton and verify LiDAR/ARKit availability before anything else is built.
**Files**: New Xcode project `MikanARStreamer/` (App target, SwiftUI lifecycle), `MikanARStreamer/App.swift`, `MikanARStreamer/ContentView.swift`.
**Steps**:
1. Create SwiftUI App project, iOS deployment target 16.0+, add `NSCameraUsageDescription` to Info.plist (required for `ARSession`).
2. On launch, check `ARWorldTrackingConfiguration.supportsFrameSemantics(.sceneDepth)`; if `false`, show a blocking full-screen error view ("This device does not support LiDAR depth capture") instead of the main UI.
3. Add a top-level `AppState: ObservableObject` (empty for now, populated in later tickets) injected via `.environmentObject`.
**Edge cases**: Simulator (no camera/LiDAR) must show the capability error, not crash. Devices with LiDAR but old iOS versions lacking `.sceneDepth` support must also be gated.
**Verification**: Run on a non-LiDAR device/simulator and confirm the error view appears; run on a LiDAR device and confirm the main view loads.

## A2 — ARSession capture + frame throttling
**Goal**: Start an `ARSession` capturing color+depth+pose, throttled to a configurable target FPS, producing a shared `frameSeq` per emitted frame.
**Files**: New `ARCaptureController.swift`.
**Steps**:
1. Define `struct CapturedFrame { let frameSeq: UInt32; let captureTimestampUs: UInt64; let colorPixelBuffer: CVPixelBuffer; let depthPixelBuffer: CVPixelBuffer?; let confidencePixelBuffer: CVPixelBuffer?; let transform: simd_float4x4; let intrinsics: simd_float3x3; let imageResolution: CGSize }`.
3. `ARCaptureController: NSObject, ARSessionDelegate` owns `ARSession`, configured with `ARWorldTrackingConfiguration` + `frameSemantics = [.sceneDepth]` (only add `.sceneDepth` if the depth-streaming setting is enabled — see A11; when disabled, still run world tracking for pose but skip requesting depth frames to save power).
4. In `session(_:didUpdate frame: ARFrame)`: apply a frame-rate gate — track `lastEmitTime`; if `now - lastEmitTime < 1.0/targetFPS`, return early (drop frame). Otherwise increment an internal `UInt32` counter (wrapping) for `frameSeq`, compute `captureTimestampUs` from `frame.timestamp * 1_000_000` (ARKit's `timestamp` is already a monotonic `CACurrentMediaTime`-based value), build a `CapturedFrame`, and publish it via a closure/Combine callback `onFrameCaptured: (CapturedFrame) -> Void`.
5. `sceneDepth` may be `nil` on a given frame even when enabled (e.g. degraded tracking) — pass `nil` through rather than reusing a stale buffer.
**Edge cases**: `frameSeq` wraps at `UInt32.max` — downstream consumers (Mikan) must handle wraparound in comparisons (note this for Track B). Tracking state `.limited`/`.notAvailable` should still emit frames (pose may be less reliable, but Mikan should decide what to do with it, not the phone). Target FPS changes mid-session (from settings) must take effect without restarting the session.
**Verification**: Log `frameSeq`/timestamp/frame-drop-rate to console at 3 different target FPS settings (15/30/60) and confirm actual emission rate matches within ~10%.

## A3 — RVL compressor (standalone, unit-testable)
**Goal**: Implement RVL encode as a pure function with no ARKit/networking dependencies, so it can be unit tested in isolation.
**Files**: New `RVLCodec.swift`.
**Steps**:
1. Implement `func rvlEncode(_ depth: [UInt16]) -> [UInt8]` per the standard RVL algorithm: iterate the depth array; maintain run-length encoding of zero-runs (invalid pixels) using a variable-length "nibble" bit-packing scheme, and delta-encode consecutive non-zero values using a zigzag+variable-bit-length code. (Reference the well-documented public algorithm — same one used by Azure Kinect's `k4a` samples and the original Microsoft "Fast Lossless Depth Image Compression" paper; implement it directly rather than porting external code, since it's ~80-120 lines of straightforward bit manipulation: a `BitWriter` helper that packs values into 4-bit nibbles with a continuation bit, alternating between counting zero-runs and encoding non-zero-value runs.)
2. Implement the corresponding `func rvlDecode(_ data: [UInt8], expectedCount: Int) -> [UInt16]` in the same file — needed for the round-trip unit test (A3 test target), even though the iPhone app itself never decodes.
3. Implement `func packConfidenceRLE(_ confidence: [UInt8]) -> [UInt8]` — simple run-length encoding (`[value, runLength]` byte pairs, runLength capped at 255 with repeated pairs for longer runs) since confidence values (0/1/2) cluster heavily.
**Edge cases**: All-zero (fully invalid) depth frame must encode/decode correctly. Single non-repeating values (worst case, no compression benefit) must not overflow buffers — always allocate conservatively or use a growable array.
**Verification**: Add a unit test target `MikanARStreamerTests`. Test: generate random `UInt16` arrays (including all-zero, all-max, and realistic depth-like data with clustered zero-runs) of size 256×192=49152, run `rvlEncode` then `rvlDecode`, assert exact round-trip equality for every case. Test `packConfidenceRLE` similarly with a decode counterpart written just for the test.

## A4 — Depth packet framer
**Goal**: Turn a captured depth+confidence frame into wire-format UDP fragments per the Wire Protocol Reference.
**Files**: New `DepthPacketFramer.swift` (depends on A3).
**Steps**:
1. `func buildDepthPackets(frameSeq: UInt32, timestampUs: UInt64, depthPixelBuffer: CVPixelBuffer, confidencePixelBuffer: CVPixelBuffer?) -> [Data]`.
2. Convert `depthPixelBuffer` (`kCVPixelFormatType_DepthFloat32`, meters) to a `[UInt16]` millimeters array: lock the base address, read `Float32` per pixel, `value <= 0 || !value.isFinite ? 0 : UInt16(clamping: Int(value * 1000))`.
3. Convert `confidencePixelBuffer` (`kCVPixelFormatType_OneComponent8`, values 0/1/2) to `[UInt8]` directly (or all-`2`s / high-confidence if `nil`, matching what a "no confidence data" convention should mean downstream — document this choice inline).
4. RVL-encode depth, RLE-encode confidence (A3), build the payload: `[UInt32 rvlLen][rvlBytes][UInt32 confLen][confBytes]` (big-endian lengths).
5. Split payload into ≤1200-byte chunks, prepend the 20-byte fragment header (magic/version/type/frameSeq/timestamp/fragIndex/fragCount) to each chunk, return as an array of `Data`.
**Edge cases**: Payload that happens to fit in a single fragment (fragCount=1) — logic must not special-case away the header. Payload larger than `UInt16.max` fragments is astronomically unlikely at this resolution but assert/clamp defensively rather than silently corrupt.
**Verification**: Unit test with a synthetic `CVPixelBuffer` (constructed via `CVPixelBufferCreate` with known values) confirms fragment count, header fields, and that concatenated payload bytes (stripping headers) match the expected `[len][rvl][len][conf]` structure.

## A5 — Pose packet framer
**Goal**: Turn a captured pose into a single wire-format UDP datagram.
**Files**: New `PosePacketFramer.swift`.
**Steps**:
1. `func buildPosePacket(frameSeq: UInt32, timestampUs: UInt64, transform: simd_float4x4, intrinsics: simd_float3x3, imageResolution: CGSize) -> Data`.
2. Serialize per the Wire Protocol Reference: magic/version/type=2/frameSeq/timestampUs, then 16 floats row-major from `transform` (note: `simd_float4x4` is column-major in memory — transpose when writing row-major, and document this explicitly since it's a common source of silent bugs), then `fx=intrinsics[0][0], fy=intrinsics[1][1], cx=intrinsics[2][0], cy=intrinsics[2][1]`, then `imageResolution.width/height` as floats. All big-endian.
**Edge cases**: `simd` matrix column/row-major confusion (call out explicitly in code comments — this is the single most likely cross-platform bug in this whole feature).
**Verification**: Unit test with a known identity transform + known intrinsics, decode the produced `Data` byte-by-byte in the test and assert values land at the documented offsets.

## A6 — UDP networking layer
**Goal**: Wrap `Network.framework` UDP sending behind a simple interface usable by the encoder/framer output.
**Files**: New `UDPStreamer.swift`.
**Steps**:
1. `class UDPStreamer { init(host: String, port: UInt16); func send(_ data: Data); func start(); func stop() }`, backed by `NWConnection(host:port:using: .udp)` on a dedicated serial `DispatchQueue`.
2. Three instances owned by the orchestrator (Track A10): video (`basePort+0`), depth (`basePort+1`), pose (`basePort+2`).
3. Handle `NWConnection.State` changes (`.ready`, `.failed`, `.cancelled`) via a status callback surfaced to the UI (Track A11's connection indicator).
**Edge cases**: Sending before the connection reaches `.ready` should queue or silently drop (never crash) — UDP is lossy by nature, dropping a few early packets during connection setup is acceptable. Host resolution failure (bad IP/hostname typed by user) must surface a clear error, not hang silently.
**Verification**: Point at a local Python `socket.socket(AF_INET, SOCK_DGRAM)` listener script (write a throwaway test script, not part of the app) bound to three ports, confirm bytes arrive matching what was sent, for all three streamers concurrently.

## A7 — Hardware H.264 encoder wrapper
**Goal**: Feed ARKit's `capturedImage` pixel buffers into `VTCompressionSession` and receive encoded NALUs.
**Files**: New `H264Encoder.swift`.
**Steps**:
1. `class H264Encoder` configures a `VTCompressionSession` with: `kVTCompressionPropertyKey_RealTime = true`, `kVTCompressionPropertyKey_ProfileLevel = kVTProfileLevel_H264_Main_AutoLevel`, `kVTCompressionPropertyKey_AllowFrameReordering = false`, `kVTCompressionPropertyKey_AverageBitRate` and `kVTCompressionPropertyKey_MaxKeyFrameInterval` driven by constructor params (wired to settings in A11).
2. `func encode(pixelBuffer: CVPixelBuffer, presentationTimeStamp: CMTime, frameSeq: UInt32)` calls `VTCompressionSessionEncodeFrame`, passing `frameSeq` through via the `sourceFrameRefcon` parameter so the output callback can associate encoded output with the originating `frameSeq`.
3. Output callback receives `CMSampleBuffer`; extract `frameSeq` back out of the refcon, and surface `(frameSeq, CMSampleBuffer)` via a closure `onEncodedFrame: (UInt32, CMSampleBuffer) -> Void`.
4. Handle encoder session recreation if pixel buffer dimensions or format change mid-session (shouldn't happen with a fixed ARKit config, but guard defensively).
**Edge cases**: `VTCompressionSessionEncodeFrame` is asynchronous — output callbacks may arrive out of submission order in theory (unlikely with `AllowFrameReordering=false`, but don't assume strict ordering; rely on the `frameSeq` refcon rather than a queue position). Encoder initialization failure (e.g. unsupported format) must propagate as an error the orchestrator can show in the UI.
**Verification**: Feed a short recorded sequence of `CVPixelBuffer`s (or live ARKit frames) and confirm `onEncodedFrame` fires once per input with monotonically non-decreasing `frameSeq` values and non-empty sample buffers; verify keyframes appear at the configured interval by inspecting `CMSampleBufferGetSampleAttachmentsArray` for the `NotSync` key.

## A8 — AVCC→Annex-B conversion + RFC 6184 RTP packetizer
**Goal**: Convert encoder output into RTP packets carrying H.264 Annex-B NAL units.
**Files**: New `H264RTPPacketizer.swift` (depends on A7's output type).
**Steps**:
1. `func extractNALUnits(from sampleBuffer: CMSampleBuffer) -> [Data]`: read the AVCC length-prefixed NALUs from the sample buffer's data buffer (`CMBlockBufferGetDataPointer`), split on the 4-byte big-endian length prefixes. On keyframes, also extract SPS/PPS from `CMSampleBufferGetFormatDescription` → `CMVideoFormatDescriptionGetH264ParameterSetAtIndex` and prepend them as their own NAL units.
2. `func packetizeRTP(nalUnits: [Data], frameSeq: UInt32, timestampUs: UInt64, ssrc: UInt32, sequenceNumberProvider: () -> UInt16) -> [Data]`: for each NAL unit ≤ ~1200 bytes, emit a single-NAL-unit-mode RTP packet; for larger ones, split into FU-A fragments per RFC 6184 §5.8. Standard 12-byte RTP header (version=2, payload type e.g. 96 dynamic, sequence number, RTP timestamp = `frameSeq`-derived monotonic 90kHz-scaled value or a running counter — either is fine since Mikan correlates by the header extension, not RTP timestamp math) plus the one-byte RTP extension header (ID=1, length=2 32-bit words) carrying `frameSeq`+`timestampUs` per the Wire Protocol Reference, attached to every packet of the access unit. Set the RTP marker bit on the last packet of the frame.
**Edge cases**: FU-A fragmentation must correctly set the Start/End bits on the FU header for the first/last fragment. Sequence number wraps at `UInt16.max` (standard RTP behavior — increment and let it wrap naturally, don't special-case). Extremely large keyframes (SPS+PPS+IDR slice) will produce many FU-A fragments — verify none exceed the UDP payload budget.
**Verification**: Capture output packets to a `.pcap`-like raw dump and open in Wireshark (or write a tiny Python RTP/H.264 parser) to confirm the RTP header, extension, and FU-A reassembly produce valid Annex-B NAL data matching the original encoder output byte-for-byte after fragment reassembly.

## A9 — RVL/RTP unit test cross-check against Mikan reference vectors
**Goal**: Once Track B4 (Mikan-side RVL decoder) exists, generate a small set of fixed test vectors on the iPhone side (or in a shared Python reference script) that both A3 and B4 can validate against, to catch algorithm mismatches early instead of during live end-to-end testing.
**Files**: `MikanARStreamerTests/RVLCrossCheckTests.swift`, plus a shared `docs/rvl_test_vectors.json` (checked into both repos or a shared location) containing a handful of `{input: [UInt16], expected_compressed_hex: String}` cases.
**Steps**: Generate 3-5 representative depth arrays (all-zero, uniform value, random with realistic zero-run clustering), encode with A3's `rvlEncode`, save input+output as the JSON fixture.
**Edge cases**: N/A (this is a test-authoring ticket).
**Verification**: Both A3's test suite and B4's test suite load the same JSON fixture and assert their respective encode/decode produces/consumes matching bytes.

## A10 — Streaming session orchestrator
**Goal**: Wire A2 (capture) → A7 (encode) → A8 (RTP) / A4 (depth) / A5 (pose) → A6 (network) into one cohesive session object.
**Files**: New `StreamingSession.swift`.
**Steps**:
1. `class StreamingSession` owns one `ARCaptureController`, one `H264Encoder`, three `UDPStreamer`s, and configuration (host/port/bitrate/fps/depth-enabled) passed in at `start()`.
2. On `ARCaptureController.onFrameCaptured`: always call `H264Encoder.encode(...)` with the color buffer; if depth streaming is enabled and `depthPixelBuffer != nil`, build+send depth packets (A4) immediately (depth doesn't need to wait on the encoder); always build+send the pose packet (A5) immediately.
3. On `H264Encoder.onEncodedFrame`: run A8's extraction+packetization, send all resulting RTP packets via the video `UDPStreamer`.
4. Expose `start(config:)`/`stop()` and a `@Published var connectionStatus`/`stats` for the UI.
**Edge cases**: Depth/pose packets for a `frameSeq` may be sent and arrive at Mikan *before* the corresponding (asynchronously encoded) video packets — this is fine given Mikan correlates by `frameSeq` with a completion timeout, not arrival order, but note it explicitly so Track B's correlation buffer is designed to tolerate any arrival order. Stopping mid-encode must not crash (drain or cancel in-flight `VTCompressionSession` callbacks cleanly via `VTCompressionSessionCompleteFrames`/invalidate).
**Verification**: Run a full session against the A6 test listener script, confirm all three streams produce continuous, `frameSeq`-consistent output over a 60-second run with no crashes, and that `stop()`/`start()` can be cycled repeatedly.

## A11 — Settings UI (SwiftUI)
**Goal**: User-facing configuration screen.
**Files**: New `SettingsView.swift`, `AppSettings.swift` (`@AppStorage`-backed model).
**Steps**:
1. Fields: Mikan host (text field, validated as IP/hostname), base port (numeric stepper, default e.g. 27015), video bitrate (segmented picker: Low/Medium/High presets mapping to Mbps values), target FPS (segmented picker: 15/30/60), "Stream Depth" toggle, "Send Confidence Map" toggle (disabled/greyed when depth streaming is off).
2. Connect/Disconnect button bound to `StreamingSession.start()`/`stop()`, with a colored status indicator (grey/yellow/green mapped to disconnected/connecting/streaming) driven by `connectionStatus`.
3. Persist all fields via `@AppStorage`.
**Edge cases**: Changing settings while actively streaming should either be disallowed (grey out fields) or require restarting the session — pick the simpler option (disallow) and document it.
**Verification**: Manual UI test on-device: change each setting, confirm persistence across app relaunch, confirm Connect/Disconnect transitions the status indicator correctly.

## A12 — App lifecycle handling
**Goal**: Handle backgrounding, idle timer, and camera permission gracefully.
**Files**: `App.swift` (extend from A1), `ARCaptureController.swift` (extend from A2).
**Steps**:
1. On `UIApplication.willResignActiveNotification`, call `StreamingSession.stop()` (ARKit sessions don't run backgrounded, so continuing to "try" wastes battery and leaves the connection in a bad state).
2. Set `UIApplication.shared.isIdleTimerDisabled = true` while a session is active, `false` when stopped.
3. Handle camera permission denial (`AVCaptureDevice.authorizationStatus(for: .video) == .denied`) with a clear in-app prompt directing the user to Settings.
**Edge cases**: Backgrounding mid-keyframe-encode should not corrupt the next session's state — ensure `StreamingSession.stop()` fully tears down and a fresh `start()` re-initializes cleanly.
**Verification**: Manual test: background the app mid-stream, confirm clean stop; foreground and reconnect, confirm a fresh working session (no stale state/crashes).

## A13 — Telemetry/debug overlay
**Goal**: On-screen stats to aid debugging without a separate tool.
**Files**: New `StatsOverlayView.swift`, extend `StreamingSession.stats`.
**Steps**: Track and display: actual measured FPS (rolling average), current video bitrate estimate, RVL compression ratio (raw vs compressed bytes, rolling average), count of frames where depth was requested but `nil` came back from ARKit, count of dropped/incomplete UDP sends (from `NWConnection` error callbacks).
**Edge cases**: Stats collection must not meaningfully impact encode/send performance (use lightweight atomic counters, update UI at ~2Hz not per-frame).
**Verification**: Manual — confirm displayed numbers look sane during a live session (FPS near target, compression ratio > 1.5x on typical indoor scenes).

## A14 — Local test receiver script (dev tool, not shipped)
**Goal**: A throwaway Python/Node script to validate the full iPhone output independent of Mikan, usable well before Track C/D exist.
**Files**: `tools/test_receiver.py` (outside the Xcode project, e.g. repo root or a `tools/` folder).
**Steps**: Bind three UDP sockets (video/depth/pose ports), log received packet counts/sizes per `frameSeq`, optionally use `ffmpeg`/`gstreamer` command-line piping to decode and display the raw RTP H.264 stream for visual confirmation, and implement a Python-side RVL decoder (can reuse the same reference logic as A9's cross-check vectors) to sanity-check depth packet integrity.
**Verification**: This script *is* the verification tool for A1-A13 — running it against a live iPhone session should show continuous, matched `frameSeq` counts across all three streams and a decodable video preview.

---

# Track B: Mikan — Protocol & Non-GPU Plumbing (C++)

## B1 — Wire protocol structs & constants header
**Goal**: Single source of truth in C++ for the wire format, referenced by all later Mikan tickets.
**Files**: New `src/Plugins/MikanARKitVideo/Private/ARKitWireProtocol.h`.
**Steps**: Define `#pragma pack`-safe structs (or explicit byte-offset read/write helper functions, since raw `struct` overlay on network bytes is fragile across compilers — prefer explicit big-endian read/write helper functions over `#pragma pack`) for: `kMagic = 0xAD01`, `ePacketType { Depth = 1, Pose = 2 }`, `DepthFragmentHeader` (fields per Wire Protocol Reference), `PosePacket` (fields per Wire Protocol Reference), and `RTPExtensionPayload { frameSeq, captureTimestampUs }`. Include `readU16BE`/`readU32BE`/`readU64BE`/`readF32BE` helpers (and write counterparts) since x86/x64 is little-endian and the wire format is big-endian.
**Edge cases**: Struct alignment/padding must never be relied upon for wire parsing — always use explicit offset-based reads.
**Verification**: Unit test (new `tests/ARKitWireProtocolTests.cpp` or wherever Mikan's existing test pattern lives — check `App::runTests` schema-guard test mentioned in project memory for the existing test harness convention) round-trips each struct through write-then-read helpers and asserts field equality.

## B2 — RVL decoder (C++, standalone, unit-testable)
**Goal**: C++ port of the RVL algorithm's decode side (Mikan only ever decodes, never encodes).
**Files**: New `src/Plugins/MikanARKitVideo/Private/RVLCodec.h/.cpp`.
**Steps**: Implement `std::vector<uint16_t> rvlDecode(const uint8_t* data, size_t length, int expectedPixelCount)` mirroring the same algorithm spec as iOS Ticket A3 (zero-run-length + delta bit-packing decode). Also implement `packConfidenceRLEDecode` counterpart for the confidence plane.
**Edge cases**: Malformed/truncated input (dropped/corrupted UDP fragments assembled incorrectly) must fail safely — return an empty result or throw a caught exception, never read out of bounds. Validate `expectedPixelCount` against decoded output length before use.
**Verification**: Load the shared `docs/rvl_test_vectors.json` fixture from Ticket A9, decode each vector's `expected_compressed_hex`, and assert the result matches the original input array. Also fuzz with random-corrupted inputs (truncated/bit-flipped) and assert no crash/UB (run under ASan if available in the existing build).

## B3 — Receive-capable UDP socket class
**Goal**: A `recvfrom`-based UDP receiver, modeled on the existing send-only `UdpMulticastSocket`.
**Files**: New `src/Plugins/MikanARKitVideo/Private/UdpReceiveSocket.h/.cpp` (referencing the pattern in `src/Libraries/MikanDMX/Private/UdpMulticastSocket.h/.cpp`, but note this is a **new class in the new plugin**, not a modification of the DMX one — don't touch `MikanDMX`).
**Steps**: RAII Winsock2 (Windows-only, matching the existing codebase's platform target) UDP socket: `bool open(uint16_t port)`, `void close()`, and either a blocking `bool receive(uint8_t* buffer, size_t bufferSize, size_t& outBytesReceived, sockaddr_in& outSender)` for use in a dedicated worker thread, or a non-blocking `select()`-based poll variant if that fits the async-init patterns better (match whatever `NetworkVideoSourceSystem`/`USBVideoSourceSystem` use for their worker-thread + `std::promise`/`std::future` async-init pattern, per the earlier research).
**Edge cases**: Port already in use → clear error, not a crash. Socket receive buffer sizing — set `SO_RCVBUF` generously (depth packets can burst) to avoid kernel-level packet drops under load.
**Verification**: Unit/manual test: bind to a test port, send known UDP payloads from a local test script (or `A14`'s Python tool), confirm bytes received match exactly, including from a genuinely remote sender (not just loopback) to catch any accidental `INADDR_LOOPBACK` binding mistakes.

## B4 — Depth channel receiver (fragment reassembly)
**Goal**: Consume raw depth UDP packets into complete per-`frameSeq` RVL-decoded depth+confidence planes.
**Files**: New `src/Plugins/MikanARKitVideo/Private/ARKitDepthReceiver.h/.cpp` (uses B1, B2, B3).
**Steps**:
1. Worker thread loop: `UdpReceiveSocket::receive()` → parse `DepthFragmentHeader` (B1) → validate `magic`/`version`/`type` → accumulate fragments into a `std::map<uint32_t /*frameSeq*/, FragmentAssemblyState>` (buffers per fragment index, a bitmask/count of received fragments, first-fragment-arrival timestamp for staleness eviction).
2. On `fragCount` fragments all received for a `frameSeq`: concatenate payload, parse `[rvlLen][rvlBytes][confLen][confBytes]`, RVL-decode (B2) into `uint16_t[256*192]` depth (mm) + `uint8_t[256*192]` confidence, push a completed `ARKitDepthFrame{frameSeq, timestampUs, depth, confidence}` to a thread-safe output queue/callback.
3. Sweep incomplete entries older than e.g. 2 frame-intervals (configurable) from the assembly map and drop them (log a "depth frame dropped: incomplete" counter for diagnostics).
**Edge cases**: Duplicate fragments (retransmission/network duplication) must not double-count toward `fragCount`. Out-of-order fragment arrival is normal for UDP — index by `fragIndex`, don't assume ordering. `frameSeq` wraparound (`UInt32.max` → `0`) must not break the staleness-sweep comparison (use wraparound-safe comparison, e.g. `(int32_t)(a - b)`).
**Verification**: Feed synthetic fragment sequences (in-order, out-of-order, with gaps, with duplicates) via a unit test harness that bypasses the real socket and calls the reassembly logic directly; assert correct frames complete, correct frames get dropped-as-stale, and no crash on adversarial input.

## B5 — Pose channel receiver
**Goal**: Consume raw pose UDP packets into per-`frameSeq` pose data.
**Files**: New `src/Plugins/MikanARKitVideo/Private/ARKitPoseReceiver.h/.cpp` (uses B1, B3).
**Steps**: Worker thread loop: `UdpReceiveSocket::receive()` → parse `PosePacket` (B1) → validate → construct `ARKitPoseFrame{frameSeq, timestampUs, transform (4x4), fx, fy, cx, cy, imageWidth, imageHeight}` → push to a thread-safe ring buffer keyed by `frameSeq` (small capacity, e.g. last 30 frames, since pose should arrive promptly and stale entries are simply irrelevant, not "incomplete" the way depth fragments are).
**Edge cases**: Same wraparound-safe `frameSeq` handling as B4. A pose packet for a `frameSeq` that's already been evicted from the ring (very late arrival) should be silently dropped, not crash.
**Verification**: Unit test feeding synthetic `PosePacket` byte sequences (including malformed/truncated ones) directly into the parse logic, asserting correct extraction and safe rejection of bad input.

## B6 — Frame correlation buffer
**Goal**: Combine video (arrives via Track C), depth (B4), and pose (B5) for the same `frameSeq` into one bundle, tolerant of any arrival order and of depth being absent (disabled) or missing (dropped).
**Files**: New `src/Plugins/MikanARKitVideo/Private/ARKitFrameCorrelator.h/.cpp`.
**Steps**:
1. `struct ARKitFrameBundle { uint32_t frameSeq; uint64_t timestampUs; /* video frame data pointer/handle from Track C */; std::optional<ARKitDepthFrame> depth; ARKitPoseFrame pose; }`.
2. Maintain a small pending-map keyed by `frameSeq`, each entry tracking which of {video, depth (if depth streaming is enabled for this session), pose} have arrived and a first-arrival timestamp.
3. `notifyVideoArrived(frameSeq, ...)`, `notifyDepthArrived(ARKitDepthFrame)`, `notifyPoseArrived(ARKitPoseFrame)` — each checks if the entry is now complete (video+pose, plus depth only if depth streaming is enabled) and if so fires a callback with the completed `ARKitFrameBundle`, removing it from the pending map.
4. Sweep entries stale beyond a timeout (e.g. 3 frame-intervals) — deliver a partial bundle (missing depth, or even missing video if pose-only) rather than silently dropping, since pose alone is still useful to the camera-pose consumer (Track E), and log which component was missing.
**Edge cases**: Depth-disabled sessions must never wait on a depth arrival that will never come — the "is complete" check must respect the current depth-enabled flag, not just structurally require all three. Must handle the case where video arrives for a `frameSeq` that never gets a pose (pose packet lost) — deliver video-only after timeout rather than blocking forever.
**Verification**: Unit test driving the three `notify*` methods in various orders (including depth-disabled mode, and deliberately-missing-component scenarios) and asserting the correct bundle (complete or partial-after-timeout) fires exactly once per `frameSeq`.

## B7 — New interfaces: `IARKitVideoDevice`/`Manager`/`Module`
**Goal**: Define the plugin contract, mirroring `INetworkVideoDevice.h`/`INetworkVideoDeviceManager.h`/`INetworkVideoDeviceModule.h`.
**Files**: New `src/Libraries/MikanCoreApp/Public/IARKitVideoDevice.h`, `IARKitVideoDeviceManager.h`, `IARKitVideoDeviceModule.h`.
**Steps**:
1. `IARKitVideoDevice : IVideoDevice` — async `open(const ARKitVideoConnectionSettings&)`/`close()`/`update(float)` lifecycle (same shape as `INetworkVideoDevice`), `ARKitVideoConnectionSettings{ uint16_t basePort; bool depthStreamingEnabled; }`, listener interface `IARKitVideoDeviceListener` with `notifyDeviceOpened()`, `notifyDeviceClosed()`, `notifyFrameBundleReceived(const ARKitFrameBundle&)` (using the bundle type from B6 — may need to move that struct into this public header, or a shared internal header both the plugin and this interface include).
2. `IARKitVideoDeviceManager` — `createVideoDevice(settings) -> IARKitVideoDevicePtr`, `destroyVideoDevice(...)`, `update(float)`.
3. `IARKitVideoDeviceModule : IMikanModule` — `createARKitVideoDeviceManager() -> IARKitVideoDeviceManagerPtr`.
**Edge cases**: N/A (pure interface definition).
**Verification**: Compiles cleanly as headers-only against the existing `MikanCoreApp` build; no runtime behavior to test yet.

## B8 — `ARKitVideoSourceSystem` module-loading scaffold
**Goal**: The ECS-side system that loads the `MikanARKitVideo` plugin DLL asynchronously, mirroring `NetworkVideoSourceSystem`.
**Files**: New `src/Editor/ECS/VideoSource/ARKitVideoSourceSystem.h/.cpp`.
**Steps**: Copy the structure of `NetworkVideoSourceSystem.cpp` (`#define ARKIT_VIDEO_DEVICE_MODULE_NAME "MikanARKitVideo"`, async background-thread module load + `startup()`, per-tick `update()` polling, retry logic for components whose `openVideoSource()` was deferred pending manager init). At this point the module DLL doesn't exist yet (Track C builds it) — this ticket can be implemented and will simply fail-to-load gracefully until Track C/E deliver the plugin, which is fine as an interim state; verify failure is graceful (logged, not crashing) in the meantime.
**Edge cases**: Same as existing `NetworkVideoSourceSystem` — missing DLL, `startup()` failure — should degrade gracefully (mark manager `failed`, skip) exactly like the existing GStreamer path does when its env var precondition isn't met.
**Verification**: Build and run Mikan with this system registered but no `MikanARKitVideo.dll` present; confirm it logs a load failure and doesn't crash or block startup.

---

# Track C: Mikan — GStreamer Video Pipeline (`MikanARKitVideo` plugin, video path)

## C1 — Plugin scaffolding (Module/Manager/Device trio)
**Goal**: Stand up the DLL skeleton, structurally copying `MikanGStreamerVideo`.
**Files**: New `src/Plugins/MikanARKitVideo/CMakeLists.txt`, `Public/MikanARKitVideoModule.h`, `Public/MikanARKitVideoExport.h`, `Private/MikanARKitVideoModule.cpp`, `Private/MikanARKitVideoDeviceManager.h/.cpp`, `Private/MikanARKitVideoDevice.h/.cpp` (implements `IARKitVideoDevice` from B7).
**Steps**: Mirror `MikanGStreamerVideoModule.cpp` (gst_init_check/gst_deinit, `AllocatePluginModule`/`FreePluginModule` exports), `MikanGStreamerVideoDeviceManager.cpp` (vector of devices, create/destroy/update), and `MikanGStreamerVideoDevice.h`'s structure — but the device's `openOnThread()` will build a *different* pipeline (C2) and, unlike the existing plugin, also owns the B4/B5/B6 depth+pose receivers and starts their worker threads on open.
**Edge cases**: Ensure `gst_init_check` failure (GStreamer runtime not installed) is handled identically to the existing plugin's pattern (env var gate before even attempting load, per `NetworkVideoSourceSystem.cpp:114-162`'s pattern — reuse the same `GSTREAMER_1_0_ROOT_MINGW_X86_64` gate check, now from `ARKitVideoSourceSystem`).
**Verification**: Build the DLL, confirm `MikanARKitVideoModule.dll` (or equivalent) is produced and Track B8's `ARKitVideoSourceSystem` can now successfully load it (open/close cycle with no pipeline logic exercised yet — stub `openOnThread` can just succeed trivially at this stage if needed to validate the module-loading path first).

## C2 — RTP/UDP GStreamer pipeline (software decode first, no CUDA yet)
**Goal**: Get a working `udpsrc → rtph264depay → h264parse → decodebin → videoconvert → appsink` pipeline pulling BGR frames, deferring `nvh264dec`/CUDA to C4, so the RTP receive path can be validated against Track A's output independently of the CUDA work.
**Files**: `src/Plugins/MikanARKitVideo/Private/MikanARKitVideoDevice.cpp`.
**Steps**: Build the pipeline string: `udpsrc port=<basePort+0> caps="application/x-rtp,media=video,encoding-name=H264,payload=96" ! rtpjitterbuffer latency=50 ! rtph264depay ! h264parse ! decodebin ! videoconvert ! video/x-raw,format=BGR ! appsink name=sink`. Reuse the existing plugin's `gst_parse_launch`, bus-watch, `gst_app_sink_try_pull_sample`-per-tick, and stream-timeout-watchdog patterns from `MikanGStreamerVideoDevice.cpp:388-556`.
**Edge cases**: `payload=96` and `encoding-name=H264` must match whatever Track A8 actually sends — verify against A8's chosen dynamic payload type. UDP port binding conflicts if a previous session didn't clean up — ensure `close()` fully tears down the pipeline/port before a re-open.
**Verification**: With Track A's iPhone app streaming, use `gst-launch-1.0` standalone first (outside Mikan) with the exact same pipeline string to confirm the phone's RTP stream is receivable and decodable at all; then confirm the same pipeline embedded in `MikanARKitVideoDevice` produces frames via `notifyVideoFrameReceived`-equivalent logging.

## C3 — Custom RTP header extension for `frameSeq` propagation
**Goal**: Get `frameSeq`+`timestampUs` from the RTP extension through to the pulled `appsink` buffer.
**Files**: New `src/Plugins/MikanARKitVideo/Private/ARKitRTPHeaderExtension.h/.cpp` (a `GstRTPHeaderExtension` subclass), wired into C2's pipeline setup.
**Steps**: Implement a `GstRTPHeaderExtension`-derived element (following the same registration mechanism GStreamer's built-in `rtponviftimestamp` extension uses — study `gstrtphdrext.h`/existing GStreamer extension examples) identified by a custom URI, registered on the `rtph264depay` element before pipeline start (`gst_rtp_header_extension_...` API — set the extension's `read()` implementation to parse the 12-byte payload per B1's `RTPExtensionPayload` and attach it as `GstReferenceTimestampMeta` (or a custom `GstMeta` if `GstReferenceTimestampMeta`'s single-`GstClockTime` shape can't carry both fields — likely need a small custom meta type carrying `frameSeq` explicitly). At the `appsink` pull site (C2), read this meta off the pulled buffer instead of relying on GStreamer PTS.
**Edge cases**: Meta must survive `rtph264depay → h264parse → decodebin`'s internal transform elements — GStreamer metas generally propagate on buffer copy/transform by design (this is exactly the mechanism ONVIF timestamp extension relies on), but verify empirically since `decodebin`'s exact element chain isn't fixed at pipeline-string-authoring time.
**Verification**: Log the extracted `frameSeq` at the `appsink` pull site and cross-check it increments matching the iPhone's `A2` `frameSeq` sequence with no gaps under normal network conditions (some gaps expected under packet loss — verify those gaps are small/rare, not systematic, which would indicate the meta isn't surviving the pipeline).

## C4 — Switch to `nvh264dec` + CUDA memory output
**Goal**: Replace software `decodebin` with hardware `nvh264dec`, negotiating CUDA memory output for zero-copy handoff to Track D.
**Files**: `MikanARKitVideoDevice.cpp` (pipeline string change), `CMakeLists.txt` (link `gstcuda-1.0`).
**Steps**: Change pipeline to `... ! h264parse ! nvh264dec ! appsink name=sink` and negotiate `video/x-raw(memory:CUDAMemory)` caps on the appsink (rather than forcing `videoconvert`/BGR system-memory output as C2 did). At the pull site, instead of `gst_buffer_map` with `GST_MAP_READ`, access the buffer's `GstCudaMemory` and retrieve its `CUdeviceptr` (via the nvcodec plugin's public API — `gst_is_cuda_memory`/`gst_cuda_memory_get_...`, check the exact API surface in the installed GStreamer version's `gst-plugins-bad` headers) for direct use as the JBU guide image in Track D.
**Edge cases**: If `nvh264dec` isn't available (verify via `gst-inspect-1.0 nvh264dec` first — this ticket has a hard dependency on an NVIDIA GPU + driver + the nvcodec plugin being present), pipeline creation will fail — surface this clearly as a device-open error rather than a silent hang, since v1 has no software-decode fallback per the locked-in design decision.
**Verification**: `gst-inspect-1.0 nvh264dec` confirms availability before starting. Confirm the pipeline negotiates CUDA memory caps successfully (log the negotiated caps string) and that a valid non-null `CUdeviceptr` is retrieved per frame.

---

# Track D: Mikan — CUDA JBU Depth Upsample

*Can start early against offline test data (the prototype's PFM files) in parallel with Track C — only the final GL-interop wiring (D4) needs a live pipeline.*

## D1 — Port JBU kernel with `uint16_t` support, drop OpenCV
**Goal**: Extract the core kernel from `D:\Github\git-BrendanWalker\CudaDepthUpsample\jbu_cuda.cu` into Mikan's tree with a real `uint16_t` code path and no OpenCV dependency.
**Files**: New `src/Plugins/MikanARKitVideo/Private/Cuda/JBUKernel.cu/.h`.
**Steps**: Copy the `jbu_kernel` template function essentially as-is (it's already raw-pointer-based). Add an explicit `template __global__ void jbu_kernel<uint16_t>(...)` instantiation and a host launch function `void jbuUpsample(const uint16_t* d_depthLow, int lowW, int lowH, const uint8_t* d_guideRGB, int guideW, int guideH, int guideStride, float* d_depthOut, int radius, float sigmaSpatial, float sigmaColor, cudaStream_t stream)` operating purely on raw device pointers (no `cv::cuda::GpuMat` anywhere) — this drops the OpenCV dependency entirely for this module.
**Edge cases**: Preserve the prototype's invalid-sample skip logic (`depthVal <= 0` treated as invalid, matches the RVL `0=invalid` convention already used on the wire — convenient consistency). Retune `radius`/`sigmaSpatial`/`sigmaColor` defaults for the actual 256×192→target-resolution ratio (the prototype's defaults produce a near-nearest-neighbor effective window at this ratio per the earlier research finding — start with a larger `radius` and validate visually in D5, don't just keep the prototype's demo values of `radius=6, sigma_spatial=7.0, sigma_color=25.0` unexamined).
**Verification**: Standalone test harness (`.cu` or `.cpp` test target, can live outside the main Mikan build initially) loading `CudaDepthUpsample`'s `data/depth_low.pfm`/`data/rgb_full.png` test assets (convert PFM float depth to a synthetic `uint16_t` mm buffer for this test), running the new `uint16_t` kernel path, and visually/numerically comparing output against the prototype's original `float` kernel output on the same inputs (should be close but not necessarily identical given the different input precision).

## D2 — Confidence-weighted upsampling
**Goal**: Use ARKit's confidence plane (from B4) to deweight/skip low-confidence depth samples in the kernel's neighbor gather.
**Files**: `Cuda/JBUKernel.cu` (extend D1).
**Steps**: Add an optional `const uint8_t* d_confidence` parameter; in the neighbor-gather loop, either skip samples with `confidence == 0` (treat as invalid, same as depth `<=0`) or apply a soft multiplicative weight (e.g. `confidence==2 ? 1.0 : confidence==1 ? 0.5 : 0.0`) — implement the soft-weight version since it degrades better than a hard cutoff, but expose the low-confidence threshold as a tunable parameter for D5's tuning pass.
**Edge cases**: All-low-confidence input frame (degenerate case) — ensure the existing "all neighbors invalid → output 0.0" fallback (from the prototype) still applies rather than producing NaN/garbage from an all-zero-weight division.
**Verification**: Extend D1's test harness with synthetic confidence maps (all-high, all-low, mixed) and confirm output degrades sensibly (low-confidence regions produce visibly smoother/more-averaged or zeroed output vs. high-confidence regions retaining detail).

## D3 — Async execution, error handling, GPU timing
**Goal**: Remove the prototype's blocking/`exit(-1)` behavior; make it safe for a long-running app.
**Files**: `Cuda/JBUKernel.cu/.h`, new `Cuda/CudaErrorHandling.h`.
**Steps**: Replace the `CUDA_SAFE_CALL`-with-`exit(-1)` macro with one that throws a caught C++ exception (or returns an error code checked by the caller) surfaced through Mikan's normal error-logging path. Remove the blocking `cudaDeviceSynchronize()` call from the hot path — launch on a dedicated `cudaStream_t` owned by the device, and use `cudaEventRecord`/`cudaStreamSynchronize` (or a fence checked on the next tick, non-blocking) only at the point where the result is actually consumed (D4's GL writeback). Add `cudaEvent_t` start/stop timing around the kernel launch, exposed as a stat (mirrors A13's telemetry spirit, on the Mikan side).
**Edge cases**: A CUDA error mid-session (e.g. driver hiccup, out-of-memory) must not crash the whole app — catch, log, and skip that frame's depth upsample (fall back to "no depth this frame" rather than terminating).
**Verification**: Inject a deliberate CUDA error (e.g. an invalid launch config) in a test build and confirm it's caught/logged rather than calling `exit()`; confirm timing numbers are logged and look reasonable (sub-frame-budget, e.g. well under 16ms for 30fps headroom) via the D1 test harness.

## D4 — CUDA-GL interop for zero-copy texture writeback
**Goal**: Write the JBU output directly into an `IMkTexture`'s underlying GL texture with no host round-trip.
**Files**: New `src/Plugins/MikanARKitVideo/Private/Cuda/CudaGLInterop.h/.cpp`.
**Steps**: Create an `IMkTexture` with `MK_R32F` format at target resolution (via the existing `CreateMkTexture()` factory in `MikanRenderer`). Register its underlying GL texture handle with `cudaGraphicsGLRegisterImage` (must happen on the thread holding the current GL context — coordinate with wherever Mikan's render-thread frame-processing tick lives, e.g. alongside `VideoFrameDistortionView::processVideoFrame()`'s existing render-thread marshaling pattern). Each frame: `cudaGraphicsMapResources` → get a `cudaArray_t` via `cudaGraphicsSubResourceGetMappedArray` → launch the JBU kernel (D1-D3) writing into that array (via a `cudaSurfaceObject_t` bound to the array, since direct kernel writes need a surface, not a raw device pointer, when the destination is a `cudaArray_t`) → `cudaGraphicsUnmapResources`.
**Edge cases**: Registration/mapping *must* occur on the GL-context-owning thread — if the depth-receive worker thread (B4) triggers this work, it must hand off to the render thread rather than calling CUDA-GL interop APIs directly from the wrong thread (a common source of silent driver-level failures). Texture resize (if target resolution ever changes) requires unregister+recreate+reregister, not a resize-in-place.
**Verification**: With Track C's live pipeline feeding a guide image and Track B4 feeding real depth data, confirm the resulting `IMkTexture` visibly contains a plausible upsampled depth map when sampled/visualized (e.g. temporarily wire it into an existing debug-texture-display path, or dump the texture to a PNG for visual inspection) rather than blank/garbage data.

## D5 — Parameter tuning pass
**Goal**: Empirically retune `radius`/`sigmaSpatial`/`sigmaColor`/confidence-threshold for the real 256×192→target-resolution use case, since the prototype's defaults were never validated at this ratio.
**Files**: Expose these as constructor/runtime parameters on the JBU wrapper (already true from D1-D2); no new files, just a tuning exercise.
**Steps**: Using D4's live pipeline output (or D1's offline harness with real captured ARKit test data if live isn't ready yet), sweep `radius` and `sigma` values, visually comparing edge-preservation vs. noise/flicker in the upsampled output against the raw low-res input.
**Edge cases**: N/A (empirical tuning).
**Verification**: Document the chosen final defaults with a brief rationale in a code comment; confirm no visible flickering/instability when the same static scene is captured repeatedly (this directly addresses the user's original flicker concern that motivated choosing lossless RVL — tuning here should preserve that lossless-source benefit rather than reintroducing instability via an overly aggressive spatial blend).

---

# Track E: Mikan — ECS / GUI / Camera Integration

## E1 — `ARKitVideoSourceComponent`/`Definition`
**Goal**: The ECS component wiring the plugin (Tracks C+D) into Mikan's video-source system, modeled on `NetworkVideoSourceComponent`.
**Files**: New `src/Editor/ECS/VideoSource/ARKitVideoSourceComponent.h/.cpp`, `ARKitVideoSourceDefinition` (same file or a sibling, matching the `NetworkVideoSourceComponent`/`NetworkVideoSourceDefinition` split).
**Steps**: Implement the `VideoSourceComponent` abstract interface (`getDevicePath`, `getDeviceAPI`, `openVideoSource`/`closeVideoSource`, `getVideoStreamingStatus`, protected `startVideoStreamInternal`/`stopVideoStreamInternal`). Hold properties: `basePort`, `depthStreamingEnabled`, JBU tuning params (`sigmaSpatial`, `sigmaColor`, `radius`, confidence threshold — from D5). Own an `IARKitVideoDevicePtr` (from B7/C1) and implement `IARKitVideoDeviceListener::notifyFrameBundleReceived` to push color frames into the existing `writeVideoFrame(...)` path (inherited from `VideoSourceComponent`) and depth frames into a new depth-texture slot (Track E3). Add `eVideoSourceType::ARKit` to `ProjectConfigConstants.h` and route it through `VideoSourceQueries`.
**Edge cases**: Component destruction while the device is mid-open (async) must not use-after-free the listener callback — follow whatever teardown-ordering guard the existing `NetworkVideoSourceComponent` uses (check its destructor/close-sequencing).
**Verification**: Add the component to a test project, open/close cycle it repeatedly through the editor UI (once E2 exists) or programmatically, confirm no leaks/crashes and that `getVideoStreamingStatus()` reflects the real device state.

## E2 — `GuiPanel_ARKitVideoSourceComponent`
**Goal**: Settings UI panel, modeled on `GuiPanel_NetworkVideoSourceComponent`.
**Files**: New `src/Editor/AppStages/Shared/GuiPanel_ARKitVideoSourceComponent.h/.cpp`, plus routing in `GuiPanel_ProjectSources.cpp` (add the `eVideoSourceType::ARKit` case alongside the existing USB/Network branches).
**Steps**: Reuse the declarative `IPropertyInterface`/`initTypedPropertyInterface<ARKitVideoSourceComponent>()` pattern; expose `basePort` and `depthStreamingEnabled` as primary fields, JBU tuning params under a collapsible "Advanced" section, and a compact-mode summary (name + port) mirroring `GuiPanel_NetworkVideoSourceComponent::drawCompactGui()`.
**Edge cases**: Property changes must go through the existing deferred-GUI-event mechanism (`addDeferredGuiEvent`) to avoid mutating state mid-ImGui-frame, exactly as the network panel does.
**Verification**: Manual editor test — add an ARKit video source via the project sources UI, confirm all fields render, edit values, confirm they persist to the project config and take effect (e.g. changing `basePort` and reconnecting actually binds the new port).

## E3 — Depth texture exposure to the compositor
**Goal**: Make the upsampled depth (Track D4's `IMkTexture`) available to the node-graph compositor.
**Files**: Extend `src/Editor/Calibration/VideoFrameDistortionView.h/.cpp` (or add a lightweight sibling class if the existing undistortion machinery is too heavyweight/inapplicable for depth — decide based on reading that class in this ticket, don't assume), new node variant in `src/Editor/NodeEditors/Nodes/` mirroring `DepthTextureSourceNode`/`eTextureSourceDepthType`.
**Steps**: Add a depth-texture slot alongside the existing color texture slot, populated by `E1`'s frame-bundle handler. Add a new `eTextureSourceDepthType` (or equivalent) enum value / node-graph node so the ARKit depth can be selected as an input to existing occlusion-masking nodes (`DepthMaskNode`) the same way Mikan's own rendered depth is today.
**Edge cases**: Depth-disabled sessions must expose *no* depth texture (or a clearly "unavailable" state) rather than a stale/blank one — the node graph should handle a missing depth source gracefully (this may already be true of `DepthMaskNode` for other reasons; verify).
**Verification**: Manual editor test — wire the new depth-texture-source node into a `DepthMaskNode` in a test compositor graph, confirm occlusion masking visibly responds to real depth data from a live ARKit session.

## E4 — `IFrameCoupledPoseProvider` + `CameraComponent` integration
**Goal**: Let `CameraComponent` pull frame-exact ARKit pose instead of composite-time VR-tracker sampling, for cameras whose tracking mount is an ARKit source.
**Files**: New interface in `src/Editor/ECS/Camera/IFrameCoupledPoseProvider.h` (or colocated with `CameraComponent.h`), implemented by `ARKitVideoSourceComponent` (extend E1), modify `src/Editor/ECS/Camera/CameraComponent.cpp` (`makeNewCameraFrameEvent`, `CameraComponent.cpp:540-611`).
**Steps**: Define `IFrameCoupledPoseProvider { virtual bool getPoseForFrame(int64_t frameIndex, glm::mat4& outTransform, MikanVideoSourceIntrinsics& outIntrinsics) = 0; }`. In `ARKitVideoSourceComponent`, maintain a small ring buffer of recent `{frameIndex, transform, intrinsics}` (populated from B6's bundles as they're consumed) so `getPoseForFrame` can look up a specific past frame index. In `CameraComponent::makeNewCameraFrameEvent`, before falling back to the existing `VRDevicePoseView`/`trackingMountId` sampling, check whether the resolved tracking-mount component implements `IFrameCoupledPoseProvider`; if so and `getPoseForFrame(frameIndex, ...)` succeeds, use that pose+intrinsics directly instead. Snapshot intrinsics on first frame, only refresh on a "significant change" threshold (avoid recomputing `recomputeCameraProjectionMatrix()` every frame unnecessarily) — define "significant" as e.g. >1% relative change in `fx`/`fy`.
**Edge cases**: Must be **fully backward compatible** — non-ARKit tracking mounts (SteamVR pucks etc.) must take the exact same code path as today, with zero behavior change. If `getPoseForFrame` fails (frame index not found — evicted from the ring, or that video frame never got a matching pose from B6), fall back to the existing VR-tracker sampling rather than leaving the camera pose stale/uninitialized.
**Verification**: Regression-test the existing non-ARKit camera-pose path is unaffected (run an existing SteamVR-tracked-camera test project, confirm identical behavior before/after this change). New test: an ARKit-tracked camera in a live session shows pose updates tightly synced to video frames (visually check for the elimination of the lag/skew artifacts the existing `trackingFrameDelay` hack was compensating for).

---

# Track F: End-to-End Verification

## F1 — Full pipeline integration test
**Goal**: Validate the complete iPhone-to-Mikan pipeline together, after all prior tracks are individually verified.
**Steps**:
1. `gst-inspect-1.0 nvh264dec` / `gst-inspect-1.0 nvcodec` on the target Mikan machine to confirm hardware decode availability (hard prerequisite per the locked-in NVIDIA-first decision).
2. Run the iPhone app (Track A) against a live Mikan instance with an ARKit video source configured (Track E2): confirm color video, upsampled depth (visualized via a debug view or the E3 compositor wiring), and camera pose all appear correctly synced.
3. Stress test: walk around with the phone to generate real pose motion + varied depth scenes, confirm no visible pose/video desync and no depth flicker (the original motivating concern for choosing lossless RVL).
4. Network resilience test: briefly disable WiFi on the phone mid-session, confirm Mikan's stream-timeout watchdog (reused from `MikanGStreamerVideoDevice`'s pattern) detects the gap and the session recovers cleanly on reconnect, without requiring an app restart on either side.
5. Long-duration soak test (e.g. 30+ minutes continuous streaming) to catch slow leaks (CUDA memory, GStreamer buffer accumulation, correlation-map growth from B6/B4 if staleness sweeping has a bug) that shorter tests wouldn't surface.
**Verification**: All of the above pass without crashes, visible desync, or resource growth over the soak test duration.