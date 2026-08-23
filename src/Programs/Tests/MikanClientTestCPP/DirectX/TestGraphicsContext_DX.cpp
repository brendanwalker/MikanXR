#include "TestApp.h"
#include "TestGraphicsContext_DX.h"
#include "TestCameraRenderTarget_DX.h"
#include "TestDirectXUtils.h"
#include "Logger.h"

#if defined(_WIN32)
#include <SDL.h>
#include <SDL_syswm.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#endif

#include <DirectXMath.h>
#include <memory>

// Constants used by the depth normalize shader
#pragma warning(push)
#pragma warning(disable : 4324) // Disable warning about structure padding due to alignas(16)
struct alignas(16) DepthNormalizeConstants
{
	float zNear;
	float zFar;
};
#pragma warning(pop)

TestGraphicsContext_DX::TestGraphicsContext_DX(TestApp* ownerApp)
	: TestGraphicsContext(ownerApp)
{
}

TestCameraRenderTargetPtr TestGraphicsContext_DX::allocateCameraRenderTarget(int cameraId)
{
	return std::make_shared<TestCameraRenderTarget_DX>(shared_from_this(), m_pd3dDevice, cameraId);
}

void* TestGraphicsContext_DX::getGraphicsDeviceInterface() const { return m_pd3dDevice; }

bool TestGraphicsContext_DX::create(int windowWidth, int windowHeight)
{
	const SDL_WindowFlags window_flags= (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	m_sdlWindow= SDL_CreateWindow("Mikan Client Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth,
								  windowHeight, window_flags);
	m_windowWidth= windowWidth;
	m_windowHeight= windowHeight;

	if (m_sdlWindow == nullptr)
	{
		MIKAN_LOG_ERROR("startup") << "Unable to initialize window: " << SDL_GetError();
		return false;
	}

	if (!createDeviceD3D())
	{
		MIKAN_LOG_ERROR("startup") << "Failed to create D3D device!";
		return false;
	}

	if (!initializeCubeShader() || !initializeDepthNormalizeShader() || !initializeQuadTextureShader())
	{
		MIKAN_LOG_ERROR("startup") << "Failed to initialize shaders!";
		return false;
	}

	if (!initializeCubeGeometry() || !initializeQuadGeometry())
	{
		MIKAN_LOG_ERROR("startup") << "Failed to initialize geometry!";
		return false;
	}

	return true;
}

void TestGraphicsContext_DX::recreateMainRenderTarget()
{
	cleanupRenderTarget();
	m_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	createRenderTarget();
}

void TestGraphicsContext_DX::renderMainTarget() const
{
	const float kClearColor[4]= {0.0f, 0.0f, 1.0f, 1.0f};

	// Bind the main render target view
	m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);

	// Clear the back buffer
	m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, kClearColor);

	// Draw the most recently rendered camera texture to the back buffer
	TestCameraRenderTargetPtr renderTarget= getCameraRenderTarget(m_lastRenderedCameraId);
	if (renderTarget)
	{
		const TestRenderMode DrawDepthMode= m_ownerApp->getRenderMode();
		auto dxRenderTarget= std::static_pointer_cast<TestCameraRenderTarget_DX>(renderTarget);

		switch (DrawDepthMode)
		{
		case TestRenderMode::Color:
			renderColorTexture(dxRenderTarget->getColorTextureSRV());
			break;
		case TestRenderMode::DepthNormalize:
			renderColorTexture(m_depthNormalizeColorTargetSRV);
			break;
		case TestRenderMode::PackedDepth:
			renderPackedDepthTexture(dxRenderTarget.get(), getOwnerApp()->getMikanAPI());
			break;
		}
	}

	// Present (without vsync) the back buffer
	m_pSwapChain->Present(0, 0);
}

