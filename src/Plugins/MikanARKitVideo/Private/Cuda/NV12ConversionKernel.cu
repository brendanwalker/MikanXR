#include <surface_indirect_functions.h>

// NV12 -> RGB(A) conversion - device code only (compiled to PTX and loaded via the
// CUDA Driver API by NV12ConversionKernel.cpp, matching JBUKernel.cu's pattern - see
// that file's own header comment for why: this project hand-writes small CUDA
// kernels rather than depending on GStreamer conversion elements whose in-process
// availability on a given dev machine isn't reliable (see project memory
// project_gstreamer_inprocess_decoder_plugin_gap - the same class of issue that
// motivated JBUKernel already).
//
// nvh264dec's CUDA memory output is NV12 (see `gst-inspect-1.0 nvh264dec` -
// video/x-raw(memory:CUDAMemory),format=NV12): a full-resolution 8-bit luma (Y)
// plane followed by a half-resolution, 2x-subsampled, interleaved chroma (UV)
// plane. This kernel reads both planes once per output pixel and writes two
// outputs derived from the same YUV->RGB conversion:
//  - RGBA8 via surf2Dwrite() into a CUDA-GL-interop-mapped surface (for display -
//    see CudaGLColorTexture in CudaGLInterop.h).
//  - Tightly-packed RGB (3 bytes/pixel, no alpha) into a plain linear device
//    buffer - this becomes JBUKernel's `guideRGB` input (see JBUKernel.cu's
//    computeJbuValue(), which indexes the guide image as 3 bytes/pixel), so the
//    JBU depth upsample doesn't need its own separate color conversion pass.
//
// Conversion assumes BT.601, limited (video) range - the conventional encoding for
// H.264 video from a phone camera. If real captured colors look off (e.g.
// washed-out/oversaturated), this is the first place to revisit - it hasn't been
// empirically verified against a real ARKit capture the way JBUKernel's tuning
// parameters were (ticket D5).
extern "C" __global__ void nv12_to_rgb_kernel(const unsigned char* yPlane, int yStrideBytes,
											  const unsigned char* uvPlane, int uvStrideBytes, int width, int height,
											  cudaSurfaceObject_t rgbaSurface, unsigned char* guideRGBOut,
											  int guideStrideBytes)
{
	const int x= blockIdx.x * blockDim.x + threadIdx.x;
	const int y= blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= width || y >= height)
		return;

	const unsigned char yVal= yPlane[y * yStrideBytes + x];

	const int uvRow= y / 2;
	const int uvCol= (x / 2) * 2;
	const unsigned char uVal= uvPlane[uvRow * uvStrideBytes + uvCol + 0];
	const unsigned char vVal= uvPlane[uvRow * uvStrideBytes + uvCol + 1];

	// BT.601 limited-range YUV -> RGB.
	const float fy= 1.164f * (static_cast<float>(yVal) - 16.0f);
	const float fu= static_cast<float>(uVal) - 128.0f;
	const float fv= static_cast<float>(vVal) - 128.0f;

	const float fr= fy + 1.596f * fv;
	const float fg= fy - 0.391f * fu - 0.813f * fv;
	const float fb= fy + 2.018f * fu;

	const unsigned char r= static_cast<unsigned char>(fminf(255.0f, fmaxf(0.0f, fr)));
	const unsigned char g= static_cast<unsigned char>(fminf(255.0f, fmaxf(0.0f, fg)));
	const unsigned char b= static_cast<unsigned char>(fminf(255.0f, fmaxf(0.0f, fb)));

	const uchar4 rgba= make_uchar4(r, g, b, 255);
	surf2Dwrite(rgba, rgbaSurface, x * static_cast<int>(sizeof(uchar4)), y, cudaBoundaryModeTrap);

	unsigned char* guideRow= guideRGBOut + y * guideStrideBytes;
	guideRow[3 * x + 0]= r;
	guideRow[3 * x + 1]= g;
	guideRow[3 * x + 2]= b;
}
