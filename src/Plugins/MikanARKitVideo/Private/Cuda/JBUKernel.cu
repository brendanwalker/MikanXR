#include <surface_indirect_functions.h>

// Joint Bilateral Upsampling kernel - device code only (compiled to PTX and loaded
// via the CUDA Driver API by JBUKernel.cpp, not linked directly into any host
// translation unit). Ported from the CudaDepthUpsample prototype's jbu_kernel<T>
// (D:\Github\git-BrendanWalker\CudaDepthUpsample\jbu_cuda.cu), which is already
// raw-device-pointer-based; this drops the prototype's OpenCV
// (cv::cuda::GpuMat)-coupled host wrapper and CUDA Runtime API (<<<>>>) launch
// entirely, per ticket D1. extern "C" linkage on both entry points below keeps
// their exported symbol names un-mangled so cuModuleGetFunction can look them up
// by plain string name.
//
// `confidence` (ticket D2) is optional - pass nullptr to get D1's original,
// confidence-unaware behavior (every valid depth sample contributes full weight).
// When supplied, it must be the same lowW x lowH dimensions as depthLow, with
// ARKit's ARConfidenceLevel convention (0=low, 1=medium, 2=high - see
// ARKitDepthReceiver.h). High confidence always contributes full weight;
// low/medium are deweighted (not hard-skipped) via confWeightLow/confWeightMedium,
// which degrades more gracefully than an outright cutoff at low confidence and
// keeps the "all neighbors excluded" degenerate case (sumW == 0) working the same
// way D1 already handles it, whether that's from all-invalid depth or
// all-zero-weighted confidence.
//
// Ticket D4 adds a second entry point, jbu_upsample_u16_surface_kernel, which
// writes the same computed value via surf2Dwrite() to a CUsurfObject instead of a
// linear/pitched CUdeviceptr - required when the output is a CUDA-GL-interop
// CUarray (e.g. an IMkTexture's backing GL texture, registered via
// cuGraphicsGLRegisterImage/mapped via cuGraphicsSubResourceGetMappedArray), since
// such arrays are opaque, texture-optimized memory that can't be addressed via a
// raw pointer at all. Both entry points share the exact same per-pixel math via
// the computeJbuValue() device helper below, so ticket D1/D2's already-verified
// (against a CPU reference) weighting logic is never duplicated/forked.
namespace
{
// `matte` (person-segmentation gating) is optional - pass nullptr to disable it (leaving
// D1/D2's confidence-and-color-only behavior unchanged). When supplied it is the iPhone's
// NATIVE-resolution person stencil (matteW x matteH, one byte per pixel, 0=not-person /
// 1=person), which is a different resolution than both the low-res depth and the output -
// but it shares the output's field of view (both derive from ARKit's capturedImage), so
// output coords map to matte coords by the scale factors matteScaleX/Y (= matteW/outW,
// matteH/outH). Gating at this guide/output resolution (rather than the coarse 256x192
// depth grid) is what makes the silhouette crisp. A tap whose matte label differs from the
// output pixel's own label is multiplied by segEdgeStrength (0 == hard skip).
__device__ float computeJbuValue(int x, int y, const unsigned short* depthLow, int lowW, int lowH, int lowStrideBytes,
								 const unsigned char* confidence, int confidenceStrideBytes, float confWeightLow,
								 float confWeightMedium, const unsigned char* guideRGB, int guideW, int guideH,
								 int guideStrideBytes, int radius, float invTwoSigmaSpatial2, float invTwoSigmaColor2,
								 float scaleX, float scaleY, const unsigned char* matte, int matteW, int matteH,
								 int matteStrideBytes, float matteScaleX, float matteScaleY, float segEdgeStrength)
{
	// Map high-res pixel (x,y) to low-res coordinates (fx,fy)
	const float fx= (x + 0.5f) * scaleX - 0.5f;
	const float fy= (y + 0.5f) * scaleY - 0.5f;

	const int lx= static_cast<int>(floorf(fx));
	const int ly= static_cast<int>(floorf(fy));

	// Guide pixel (RGB) at the output pixel itself
	const unsigned char* guideRow= guideRGB + y * guideStrideBytes;
	const unsigned char gR= guideRow[3 * x + 0];
	const unsigned char gG= guideRow[3 * x + 1];
	const unsigned char gB= guideRow[3 * x + 2];

	// Segmentation label of the output pixel itself, sampled from the high-res matte at
	// the output pixel's mapped position - each tap below is gated against this so the JBU
	// average never crosses the person/background boundary. Only read when a matte is given.
	unsigned char centerSeg= 0;
	if (matte != nullptr)
	{
		const int cmx= min(matteW - 1, max(0, static_cast<int>(roundf(x * matteScaleX))));
		const int cmy= min(matteH - 1, max(0, static_cast<int>(roundf(y * matteScaleY))));
		centerSeg= (matte + cmy * matteStrideBytes)[cmx];
	}

	float sumW= 0.0f;
	float sumD= 0.0f;

	// Window in low-res coordinates corresponding to `radius` high-res pixels
	const int winLowRadiusX= max(1, static_cast<int>(ceilf(radius * scaleX)));
	const int winLowRadiusY= max(1, static_cast<int>(ceilf(radius * scaleY)));

	const int x0= max(0, lx - winLowRadiusX);
	const int x1= min(lowW - 1, lx + winLowRadiusX);
	const int y0= max(0, ly - winLowRadiusY);
	const int y1= min(lowH - 1, ly + winLowRadiusY);

	for (int j= y0; j <= y1; ++j)
	{
		const unsigned short* depthRow=
			reinterpret_cast<const unsigned short*>(reinterpret_cast<const unsigned char*>(depthLow) + j * lowStrideBytes);
		// High-res y-center of low-res row j
		const float hy= (j + 0.5f) / scaleY - 0.5f;

		for (int i= x0; i <= x1; ++i)
		{
			const unsigned short dval= depthRow[i];
			// 0 = invalid, matching the RVL wire-format convention (see
			// ARKitDepthReceiver.h / ARKitWireProtocol.h) - no isfinite() check
			// needed here (unlike the prototype's float path) since a uint16_t
			// value can never be NaN/Inf.
			if (dval == 0)
				continue;
			const float depthVal= static_cast<float>(dval);

			// Confidence weight first (cheap: at most one extra byte read, no
			// guide-image access) so a fully-deweighted sample (confWeightLow set
			// to 0, the default when no confidence data changes behavior) skips
			// the more expensive guide-color lookup below entirely.
			float wConfidence= 1.0f;
			if (confidence != nullptr)
			{
				const unsigned char* confRow=
					reinterpret_cast<const unsigned char*>(confidence) + j * confidenceStrideBytes;
				const unsigned char c= confRow[i];
				wConfidence= (c >= 2) ? 1.0f : (c == 1) ? confWeightMedium : confWeightLow;
				if (wConfidence <= 0.0f)
					continue;
			}

			// High-res x-center of low-res column i (needed for the matte gate + spatial)
			const float hx= (i + 0.5f) / scaleX - 0.5f;

			// Segmentation gating (cheap: one byte read, no guide access) - sample the
			// high-res matte at this tap's projected output position; a tap on the other
			// side of the silhouette from the output pixel is deweighted by segEdgeStrength
			// (0 == hard skip), so it can't drag the average across the boundary. Checked
			// before the guide-color lookup so a hard-rejected tap skips that read.
			float wSeg= 1.0f;
			if (matte != nullptr)
			{
				const int tmx= min(matteW - 1, max(0, static_cast<int>(roundf(hx * matteScaleX))));
				const int tmy= min(matteH - 1, max(0, static_cast<int>(roundf(hy * matteScaleY))));
				const unsigned char tapSeg= (matte + tmy * matteStrideBytes)[tmx];
				if (tapSeg != centerSeg)
				{
					wSeg= segEdgeStrength;
					if (wSeg <= 0.0f)
						continue;
				}
			}

			const float dx= x - hx;
			const float dy= y - hy;
			const float spatial2= dx * dx + dy * dy;
			const float wSpatial= __expf(-spatial2 * invTwoSigmaSpatial2);

			// Nearest guide sample at this low-res tap's projected high-res position
			const int gx= min(guideW - 1, max(0, static_cast<int>(roundf(hx))));
			const int gy= min(guideH - 1, max(0, static_cast<int>(roundf(hy))));
			const unsigned char* guideRowLow= guideRGB + gy * guideStrideBytes;
			const unsigned char r2= guideRowLow[3 * gx + 0];
			const unsigned char g2= guideRowLow[3 * gx + 1];
			const unsigned char b2= guideRowLow[3 * gx + 2];

			const float dr= static_cast<float>(gR) - static_cast<float>(r2);
			const float dg= static_cast<float>(gG) - static_cast<float>(g2);
			const float db= static_cast<float>(gB) - static_cast<float>(b2);
			const float color2= dr * dr + dg * dg + db * db;
			const float wColor= __expf(-color2 * invTwoSigmaColor2);

			const float w= wSpatial * wColor * wConfidence * wSeg;
			sumW+= w;
			sumD+= w * depthVal;
		}
	}

	return (sumW > 0.0f) ? (sumD / sumW) : 0.0f;
}
} // namespace

