#include "MikanCameraRenderTarget_DX.h"
#include "MikanDirectXMath.h"
#include "MikanDirectXUtils.h"
#include "MikanCameraEvents.h"
#include "MikanCameraRequests.h"
#include "Logger.h"

MikanCameraRenderTarget_DX::MikanCameraRenderTarget_DX(
	IMikanAPIPtr mikanAPI,
	ID3D11Device* d3dDevice,
	MikanCameraID cameraId)
	: TestCameraRenderTarget(mikanAPI, cameraId)
	, m_d3dDevice(d3dDevice)
{
}

MikanCameraRenderTarget_DX::~MikanCameraRenderTarget_DX()
{
	// We should have already called dispose() and cleaned this stuff up before the destructor is called
	assert(m_colorTargetTexture == nullptr);
	assert(m_colorTargetView == nullptr);
	assert(m_colorTargetSRV == nullptr);
	assert(!m_bHasAllocatedRemoteTexture);
}

DXGI_FORMAT MikanCameraRenderTarget_DX::getDepthResourceFormat(DXGI_FORMAT depthFormat)
{
	DXGI_FORMAT resultFormat= DXGI_FORMAT_UNKNOWN;

	switch (depthFormat)
	{
	case DXGI_FORMAT_D16_UNORM:
		resultFormat= DXGI_FORMAT_R16G16_TYPELESS;
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		resultFormat = DXGI_FORMAT_R24G8_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT:
		resultFormat = DXGI_FORMAT_R32_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		resultFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
		break;
	}

	return resultFormat;
}

DXGI_FORMAT MikanCameraRenderTarget_DX::getDepthSRVFormat(DXGI_FORMAT depthFormat)
{
	DXGI_FORMAT resultFormat = DXGI_FORMAT_UNKNOWN;

	switch (depthFormat)
	{
	case DXGI_FORMAT_D16_UNORM:
		resultFormat = DXGI_FORMAT_R16_FLOAT;
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		resultFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT:
		resultFormat = DXGI_FORMAT_R32_FLOAT;
		break;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		resultFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		break;
	}

	return resultFormat;
}

bool MikanCameraRenderTarget_DX::createGraphicsAPIResources(int textureWidth, int textureHeight)
{
	// Create the render target resources
	// -------
	bool bSuccess = true;

	// Create the color render target texture.
	{
		HRESULT result;

		// Setup the color render target texture description.
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = (UINT)textureWidth;
		textureDesc.Height = (UINT)textureHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_TYPELESS; //  DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the render target texture.
		result = m_d3dDevice->CreateTexture2D(&textureDesc, NULL, &m_colorTargetTexture);
		if (FAILED(result))
		{
			MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources")
				<< "Failed to create Color DX Texture2D "
				<< "(format: DXGI_FORMAT_B8G8R8A8_UNORM"
				<< ", size: " << textureWidth << "x" << textureHeight
				<< ")";
			return false;
		}

		// Setup the description of the render target view.
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
		renderTargetViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		// Create the render target view.
		result = m_d3dDevice->CreateRenderTargetView(m_colorTargetTexture, &renderTargetViewDesc, &m_colorTargetView);
		if (FAILED(result))
		{
			MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources") << "Failed to create color render target view";
			return false;
		}

		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
		shaderResourceViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		// Create the shader resource view.
		result = m_d3dDevice->CreateShaderResourceView(m_colorTargetTexture, &shaderResourceViewDesc, &m_colorTargetSRV);
		if (FAILED(result))
		{
			MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources") << "Failed to create color shader resource view";
			return false;
		}
	}

	// Create the depth render target texture
	{
		HRESULT result;

		DXGI_FORMAT depthViewFormat = DXGI_FORMAT_D32_FLOAT;
		DXGI_FORMAT resourceFormat = getDepthResourceFormat(depthViewFormat);
		DXGI_FORMAT srvFormat = getDepthSRVFormat(depthViewFormat);

		// Setup the depth render target texture description.
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = (UINT)textureWidth;
		textureDesc.Height = (UINT)textureHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = resourceFormat;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the render target texture.
		result = m_d3dDevice->CreateTexture2D(&textureDesc, NULL, &m_floatDepthTargetTexture);
		if (FAILED(result))
		{
			MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources")
				<< "Failed to create Float Depth DX Texture2D "
				<< "(format: " << DXGIFormatToString(resourceFormat)
				<< ", size: " << textureWidth << "x" << textureHeight
				<< ")";
			return false;
		}

		// Create the depth stencil view.
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
		depthStencilViewDesc.Format = depthViewFormat;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_d3dDevice->CreateDepthStencilView(m_floatDepthTargetTexture, &depthStencilViewDesc, &m_floatDepthTargetView);
		if (FAILED(result))
		{
			MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources") 
				<< "Failed to create depth stencil view"
				<< "(format: " << DXGIFormatToString(depthViewFormat)
				<< ", size: " << textureWidth << "x" << textureHeight
				<< ")";
			return false;
		}

		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
		shaderResourceViewDesc.Format = srvFormat;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		// Create the shader resource view.
		result = m_d3dDevice->CreateShaderResourceView(m_floatDepthTargetTexture, &shaderResourceViewDesc, &m_floatDepthTargetSRV);
		if (FAILED(result))
		{
			MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources") 
				<< "Failed to create depth shader resource view"
				<< "(format: " << DXGIFormatToString(srvFormat)
				<< ", size: " << textureWidth << "x" << textureHeight
				<< ")";
			return false;
		}
	}

	// Remember the size of the render target once created
	m_width = textureWidth;
	m_height = textureHeight;

	return true;
}

