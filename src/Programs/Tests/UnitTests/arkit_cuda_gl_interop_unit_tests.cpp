//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include "unit_test.h"

// Same compile-time guard as before - this file needs CUDA (NV12_KERNEL_PTX_PATH
// is only defined when MIKAN_WITH_GSTREAMER is ON).
#if defined(NV12_KERNEL_PTX_PATH)

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <assert.h>
#include <cmath>
#include <cstdint>
#include <vector>

#include <GL/glew.h>

#include <cuda.h>

#include "CudaGLInterop.h"
#include "IMkTexture.h"
#include "NV12ConversionKernel.h"

// Full end-to-end coverage for ticket D4/E3's CudaGLColorTexture - the one piece of
// this CUDA-GL interop pipeline that genuinely needs a live OpenGL context, which
// this otherwise-headless console test executable doesn't normally have. This file
// creates a minimal, hidden, off-screen WGL context purely for this one test's own
// use (no SDL/MikanWindow dependency - self-contained Win32+WGL, in the same
// spirit as MikanARKitVideoDevice.cpp's own direct <windows.h> usage). Feeds a
// small constant-color NV12 input through NV12ConversionKernel::convert() and
// confirms the RGBA output is visible via a glGetTexImage readback - the same
// round-trip MikanARKitVideoDevice::updateColorTexture() performs live per frame.
// Still gated at runtime (skips gracefully, doesn't fail) if window/GL/CUDA-GL-
// interop context creation fails, since a headless CI runner with no display
// adapter is a real possibility this repo has no other precedent for handling.
namespace
{
bool cudaGlInteropTestCreateHiddenGLContext(HWND& outWindow, HDC& outDC, HGLRC& outGLRC)
{
	outWindow= nullptr;
	outDC= nullptr;
	outGLRC= nullptr;

	static const wchar_t* kClassName= L"MikanARKitVideoCudaGLInteropTestWindow";
	static bool classRegistered= false;
	if (!classRegistered)
	{
		WNDCLASSW windowClass= {};
		windowClass.lpfnWndProc= DefWindowProcW;
		windowClass.hInstance= GetModuleHandleW(nullptr);
		windowClass.lpszClassName= kClassName;
		if (RegisterClassW(&windowClass) == 0)
			return false;
		classRegistered= true;
	}

	HWND window= CreateWindowExW(0, kClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 64, 64, nullptr, nullptr,
								 GetModuleHandleW(nullptr), nullptr);
	if (window == nullptr)
		return false;

	HDC dc= GetDC(window);
	if (dc == nullptr)
	{
		DestroyWindow(window);
		return false;
	}

	PIXELFORMATDESCRIPTOR pfd= {};
	pfd.nSize= sizeof(pfd);
	pfd.nVersion= 1;
	pfd.dwFlags= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType= PFD_TYPE_RGBA;
	pfd.cColorBits= 32;
	pfd.cDepthBits= 24;

	const int pixelFormat= ChoosePixelFormat(dc, &pfd);
	if (pixelFormat == 0 || !SetPixelFormat(dc, pixelFormat, &pfd))
	{
		ReleaseDC(window, dc);
		DestroyWindow(window);
		return false;
	}

	HGLRC glrc= wglCreateContext(dc);
	if (glrc == nullptr || !wglMakeCurrent(dc, glrc))
	{
		if (glrc != nullptr)
			wglDeleteContext(glrc);
		ReleaseDC(window, dc);
		DestroyWindow(window);
		return false;
	}

	static bool glewInitialized= false;
	if (!glewInitialized)
	{
		glewExperimental= GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			wglMakeCurrent(nullptr, nullptr);
			wglDeleteContext(glrc);
			ReleaseDC(window, dc);
			DestroyWindow(window);
			return false;
		}
		glewInitialized= true;
	}

	outWindow= window;
	outDC= dc;
	outGLRC= glrc;
	return true;
}

void cudaGlInteropTestDestroyHiddenGLContext(HWND window, HDC dc, HGLRC glrc)
{
	wglMakeCurrent(nullptr, nullptr);
	if (glrc != nullptr)
		wglDeleteContext(glrc);
	if (window != nullptr && dc != nullptr)
		ReleaseDC(window, dc);
	if (window != nullptr)
		DestroyWindow(window);
}

bool cudaGlInteropTestCudaDeviceAvailable(CUdevice& outDevice)
{
	if (cuInit(0) != CUDA_SUCCESS)
		return false;

	int deviceCount= 0;
	if (cuDeviceGetCount(&deviceCount) != CUDA_SUCCESS || deviceCount <= 0)
		return false;

	return cuDeviceGet(&outDevice, 0) == CUDA_SUCCESS;
}
} // namespace