extern "C" __global__ void jbu_upsample_u16_kernel(
	const unsigned short* depthLow, int lowW, int lowH, int lowStrideBytes, const unsigned char* confidence,
	int confidenceStrideBytes, float confWeightLow, float confWeightMedium, const unsigned char* guideRGB,
	int guideW, int guideH, int guideStrideBytes, float* depthOut, int outW, int outH, int outStrideBytes, int radius,
	float invTwoSigmaSpatial2, float invTwoSigmaColor2, float scaleX, float scaleY, const unsigned char* matte,
	int matteW, int matteH, int matteStrideBytes, float matteScaleX, float matteScaleY, float segEdgeStrength)
{
	const int x= blockIdx.x * blockDim.x + threadIdx.x;
	const int y= blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= outW || y >= outH)
		return;

	const float outVal= computeJbuValue(x, y, depthLow, lowW, lowH, lowStrideBytes, confidence, confidenceStrideBytes,
										confWeightLow, confWeightMedium, guideRGB, guideW, guideH, guideStrideBytes,
										radius, invTwoSigmaSpatial2, invTwoSigmaColor2, scaleX, scaleY, matte, matteW,
										matteH, matteStrideBytes, matteScaleX, matteScaleY, segEdgeStrength);

	float* outRow= reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(depthOut) + y * outStrideBytes);
	outRow[x]= outVal;
}

