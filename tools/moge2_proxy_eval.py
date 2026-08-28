"""Evaluate MoGe-2 as the geometry source for the depth-derived proxy mesh.

The decisions this informs, in order of weight:

  1. Silhouette alignment. If the proxy mesh is to replace hand-placed model
     stencils as a shadow catcher, its depth discontinuities must land on the
     image's silhouette edges. Measured directly: chamfer distance from
     depth-edge pixels to the nearest Canny edge of the plate.
  2. Metric plausibility and FOV conditioning. MoGe-2 claims metric output and
     accepts a known horizontal FOV (which Mikan has from calibration). The
     sweep measures how much the conditioning moves the answer - i.e. what
     feeding the real calibration in buys over letting the model guess.
  3. Normals. scene-lighting.md concluded depth-DERIVED normals corrupt the SH
     lighting fit (affine ambiguity warps the surface) and that the experiment
     worth running is a model that predicts normals DIRECTLY. MoGe-2's normal
     head is exactly that, so the SH fit is re-run here with MoGe-2 normals
     against the Marigold reference - same shading, only the normals differ.

Also writes oblique point-splat renders (raw and discontinuity-culled) so the
flying-pixel skirts can be eyeballed, which no scalar metric replaces.

Developer tool. Not shipped. Needs `moge` + `utils3d` (pip install --user
--no-deps from the microsoft/MoGe and EasternJournalist/utils3d repos).

Coordinate note: MoGe outputs OpenCV camera convention (+X right, +Y down,
+Z forward). Marigold and the SH fit use +X right, +Y up, +Z toward viewer,
so normals are compared as (nx, -ny, -nz).
"""

import argparse
import os
import sys

import cv2
import numpy as np

sys.path.insert(0, "tools")
from compare_normal_sources import summarize  # noqa: E402

MODEL_NAME = "Ruicheng/moge-2-vitl-normal"


def load_model():
    import torch
    from moge.model.v2 import MoGeModel

    device = "cuda" if torch.cuda.is_available() else "cpu"
    model = MoGeModel.from_pretrained(MODEL_NAME).to(device).eval()
    return model, device


def run_inference(model, device, rgb_u8, fov_x=None):
    """Returns dict of numpy maps: points, depth, mask, normal, fov_x_deg."""
    import torch

    image = torch.tensor(rgb_u8 / 255.0, dtype=torch.float32, device=device).permute(2, 0, 1)
    with torch.no_grad():
        out = model.infer(image, fov_x=fov_x)

    result = {k: v.cpu().numpy() for k, v in out.items()}
    fx = result["intrinsics"][0, 0]  # normalized: focal / width
    result["fov_x_deg"] = float(np.degrees(2.0 * np.arctan(0.5 / fx)))
    return result


# ---------------------------------------------------------------------------
# Silhouette / discontinuity analysis


def depth_edge_mask(depth, mask, ratio):
    """Pixels adjacent (4-neighborhood) to a depth jump exceeding `ratio`.

    These are the pixels whose mesh triangles a proxy builder would cull; the
    open question is whether they sit ON the image silhouettes.
    """
    edge = np.zeros(depth.shape, dtype=bool)
    d = np.where(mask, depth, np.nan)
    for axis in (0, 1):
        a = np.take(d, range(0, d.shape[axis] - 1), axis=axis)
        b = np.take(d, range(1, d.shape[axis]), axis=axis)
        jump = np.maximum(a, b) / np.minimum(a, b) > ratio
        jump = np.nan_to_num(jump.astype(np.float32)).astype(bool)
        pad_lo = [(0, 1) if ax == axis else (0, 0) for ax in range(2)]
        pad_hi = [(1, 0) if ax == axis else (0, 0) for ax in range(2)]
        edge |= np.pad(jump, pad_lo)
        edge |= np.pad(jump, pad_hi)
    return edge & mask


def silhouette_alignment(rgb_u8, depth_edges):
    """Chamfer distance (pixels) from each depth-edge pixel to the nearest
    image edge. Median answers 'do they line up'; p95 catches hallucinated
    depth edges with no image evidence."""
    gray = cv2.cvtColor(rgb_u8, cv2.COLOR_RGB2GRAY)
    canny = cv2.Canny(gray, 50, 150)
    dist_to_image_edge = cv2.distanceTransform(
        (canny == 0).astype(np.uint8), cv2.DIST_L2, 5)
    samples = dist_to_image_edge[depth_edges]
    return samples


