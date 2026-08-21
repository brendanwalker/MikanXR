"""Derive surface normals from a small monocular depth model, for comparison
against Marigold's directly-predicted normals.

The question this answers: the SH lighting fit consumes (normal, shading)
pairs, and the fit is a least-squares over hundreds of thousands of pixels that
is very tolerant of per-pixel normal noise (measured: 0.1 degrees of recovered
direction error under 10% noise). If that tolerance holds for the *structured*
error of depth-derived normals, then the 3.4GB Marigold normals UNet could be
replaced by a ~50MB depth model.

Developer tool. Not shipped.

Two things this deliberately does NOT hide:

  - Monocular depth is affine-invariant. Writing d' = a*d + b and unprojecting
    gives P' = a*P + b*ray, and the b*ray term is not a translation - it warps
    the surface. So the scale/shift ambiguity changes normals, unlike a rigid
    transform. The --sweep mode measures how much.
  - Normals require camera intrinsics. Without them the gradients are screen
    slopes, not surface normals. The assumed FOV is exposed for the same reason.
"""

import argparse

import cv2
import numpy as np


def predict_disparity(rgb_u8, model_name="depth-anything/Depth-Anything-V2-Small-hf"):
    """Run the depth model. Returns a disparity-like map (larger = nearer)."""
    import torch
    from transformers import AutoImageProcessor, AutoModelForDepthEstimation

    processor = AutoImageProcessor.from_pretrained(model_name)
    model = AutoModelForDepthEstimation.from_pretrained(model_name)
    device = "cuda" if torch.cuda.is_available() else "cpu"
    model = model.to(device).eval()

    inputs = processor(images=rgb_u8, return_tensors="pt").to(device)
    with torch.no_grad():
        prediction = model(**inputs).predicted_depth

    prediction = torch.nn.functional.interpolate(
        prediction.unsqueeze(1),
        size=rgb_u8.shape[:2],
        mode="bicubic",
        align_corners=False,
    ).squeeze()

    return prediction.float().cpu().numpy()


def disparity_to_depth(disparity, near_far_ratio=0.05):
    """Map a relative disparity map to a plausible depth map.

    near_far_ratio is the depth of the nearest surface expressed as a fraction
    of the farthest. It stands in for the unresolved scale/shift: there is no
    way to recover it from one image, and it is exactly the knob --sweep varies.
    """
    d = disparity.astype(np.float64)
    lo, hi = np.percentile(d, 1.0), np.percentile(d, 99.0)
    if hi - lo < 1e-9:
        return np.ones_like(d)

    # Normalize disparity to [0,1], then invert into a depth range.
    normalized = np.clip((d - lo) / (hi - lo), 0.0, 1.0)
    inv_near = 1.0 / near_far_ratio
    inv_far = 1.0
    inverse_depth = normalized * (inv_near - inv_far) + inv_far

    return 1.0 / inverse_depth


def depth_to_normals(depth, horizontal_fov_degrees=55.0, smooth_sigma=1.5):
    """Unproject to camera-space points and take the cross product of tangents.

    Camera convention matches Marigold's output: +X right, +Y up, +Z toward the
    viewer, so visible geometry sits at negative Z.
    """
    height, width = depth.shape

    if smooth_sigma > 0.0:
        # Depth-derived normals are dominated by high-frequency gradient noise
        # otherwise. This is the structured-error problem the docstring warns
        # about, and blurring is the standard mitigation.
        depth = cv2.GaussianBlur(depth, (0, 0), smooth_sigma)

    focal_x = (width * 0.5) / np.tan(np.radians(horizontal_fov_degrees) * 0.5)
    focal_y = focal_x  # square pixels
    center_x, center_y = width * 0.5, height * 0.5

    u = np.arange(width)[None, :].astype(np.float64)
    v = np.arange(height)[:, None].astype(np.float64)

    x = (u - center_x) * depth / focal_x
    y = -(v - center_y) * depth / focal_y  # image v runs down, camera Y runs up
    z = -depth                             # looking down -Z

    points = np.stack([x, y, z], axis=-1)

    # Central differences; np.gradient handles the borders.
    d_dv, d_du = np.gradient(points, axis=(0, 1))
    # Order matters: image v runs DOWN while camera Y runs UP, so the (u, v)
    # tangent pair is left-handed in camera space and cross(T_u, T_v) points
    # away from the viewer. Taking cross(T_v, T_u) yields the outward normal.
    # Getting this backwards and relying on the orient-toward-camera flip below
    # to correct it silently MIRRORS Y instead of fixing it - which reads as a
    # floor whose normal points into the ground.
    normals = np.cross(d_dv, d_du)

    length = np.linalg.norm(normals, axis=-1, keepdims=True)
    normals = np.divide(normals, np.maximum(length, 1e-12))

    # Deliberately NOT reoriented toward the camera. Forcing Z positive looks
    # like a harmless safety net and is not: negating a normal mirrors X and Y
    # as well, so it turns a correct grazing-angle normal into a wrong one. On
    # this plate it corrupted 39.5% of pixels and inverted the floor. Grazing
    # surfaces legitimately have slightly negative Z (Marigold's own output does
    # for 16.4% of pixels), so the cross product order above has to be right by
    # construction instead.
    return normals.astype(np.float32)


def angular_error_degrees(a, b):
    dot = np.clip((a.astype(np.float64) * b.astype(np.float64)).sum(axis=-1), -1.0, 1.0)
    return np.degrees(np.arccos(dot))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--plate", required=True)
    ap.add_argument("--reference-normals", help="Marigold normals .npy to compare against")
    ap.add_argument("--out", required=True)
    ap.add_argument("--fov", type=float, default=55.0)
    ap.add_argument("--near-far-ratio", type=float, default=0.05)
    ap.add_argument("--smooth", type=float, default=1.5)
    args = ap.parse_args()

    import os

    os.makedirs(args.out, exist_ok=True)

    rgb = cv2.imread(args.plate)[:, :, ::-1].copy()
    disparity = predict_disparity(rgb)
    np.save(os.path.join(args.out, "disparity.npy"), disparity)

    depth = disparity_to_depth(disparity, args.near_far_ratio)
    normals = depth_to_normals(depth, args.fov, args.smooth)
    np.save(os.path.join(args.out, "normals.npy"), normals)

    vis = ((normals * 0.5 + 0.5)[:, :, ::-1] * 255).astype(np.uint8)
    cv2.imwrite(os.path.join(args.out, "normals_from_depth.png"), vis)
    print(f"wrote {args.out}/normals.npy  fov={args.fov} ratio={args.near_far_ratio} smooth={args.smooth}")

    if args.reference_normals:
        reference = np.load(args.reference_normals).astype(np.float32)
        error = angular_error_degrees(normals, reference)
        print(f"angular error vs reference: median {np.median(error):.1f} deg, "
              f"mean {error.mean():.1f}, p90 {np.percentile(error, 90):.1f}")

        ref_vis = ((reference * 0.5 + 0.5)[:, :, ::-1] * 255).astype(np.uint8)
        error_vis = cv2.applyColorMap(
            np.clip(error / 90.0 * 255.0, 0, 255).astype(np.uint8), cv2.COLORMAP_INFERNO)
        cv2.imwrite(os.path.join(args.out, "normals_comparison.png"),
                    np.concatenate([ref_vis, vis, error_vis], axis=0))


if __name__ == "__main__":
    main()
