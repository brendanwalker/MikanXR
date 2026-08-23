"""Reference implementation of the scene lighting estimator's SH fit.

This is the Python twin of the C++ SceneLightingEstimator. It exists to (a) let
the approach be judged visually before committing to the ONNX/C++ path, and
(b) generate the reference numbers the C++ unit test is checked against.

It does NOT ship with Mikan and is not invoked at runtime.

Usage:
    python tools/sh_lighting_fit.py --normals N.npy --shading S.npy \
        [--plate plate.png] [--albedo A.npy] [--out DIR] [--robust]

Inputs are the raw Marigold outputs: normals in camera space, HxWx3 in [-1,1];
diffuse shading, HxWx3, linear space.
"""

import argparse
import os

import cv2
import numpy as np

# Order-2 real SH. Diffuse irradiance is band-limited (Ramamoorthi & Hanrahan
# 2001): these 9 coefficients capture ~99% of it for a Lambertian surface.
SH_C = np.array([0.282095, 0.488603, 0.488603, 0.488603,
                 1.092548, 1.092548, 0.315392, 1.092548, 0.546274])
# Lambertian convolution coefficients A-hat per band: [pi, 2pi/3, pi/4]
SH_AHAT = np.array([3.14159265] + [2.09439510] * 3 + [0.78539816] * 5)


def sh_basis(n):
    """Evaluate the 9 order-2 real SH basis functions for unit vectors n (...,3)."""
    x, y, z = n[..., 0], n[..., 1], n[..., 2]
    return np.stack([
        np.full_like(x, SH_C[0]),
        SH_C[1] * y,
        SH_C[2] * z,
        SH_C[3] * x,
        SH_C[4] * x * y,
        SH_C[5] * y * z,
        SH_C[6] * (3.0 * z * z - 1.0),
        SH_C[7] * x * z,
        SH_C[8] * (x * x - y * y),
    ], axis=-1)


def irradiance_design_matrix(n):
    """Rows map radiance-SH coefficients -> Lambertian irradiance E(n)."""
    return sh_basis(n) * SH_AHAT


def build_mask(normals, shading):
    """Drop pixels that carry no usable radiometric constraint."""
    unit = np.abs(np.linalg.norm(normals, axis=-1) - 1.0) < 0.1
    lit = shading.min(axis=-1) > 1e-4      # fully black -> no signal, only occlusion
    unsaturated = shading.max(axis=-1) < 0.99
    return unit & lit & unsaturated


def fit_sh(normals, shading, mask=None, robust=False, ridge=1e-6, band_ridge=0.1):
    """Least-squares solve for the 9x3 radiance SH coefficients.

    robust=True runs IRLS that only down-weights pixels DARKER than the model.
    Cast shadows and AO can only ever darken a pixel, so fitting the upper
    envelope estimates the unoccluded environment rather than the mean of
    (lit + shadowed) samples.

    band_ridge penalizes the l=2 band. This is NOT cosmetic. An unconstrained
    9-coefficient fit on real data lands on a solution whose reconstructed
    radiance is negative over ~31% of the sphere - physically impossible, and
    the classic low-order SH ringing signature (measured band energies came out
    l0=0.25 l1=0.31 l2=0.56, i.e. most energy in the quadratic band). Real
    shading violates the single-unoccluded-environment model via cast shadows
    and spatially varying light, and the l=2 band absorbs that error rather
    than any real signal. Penalizing it at 0.1 drops negative solid angle to
    0% while costing only ~0.025 of R^2.
    """
    n = normals.reshape(-1, 3)
    e = shading.reshape(-1, 3)
    if mask is not None:
        m = mask.reshape(-1)
        n, e = n[m], e[m]

    A = irradiance_design_matrix(n)
    AtA = A.T @ A
    band_penalty = np.diag([0.0] * 4 + [1.0] * 5) * band_ridge * np.trace(AtA) / 9.0
    reg = ridge * np.eye(9) + band_penalty

    def solve(w):
        Aw = A * w[:, None]
        return np.linalg.solve(Aw.T @ A + reg, Aw.T @ e)

    w = np.ones(len(A))
    L = solve(w)
    if robust:
        for _ in range(12):
            L = solve(w)
            r = (e - A @ L).mean(axis=1)
            s = 1.4826 * np.median(np.abs(r - np.median(r))) + 1e-9
            w = np.where(r >= 0.0, 1.0, 1.0 / (1.0 + (np.abs(r) / (1.5 * s)) ** 2))

    resid = A @ L - e
    denom = ((e - e.mean(axis=0)) ** 2).sum()
    stats = {
        "pixels": len(A),
        "cond": float(np.linalg.cond(A.T @ A)),
        "rms": float(np.sqrt((resid ** 2).mean())),
        "r2": float(1.0 - (resid ** 2).sum() / max(denom, 1e-12)),
    }
    return L, stats


def dominant_direction(L):
    """Direction of peak radiance from the l=1 band (the SH 'centroid' of light)."""
    g = L.mean(axis=1)
    d = np.array([g[3], g[1], g[2]])  # (x, y, z) from (Y11, Y1-1, Y10)
    norm = np.linalg.norm(d)
    return d / norm if norm > 1e-9 else np.array([0.0, 0.0, 1.0])