// Ticket D4: identical math to jbu_upsample_u16_kernel above, but writes via
// surf2Dwrite() to a CUsurfObject (e.g. a CUDA-GL-interop-mapped texture) instead
// of a linear/pitched device pointer. surf2Dwrite's byte offset is the x
// coordinate scaled by the element size (4 bytes for a float/R32F surface) - see
// CudaGLInterop.h for how the CUsurfObject this expects is created (must wrap a
// single-channel 32-bit float CUarray, matching IMkTexture's MK_R32F format).
extern "C" __global__ void jbu_upsample_u16_surface_kernel(
	const unsigned short* depthLow, int lowW, int lowH, int lowStrideBytes, const unsigned char* confidence,
	int confidenceStrideBytes, float confWeightLow, float confWeightMedium, const unsigned char* guideRGB,
	int guideW, int guideH, int guideStrideBytes, cudaSurfaceObject_t depthOutSurface, int outW, int outH, int radius,
	float invTwoSigmaSpatial2, float invTwoSigmaColor2, float scaleX, float scaleY, const unsigned char* matte,
	int matteW, int matteH, int matteStrideBytes, float matteScaleX, float matteScaleY, float segEdgeStrength)
{
	const int x= blockIdx.x * blockDim.x + threadIdx.x;
	const int y= blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= outW || y >= outH)
		return;

	const float outVal= computeJbuValue(x, y, depthLow, lowW, lowH, lowStrideBytes, confidence, confidenceStrideBytes,
										confWeightLow, confWeightMedium, guideRGB, guideW, guideH, guideStrideBytes,
										radius, invTwoSigmaSpatial2, invTwoSigmaColor2, scaleX, scaleY, matte, matteW,
										matteH, matteStrideBytes, matteScaleX, matteScaleY, segEdgeStrength);

	surf2Dwrite(outVal, depthOutSurface, x * static_cast<int>(sizeof(float)), y, cudaBoundaryModeTrap);
}

