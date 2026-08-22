# Depth Proxy Mesh Capture

How Mikan turns one captured video frame into a metric, camera-space proxy mesh and registers it as
a model stencil (shadow catcher) - replacing hand-placed proxy geometry with a single capture from
the camera's point of view.

The model measurements that justify this design (silhouette alignment, FOV sensitivity, normals
quality) live in [scene-lighting.md](./scene-lighting.md); this doc covers the pipeline itself.

---

## The model

One checkpoint: **MoGe-2 ViT-L with the normal head** (`Ruicheng/moge-2-vitl-normal`), run
in-process through ONNX Runtime. MIT licensed, code and weights. Unlike Marigold there is no local
export step - the authors publish an official ONNX export (`Ruicheng/moge-2-vitl-normal-onnx`,
opset 14, single 1.3GB `model.onnx`), fetched by `tools/fetch_moge2_onnx.py` into `models/moge2/`.

The graph takes plain RGB in [0,1] at **native resolution** (ImageNet normalization is baked in)
plus a scalar `num_tokens` (1200..3600), and returns:

| Output | Meaning |
|---|---|
| `points` | AFFINE point map - camera space up to an unknown Z shift and global scale |
| `normal` | unit normals, directly predicted (not derived from the depth) |
| `mask` | validity probability, threshold at 0.5 |
| `metric_scale` | scalar taking the shifted geometry to metres |

Verified behavior on this machine: the graph runs under both the CPU and DirectML execution
providers with **identical output** (0.000% depth difference), DirectML ~7x faster (1.24s vs 8.9s
steady-state at 832x480, 3600 tokens). The ONNX pipeline matches the PyTorch reference to 0.04%
depth / 0.03 degrees of normal error.

`tools/moge2_onnx_pipeline.py` is the **executable specification for the C++ port**
(`MoGeInference`): preprocessing, the forward pass, and the metric recovery below map 1:1. The C++
implementation was validated against it per-vertex to 0.001% at p99.

## Metric recovery needs the calibrated FOV

The point map is affine; recovering metric camera space takes three steps
(`MoGeInference::run`):

1. **Focal from the calibrated horizontal FOV**, closed form. Mikan always has this
   (`MikanMonoIntrinsics::hfov` describes the undistorted image, which is also what the model is
   fed), so the reference implementation's focal-*estimation* path is deliberately not ported.
   This is not a nicety: metric scale rides directly on the assumed FOV - a 45 degree guess
   against a ~56 degree truth shifts depth 22-36%.
2. **Z shift** via a 1-D least squares `min |focal * xy/(z+shift) - uv|` over the point map
   nearest-downsampled to 64x64. One trap, found by unit test: every denominator `z + shift` is a
   depth and must be positive at the solution, but the raw affine `z` can be negative - starting
   the solve at zero then puts a pole between the start and the answer and the iteration stalls
   against it. `solveDepthShift` starts just past the largest pole instead. (The reference starts
   at zero unconditionally; the two agree wherever the reference converges.)
3. **Scale** by `metric_scale`, then the point map is rebuilt from depth + intrinsics so points
   reproject exactly (`force_projection` in the reference).

Everything `MoGeInference::Result` exposes is converted to **Mikan camera space** (+X right, +Y up,
+Z toward the viewer; visible geometry at negative Z) at the API boundary - the raw model output is
OpenCV convention (+Y down, +Z forward) and the Y/Z negation lives in exactly one place. Invalid
pixels carry +inf depth and a ZERO normal, which downstream unit-length checks reject for free.

## Mesh generation

`DepthMeshGenerator` builds a regular grid over the depth map (default stride 4) and emits two
triangles per cell, **cutting the mesh at depth discontinuities**: a triangle is dropped when any
of its corners' pairwise depth ratio exceeds `discontinuityRatio` (default 1.15). Connecting a
silhouette to the background would create stretched skirt triangles that catch shadows in mid-air;
measured on the reference plates, the model's depth edges land within ~1px of the image silhouettes
and commit to near-or-far rather than averaging, so ratio culling is sufficient and the mesh
cleanly separates at object boundaries.

Geometry beyond `maxDepth` (default 20m) is dropped - distant background adds triangles but cannot
receive a meaningful contact shadow.

The OBJ written per capture carries positions (camera-space metres), per-vertex normals, and
video-frame UVs (so the plate or a live feed can be projected back onto the proxy). No `.mtl`: the
in-editor importer supplies a default material when a file declares none.

## The capture tool

`AppStage_DepthMeshCapture`, reached from a camera's **Capture Depth Mesh** component function
(registered through `IFunctionInterface`, so also reachable from Lua and remotely). Same shape as
the scene-lighting capture stage:

pick camera -> live video while framing -> **Capture** runs the model -> the panel shows mesh
statistics and a depth overlay (red = near, blue = far) drawn over the frame so silhouette
alignment can be judged -> **Create Stencil** writes the mesh and registers it.

Failure modes are loud and specific, in the order they are checked: model missing from
`models/moge2` (fetch tool named in the message), no video frame, no calibrated intrinsics, and -
only at Create Stencil time - no tracked camera pose and no loaded project.

What Create Stencil does:

- Writes `<project>/models/DepthProxy_<timestamp>.obj`. The name is unique per capture because the
  model resource cache keys on the file path - regenerating under a reused path would serve the
  stale mesh.
- Creates a `ModelStencilComponent` through `ModelStencilSystem::addNewObjectByTypedDefinition`
  with the **absolute** OBJ path (the importer loads the stored path verbatim; a relative one
  silently fails to resolve).
- Sets the stencil's world transform to the capturing camera's `getStageSpaceAperturePose`. The
  vertices are camera-local, so the stencil rides the camera pose and the OBJ stays reusable.

Nothing further is needed for connected clients: creating the stencil fires the component-id-list
property event, clients refetch the stencil list, and the render geometry ships over the wire via
`GetModelStencilRenderGeometry` - the Unreal actor's shadow mesh component picks the proxy up
automatically.

The ONNX session is owned by the stage (loaded on first capture, freed on exit), and inference
blocks the UI for about a second on GPU - same deliberate choice, and same CPU-fallback warning, as
the lighting capture stage.

## Headless use

```
MikanCmd -depthMesh -image=<path> -fov=<degrees> [-obj=<path>] [-stride=<n>] [-maxDepth=<m>] [-cpu]
```

Prints the mesh statistics and writes the OBJ. This is also the validation path against
`tools/moge2_onnx_pipeline.py`.

Unit tests (`MikanCmd -runTests`, `depth_mesh_generator` module) cover the shift solver (including
the negative-z pole case), discontinuity culling, mask/max-depth rejection, and OBJ consistency.

## Known limits

- **Thin structures and transparency** reconstruct poorly (wheat stalks fuzz, frosted glass leans
  toward the surface behind it) - both visible in the eval renders under `build/moge2_eval/`.
- **Single viewpoint**: surfaces the camera cannot see do not exist in the proxy. Disocclusions
  are holes, not hallucinated geometry - correct for a shadow catcher, insufficient for occluding
  a character walking behind real furniture (occlusion-grade edges are a separate problem).
- **Residual metric error**: with the calibrated FOV supplied, what remains is the model's own
  scale error (typically single-digit percent). If it matters, the tracked floor plane or visible
  anchors give ground-truth depths to fit a residual scale against - deferred until the raw
  output demonstrably isn't good enough.
