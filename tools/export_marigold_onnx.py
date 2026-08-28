"""Export the Marigold models to ONNX for in-process inference in Mikan.

Developer tool, run ONCE. Mikan never invokes this and never needs Python at
runtime - it consumes only the .onnx / .bin artifacts this writes.

Requires diffusers==0.34.0 (see docs/reference/scene-lighting.md for why newer
versions fail under torch 2.4.1).

Usage:
    python tools/export_marigold_onnx.py --out models/marigold
    python tools/export_marigold_onnx.py --out models/marigold --components vae

Artifacts written to --out:
    vae_encoder.onnx        image [1,3,H,W] in [-1,1]  -> latent mode [1,4,H/8,W/8]
                            (NOT scaled; caller multiplies by scaling_factor)
    vae_decoder.onnx        latent [1,4,h,w]           -> image [1,3,h*8,w*8] in [-1,1]
    unet_iid_lighting.onnx  sample [1,16,h,w], t, ehs  -> [1,12,h,w]  (3 targets x 4ch)
    unet_normals.onnx       sample [1,8,h,w],  t, ehs  -> [1,4,h,w]
    empty_text_embedding.bin  float32 [1,2,1024], row major
    scheduler.json            alphas_cumprod + the config the C++ DDIM step needs
"""

import argparse
import json
import os

import numpy as np
import torch

IID_REPO = "prs-eth/marigold-iid-lighting-v1-1"
NORMALS_REPO = "prs-eth/marigold-normals-v1-1"
OPSET = 17


class UNetWrapper(torch.nn.Module):
    """torch.onnx.export needs a plain forward with positional tensor args."""

    def __init__(self, unet):
        super().__init__()
        self.unet = unet

    def forward(self, sample, timestep, encoder_hidden_states):
        return self.unet(sample, timestep, encoder_hidden_states=encoder_hidden_states, return_dict=False)[0]


class VaeEncoderWrapper(torch.nn.Module):
    """Deterministic mode() of the latent distribution - what Marigold uses."""

    def __init__(self, vae):
        super().__init__()
        self.vae = vae

    def forward(self, image):
        return self.vae.encode(image).latent_dist.mode()


class VaeDecoderWrapper(torch.nn.Module):
    def __init__(self, vae):
        super().__init__()
        self.vae = vae

    def forward(self, latent):
        return self.vae.decode(latent, return_dict=False)[0]