// -- Debug visualization kernels (Track E seg-gating diagnostics) ----------------------
// These bypass the bilateral math entirely and nearest-upsample a raw low-res plane into
// the same R32F depth-preview surface, so the video-settings window can show what's
// actually arriving before/independent of JBU. Values are written already-scaled for the
// preview shader (Internal_PT_VisualizeARKitDepth), which displays raw millimeters / 5000
// as grayscale.

// Nearest-neighbor view of the raw low-res uint16 depth (millimeters, unfiltered) - shows
// the blocky pre-JBU depth so it visibly contrasts with the upsampled output.
extern "C" __global__ void viz_depth_nearest_surface_kernel(const unsigned short* depthLow, int lowW, int lowH,
															 int lowStrideBytes, cudaSurfaceObject_t outSurface,
															 int outW, int outH, float scaleX, float scaleY)
{
	const int x= blockIdx.x * blockDim.x + threadIdx.x;
	const int y= blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= outW || y >= outH)
		return;

	const int lx= min(lowW - 1, max(0, static_cast<int>(roundf((x + 0.5f) * scaleX - 0.5f))));
	const int ly= min(lowH - 1, max(0, static_cast<int>(roundf((y + 0.5f) * scaleY - 0.5f))));
	const unsigned short* row=
		reinterpret_cast<const unsigned short*>(reinterpret_cast<const unsigned char*>(depthLow) + ly * lowStrideBytes);

	surf2Dwrite(static_cast<float>(row[lx]), outSurface, x * static_cast<int>(sizeof(float)), y, cudaBoundaryModeTrap);
}

// Nearest-neighbor view of the raw low-res person matte (0/1) - background pixels write 0
// (black), person pixels write personValue (bright under the /5000 shader), so an empty or
// misaligned matte is instantly obvious.
extern "C" __global__ void viz_matte_nearest_surface_kernel(const unsigned char* matte, int lowW, int lowH,
															 int lowStrideBytes, cudaSurfaceObject_t outSurface,
															 int outW, int outH, float scaleX, float scaleY,
															 float personValue)
{
	const int x= blockIdx.x * blockDim.x + threadIdx.x;
	const int y= blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= outW || y >= outH)
		return;

	const int lx= min(lowW - 1, max(0, static_cast<int>(roundf((x + 0.5f) * scaleX - 0.5f))));
	const int ly= min(lowH - 1, max(0, static_cast<int>(roundf((y + 0.5f) * scaleY - 0.5f))));
	const unsigned char label= (matte + ly * lowStrideBytes)[lx];

	surf2Dwrite(label != 0 ? personValue : 0.0f, outSurface, x * static_cast<int>(sizeof(float)), y,
				cudaBoundaryModeTrap);
}