def render_latlong(L, width=256, height=128):
    """Radiance environment map, equirectangular. This is what becomes the HDRI."""
    v = (np.arange(height) + 0.5) / height
    u = (np.arange(width) + 0.5) / width
    theta = v[:, None] * np.pi            # 0=+Y pole
    phi = u[None, :] * 2.0 * np.pi
    y = np.cos(theta) * np.ones_like(phi)
    r = np.sin(theta) * np.ones_like(phi)
    x = r * np.cos(phi)
    z = r * np.sin(phi)
    n = np.stack([x, y, z], axis=-1)
    return sh_basis(n) @ L


def render_lit_sphere(L, size=192):
    """A unit sphere lit by the recovered environment - a stand-in for the character."""
    g = (np.arange(size) + 0.5) / size * 2.0 - 1.0
    px, py = np.meshgrid(g, -g)
    r2 = px ** 2 + py ** 2
    inside = r2 <= 1.0
    pz = np.sqrt(np.clip(1.0 - r2, 0.0, None))
    n = np.stack([px, py, pz], axis=-1)
    img = (irradiance_design_matrix(n) @ L) / np.pi   # Lambertian, albedo 1
    return np.where(inside[..., None], img, 0.0), inside


def _tonemap(img, scale=None):
    """Linear -> 8-bit sRGB for display."""
    x = np.clip(img, 0.0, None)
    if scale is None:
        p = np.percentile(x, 99.5)
        scale = 1.0 / p if p > 1e-9 else 1.0
    x = np.clip(x * scale, 0.0, 1.0)
    return (np.power(x, 1.0 / 2.2) * 255.0).astype(np.uint8)


def _label(img, text):
    out = img.copy()
    cv2.rectangle(out, (0, 0), (out.shape[1], 22), (0, 0, 0), -1)
    cv2.putText(out, text, (6, 16), cv2.FONT_HERSHEY_SIMPLEX, 0.45, (255, 255, 255), 1, cv2.LINE_AA)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--normals", required=True)
    ap.add_argument("--shading", required=True)
    ap.add_argument("--albedo")
    ap.add_argument("--plate")
    ap.add_argument("--out", default="build/marigold_ref")
    ap.add_argument("--robust", action="store_true")
    ap.add_argument("--band-ridge", type=float, default=0.1)
    args = ap.parse_args()

    normals = np.load(args.normals).astype(np.float64)
    shading = np.load(args.shading).astype(np.float64)
    H, W, _ = normals.shape

    mask = build_mask(normals, shading)
    L, stats = fit_sh(normals, shading, mask, robust=args.robust, band_ridge=args.band_ridge)
    d = dominant_direction(L)
    ambient = L[0] * SH_C[0] * SH_AHAT[0]

    print(f"pixels used : {stats['pixels']} / {H * W}")
    print(f"cond(AtA)   : {stats['cond']:.1f}")
    print(f"residual RMS: {stats['rms']:.4f}   (mean shading {shading[mask].mean():.4f})")
    print(f"R^2         : {stats['r2']:.4f}")
    print(f"key dir     : {np.round(d, 3)}  (camera space, +Z toward viewer)")
    print(f"ambient RGB : {np.round(ambient, 4)}")

    os.makedirs(args.out, exist_ok=True)
    np.save(os.path.join(args.out, "sh_coeffs.npy"), L)

    # Reconstruct the shading the recovered environment predicts, for comparison.
    predicted = (irradiance_design_matrix(normals) @ L)

    # --- contact sheet ---
    tiles = []
    if args.plate and os.path.exists(args.plate):
        tiles.append(_label(cv2.imread(args.plate), "plate"))
    nrm_vis = ((normals * 0.5 + 0.5)[:, :, ::-1] * 255).astype(np.uint8)
    tiles.append(_label(nrm_vis, "normals (Marigold)"))

    scale = 1.0 / max(np.percentile(shading, 99.5), 1e-9)
    tiles.append(_label(_tonemap(shading, scale)[:, :, ::-1], "shading (Marigold)"))
    tiles.append(_label(_tonemap(predicted, scale)[:, :, ::-1], "shading (SH fit)"))

    err = np.abs(predicted - shading).mean(axis=-1)
    err_vis = cv2.applyColorMap(_tonemap(err[..., None] * np.ones(3), scale), cv2.COLORMAP_INFERNO)
    tiles.append(_label(err_vis, "abs error"))

    if args.albedo and os.path.exists(args.albedo):
        alb = np.load(args.albedo).astype(np.float64)
        relit = alb * predicted
        tiles.append(_label(_tonemap(relit)[:, :, ::-1], "albedo x SH shading"))

    grid = np.concatenate([np.concatenate(tiles[i:i + 2], axis=1)
                           for i in range(0, len(tiles) - len(tiles) % 2, 2)], axis=0)
    sheet = os.path.join(args.out, "sh_validation.png")
    cv2.imwrite(sheet, grid)

    env = render_latlong(L, 512, 256)
    cv2.imwrite(os.path.join(args.out, "env_latlong.png"), _tonemap(env)[:, :, ::-1])

    sphere, _ = render_lit_sphere(L, 256)
    cv2.imwrite(os.path.join(args.out, "lit_sphere.png"), _tonemap(sphere)[:, :, ::-1])

    print(f"\nwrote {sheet}")
    print(f"wrote {os.path.join(args.out, 'env_latlong.png')}")
    print(f"wrote {os.path.join(args.out, 'lit_sphere.png')}")


if __name__ == "__main__":
    main()