bool TestGraphicsContext_DX::renderToCameraTarget(TestCameraRenderTarget* cameraRenderTarget)
{
	auto* dxRenderTarget= static_cast<TestCameraRenderTarget_DX*>(cameraRenderTarget);

	const DirectX::XMFLOAT3& cameraPos= dxRenderTarget->getCameraPosition();
	const DirectX::XMFLOAT3& cameraForward= dxRenderTarget->getCameraForward();
	const DirectX::XMFLOAT3& cameraUp= dxRenderTarget->getCameraUp();
	const DirectX::XMFLOAT3& cameraRight= dxRenderTarget->getCameraRight();
	const DirectX::XMMATRIX viewProj= dxRenderTarget->getViewProjectionMatrix();
	renderCube(viewProj, cameraPos, cameraForward, cameraUp, cameraRight);

	// If we're in depth normalize mode, also render the normalized depth texture for this camera.
	if (m_ownerApp->getRenderMode() == TestRenderMode::DepthNormalize)
	{
		renderNormalizedDepthTexture(dxRenderTarget);
	}

	// Remember the last rendered camera ID
	// We will render this camera in renderMainTarget
	m_lastRenderedCameraId= dxRenderTarget->getCameraId();

	return true;
}

void TestGraphicsContext_DX::renderNormalizedDepthTexture(TestCameraRenderTarget_DX* dxRenderTarget)
{
	// Create the color render target texture.
	if (m_depthNormalizeTargetWidth != dxRenderTarget->getWidth()
		|| m_depthNormalizeTargetHeight != dxRenderTarget->getHeight())
	{
		// Release existing resources if they exist
		if (m_depthNormalizeColorTargetSRV)
		{
			m_depthNormalizeColorTargetSRV->Release();
			m_depthNormalizeColorTargetSRV= nullptr;
		}

		if (m_depthNormalizeColorTargetView)
		{
			m_depthNormalizeColorTargetView->Release();
			m_depthNormalizeColorTargetView= nullptr;
		}

		if (m_depthNormalizeColorTargetTexture)
		{
			m_depthNormalizeColorTargetTexture->Release();
			m_depthNormalizeColorTargetTexture= nullptr;
		}

		// Create new resources with the correct size
		if (!createColorRenderTargetResources(m_pd3dDevice, dxRenderTarget->getWidth(), dxRenderTarget->getHeight(),
											  &m_depthNormalizeColorTargetTexture, &m_depthNormalizeColorTargetView,
											  &m_depthNormalizeColorTargetSRV))
		{
			MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources")
				<< "Failed to create color render target";
			return;
		}

		m_depthNormalizeTargetWidth= dxRenderTarget->getWidth();
		m_depthNormalizeTargetHeight= dxRenderTarget->getHeight();
	}

	// Bind the main render target view
	m_pd3dDeviceContext->OMSetRenderTargets(1, &m_depthNormalizeColorTargetView, nullptr);

	// Assign the quad vertices
	UINT stride= sizeof(DirectX::XMFLOAT3) + sizeof(DirectX::XMFLOAT2); // position (float3) + texcoord (float2)
	UINT offset= 0;
	m_pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_quadVertexBuffer, &stride, &offset);

	// Assign the normalized depth shader
	m_pd3dDeviceContext->IASetInputLayout(m_quadInputLayout);
	m_pd3dDeviceContext->VSSetShader(m_depthNormalizeVertexShader, nullptr, 0);
	m_pd3dDeviceContext->PSSetShader(m_depthNormalizePixelShader, nullptr, 0);

	// Set the texture
	ID3D11ShaderResourceView* textureSRV= dxRenderTarget->getFloatDepthTextureSRV();
	m_pd3dDeviceContext->PSSetShaderResources(0, 1, &textureSRV);
	m_pd3dDeviceContext->PSSetSamplers(0, 1, &m_depthNormalizeSamplerState);

	// Update the depth normalization constants
	DepthNormalizeConstants depthNormalizeConstants= {};
	depthNormalizeConstants.zNear= dxRenderTarget->getZNear();
	depthNormalizeConstants.zFar= dxRenderTarget->getZFar();

	D3D11_MAPPED_SUBRESOURCE pMappedResource= {};
	m_pd3dDeviceContext->Map(m_depthNormalizeConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pMappedResource);
	memcpy(pMappedResource.pData, &depthNormalizeConstants, sizeof(DepthNormalizeConstants));
	m_pd3dDeviceContext->Unmap(m_depthNormalizeConstantBuffer, 0);
	m_pd3dDeviceContext->PSSetConstantBuffers(0, 1, &m_depthNormalizeConstantBuffer);

	// Draw the quad
	m_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pd3dDeviceContext->Draw(6, 0);
}

