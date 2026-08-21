"""Run the Marigold reference pipeline on one image or video frame.

Developer tool. Produces the normals/shading/albedo .npy files that
sh_lighting_fit.py consumes, plus the reference numbers the C++
SceneLightingEstimator is validated against. Not shipped, not called at runtime.

Usage:
    python tools/marigold_reference.py --input plate.png --out build/ref_x
    python tools/marigold_reference.py --input clip.mp4 --frame 0 --out build/ref_x
"""

import argparse
import os

import cv2
import numpy as np
import torch
from PIL import Image

IID_REPO = "prs-eth/marigold-iid-lighting-v1-1"
NORMALS_REPO = "prs-eth/marigold-normals-v1-1"


def load_image(path, frame):
    if os.path.splitext(path)[1].lower() in (".mp4", ".mov", ".avi", ".mkv"):
        cap = cv2.VideoCapture(path)
        cap.set(cv2.CAP_PROP_POS_FRAMES, frame)
        ok, bgr = cap.read()
        cap.release()
        if not ok:
            raise RuntimeError(f"could not read frame {frame} from {path}")
        return Image.fromarray(bgr[:, :, ::-1])
    return Image.open(path).convert("RGB")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--input", required=True)
    ap.add_argument("--frame", type=int, default=0)
    ap.add_argument("--out", required=True)
    ap.add_argument("--steps", type=int, default=4)
    ap.add_argument("--resolution", type=int, default=768)
    args = ap.parse_args()

    os.makedirs(args.out, exist_ok=True)
    img = load_image(args.input, args.frame)
    img.save(os.path.join(args.out, "plate.png"))
    print(f"input {img.size} -> {args.out}")

    device = "cuda" if torch.cuda.is_available() else "cpu"
    dtype = torch.float16 if device == "cuda" else torch.float32
    kw = dict(num_inference_steps=args.steps,
              processing_resolution=args.resolution,
              output_type="np")

    from diffusers import MarigoldIntrinsicsPipeline, MarigoldNormalsPipeline

    pipe = MarigoldIntrinsicsPipeline.from_pretrained(
        IID_REPO, variant="fp16", torch_dtype=dtype).to(device)
    out = pipe(img, **kw)
    for i, name in enumerate(pipe.target_properties["target_names"]):
        np.save(os.path.join(args.out, f"{name}.npy"), out.prediction[i])
        print(f"  {name}: {out.prediction[i].shape}")
    del pipe
    torch.cuda.empty_cache()

    npipe = MarigoldNormalsPipeline.from_pretrained(
        NORMALS_REPO, variant="fp16", torch_dtype=dtype).to(device)
    nout = npipe(img, **kw)
    n = nout.prediction[0] if nout.prediction.ndim == 4 else nout.prediction
    np.save(os.path.join(args.out, "normals.npy"), n)
    print(f"  normals: {n.shape}")


if __name__ == "__main__":
    main()
