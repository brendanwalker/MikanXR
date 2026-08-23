"""MoGe-2 inference in ONNX Runtime + numpy only, no torch.

This is the executable specification for the C++ port, in the same sense
`marigold_onnx_pipeline.py` is for the Marigold pipeline: preprocessing, the
single forward pass, and the post-network recovery of metric camera-space
geometry all map 1:1 to what the C++ implementation must do.

The network (Ruicheng/moge-2-vitl-normal-onnx, opset 14) takes
    image      [B, 3, H, W] float32, plain RGB in [0, 1] at native resolution
               (ImageNet normalization is baked into the graph)
    num_tokens []           int64 scalar, 1200..3600 (more = finer + slower)
and returns
    points       [B, H, W, 3] AFFINE point map - camera space up to an
                 unknown Z shift and a global scale
    normal       [B, H, W, 3] unit normals, OpenCV camera convention
                 (+X right, +Y down, +Z forward)
    mask         [B, H, W]    sigmoid probability, threshold at 0.5
    metric_scale [B]          scale factor to metres

Post-network recovery, given the CALIBRATED horizontal FOV (Mikan always has
it, so the focal-estimation path of the reference implementation is
deliberately not ported):

    1. focal (relative to half the image diagonal) from fov_x, closed form.
    2. Z shift: 1-D least squares `min |focal * xy/(z+shift) - uv|` over the
       point map nearest-downsampled to 64x64 (masked). Levenberg-Marquardt
       from shift=0; the problem is smooth and converges in a few steps.
    3. depth = (z + shift) * metric_scale; pixels with depth <= 0 leave the
       mask. The point map is then RECOMPUTED from depth + intrinsics
       (force_projection in the reference) so points exactly reproject.

The normals need no postprocessing and are independent of shift/scale.

Developer tool. Not shipped.
"""

import argparse
import time

import cv2
import numpy as np

NUM_TOKENS_RANGE = (1200, 3600)  # from the checkpoint's num_tokens_range


def normalized_view_plane_uv(width, height):
    """UV grid with corners at (+-w/diagonal, +-h/diagonal), pixel centers."""
    aspect_ratio = width / height
    span_x = aspect_ratio / (1.0 + aspect_ratio**2) ** 0.5
    span_y = 1.0 / (1.0 + aspect_ratio**2) ** 0.5
    u = np.linspace(-span_x * (width - 1) / width, span_x * (width - 1) / width,
                    width, dtype=np.float32)
    v = np.linspace(-span_y * (height - 1) / height, span_y * (height - 1) / height,
                    height, dtype=np.float32)
    return np.stack(np.meshgrid(u, v, indexing="xy"), axis=-1)


def focal_from_fov_x(fov_x_degrees, aspect_ratio):
    """Focal length relative to half the image diagonal."""
    return aspect_ratio / (1.0 + aspect_ratio**2) ** 0.5 \
        / np.tan(np.radians(fov_x_degrees) / 2.0)


def solve_shift(points, mask, focal, downsample=(64, 64), iterations=20, ftol=1e-3):
    """1-D Levenberg-Marquardt for the Z shift that makes the affine point map
    reproject through the known focal. Mirrors moge solve_optimal_shift."""
    points_lr = cv2.resize(points, downsample, interpolation=cv2.INTER_NEAREST)
    mask_lr = cv2.resize(mask.astype(np.uint8), downsample,
                         interpolation=cv2.INTER_NEAREST).astype(bool)
    uv_lr = cv2.resize(normalized_view_plane_uv(points.shape[1], points.shape[0]),
                       downsample, interpolation=cv2.INTER_NEAREST)

    xy = points_lr[mask_lr][:, :2]
    z = points_lr[mask_lr][:, 2]
    uv = uv_lr[mask_lr]
    if z.size < 2:
        return 0.0

    def residuals(shift):
        return (focal * xy / (z + shift)[:, None] - uv).ravel()

    # Start just past the largest pole: every denominator z + shift must be
    # positive at the solution (it IS the depth), and starting at 0 with
    # negative z present puts a singularity between the start and the answer.
    shift = max(0.0, float(-z.min()) + 1e-3 * max(1.0, float(z.max() - z.min())))
    damping = 1e-3
    cost = float((residuals(shift) ** 2).sum())
    for _ in range(iterations):
        r = residuals(shift)
        jacobian = (-focal * xy / ((z + shift) ** 2)[:, None]).ravel()
        gradient = float(jacobian @ r)
        hessian = float(jacobian @ jacobian)
        step = -gradient / (hessian * (1.0 + damping))
        new_cost = float((residuals(shift + step) ** 2).sum())
        if new_cost < cost:
            shift += step
            if abs(cost - new_cost) < ftol * cost:
                break
            cost, damping = new_cost, max(damping * 0.3, 1e-9)
        else:
            damping *= 10.0
    return shift