void TestGraphicsContext_DX::dispose()
{
	cleanupDeviceD3D();

	if (m_sdlWindow != nullptr)
	{
		SDL_DestroyWindow(m_sdlWindow);
		m_sdlWindow= nullptr;
	}
}

bool TestGraphicsContext_DX::createDeviceD3D()
{
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(m_sdlWindow, &wmInfo);
	HWND hWnd= (HWND)wmInfo.info.win.window;

	// Setup swap chain
	// This is a basic setup.
	// Optimally could use e.g. DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently.
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount= 2;
	sd.BufferDesc.Width= 0;
	sd.BufferDesc.Height= 0;
	sd.BufferDesc.Format= DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator= 60;
	sd.BufferDesc.RefreshRate.Denominator= 1;
	sd.Flags= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow= hWnd;
	sd.SampleDesc.Count= 1;
	sd.SampleDesc.Quality= 0;
	sd.Windowed= TRUE;
	sd.SwapEffect= DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags= 0;
	// createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2]= {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0,
	};
	HRESULT res= D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
											   featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain,
											   &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);

	// Try high-performance WARP software driver if hardware is not available
	if (res == DXGI_ERROR_UNSUPPORTED)
	{
		res= D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray,
										   2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel,
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

	// Cleanup cube shader resources
	if (m_cubeVertexBuffer)
	{
		m_cubeVertexBuffer->Release();
		m_cubeVertexBuffer= nullptr;
	}

	if (m_cubeInputLayout)
	{
		m_cubeInputLayout->Release();
		m_cubeInputLayout= nullptr;
	}

	if (m_cubeConstantBuffer)
	{
		m_cubeConstantBuffer->Release();
		m_cubeConstantBuffer= nullptr;
	}

	if (m_cubePixelShader)
	{
		m_cubePixelShader->Release();
		m_cubePixelShader= nullptr;
	}

	if (m_cubeVertexShader)
	{
		m_cubeVertexShader->Release();
		m_cubeVertexShader= nullptr;
	}

	// Cleanup depth normalize shader resources
	if (m_depthNormalizeSamplerState)
	{
		m_depthNormalizeSamplerState->Release();
		m_depthNormalizeSamplerState= nullptr;
	}

	if (m_depthNormalizeConstantBuffer)
	{
		m_depthNormalizeConstantBuffer->Release();
		m_depthNormalizeConstantBuffer= nullptr;
	}

	if (m_depthNormalizePixelShader)
	{
		m_depthNormalizePixelShader->Release();
		m_depthNormalizePixelShader= nullptr;
	}

	if (m_depthNormalizeVertexShader)
	{
		m_depthNormalizeVertexShader->Release();
		m_depthNormalizeVertexShader= nullptr;
	}

	// Cleanup depth normalize color target resources
	if (m_depthNormalizeColorTargetSRV)
	{
		m_depthNormalizeColorTargetSRV->Release();
		m_depthNormalizeColorTargetSRV= nullptr;
	}

	if (m_depthNormalizeColorTargetView)
	{
		m_depthNormalizeColorTargetView->Release();
		m_depthNormalizeColorTargetView= nullptr;
	}

	if (m_depthNormalizeColorTargetTexture)
	{
		m_depthNormalizeColorTargetTexture->Release();
		m_depthNormalizeColorTargetTexture= nullptr;
	}

	// Cleanup quad texture shader resources
	if (m_quadVertexBuffer)
	{
		m_quadVertexBuffer->Release();
		m_quadVertexBuffer= nullptr;
	}

	if (m_quadInputLayout)
	{
		m_quadInputLayout->Release();
		m_quadInputLayout= nullptr;
	}

	if (m_quadTextureSamplerState)
	{
		m_quadTextureSamplerState->Release();
		m_quadTextureSamplerState= nullptr;
	}

	if (m_quadTexturePixelShader)
	{
		m_quadTexturePixelShader->Release();
		m_quadTexturePixelShader= nullptr;
	}

	if (m_quadTextureVertexShader)
	{
		m_quadTextureVertexShader->Release();
		m_quadTextureVertexShader= nullptr;
	}

	// Cleanup D3D device resources
	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain= nullptr;
	}

	if (m_pd3dDeviceContext)
	{
		m_pd3dDeviceContext->Release();
		m_pd3dDeviceContext= nullptr;
	}

	if (m_pd3dDevice)
	{
		m_pd3dDevice->Release();
		m_pd3dDevice= nullptr;
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
		m_mainRenderTargetView= nullptr;
	}
}