//-- private functions -----
static bool arkit_cuda_gl_interop_test_writeback_visible_via_gl_readback()
{
	UNIT_TEST_BEGIN(
		"CudaGLColorTexture: an NV12ConversionKernel write is visible when the texture is read back through GL")

	HWND window= nullptr;
	HDC dc= nullptr;
	HGLRC glrc= nullptr;
	if (!cudaGlInteropTestCreateHiddenGLContext(window, dc, glrc))
	{
		// Off-screen GL context creation isn't guaranteed to work in every
		// environment this suite runs in (e.g. no display adapter) - not a
		// failure of this test, nothing further to verify.
		UNIT_TEST_COMPLETE()
	}

	CUdevice device;
	if (!cudaGlInteropTestCudaDeviceAvailable(device))
	{
		cudaGlInteropTestDestroyHiddenGLContext(window, dc, glrc);
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	NV12ConversionKernel kernel;
	success= success && kernel.init(NV12_KERNEL_PTX_PATH);
	assert(success);

	CudaGLColorTexture colorTexture;
	success= success && colorTexture.init(64, 48);
	assert(success);

	const int outW= colorTexture.getWidth();
	const int outH= colorTexture.getHeight();

	// Y=235/U=128/V=128 is a constant, chosen so BT.601 limited-range decode
	// (see NV12ConversionKernel.cu) yields RGBA(254,254,254,255) uniformly:
	// fy = 1.164 * (235 - 16) = 254.916 -> 254; fu=fv=0 so fr=fg=fb=fy.
	const int yStrideBytes= outW;
	const int uvStrideBytes= outW; // interleaved UV, half-height, same row stride as Y
	std::vector<uint8_t> yPlane(static_cast<size_t>(yStrideBytes) * outH, 235);
	std::vector<uint8_t> uvPlane(static_cast<size_t>(uvStrideBytes) * (outH / 2), 128);

	const int guideStrideBytes= outW * 3;
	std::vector<uint8_t> guideReadback(static_cast<size_t>(guideStrideBytes) * outH, 0);

	CUdeviceptr d_yPlane= 0, d_uvPlane= 0, d_guideRgb= 0;
	success= success && (cuMemAlloc(&d_yPlane, yPlane.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_uvPlane, uvPlane.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guideRgb, guideReadback.size()) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_yPlane, yPlane.data(), yPlane.size()) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_uvPlane, uvPlane.data(), uvPlane.size()) == CUDA_SUCCESS);
	assert(success);

	CUsurfObject surface= 0;
	success= success && colorTexture.beginCudaAccess(surface);
	assert(success);

	success= success
			 && kernel.convert(d_yPlane, yStrideBytes, d_uvPlane, uvStrideBytes, outW, outH, surface, d_guideRgb,
							   guideStrideBytes);
	assert(success);
	success= success && kernel.synchronize();
	assert(success);

	success= success && colorTexture.endCudaAccess(nullptr);
	assert(success);

	std::vector<uint8_t> readback(static_cast<size_t>(outW) * outH * 4, 0);
	if (success)
	{
		glBindTexture(GL_TEXTURE_2D, colorTexture.getTexture()->getGlTextureId());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, readback.data());
		glBindTexture(GL_TEXTURE_2D, 0);
		success= success && (glGetError() == GL_NO_ERROR);
		assert(success);
	}

	bool allMatch= true;
	for (size_t i= 0; i + 3 < readback.size() && allMatch; i+= 4)
	{
		allMatch=
			(readback[i + 0] == 254 && readback[i + 1] == 254 && readback[i + 2] == 254 && readback[i + 3] == 255);
	}
	success= success && allMatch;
	assert(success);

	if (d_yPlane != 0)
		cuMemFree(d_yPlane);
	if (d_uvPlane != 0)
		cuMemFree(d_uvPlane);
	if (d_guideRgb != 0)
		cuMemFree(d_guideRgb);
	colorTexture.shutdown();
	kernel.shutdown();
	if (context != nullptr)
		cuCtxDestroy(context);
	cudaGlInteropTestDestroyHiddenGLContext(window, dc, glrc);

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_cuda_gl_interop_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_cuda_gl_interop")
	UNIT_TEST_MODULE_CALL_TEST(arkit_cuda_gl_interop_test_writeback_visible_via_gl_readback);
	UNIT_TEST_MODULE_END()
}

#else // !defined(NV12_KERNEL_PTX_PATH)

bool run_arkit_cuda_gl_interop_unit_tests()
{
	fprintf(stdout, "[arkit_cuda_gl_interop]\n");
	fprintf(stdout, "  skipped - built without MIKAN_WITH_GSTREAMER\n");
	return true;
}

#endif // defined(NV12_KERNEL_PTX_PATH)