def postprocess(points, normal, mask_probability, metric_scale, fov_x_degrees):
    """Affine network output -> metric camera-space maps. Returns a dict of
    depth / points / normal / mask / intrinsics (normalized)."""
    height, width = points.shape[:2]
    aspect_ratio = width / height
    mask = mask_probability > 0.5

    focal = focal_from_fov_x(fov_x_degrees, aspect_ratio)
    shift = solve_shift(points, mask, focal)

    depth = points[..., 2] + shift
    mask &= depth > 0.0
    depth = depth * metric_scale

    # Normalized intrinsics (fx in units of width, fy of height, center 0.5).
    fx = focal / 2.0 * (1.0 + aspect_ratio**2) ** 0.5 / aspect_ratio
    fy = focal / 2.0 * (1.0 + aspect_ratio**2) ** 0.5
    intrinsics = np.array([[fx, 0.0, 0.5], [0.0, fy, 0.5], [0.0, 0.0, 1.0]],
                          dtype=np.float32)

    # force_projection: rebuild the point map from depth so points reproject
    # exactly through the calibrated intrinsics.
    u = (np.arange(width, dtype=np.float32) + 0.5) / width
    v = (np.arange(height, dtype=np.float32) + 0.5) / height
    uu, vv = np.meshgrid(u, v, indexing="xy")
    points_metric = np.stack([(uu - 0.5) / fx * depth, (vv - 0.5) / fy * depth,
                              depth], axis=-1)

    return {
        "depth": np.where(mask, depth, np.inf).astype(np.float32),
        "points": np.where(mask[..., None], points_metric, np.inf).astype(np.float32),
        "normal": np.where(mask[..., None], normal, 0.0).astype(np.float32),
        "mask": mask,
        "intrinsics": intrinsics,
        "shift": float(shift),
    }


def run_onnx(session, rgb_u8, fov_x_degrees, num_tokens=NUM_TOKENS_RANGE[1]):
    image = (rgb_u8.astype(np.float32) / 255.0).transpose(2, 0, 1)[None]
    points, normal, mask_probability, metric_scale = session.run(
        ["points", "normal", "mask", "metric_scale"],
        {"image": image, "num_tokens": np.array(num_tokens, dtype=np.int64)})
    return postprocess(points[0], normal[0], mask_probability[0],
                       float(metric_scale[0]), fov_x_degrees)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--image", required=True)
    ap.add_argument("--model", required=True, help="path to model.onnx")
    ap.add_argument("--fov-x", type=float, required=True,
                    help="calibrated horizontal FOV in degrees")
    ap.add_argument("--num-tokens", type=int, default=NUM_TOKENS_RANGE[1])
    ap.add_argument("--provider", default="auto",
                    choices=["auto", "dml", "cpu"])
    ap.add_argument("--output", default=None, help=".npz path for the results")
    ap.add_argument("--validate", action="store_true",
                    help="also run the torch reference (needs moge) and compare")
    args = ap.parse_args()

    import onnxruntime as ort
    available = ort.get_available_providers()
    if args.provider == "dml" or (args.provider == "auto" and "DmlExecutionProvider" in available):
        providers = ["DmlExecutionProvider", "CPUExecutionProvider"]
    else:
        providers = ["CPUExecutionProvider"]
    session = ort.InferenceSession(args.model, providers=providers)
    print(f"providers: {session.get_providers()}")

    bgr = cv2.imread(args.image)
    rgb = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)

    start = time.perf_counter()
    result = run_onnx(session, rgb, args.fov_x, args.num_tokens)
    first = time.perf_counter() - start
    start = time.perf_counter()
    result = run_onnx(session, rgb, args.fov_x, args.num_tokens)
    print(f"inference: {first:.2f}s first (session warmup), "
          f"{time.perf_counter() - start:.2f}s steady-state")

    finite = result["depth"][result["mask"]]
    print(f"shift {result['shift']:+.4f} | depth 5/50/95%: "
          f"{np.percentile(finite, 5):.2f} / {np.percentile(finite, 50):.2f} / "
          f"{np.percentile(finite, 95):.2f} m | mask {result['mask'].mean() * 100:.1f}%")

    if args.output:
        np.savez(args.output, **{k: v for k, v in result.items() if k != "shift"})
        print(f"wrote {args.output}")

    if args.validate:
        import torch
        from moge.model.v2 import MoGeModel

        device = "cuda" if torch.cuda.is_available() else "cpu"
        model = MoGeModel.from_pretrained("Ruicheng/moge-2-vitl-normal").to(device).eval()
        image = torch.tensor(rgb / 255.0, dtype=torch.float32, device=device).permute(2, 0, 1)
        with torch.no_grad():
            # use_fp16 off so the comparison isolates ONNX-vs-torch, not precision
            reference = model.infer(image, num_tokens=args.num_tokens,
                                    fov_x=args.fov_x, use_fp16=False)
        reference = {k: v.cpu().numpy() for k, v in reference.items()}

        both = result["mask"] & reference["mask"]
        mask_disagree = (result["mask"] ^ reference["mask"]).mean()
        depth_err = np.abs(result["depth"][both] - reference["depth"][both]) \
            / reference["depth"][both]
        dot = np.clip((result["normal"] * reference["normal"]).sum(-1)[both], -1, 1)
        normal_err = np.degrees(np.arccos(dot))
        print(f"vs torch reference: depth relerr median {np.median(depth_err) * 100:.2f}% "
              f"p99 {np.percentile(depth_err, 99) * 100:.2f}% | "
              f"normal median {np.median(normal_err):.3f} deg "
              f"p99 {np.percentile(normal_err, 99):.3f} deg | "
              f"mask disagreement {mask_disagree * 100:.2f}%")


if __name__ == "__main__":
    main()