bool TestGraphicsContext_DX::initializeQuadGeometry()
{
	struct QuadVertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoord;
	};

	// Fullscreen quad vertex data (position + texcoord)
	QuadVertex vertices[]= {
		{DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)},
		{DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f)},
		{DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)},

		{DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)},
		{DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)},
		{DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)},
	};

	// Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage= D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth= sizeof(vertices);
	bufferDesc.BindFlags= D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags= 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem= vertices;

	HRESULT hr= m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, &m_quadVertexBuffer);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("initializeQuadGeometry") << "Failed to create quad vertex buffer";
		return false;
	}

	return true;
}

bool TestGraphicsContext_DX::initializeCubeGeometry()
{
	struct CubeVertex
	{
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 color;
	};

	// Cube vertex data (position + color)
	CubeVertex vertices[]= {
		// Front face (Red)
		{DirectX::XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, 1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, 1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},

		// Back face (Green)
		{DirectX::XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},

		// Top face (Blue)
		{DirectX::XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, 1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},

		// Bottom face (Yellow)
		{DirectX::XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},

		// Left face (Magenta)
		{DirectX::XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)},

		// Right face (Cyan)
		{DirectX::XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, 1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)},
	};

	m_cubeVertexCount= ARRAYSIZE(vertices);

	// Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage= D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth= sizeof(vertices);
	bufferDesc.BindFlags= D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags= 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem= vertices;

	HRESULT hr= m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, &m_cubeVertexBuffer);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("initializeCubeGeometry") << "Failed to create cube vertex buffer";
		return false;
	}

	return true;
}

bool TestGraphicsContext_DX::initializeCubeShader()
{
	const std::string shaderCode= R"(
		struct VS_INPUT
		{
			float4 pos : POSITION;
			float4 col : COLOR;
		};
		struct PS_INPUT
		{
			float4 pos : SV_POSITION;
			float4 col : COLOR;
		};

		float4x4 worldViewProj;

		PS_INPUT vs_main(VS_INPUT input)
		{
			PS_INPUT output = (PS_INPUT)0;

			output.pos = mul(input.pos, worldViewProj);
			output.col = input.col;

			return output;
		}

		float4 ps_main(PS_INPUT input) : SV_TARGET
		{
			return input.col;
		}
	)";

	// Compile and create shaders
	ID3DBlob* vertexShaderByteCode= nullptr;
	if (!compileAndCreateShaders(m_pd3dDevice, shaderCode, &m_cubeVertexShader, &m_cubePixelShader,
								 &vertexShaderByteCode))
	{
		return false;
	}

	// Create constant buffer
	if (!createConstantBuffer(m_pd3dDevice, sizeof(DirectX::XMMATRIX), D3D11_USAGE_DEFAULT, 0, &m_cubeConstantBuffer))
	{
		vertexShaderByteCode->Release();
		return false;
	}

	// Create input layout
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements= {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}};

	if (!createInputLayout(m_pd3dDevice, inputElements, vertexShaderByteCode, &m_cubeInputLayout))
	{
		vertexShaderByteCode->Release();
		return false;
	}

	vertexShaderByteCode->Release();
	return true;
}

