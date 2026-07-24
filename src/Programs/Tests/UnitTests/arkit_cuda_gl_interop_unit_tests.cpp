//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include "unit_test.h"

// Same compile-time guard as arkit_jbu_kernel_unit_tests.cpp - this file needs
// CUDA (JBU_KERNEL_PTX_PATH is only defined when MIKAN_WITH_GSTREAMER is ON).
#if defined(JBU_KERNEL_PTX_PATH)

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
#include "JBUKernel.h"

// Full end-to-end coverage for ticket D4's CudaGLDepthTexture - the one piece of
// Track D that genuinely needs a live OpenGL context, which this otherwise-
// headless console test executable doesn't normally have (see
// arkit_jbu_kernel_unit_tests.cpp's file-level comment for why the kernel math
// itself is tested separately, without GL). This file creates a minimal, hidden,
// off-screen WGL context purely for this one test's own use (no SDL/MikanWindow
// dependency - self-contained Win32+WGL, in the same spirit as
// MikanARKitVideoDevice.cpp's own direct <windows.h> usage). Confirmed working on
// this dev machine: real GL texture creation, cuGraphicsGLRegisterImage,
// map/write-via-kernel/unmap, and a glGetTexImage readback all round-trip
// correctly. Still gated at runtime (skips gracefully, doesn't fail) if
// window/GL/CUDA-GL-interop context creation fails, since a headless CI runner
// with no display adapter is a real possibility this repo has no other precedent
// for handling.
//
// IMPORTANT test-ordering note: this module must run BEFORE
// run_arkit_jbu_kernel_unit_tests in unit_test_suite.cpp - see the comment there
// (and project memory: project_cuda_context_corruption_after_fault) for why a
// module that creates its own fresh CUDA context must never run after
// arkit_jbu_kernel's deliberate-fault test.
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
	UNIT_TEST_BEGIN("CudaGLDepthTexture: a JBU write via CUDA is visible when the texture is read back through GL")

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

	JBUKernel kernel;
	success= success && kernel.init(JBU_KERNEL_PTX_PATH);
	assert(success);

	CudaGLDepthTexture depthTexture;
	success= success && depthTexture.init(64, 48);
	assert(success);

	const int lowW= 16, lowH= 12;
	const int outW= depthTexture.getWidth();
	const int outH= depthTexture.getHeight();
	const uint16_t kConstantDepthMM= 4321;

	std::vector<uint16_t> depthLow(static_cast<size_t>(lowW) * lowH, kConstantDepthMM);
	std::vector<uint8_t> guide(static_cast<size_t>(outW) * outH * 3, 128);

	CUdeviceptr d_depthLow= 0, d_guide= 0;
	success= success && (cuMemAlloc(&d_depthLow, depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guide, guide.size()) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_depthLow, depthLow.data(), depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_guide, guide.data(), guide.size()) == CUDA_SUCCESS);
	assert(success);

	CUsurfObject surface= 0;
	success= success && depthTexture.beginCudaAccess(surface);
	assert(success);

	JBUParams params;
	success= success
			 && kernel.upsampleToSurface(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), 0, 0,
										 d_guide, outW, outH, outW * 3, surface, outW, outH, params);
	assert(success);
	success= success && kernel.synchronize();
	assert(success);

	success= success && depthTexture.endCudaAccess(nullptr);
	assert(success);

	std::vector<float> readback(static_cast<size_t>(outW) * outH, -1.0f);
	if (success)
	{
		glBindTexture(GL_TEXTURE_2D, depthTexture.getTexture()->getGlTextureId());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, readback.data());
		glBindTexture(GL_TEXTURE_2D, 0);
		success= success && (glGetError() == GL_NO_ERROR);
		assert(success);
	}

	bool allMatch= true;
	for (float v : readback)
	{
		if (std::fabs(v - static_cast<float>(kConstantDepthMM)) > 0.5f)
		{
			allMatch= false;
			break;
		}
	}
	success= success && allMatch;
	assert(success);

	if (d_depthLow != 0)
		cuMemFree(d_depthLow);
	if (d_guide != 0)
		cuMemFree(d_guide);
	depthTexture.shutdown();
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

#else // !defined(JBU_KERNEL_PTX_PATH)

bool run_arkit_cuda_gl_interop_unit_tests()
{
	fprintf(stdout, "[arkit_cuda_gl_interop]\n");
	fprintf(stdout, "  skipped - built without MIKAN_WITH_GSTREAMER\n");
	return true;
}

#endif // defined(JBU_KERNEL_PTX_PATH)