# ---------------------------------------------------------------------------
# Oblique renders


def splat_render(points, colors, valid, yaw_degrees, width, height, fx, fy, cx, cy):
    """Z-buffered 1px point splat from a yawed viewpoint.

    Painter's order (far first) stands in for a z-buffer. Skirt/flying pixels
    show up as streaks bridging silhouette gaps; culled renders show the holes
    left behind instead.
    """
    p = points[valid].reshape(-1, 3)
    c = colors[valid].reshape(-1, 3)

    pivot = np.array([0.0, 0.0, np.median(p[:, 2])])
    yaw = np.radians(yaw_degrees)
    rotation = np.array([
        [np.cos(yaw), 0.0, np.sin(yaw)],
        [0.0, 1.0, 0.0],
        [-np.sin(yaw), 0.0, np.cos(yaw)],
    ])
    p = (p - pivot) @ rotation.T + pivot

    in_front = p[:, 2] > 1e-6
    p, c = p[in_front], c[in_front]

    u = np.round(fx * p[:, 0] / p[:, 2] + cx).astype(np.int64)
    v = np.round(fy * p[:, 1] / p[:, 2] + cy).astype(np.int64)
    on_screen = (u >= 0) & (u < width) & (v >= 0) & (v < height)
    u, v, z, c = u[on_screen], v[on_screen], p[on_screen, 2], c[on_screen]

    order = np.argsort(-z)  # far first; nearer points overwrite
    image = np.zeros((height, width, 3), dtype=np.uint8)
    image[v[order], u[order]] = c[order]
    return image


# ---------------------------------------------------------------------------


