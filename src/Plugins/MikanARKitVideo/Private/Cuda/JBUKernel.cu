// Joint Bilateral Upsampling kernel - device code only (compiled to PTX and loaded
// via the CUDA Driver API by JBUKernel.cpp, not linked directly into any host
// translation unit). Ported from the CudaDepthUpsample prototype's jbu_kernel<T>
// (D:\Github\git-BrendanWalker\CudaDepthUpsample\jbu_cuda.cu), which is already
// raw-device-pointer-based; this drops the prototype's OpenCV
// (cv::cuda::GpuMat)-coupled host wrapper and CUDA Runtime API (<<<>>>) launch
// entirely, per ticket D1. extern "C" linkage keeps the exported symbol name
// un-mangled so cuModuleGetFunction can look it up by plain string name.
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
extern "C" __global__ void jbu_upsample_u16_kernel(
	const unsigned short* depthLow, int lowW, int lowH, int lowStrideBytes, const unsigned char* confidence,
	int confidenceStrideBytes, float confWeightLow, float confWeightMedium, const unsigned char* guideRGB,
	int guideW, int guideH, int guideStrideBytes, float* depthOut, int outW, int outH, int outStrideBytes, int radius,
	float invTwoSigmaSpatial2, float invTwoSigmaColor2, float scaleX, float scaleY)
{
	const int x= blockIdx.x * blockDim.x + threadIdx.x;
	const int y= blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= outW || y >= outH)
		return;

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

			// High-res x-center of low-res column i
			const float hx= (i + 0.5f) / scaleX - 0.5f;

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

			const float w= wSpatial * wColor * wConfidence;
			sumW+= w;
			sumD+= w * depthVal;
		}
	}

	const float outVal= (sumW > 0.0f) ? (sumD / sumW) : 0.0f;
	float* outRow= reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(depthOut) + y * outStrideBytes);
	outRow[x]= outVal;
}