bool TestGraphicsContext_DX::initializeDepthNormalizeShader()
{
	const std::string shaderCode= R"(
		Texture2D<float> InputTexture : register(t0);
		SamplerState samLinear : register(s0);

		cbuffer ConstantBuffer : register(b0)
		{
			float zNear;
			float zFar;
		};

		struct VS_INPUT
		{
			float3 position : POSITION;
			float2 texCoord : TEXCOORD;
		};

		struct PS_INPUT
		{
			float4 pos : SV_POSITION;
			float2 uv : TEXCOORD;
		};

		PS_INPUT vs_main(VS_INPUT input)
		{
			PS_INPUT output;
			output.pos = float4(input.position, 1.0f);
			output.uv = input.texCoord;
			return output;
		}

		float4 ps_main(PS_INPUT input) : SV_TARGET
		{
			// Read the raw depth value from the input float depth texture [0, 1]
			// DirectX uses [0, 1] NDC range (unlike OpenGL's [-1, 1])
			float depth = InputTexture.Sample(samLinear, input.uv).r;

			// Convert to linear eye-space depth using DirectX perspective projection formula
			float eyeDepth = zNear * zFar / (zFar - depth * (zFar - zNear));

			// Normalize to [0, 1] range for visualization
			float zNorm = eyeDepth / zFar;

			return float4(zNorm, zNorm, zNorm, 1.0);
		}
	)";

	// Compile and create shaders
	ID3DBlob* vertexShaderByteCode= nullptr;
	if (!compileAndCreateShaders(m_pd3dDevice, shaderCode, &m_depthNormalizeVertexShader, &m_depthNormalizePixelShader,
								 &vertexShaderByteCode))
	{
		return false;
	}

	if (!createConstantBuffer(m_pd3dDevice, sizeof(DepthNormalizeConstants), D3D11_USAGE_DYNAMIC,
							  D3D11_CPU_ACCESS_WRITE, &m_depthNormalizeConstantBuffer))
	{
		vertexShaderByteCode->Release();
		return false;
	}

	// Create sampler state
	if (!createSamplerState(m_pd3dDevice, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP,
							&m_depthNormalizeSamplerState))
	{
		vertexShaderByteCode->Release();
		return false;
	}

	vertexShaderByteCode->Release();
	return true;
}

bool TestGraphicsContext_DX::initializeQuadTextureShader()
{
	const std::string shaderCode= R"(
		Texture2D<float4> InputTexture : register(t0);
		SamplerState samLinear : register(s0);

		struct VS_INPUT
		{
			float3 position : POSITION;
			float2 texCoord : TEXCOORD;
		};

		struct PS_INPUT
		{
			float4 pos : SV_POSITION;
			float2 uv : TEXCOORD;
		};

		PS_INPUT vs_main(VS_INPUT input)
		{
			PS_INPUT output;
			output.pos = float4(input.position, 1.0f);
			output.uv = input.texCoord;
			return output;
		}

		float4 ps_main(PS_INPUT input) : SV_TARGET
		{
			return InputTexture.Sample(samLinear, input.uv);
		}
	)";

	// Compile and create shaders
	ID3DBlob* vertexShaderByteCode= nullptr;
	if (!compileAndCreateShaders(m_pd3dDevice, shaderCode, &m_quadTextureVertexShader, &m_quadTexturePixelShader,
								 &vertexShaderByteCode))
	{
		return false;
	}

	// Create input layout
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements= {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}};

	if (!createInputLayout(m_pd3dDevice, inputElements, vertexShaderByteCode, &m_quadInputLayout))
	{
		vertexShaderByteCode->Release();
		return false;
	}

	// Create sampler state
	if (!createSamplerState(m_pd3dDevice, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP,
							&m_quadTextureSamplerState))
	{
		vertexShaderByteCode->Release();
		return false;
	}

	vertexShaderByteCode->Release();
	return true;
}

void TestGraphicsContext_DX::renderColorTexture(ID3D11ShaderResourceView* textureSRV) const
{
	// Assign the quad vertices
	UINT stride= sizeof(DirectX::XMFLOAT3) + sizeof(DirectX::XMFLOAT2); // position (float3) + texcoord (float2)
	UINT offset= 0;
	m_pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_quadVertexBuffer, &stride, &offset);

	// Assign the quad shader
	m_pd3dDeviceContext->IASetInputLayout(m_quadInputLayout);
	m_pd3dDeviceContext->VSSetShader(m_quadTextureVertexShader, nullptr, 0);
	m_pd3dDeviceContext->PSSetShader(m_quadTexturePixelShader, nullptr, 0);

	// Set the texture
	m_pd3dDeviceContext->PSSetShaderResources(0, 1, &textureSRV);
	m_pd3dDeviceContext->PSSetSamplers(0, 1, &m_quadTextureSamplerState);

	// Draw the quad
	m_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pd3dDeviceContext->Draw(6, 0);
}

