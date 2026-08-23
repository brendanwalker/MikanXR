# Compositor

How MikanXR produces the mixed-reality composite: undistorted camera video layered with client-rendered frames and masked by stencils, evaluated by a node graph every video frame. Video capture itself is covered in [videosources.md](./videosources.md), the client protocol in [wire-protocol.md](./wire-protocol.md), the ECS object model in [objects.md](./objects.md), and the node graph machinery in [scripting.md](./scripting.md).

---

## What the compositor produces

The compositor is an ECS component: `CompositorComponent` in `src/Editor/ECS/Compositor/`, owned by `CompositorObjectSystem`. Each compositor binds one camera (`getCameraComponent()`, which supplies the video source) and one owner scene. Its output is an 8-bit sRGB RGBA texture, sized to the video frame, exposed via `CompositorComponent::getCompositedFrameTexture()`.

Compositing is driven by a `CompositorNodeGraph` (`src/Editor/NodeEditors/Graphs/CompositorNodeGraph.h`), a node graph asset referenced by path from the compositor's definition. There is no fixed layer stack; the "layer model" is whatever chain of draw nodes the graph author wires off the graph's `OnCompositeFrame` event node. A typical graph draws the camera video first, then one or more client layers on top.

`CompositorDefinition` (the persisted config) stores: `camera_id`, `owner_scene_id`, `compositor_graph_path` (a `NodeGraphAssetReference`), `spout_enable_output`, and `spout_output_name`. It round-trips through Configuru JSON (`writeToJSON`/`readFromJSON`) with the rest of the project config, and every field is also remotely readable/writable through the property system (`getPropertyValue`/`setPropertyValue`, see [wire-protocol.md](./wire-protocol.md)).

---

## Frame pacing and the frame event queue

`CompositorComponent::update()` runs during `ProjectManager::update()` each tick and does two things, in order:

- `tryCompositeOldestFrame()`: composites the frame at the front of `m_frameEventQueue`, but only once the queue is full (`VideoFrameDistortionView::getMaxFrameQueueSize()`, from the video source's frame queue size setting) and a new video frame has arrived. Filling the queue first maximizes the time clients have to render each frame; gating on new video frames locks the composite rate to the source frame rate.

- `tryEnqueueNewFrame()`: calls `VideoFrameDistortionView::readAndProcessVideoFrame()` to pull and undistort the next camera frame, builds a `MikanCameraNewFrameEvent` (frame index plus camera pose/intrinsics), pushes it on the queue, and publishes it to all clients via `CameraRequestHandler::publishCameraNewFrameEvent()`.

So each frame makes a round trip: the editor announces frame N with a camera pose, clients render their scene from that pose and submit textures, and the editor composites frame N several ticks later when the queue cycles around. `getPendingCompositedFrameIndex()` is the frame currently awaiting composite; `OnNewFrameComposited` fires after each composite. If the queue overflows, oldest frames are dropped with an error log.

Video always comes through a `VideoFrameDistortionView` in `eVideoFrameProcessorMode::COMPOSITOR` mode with `eVideoDisplayMode::mode_undistored`. Compositing always uses the undistorted frame (see [videosources.md](./videosources.md)).

---

## How client frames arrive

Client-rendered frames never travel over the websocket. Clients allocate a shared GPU texture (Spout2-based, `MikanSharedTexture` library, see [modules.md](./modules.md)) and the editor reads it with a `SharedTextureReadAccessor` (`src/Editor/Interprocess/SharedTextureReader`). `ClientSourceManager` (`src/Editor/ECS/TextureSource/ClientSourceManager.h`) listens for `MikanServer` render-target allocate/update/release events and keeps a per-client `ClientTextureFrameQueue` (default depth 3) keyed by client id and camera id, so a texture for frame N is still available when the compositor gets around to compositing frame N.

In the scene, a `ClientTextureSourceComponent` (a `TextureSourceComponent` subclass) names a client source; in the graph, `ColorTextureSourceNode` and `DepthTextureSourceNode` reference a texture source component and fetch the client texture for the pending frame index. `ColorTextureSourceNode` also carries `eTextureSourceColorType` (colorRGB/colorRGBA plus multiplicative shadowRGB/shadowRGBA variants) and an author-selected fallback (`eColorTextureFallbackMode`: transparent black, opaque black, opaque white, or auto by type) emitted when no client texture is available yet. The fallback is author-selected per node rather than inferred because the correct identity depends on the downstream layer's material and blend mode, which C++ cannot introspect (materials are GLSL shaders): an inverted-alpha "normal" layer needs opaque black so the inverted output alpha is zero, while a multiply layer needs white. A wrong choice paints over the video (a transparent-black fallback through an inverted-alpha shader composites as opaque black). `autoByType` preserves the legacy per-color-type behavior and is the default for graphs saved before the option existed. `CompositorNodeGraph::gatherAllReferencedClientSourceIDs()` walks the graph to report which client sources it consumes.

---

## The compositor node graph

`CompositorNodeGraph::compositeFrame(NodeEvaluator&)` is the per-frame entry point, called from `CompositorComponent::evaluateCompositorNodeGraph()`. It:

- resizes and (re)creates two framebuffers to the video dimensions: `m_compositingFrameBuffer` (RGBA16, a linear working buffer, since compositing in linear space keeps blend math correct and the extra depth avoids banding) and `m_outputFrameBuffer` (8-bit sRGB output);
- binds the working framebuffer with depth test disabled and runs `NodeEvaluator::evaluateFlowPinChain()` from the `OnCompositeFrame` `EventNode`;
- runs `encodeLinearFrameToSRGB()`, a fullscreen pass with the internal `Internal_PT_LinearToSRGB` material, into the output buffer.

Node types the graph can spawn (registered in the `CompositorNodeGraph` constructor): `EventNode`, `DrawLayerNode`, `DrawShapesNode`, `DepthMaskNode`, `ApplyMaterialNode`, `ColorTextureSourceNode`, `DepthTextureSourceNode`, `VideoTextureNode`, `TextureNode`, `MaterialNode`, `StencilNode`, `StencilSelectNode`, `ShapeNode`, `ShapeSelectNode`, `ArrayNode`, `MousePosNode`, `TimeNode`. `VideoTextureNode` yields the camera texture (raw or distortion-corrected, `eVideoTextureSource`). Evaluation errors are collected as `NodeEvaluationError` values; `CompositorComponent::getLastNodeEvalErrors()` exposes them and the node editor window displays them.

`DrawLayerNode` is the core layer primitive. It takes a material (`GraphMaterialProperty`) and an optional stencil array, then draws a fullscreen quad (`getLayerMesh()`, or the v-flipped variant for sources with inverted texture coordinates) with that material. Its input pins are rebuilt dynamically from the bound material's shader uniforms: each float/vec2/vec3/vec4/texture uniform becomes a pin, with per-node default values persisted in `DrawLayerNodeConfig`. Per-node options: `eCompositorBlendMode` (off, normal `SRC_ALPHA/ONE_MINUS_SRC_ALPHA`, multiply `DST_COLOR/ZERO`), `eCompositorStencilMode` (none/inside/outside), vertical flip, and invert-when-camera-inside.

---

## Stencils

Stencils are scene objects (`QuadStencilComponent`, `BoxStencilComponent`, `ModelStencilComponent` with their object systems, see [objects.md](./objects.md)) that constrain where a layer draws. Before drawing its layer quad, `DrawLayerNode` rasterizes the relevant stencil geometry into the OpenGL stencil buffer with color and depth writes masked off (`evaluateQuadStencils`/`evaluateBoxStencils`/`evaluateModelStencils`), using the bound camera's aperture view-projection matrix so stencils are placed exactly as the physical camera sees them. The stencil mode then selects drawing inside or outside that mask.

Supporting machinery in `CompositorNodeGraph`: unit quad/box meshes for quad and box stencils, plus `getOrLoadStencilRenderModel()` which loads a model stencil's OBJ through `MikanModelResourceManager` and caches it per stencil id (`m_stencilMeshCache`); the cache entry is flushed when the stencil's model path changes. A parallel depth-model cache (`getOrLoadDepthRenderModel()`, `Internal_P_LinearDepth` material) supports depth masking. Quad stencils that are double-sided support the "magic portal" inversion: when the camera is behind every double-sided stencil quad, the mask logic inverts (`m_bInvertWhenCameraInside`).

---

## Materials and shaders

Layer materials are `MkMaterial` objects from the `MikanRenderer` OpenGL abstraction (see [modules.md](./modules.md)), managed by `MikanShaderCache` (`src/Editor/Renderer/MikanShaderCache.h`). Two populations exist:

- Built-in internal materials compiled from embedded GLSL in `GlShaderCache.cpp`, named by the `INTERNAL_MATERIAL_*` constants in `IMkShaderCache.h` (fullscreen texture blits, linear/sRGB conversion, linear depth, solid color, NV12 conversion, depth packing, etc.).

- User materials loaded from `*.mat` files via `MaterialAssetReference` and `MikanShaderConfig`: a Configuru config naming the material, vertex/fragment shader file paths, a uniform-to-semantic map, and vertex attributes. These are what `MaterialNode`/`GraphMaterialProperty` feed into `DrawLayerNode`, and their uniforms become the node's dynamic pins.

Draw state (blend, stencil, masks, viewport) is managed through the scoped `MkStateStack`/`MkScopedState` system rather than raw GL calls.

---

## Outputs

- **Editor display.** `CompositorOutputEditorWindow` (`src/Editor/NodeEditors/Windows/`) is a dedicated output window that draws the composited frame texture on a fullscreen quad over the bound scene. Because this window is meant to show what the shot looks like, the editor's authoring overlay is suppressed in it unless the `debug_render_in_compositor` editor setting ("Debug Render in Compositor", off by default) is on; the per-object "Render X" flags then still apply on top of it. Scene instances are cleared each frame either way, so toggling the overlay off cannot strand renderables belonging to deleted objects. `CompositorComponent::renderToViewportQuad()` is a helper that blits the composited texture into the current viewport with the fullscreen RGB material (no in-tree caller found, unverified use).

- **Spout.** When `spout_enable_output` is set, `startOutputStreaming()` creates an `ISharedTextureWriteAccessor` (`createSharedTextureWriteAccessor`, Spout2/`MikanSharedTexture`) named by `spout_output_name` (default `DEFAULT_SPOUT_OUTPUT_NAME`), RGBA32/OpenGL, and each composited frame is pushed via `writeColorFrameTexture()`. OBS or any Spout receiver can pick this up. Toggling the definition properties starts/stops the sender live.

- **Video recording.** No video-file recorder currently exists in the compositor path; `eSupportedCodec` (MP4V/MJPG/RGBA) in `CompositorConstants.h` is defined but unreferenced outside that file.

---

## Frame anatomy

Where a frame goes, from `App::exec` (`src/Editor/AppCore/App.cpp`) down:

1. `App::exec` loops `tick()` under a `FrameTimer(11)` (~90 fps cap). `App::tick` clamps `deltaSeconds` to 0.1 s, pumps the CEF message loop, polls SDL events, then calls `tickWindows()`.
2. `tickWindows` runs `update(deltaSeconds)` then `render()` for each `EditorWindow` (main window plus any node editor / compositor output windows).
3. `MainWindow::update`: `MikanServer::update()` first (websocket socket events and queued client requests, which is also where client render-target updates land in `ClientSourceManager`), then `LuaDebugServer::poll()`, SDL event handling, and `ProjectManager::update()`, which ticks the object systems, running `CompositorComponent::update()` (composite oldest frame, then enqueue/publish the next camera frame). Finally the current `AppStage` GUI and update.
4. `MainWindow::render`: renders each enabled `MikanViewport` for the app stage, then the UI, then presents. The compositor's GL work happens during the update phase (inside node graph evaluation), not here; render only displays already-composited textures.
