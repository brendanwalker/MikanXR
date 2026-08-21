"""A/B the SH lighting fit using Marigold normals vs depth-derived normals.

The decision this informs: Marigold's normals UNet is 3.4GB and one of two
denoise loops. Depth Anything V2 Small is ~50MB fp16 and a single forward pass.
If the recovered lighting agrees, the geometric half of the pipeline is
overbuilt.

The metric that matters is NOT how well the two normal fields agree per pixel -
the SH fit is a least squares over ~400k samples and averages noise away. It is
how far apart the two recovered environments end up.

Developer tool. Not shipped.
"""

import argparse
import sys

import numpy as np

sys.path.insert(0, "tools")
from depth_to_normals import angular_error_degrees, depth_to_normals, disparity_to_depth  # noqa: E402
from sh_lighting_fit import (  # noqa: E402
    SH_AHAT,
    SH_C,
    build_mask,
    dominant_direction,
    fit_sh,
    render_latlong,
)


def summarize(label, normals, shading, reference_environment=None):
    mask = build_mask(normals, shading)
    L, stats = fit_sh(normals, shading, mask)
    direction = dominant_direction(L)
    ambient = L[0] * SH_C[0] * SH_AHAT[0]

    l0 = np.linalg.norm(L[0])
    l1 = np.sqrt((L[1:4] ** 2).sum())

    row = (f"{label:<22} R2={stats['r2']:+.3f}  l1/l0={l1 / max(l0, 1e-9):.3f}  "
           f"dir=[{direction[0]:+.3f} {direction[1]:+.3f} {direction[2]:+.3f}]  "
           f"ambientR={ambient[0]:.4f}")

    if reference_environment is not None:
        reference_direction = dominant_direction(reference_environment)
        angle = np.degrees(np.arccos(np.clip(direction @ reference_direction, -1, 1)))
        relative = np.abs(L - reference_environment).max() / max(np.abs(reference_environment).max(), 1e-9)
        row += f"  | vs ref: dir {angle:5.1f} deg, coeff {relative * 100:4.1f}%"

    print(row)
    return L


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--reference-dir", default="build/marigold_ref")
    ap.add_argument("--disparity", default="build/depth_normals/disparity.npy")
    ap.add_argument("--sweep", action="store_true", help="vary FOV / depth range / smoothing")
    args = ap.parse_args()

    shading = np.load(f"{args.reference_dir}/shading.npy").astype(np.float64)
    marigold_normals = np.load(f"{args.reference_dir}/normals.npy").astype(np.float64)
    disparity = np.load(args.disparity)

    print("Same shading for every row; only the NORMALS differ.\n")
    reference = summarize("marigold normals", marigold_normals, shading)
    print()

    if not args.sweep:
        depth = disparity_to_depth(disparity, 0.05)
        normals = depth_to_normals(depth, 55.0, 1.5).astype(np.float64)
        summarize("depth normals", normals, shading, reference)
        return

    # The scale/shift ambiguity and the unknown FOV both change the recovered
    # normals, so measure how much they move the answer rather than assuming.
    print("--- assumed horizontal FOV (near/far 0.05, smooth 1.5) ---")
    for fov in (35.0, 45.0, 55.0, 70.0, 90.0):
        normals = depth_to_normals(disparity_to_depth(disparity, 0.05), fov, 1.5).astype(np.float64)
        summarize(f"  fov {fov:.0f}", normals, shading, reference)

    print("\n--- depth near/far ratio, the scale/shift stand-in (fov 55) ---")
    for ratio in (0.02, 0.05, 0.1, 0.25, 0.5):
        normals = depth_to_normals(disparity_to_depth(disparity, ratio), 55.0, 1.5).astype(np.float64)
        summarize(f"  ratio {ratio:.2f}", normals, shading, reference)

    print("\n--- depth smoothing sigma (fov 55, ratio 0.05) ---")
    for sigma in (0.0, 1.5, 3.0, 6.0, 12.0):
        normals = depth_to_normals(disparity_to_depth(disparity, 0.05), 55.0, sigma).astype(np.float64)
        error = angular_error_degrees(normals, marigold_normals)
        L = summarize(f"  sigma {sigma:.1f}", normals.astype(np.float64), shading, reference)
        print(f"{'':<24}(median normal error vs marigold {np.median(error):.1f} deg)")


if __name__ == "__main__":
    main()
