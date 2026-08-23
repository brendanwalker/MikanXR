"""Validate the ONNX/numpy pipeline against the PyTorch reference.

Feeds BOTH implementations the same fixed initial latents so the comparison is
exact rather than statistical (the denoise loop starts from random noise, so
without pinning it the two runs are not comparable).

Developer tool. Not shipped.
"""

import argparse
import sys

import cv2
import numpy as np
import torch

sys.path.insert(0, "tools")
from marigold_onnx_pipeline import MarigoldOnnx  # noqa: E402


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--models", default="models/marigold")
    ap.add_argument("--image", default="build/marigold_ref/plate.png")
    ap.add_argument("--provider", default="CPUExecutionProvider")
    ap.add_argument("--resolution", type=int, default=768)
    args = ap.parse_args()

    rgb = cv2.imread(args.image)[:, :, ::-1].copy()
    pipe = MarigoldOnnx(args.models, provider=args.provider)
    print(f"ORT provider: {pipe.provider}")

    # Shared fixed noise, shaped from the actual latent grid.
    image, padding, original = pipe.preprocess(rgb, args.resolution)
    _, _, h, w = pipe.encode(image).shape
    rng = np.random.default_rng(1234)
    latents = {
        "iid": rng.standard_normal((1, 12, h, w)).astype(np.float32),
        "normals": rng.standard_normal((1, 4, h, w)).astype(np.float32),
    }
    print(f"latent grid {h}x{w}  (padded image {image.shape[2]}x{image.shape[3]})")

    got = pipe.run(rgb, args.resolution, latents=latents)

    from diffusers import MarigoldIntrinsicsPipeline, MarigoldNormalsPipeline
    from PIL import Image

    pil = Image.fromarray(rgb)
    kw = dict(num_inference_steps=4, processing_resolution=args.resolution, output_type="np")

    ref = {}
    p = MarigoldIntrinsicsPipeline.from_pretrained(
        "prs-eth/marigold-iid-lighting-v1-1", torch_dtype=torch.float32)
    out = p(pil, latents=torch.from_numpy(latents["iid"]), **kw)
    for i, name in enumerate(p.target_properties["target_names"]):
        ref[name] = out.prediction[i]
    del p

    n = MarigoldNormalsPipeline.from_pretrained(
        "prs-eth/marigold-normals-v1-1", torch_dtype=torch.float32)
    nout = n(pil, latents=torch.from_numpy(latents["normals"]), **kw)
    ref["normals"] = nout.prediction[0] if nout.prediction.ndim == 4 else nout.prediction
    del n

    print()
    ok = True
    for k in ("albedo", "shading", "residual", "normals"):
        a, b = got[k].astype(np.float64), ref[k].astype(np.float64)
        if a.shape != b.shape:
            print(f"{k:9s} SHAPE MISMATCH {a.shape} vs {b.shape}")
            ok = False
            continue
        d = np.abs(a - b)
        rng_ = max(b.max() - b.min(), 1e-9)
        print(f"{k:9s} max|d|={d.max():.4e}  mean|d|={d.mean():.4e}  "
              f"rel={d.max()/rng_:.4e}  corr={np.corrcoef(a.ravel(), b.ravel())[0,1]:.6f}")
        ok &= d.mean() / rng_ < 1e-2
    print("\nOK" if ok else "\nDIVERGENT")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