void MikanCameraRenderTarget_DX::freeGraphicsAPIResources()
{
	m_width = 0;
	m_height = 0;

	if (m_colorTargetSRV)
	{
		m_colorTargetSRV->Release();
		m_colorTargetSRV = nullptr;
	}

	if (m_colorTargetView)
	{
		m_colorTargetView->Release();
		m_colorTargetView = nullptr;
	}

	if (m_colorTargetTexture)
	{
		m_colorTargetTexture->Release();
		m_colorTargetTexture = nullptr;
	}

	if (m_floatDepthTargetSRV)
	{
		m_floatDepthTargetSRV->Release();
		m_floatDepthTargetSRV = nullptr;
	}

	if (m_floatDepthTargetView)
	{
		m_floatDepthTargetView->Release();
		m_floatDepthTargetView = nullptr;
	}

	if (m_floatDepthTargetTexture)
	{
		m_floatDepthTargetTexture->Release();
		m_floatDepthTargetTexture = nullptr;
	}
}

void* MikanCameraRenderTarget_DX::getGraphicsApiColorTexturePtr() const
{
	return m_colorTargetTexture;
}

void* MikanCameraRenderTarget_DX::getGraphicsApiDepthTexturePtr() const
{
	return m_floatDepthTargetTexture;
}

void MikanCameraRenderTarget_DX::updateCameraViewMatrix(const MikanCameraNewFrameEvent& newFrameEvent)
{
	const MikanVector3f& cameraForward = newFrameEvent.camera_forward;
	const MikanVector3f& cameraUp = newFrameEvent.camera_up;
	const MikanVector3f& cameraPosition = newFrameEvent.camera_position;

	m_viewMatrix = mikan_camera_pose_to_directx_view_matrix(cameraForward, cameraUp, cameraPosition);
}

void MikanCameraRenderTarget_DX::updateCameraProjectionMatrix(const MikanCameraNewFrameEvent& newFrameEvent)
{
	const float fx = newFrameEvent.focal_length.x;
	const float fy = newFrameEvent.focal_length.y;
	const float cx = newFrameEvent.principal_point.x;
	const float cy = newFrameEvent.principal_point.y;
	const float width = newFrameEvent.pixel_size.x;
	const float height = newFrameEvent.pixel_size.y;
	const float zNear = newFrameEvent.z_bounds.x;
	const float zFar = newFrameEvent.z_bounds.y;

	m_projMatrix = mikan_camera_intrinsics_to_directx_projection_matrix(fx, fy, cx, cy, width, height, zNear, zFar);
}

void MikanCameraRenderTarget_DX::bindGraphicsAPIResource() const
{
	// Get the immediate context from the device
	ID3D11DeviceContext* deviceContext = nullptr;
	m_d3dDevice->GetImmediateContext(&deviceContext);

	if (deviceContext == nullptr)
	{
		MIKAN_LOG_ERROR("MikanCameraRenderTarget_DX::bindGraphicsAPIResource")
			<< "Failed to get device context";
		return;
	}

	// Set the output render views (depth target first, then color target)
	deviceContext->OMSetRenderTargets(1, &m_colorTargetView, m_floatDepthTargetView);

	// Set the viewport dimensions
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(m_width);
	viewport.Height = static_cast<float>(m_height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &viewport);

	// Clear the render target view (black with alpha 0)
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	deviceContext->ClearRenderTargetView(m_colorTargetView, clearColor);

	// Clear the depth stencil view (depth value 1.0, stencil 0)
	deviceContext->ClearDepthStencilView(m_floatDepthTargetView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Release the device context reference
	deviceContext->Release();
}

void MikanCameraRenderTarget_DX::unbindGraphicsAPIResource()
{
	ID3D11RenderTargetView* nullRTV = nullptr;
	ID3D11DepthStencilView* nullDSV = nullptr;

	deviceContext->OMSetRenderTargets(1, &nullRTV, nullDSV);
}