def export(module, args, path, input_names, output_names, dynamic_axes):
    """Export to ONNX, consolidating any external weight data into ONE .data file.

    A model over the 2GB protobuf limit (both UNets are ~3.4GB in fp32) forces
    torch.onnx.export into external-data mode, where it writes one loose file
    PER TENSOR, named after the tensor, into the output directory. That is
    thousands of files, and because the names are not namespaced per model, a
    second export into the same directory silently overwrites the first one's
    tensors. So each export goes to its own scratch directory and is then
    re-saved with all tensors collapsed into a single sidecar.
    """
    import shutil

    import onnx

    name = os.path.basename(path)
    scratch = os.path.join(os.path.dirname(path), f".scratch_{name}")
    if os.path.isdir(scratch):
        shutil.rmtree(scratch)
    os.makedirs(scratch)

    print(f"  exporting {name} ...", flush=True)
    torch.onnx.export(
        module,
        args,
        os.path.join(scratch, name),
        input_names=input_names,
        output_names=output_names,
        dynamic_axes=dynamic_axes,
        opset_version=OPSET,
        do_constant_folding=True,
    )

    model = onnx.load(os.path.join(scratch, name))  # pulls in the external tensors
    for stale in (path, path + ".data"):
        if os.path.exists(stale):
            os.remove(stale)
    onnx.save_model(
        model,
        path,
        save_as_external_data=True,
        all_tensors_to_one_file=True,
        location=name + ".data",
        size_threshold=1024,
        convert_attribute=False,
    )
    shutil.rmtree(scratch)

    graph_mb = os.path.getsize(path) / 1e6
    data_path = path + ".data"
    data_mb = os.path.getsize(data_path) / 1e6 if os.path.exists(data_path) else 0.0
    print(f"    wrote {graph_mb:.0f} MB graph + {data_mb:.0f} MB weights" if data_mb
          else f"    wrote {graph_mb:.0f} MB")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="models/marigold")
    ap.add_argument("--components", default="all",
                    choices=["all", "vae", "unets", "aux"])
    ap.add_argument("--height", type=int, default=768, help="sample shape used for tracing")
    ap.add_argument("--width", type=int, default=768)
    args = ap.parse_args()

    os.makedirs(args.out, exist_ok=True)
    # Export on CPU in fp32: tracing on CUDA/fp16 bakes device-specific kernels and
    # fp16 constant folding loses precision. ORT can quantize afterwards if wanted.
    device, dtype = "cpu", torch.float32
    h, w = args.height // 8, args.width // 8

    from diffusers import AutoencoderKL, DDIMScheduler, UNet2DConditionModel

    if args.components in ("all", "aux"):
        print("[aux] empty text embedding + scheduler constants")
        from transformers import CLIPTextModel, CLIPTokenizer

        tok = CLIPTokenizer.from_pretrained(IID_REPO, subfolder="tokenizer")
        enc = CLIPTextModel.from_pretrained(IID_REPO, subfolder="text_encoder", torch_dtype=dtype)
        # Marigold tokenizes the empty prompt with padding="do_not_pad", which
        # yields just [BOS, EOS] -> [1,2,1024]. NOT the usual 77-token padding.
        ids = tok("", padding="do_not_pad", max_length=tok.model_max_length,
                  truncation=True, return_tensors="pt").input_ids
        with torch.no_grad():
            emb = enc(ids)[0]
        print(f"  empty text embedding {tuple(emb.shape)}")
        emb.numpy().astype(np.float32).tofile(os.path.join(args.out, "empty_text_embedding.bin"))

        sched = DDIMScheduler.from_pretrained(IID_REPO, subfolder="scheduler")
        sched.set_timesteps(4)
        meta = {
            "num_train_timesteps": sched.config.num_train_timesteps,
            "prediction_type": sched.config.prediction_type,
            "beta_schedule": sched.config.beta_schedule,
            "beta_start": sched.config.beta_start,
            "beta_end": sched.config.beta_end,
            "timestep_spacing": sched.config.timestep_spacing,
            "steps_offset": getattr(sched.config, "steps_offset", 0),
            "clip_sample": sched.config.clip_sample,
            "set_alpha_to_one": sched.config.set_alpha_to_one,
            "default_denoising_steps": 4,
            "vae_scaling_factor": 0.18215,
            "empty_text_embedding_shape": list(emb.shape),
            "timesteps_4step": [int(t) for t in sched.timesteps],
            "alphas_cumprod": [float(a) for a in sched.alphas_cumprod],
        }
        with open(os.path.join(args.out, "scheduler.json"), "w") as f:
            json.dump(meta, f, indent=1)
        print(f"  timesteps(4) = {meta['timesteps_4step']}  pred={meta['prediction_type']}")

    if args.components in ("all", "vae"):
        print("[vae] shared between both checkpoints (verified byte-identical)")
        vae = AutoencoderKL.from_pretrained(IID_REPO, subfolder="vae", torch_dtype=dtype).to(device).eval()
        with torch.no_grad():
            export(VaeEncoderWrapper(vae), (torch.randn(1, 3, args.height, args.width),),
                   os.path.join(args.out, "vae_encoder.onnx"),
                   ["image"], ["latent"],
                   {"image": {2: "H", 3: "W"}, "latent": {2: "h", 3: "w"}})
            export(VaeDecoderWrapper(vae), (torch.randn(1, 4, h, w),),
                   os.path.join(args.out, "vae_decoder.onnx"),
                   ["latent"], ["image"],
                   {"latent": {2: "h", 3: "w"}, "image": {2: "H", 3: "W"}})
        del vae

    if args.components in ("all", "unets"):
        ehs = torch.randn(1, 2, 1024)
        t = torch.tensor(999, dtype=torch.int64)
        for repo, name, in_ch in ((IID_REPO, "unet_iid_lighting.onnx", 16),
                                  (NORMALS_REPO, "unet_normals.onnx", 8)):
            print(f"[unet] {name} (in_channels={in_ch})")
            unet = UNet2DConditionModel.from_pretrained(repo, subfolder="unet", torch_dtype=dtype).to(device).eval()
            with torch.no_grad():
                export(UNetWrapper(unet), (torch.randn(1, in_ch, h, w), t, ehs),
                       os.path.join(args.out, name),
                       ["sample", "timestep", "encoder_hidden_states"], ["out"],
                       {"sample": {2: "h", 3: "w"}, "out": {2: "h", 3: "w"}})
            del unet

    print(f"\ndone -> {args.out}")


if __name__ == "__main__":
    main()
