#include "TestApp.h"
#include "TestGraphicsContext_DX.h"
#include "DirectX/TestCameraRenderTarget_DX.h"
#include "Logger.h"

#if defined(_WIN32)
	#include <SDL.h>
	#include <SDL_syswm.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_syswm.h>
#endif

#include <memory>

TestGraphicsContext_DX::TestGraphicsContext_DX(TestApp* ownerApp)
	: TestGraphicsContext(ownerApp)
{}

TestCameraRenderTargetPtr TestGraphicsContext_DX::allocateCameraRenderTarget(
	int cameraId)
{
	return 
		std::make_shared<TestCameraRenderTarget_DX>(
			shared_from_this(), m_pd3dDevice, cameraId);
}

bool TestGraphicsContext_DX::create(int windowWidth, int windowHeight)
{
	bool success = true;

	const char* glsl_version = nullptr;
	if (success)
	{
		const SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

		m_sdlWindow = SDL_CreateWindow("Mikan Client Test",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			windowWidth, windowHeight,
			window_flags);
		m_windowWidth = windowWidth;
		m_windowHeight = windowHeight;

		if (m_sdlWindow == NULL)
		{
			MIKAN_LOG_ERROR("startup") << "Unable to initialize window: " << SDL_GetError();
			success = false;
		}
	}

	// Initialize Direct3D
	if (!createDeviceD3D())
	{
		cleanupDeviceD3D();
		return 1;
	}



	return success;
}

void TestGraphicsContext_DX::recreateMainRenderTarget()
{
	cleanupRenderTarget();
	m_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	createRenderTarget();
}

void TestGraphicsContext_DX::renderMainTarget() const
{
	const float kClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

	// Bind the main render target view
	m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);

	// Clear the back buffer 
	m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, kClearColor);

	// TODO: Draw the most recently rendered camera texture to the back buffer

	// Present (without vsync) the back buffer
	m_pSwapChain->Present(0, 0);
}

bool TestGraphicsContext_DX::renderToCameraTarget(
	TestCameraRenderTarget* cameraRenderTarget)
{
	//TODO
	//DirectX::XMVECTOR cubePosition =
	//	cameraPosition +
	//	cameraForward * cubeOffset.z +
	//	cameraUp * cubeOffset.y +
	//	cameraRight * cubeOffset.x;

	//DirectX::XMMATRIX cubeTransform =
	//	DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
	//	DirectX::XMMatrixRotationX(time) *
	//	DirectX::XMMatrixRotationY(time * 2.0f) *
	//	DirectX::XMMatrixRotationZ(time * 0.7f) *
	//	DirectX::XMMatrixTranslationFromVector(cubePosition);

	// TODO: Render the scene to the render target texture using the camera intrinsics and extrinsics from the new frame event.
	//m_pd3dDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	//m_pd3dDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
	//m_pd3dDeviceContext->Draw(3, 0);

	return true;
}

void TestGraphicsContext_DX::dispose()
{
	cleanupDeviceD3D();

	if (m_sdlWindow != nullptr)
	{
		SDL_DestroyWindow(m_sdlWindow);
		m_sdlWindow = nullptr;
	}
}

bool TestGraphicsContext_DX::createDeviceD3D()
{
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(m_sdlWindow, &wmInfo);
	HWND hWnd = (HWND)wmInfo.info.win.window;

	// Setup swap chain
	// This is a basic setup. 
	// Optimally could use e.g. DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently. 
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = 
		D3D11CreateDeviceAndSwapChain(
			nullptr, 
			D3D_DRIVER_TYPE_HARDWARE, 
			nullptr, 
			createDeviceFlags, 
			featureLevelArray, 
			2, 
			D3D11_SDK_VERSION, 
			&sd, 
			&m_pSwapChain, 
			&m_pd3dDevice, 
			&featureLevel, 
			&m_pd3dDeviceContext);

	// Try high-performance WARP software driver if hardware is not available
	if (res == DXGI_ERROR_UNSUPPORTED) 
	{
		res = D3D11CreateDeviceAndSwapChain(
			nullptr, 
			D3D_DRIVER_TYPE_WARP, 
			nullptr, 
			createDeviceFlags, 
			featureLevelArray, 
			2, 
			D3D11_SDK_VERSION, 
			&sd, 
			&m_pSwapChain, 
			&m_pd3dDevice, 
			&featureLevel, 
			&m_pd3dDeviceContext);
	}

	if (res != S_OK)
	{
		return false;
	}

	createRenderTarget();

	return true;
}

void TestGraphicsContext_DX::cleanupDeviceD3D()
{
	cleanupRenderTarget();

	if (m_pSwapChain) 
	{
		m_pSwapChain->Release(); 
		m_pSwapChain = nullptr; 
	}

	if (m_pd3dDeviceContext) 
	{ 
		m_pd3dDeviceContext->Release(); 
		m_pd3dDeviceContext = nullptr; 
	}

	if (m_pd3dDevice) 
	{ 
		m_pd3dDevice->Release(); 
		m_pd3dDevice = nullptr; 
	}
}

void TestGraphicsContext_DX::createRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;

	m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (pBackBuffer != nullptr)
	{
		m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
	}
	pBackBuffer->Release();
}

void TestGraphicsContext_DX::cleanupRenderTarget()
{
	if (m_mainRenderTargetView) 
	{ 
		m_mainRenderTargetView->Release(); 
		m_mainRenderTargetView = nullptr; 
	}
}