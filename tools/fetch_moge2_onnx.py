"""Download the pre-exported MoGe-2 ONNX model into models/moge2.

The checkpoint is Ruicheng/moge-2-vitl-normal-onnx (MIT, ~1.3GB) - the ViT-L
variant with the normal head, exported by the MoGe authors at opset 14. Unlike
Marigold there is no local export step: the official export is a single
self-contained model.onnx, verified to run under both the CPU and DirectML
execution providers with identical output (see docs/reference/scene-lighting.md).

Runs once on a developer machine; models/ is gitignored.
"""

import argparse
import shutil
from pathlib import Path

REPO_ID = "Ruicheng/moge-2-vitl-normal-onnx"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--output-dir", default="models/moge2")
    args = ap.parse_args()

    from huggingface_hub import hf_hub_download

    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    cached = hf_hub_download(REPO_ID, "model.onnx")
    target = output_dir / "model.onnx"
    shutil.copyfile(cached, target)
    print(f"wrote {target} ({target.stat().st_size / 1e9:.2f} GB)")


if __name__ == "__main__":
    main()