void TestGraphicsContext_DX::renderPackedDepthTexture(TestCameraRenderTarget_DX* dxRenderTarget,
													  IMikanAPIPtr mikanApi) const
{
	MikanCameraID cameraId= dxRenderTarget->getCameraId();

	void* packedDepthTextureHandle= nullptr;
	if (mikanApi->getCameraPackDepthTextureResourcePtr(cameraId, &packedDepthTextureHandle) == MikanAPIResult::Success)
	{
		auto* packDepthSRV= reinterpret_cast<ID3D11ShaderResourceView*>(packedDepthTextureHandle);

		if (packDepthSRV)
		{
			renderColorTexture(packDepthSRV);
		}
	}
}

void TestGraphicsContext_DX::renderCube(const DirectX::XMMATRIX& viewProj, const DirectX::XMFLOAT3& cameraPosition,
										const DirectX::XMFLOAT3& cameraForward, const DirectX::XMFLOAT3& cameraUp,
										const DirectX::XMFLOAT3& cameraRight) const
{
	const float time= m_ownerApp->getTimeSeconds();
	const MikanVector3f cubeOffset= m_ownerApp->getCubeOffset();

	// Assign the cube vertices
	UINT stride= sizeof(DirectX::XMFLOAT4) * 2; // position (float4) + color (float4)
	UINT offset= 0;
	m_pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_cubeVertexBuffer, &stride, &offset);

	// Assign the cube shader
	m_pd3dDeviceContext->IASetInputLayout(m_cubeInputLayout);
	m_pd3dDeviceContext->VSSetShader(m_cubeVertexShader, nullptr, 0);
	m_pd3dDeviceContext->PSSetShader(m_cubePixelShader, nullptr, 0);

	// Compute cube position
	DirectX::XMVECTOR xmCameraPos= DirectX::XMLoadFloat3(&cameraPosition);
	DirectX::XMVECTOR xmCameraFwd= DirectX::XMLoadFloat3(&cameraForward);
	DirectX::XMVECTOR xmCameraUp= DirectX::XMLoadFloat3(&cameraUp);
	DirectX::XMVECTOR xmCameraRight= DirectX::XMLoadFloat3(&cameraRight);

	DirectX::XMVECTOR cubePosition= xmCameraPos;
	cubePosition= DirectX::XMVectorMultiplyAdd(xmCameraFwd, DirectX::XMVectorReplicate(cubeOffset.z), cubePosition);
	cubePosition= DirectX::XMVectorMultiplyAdd(xmCameraUp, DirectX::XMVectorReplicate(cubeOffset.y), cubePosition);
	cubePosition= DirectX::XMVectorMultiplyAdd(xmCameraRight, DirectX::XMVectorReplicate(cubeOffset.x), cubePosition);

	// Build cube transformation matrix
	DirectX::XMMATRIX cubeModelViewProj= DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) * DirectX::XMMatrixRotationX(time)
										 * DirectX::XMMatrixRotationY(time * 2.0f)
										 * DirectX::XMMatrixRotationZ(time * 0.7f)
										 * DirectX::XMMatrixTranslationFromVector(cubePosition) * viewProj;

	// Transpose for HLSL (column-major)
	cubeModelViewProj= DirectX::XMMatrixTranspose(cubeModelViewProj);

	// Update constant buffer
	m_pd3dDeviceContext->UpdateSubresource(m_cubeConstantBuffer, 0, nullptr, &cubeModelViewProj, 0, 0);

	// Set the constant buffer
	m_pd3dDeviceContext->VSSetConstantBuffers(0, 1, &m_cubeConstantBuffer);

	// Draw the cube
	m_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pd3dDeviceContext->Draw(m_cubeVertexCount, 0);
}