// -- Guided stencil upsampling (human matte refinement) --------------------------------
// Joint-bilateral-upsamples the low-res 0/1 person stencil against the full-res RGB guide,
// producing a soft alpha in [0,1] whose edge snaps to the image's color edge - a crisp
// matte far sharper than nearest-upsampling the coarse stencil. Same guide weighting as
// the depth JBU, but with three deliberate differences: 0 is a VALID label (background),
// not "invalid" (so no skip); there's no confidence plane; and there's no self-gating
// (this pass IS producing the matte). outputScale lets the depth-preview path write
// alpha*5000 for the /5000 grayscale shader, while the real R32F matte texture uses 1.0.
namespace
{
__device__ float computeStencilAlpha(int x, int y, const unsigned char* stencil, int stencilW, int stencilH,
									 int stencilStrideBytes, const unsigned char* guideRGB, int guideW, int guideH,
									 int guideStrideBytes, int radius, float invTwoSigmaSpatial2, float invTwoSigmaColor2,
									 float scaleX, float scaleY)
{
	const float fx= (x + 0.5f) * scaleX - 0.5f;
	const float fy= (y + 0.5f) * scaleY - 0.5f;
	const int lx= static_cast<int>(floorf(fx));
	const int ly= static_cast<int>(floorf(fy));

	const unsigned char* guideRow= guideRGB + y * guideStrideBytes;
	const unsigned char gR= guideRow[3 * x + 0];
	const unsigned char gG= guideRow[3 * x + 1];
	const unsigned char gB= guideRow[3 * x + 2];

	float sumW= 0.0f;
	float sumV= 0.0f;

	const int winRadiusX= max(1, static_cast<int>(ceilf(radius * scaleX)));
	const int winRadiusY= max(1, static_cast<int>(ceilf(radius * scaleY)));

	const int x0= max(0, lx - winRadiusX);
	const int x1= min(stencilW - 1, lx + winRadiusX);
	const int y0= max(0, ly - winRadiusY);
	const int y1= min(stencilH - 1, ly + winRadiusY);

	for (int j= y0; j <= y1; ++j)
	{
		const unsigned char* stencilRow= stencil + j * stencilStrideBytes;
		const float hy= (j + 0.5f) / scaleY - 0.5f;

		for (int i= x0; i <= x1; ++i)
		{
			const float label= static_cast<float>(stencilRow[i]); // 0 or 1, every sample valid

			const float hx= (i + 0.5f) / scaleX - 0.5f;
			const float dx= x - hx;
			const float dy= y - hy;
			const float spatial2= dx * dx + dy * dy;
			const float wSpatial= __expf(-spatial2 * invTwoSigmaSpatial2);

			const int gx= min(guideW - 1, max(0, static_cast<int>(roundf(hx))));
			const int gy= min(guideH - 1, max(0, static_cast<int>(roundf(hy))));
			const unsigned char* guideRowLow= guideRGB + gy * guideStrideBytes;
			const float dr= static_cast<float>(gR) - static_cast<float>(guideRowLow[3 * gx + 0]);
			const float dg= static_cast<float>(gG) - static_cast<float>(guideRowLow[3 * gx + 1]);
			const float db= static_cast<float>(gB) - static_cast<float>(guideRowLow[3 * gx + 2]);
			const float color2= dr * dr + dg * dg + db * db;
			const float wColor= __expf(-color2 * invTwoSigmaColor2);

			const float w= wSpatial * wColor;
			sumW+= w;
			sumV+= w * label;
		}
	}

	return (sumW > 0.0f) ? (sumV / sumW) : 0.0f;
}
} // namespace

extern "C" __global__ void jbu_upsample_stencil_surface_kernel(
	const unsigned char* stencil, int stencilW, int stencilH, int stencilStrideBytes, const unsigned char* guideRGB,
	int guideW, int guideH, int guideStrideBytes, cudaSurfaceObject_t outSurface, int outW, int outH, int radius,
	float invTwoSigmaSpatial2, float invTwoSigmaColor2, float scaleX, float scaleY, float outputScale)
{
	const int x= blockIdx.x * blockDim.x + threadIdx.x;
	const int y= blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= outW || y >= outH)
		return;

	const float alpha= computeStencilAlpha(x, y, stencil, stencilW, stencilH, stencilStrideBytes, guideRGB, guideW,
										   guideH, guideStrideBytes, radius, invTwoSigmaSpatial2, invTwoSigmaColor2,
										   scaleX, scaleY);

	surf2Dwrite(alpha * outputScale, outSurface, x * static_cast<int>(sizeof(float)), y, cudaBoundaryModeTrap);
}