def evaluate_plate(model, device, reference_dir, output_dir, edge_ratio, sweep_fovs):
    name = os.path.basename(os.path.normpath(reference_dir))
    os.makedirs(output_dir, exist_ok=True)

    bgr = cv2.imread(os.path.join(reference_dir, "plate.png"))
    rgb = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)
    height, width = rgb.shape[:2]

    print(f"\n=== {name} ({width}x{height}) ===")

    base = run_inference(model, device, rgb)
    depth = base["depth"]
    mask = base["mask"].astype(bool) & np.isfinite(depth)
    normal = base.get("normal")

    valid_depth = depth[mask]
    lo, mid, hi = np.percentile(valid_depth, [5, 50, 95])
    print(f"estimated FOV_x {base['fov_x_deg']:.1f} deg | metric depth 5/50/95%: "
          f"{lo:.2f} / {mid:.2f} / {hi:.2f} m | mask coverage {mask.mean() * 100:.1f}%")

    np.save(os.path.join(output_dir, "depth.npy"), depth)
    np.save(os.path.join(output_dir, "mask.npy"), mask)
    depth_vis = np.where(mask, depth, np.nan)
    vis = cv2.applyColorMap(
        (np.clip((np.nan_to_num(depth_vis, nan=hi) - lo) / max(hi - lo, 1e-6), 0, 1) * 255)
        .astype(np.uint8), cv2.COLORMAP_TURBO)
    cv2.imwrite(os.path.join(output_dir, "depth_vis.png"), vis)

    # --- silhouette alignment ---------------------------------------------
    print(f"--- silhouette alignment (depth-edge -> nearest Canny edge, px) ---")
    for ratio in (1.05, edge_ratio, 1.25):
        edges = depth_edge_mask(depth, mask, ratio)
        chamfer = silhouette_alignment(rgb, edges)
        if chamfer.size == 0:
            print(f"  ratio {ratio:.2f}: no depth edges")
            continue
        print(f"  ratio {ratio:.2f}: edge pixels {edges.mean() * 100:4.1f}%  "
              f"median {np.median(chamfer):.1f}  p95 {np.percentile(chamfer, 95):.1f}")

    edges = depth_edge_mask(depth, mask, edge_ratio)
    overlay = rgb.copy()
    overlay[edges] = (255, 0, 0)
    cv2.imwrite(os.path.join(output_dir, "depth_edges_on_plate.png"),
                cv2.cvtColor(overlay, cv2.COLOR_RGB2BGR))

    # --- oblique renders ---------------------------------------------------
    fx = base["intrinsics"][0, 0] * width
    fy = base["intrinsics"][1, 1] * height
    cx = base["intrinsics"][0, 2] * width
    cy = base["intrinsics"][1, 2] * height
    for yaw in (-25.0, 25.0):
        raw = splat_render(base["points"], rgb, mask, yaw, width, height, fx, fy, cx, cy)
        culled = splat_render(base["points"], rgb, mask & ~edges, yaw,
                              width, height, fx, fy, cx, cy)
        tag = f"{'m' if yaw < 0 else 'p'}{abs(yaw):.0f}"
        cv2.imwrite(os.path.join(output_dir, f"oblique_{tag}_raw.png"),
                    cv2.cvtColor(raw, cv2.COLOR_RGB2BGR))
        cv2.imwrite(os.path.join(output_dir, f"oblique_{tag}_culled.png"),
                    cv2.cvtColor(culled, cv2.COLOR_RGB2BGR))
    print(f"wrote depth/edge/oblique images to {output_dir}")

    # --- normals: direct head vs Marigold, and through the SH fit ----------
    if normal is not None:
        moge_normals = normal.astype(np.float64) * np.array([1.0, -1.0, -1.0])
        np.save(os.path.join(output_dir, "normals.npy"), moge_normals)

        marigold = np.load(os.path.join(reference_dir, "normals.npy")).astype(np.float64)
        shading = np.load(os.path.join(reference_dir, "shading.npy")).astype(np.float64)

        both = mask & (np.linalg.norm(moge_normals, axis=-1) > 0.5)
        dot = np.clip((moge_normals * marigold).sum(-1)[both], -1, 1)
        angular = np.degrees(np.arccos(dot))
        print(f"--- normals: MoGe-2 direct head vs Marigold ---")
        print(f"  median {np.median(angular):.1f} deg, p90 {np.percentile(angular, 90):.1f} deg "
              f"(depth-derived normals were 41 deg median)")

        print(f"--- SH lighting fit: same shading, only the normals differ ---")
        reference = summarize("  marigold normals", marigold, shading)
        summarize("  moge2 normals", np.where(both[..., None], moge_normals, 0.0),
                  shading, reference)

    # --- FOV conditioning sensitivity --------------------------------------
    print(f"--- FOV conditioning (vs estimated {base['fov_x_deg']:.1f} deg) ---")
    for fov in sweep_fovs:
        run = run_inference(model, device, rgb, fov_x=fov)
        run_mask = run["mask"].astype(bool) & np.isfinite(run["depth"]) & mask
        depth_shift = np.median(np.abs(run["depth"][run_mask] - depth[run_mask])
                                / depth[run_mask])
        row = (f"  fov {fov:4.1f}: median depth {np.median(run['depth'][run_mask]):6.2f} m, "
               f"median |ddepth|/depth {depth_shift * 100:5.1f}%")
        if normal is not None and run.get("normal") is not None:
            run_normals = run["normal"].astype(np.float64) * np.array([1.0, -1.0, -1.0])
            dot = np.clip((run_normals * (normal * np.array([1.0, -1.0, -1.0])))
                          .sum(-1)[run_mask], -1, 1)
            row += f", median normal shift {np.median(np.degrees(np.arccos(dot))):.1f} deg"
        print(row)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("reference_dirs", nargs="*",
                    default=["build/marigold_ref", "build/ref_shadow"])
    ap.add_argument("--output-root", default="build/moge2_eval")
    ap.add_argument("--edge-ratio", type=float, default=1.15,
                    help="adjacent-pixel depth ratio that flags a discontinuity")
    ap.add_argument("--sweep-fovs", type=float, nargs="*",
                    default=[45.0, 55.0, 70.0])
    args = ap.parse_args()

    model, device = load_model()
    print(f"{MODEL_NAME} on {device}")

    for reference_dir in args.reference_dirs:
        name = os.path.basename(os.path.normpath(reference_dir))
        evaluate_plate(model, device, reference_dir,
                       os.path.join(args.output_root, name),
                       args.edge_ratio, args.sweep_fovs)


if __name__ == "__main__":
    main()
