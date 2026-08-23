"""Pure ONNX Runtime + numpy implementation of the Marigold pipeline.

This is the executable specification for the C++ SceneLightingEstimator: every
step here maps 1:1 onto what the C++ does, with no diffusers/torch involved.
Validated against the PyTorch pipeline by tools/validate_onnx_pipeline.py.

Developer tool. Not shipped.
"""

import json
import os

import cv2
import numpy as np
import onnxruntime as ort

VAE_SCALE = 0.18215
LATENT_CH = 4

# The exported UNet graphs bake in their downsample/upsample chain, so latent
# dims must be divisible by 8 -> the padded image must be divisible by 64.
# (diffusers' PyTorch UNet threads an explicit upsample_size through to handle
# odd sizes; that is a Python-level shape computation and does not survive
# tracing. Marigold itself only pads to 8, which is why feeding a Marigold-sized
# tensor straight into the ONNX graph can fail on a skip-connection concat.)
LATENT_ALIGN = 8
IMAGE_ALIGN = LATENT_ALIGN * 8


class MarigoldOnnx:
    def __init__(self, model_dir, provider="DmlExecutionProvider"):
        avail = ort.get_available_providers()
        providers = [provider] if provider in avail else ["CPUExecutionProvider"]
        self.provider = providers[0]

        def sess(name):
            return ort.InferenceSession(os.path.join(model_dir, name), providers=providers)

        self.enc = sess("vae_encoder.onnx")
        self.dec = sess("vae_decoder.onnx")
        self.unet_iid = sess("unet_iid_lighting.onnx")
        self.unet_normals = sess("unet_normals.onnx")

        with open(os.path.join(model_dir, "scheduler.json")) as f:
            self.sched = json.load(f)
        self.alphas = np.asarray(self.sched["alphas_cumprod"], dtype=np.float64)
        self.ehs = np.fromfile(
            os.path.join(model_dir, "empty_text_embedding.bin"), dtype=np.float32
        ).reshape(self.sched["empty_text_embedding_shape"])

    # -- preprocessing ---------------------------------------------------
    @staticmethod
    def preprocess(rgb_u8, processing_resolution=768):
        """uint8 HxWx3 RGB -> (NCHW float32 in [-1,1], padding, original size)."""
        h, w = rgb_u8.shape[:2]
        mx = max(h, w)
        nh, nw = h * processing_resolution // mx, w * processing_resolution // mx
        # INTER_AREA is the antialiased downscale; matches Marigold's is_aa=True intent
        img = cv2.resize(rgb_u8, (nw, nh), interpolation=cv2.INTER_AREA)
        ph, pw = -nh % IMAGE_ALIGN, -nw % IMAGE_ALIGN
        img = cv2.copyMakeBorder(img, 0, ph, 0, pw, cv2.BORDER_REPLICATE)
        x = img.astype(np.float32) / 255.0 * 2.0 - 1.0
        return x.transpose(2, 0, 1)[None], (ph, pw), (h, w)

    @staticmethod
    def postprocess(x, padding, original):
        """NCHW in [0,1] -> HxWx3, unpadded and resized back to the source size."""
        ph, pw = padding
        img = x[0].transpose(1, 2, 0)
        H, W = img.shape[:2]
        img = img[: H - ph, : W - pw]
        oh, ow = original
        if (img.shape[0], img.shape[1]) != (oh, ow):
            img = cv2.resize(img, (ow, oh), interpolation=cv2.INTER_LINEAR)
        return img

    # -- vae -------------------------------------------------------------
    def encode(self, image_nchw):
        latent = self.enc.run(None, {"image": image_nchw})[0]
        return latent * VAE_SCALE

    def decode(self, latent):
        return self.dec.run(None, {"latent": (latent / VAE_SCALE).astype(np.float32)})[0]

    # -- ddim ------------------------------------------------------------
    def ddim_step(self, model_output, t, sample):
        """One DDIM step, v_prediction, eta=0. Mirrors DDIMScheduler.step."""
        n_train = self.sched["num_train_timesteps"]
        n_steps = len(self.sched["timesteps_4step"])
        prev_t = t - n_train // n_steps

        a_t = self.alphas[t]
        # set_alpha_to_one=False -> the final step falls back to alphas_cumprod[0]
        a_prev = self.alphas[prev_t] if prev_t >= 0 else self.alphas[0]
        b_t = 1.0 - a_t

        v = model_output.astype(np.float64)
        s = sample.astype(np.float64)
        pred_x0 = (a_t ** 0.5) * s - (b_t ** 0.5) * v
        pred_eps = (a_t ** 0.5) * v + (b_t ** 0.5) * s
        if self.sched["clip_sample"]:
            pred_x0 = np.clip(pred_x0, -1.0, 1.0)
        prev = (a_prev ** 0.5) * pred_x0 + ((1.0 - a_prev) ** 0.5) * pred_eps
        return prev.astype(np.float32)

    def denoise(self, session, image_latent, n_targets, noise):
        pred = noise.astype(np.float32)
        for t in self.sched["timesteps_4step"]:
            sample = np.concatenate([image_latent, pred], axis=1).astype(np.float32)
            v = session.run(None, {
                "sample": sample,
                "timestep": np.array(t, dtype=np.int64),
                "encoder_hidden_states": self.ehs,
            })[0]
            pred = self.ddim_step(v, t, pred)
        return pred

    # -- public ----------------------------------------------------------
    def run(self, rgb_u8, processing_resolution=768, seed=0, latents=None):
        image, padding, original = self.preprocess(rgb_u8, processing_resolution)
        image_latent = self.encode(image)
        _, _, h, w = image_latent.shape
        rng = np.random.default_rng(seed)

        out = {}

        n_t = 3
        noise = latents["iid"] if latents else rng.standard_normal((1, n_t * LATENT_CH, h, w), dtype=np.float32)
        pred = self.denoise(self.unet_iid, image_latent, n_t, noise)
        for i, name in enumerate(("albedo", "shading", "residual")):
            d = self.decode(pred[:, i * LATENT_CH:(i + 1) * LATENT_CH])
            d = (np.clip(d, -1.0, 1.0) + 1.0) / 2.0
            out[name] = self.postprocess(d, padding, original)

        noise = latents["normals"] if latents else rng.standard_normal((1, LATENT_CH, h, w), dtype=np.float32)
        pred = self.denoise(self.unet_normals, image_latent, 1, noise)
        d = np.clip(self.decode(pred), -1.0, 1.0)
        # use_full_z_range=True for this checkpoint, so no z remap - just normalize
        d = d / np.maximum(np.linalg.norm(d, axis=1, keepdims=True), 1e-12)
        out["normals"] = self.postprocess(d, padding, original)
        out["normals"] /= np.maximum(
            np.linalg.norm(out["normals"], axis=-1, keepdims=True), 1e-12)

        return out
