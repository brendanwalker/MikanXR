#include "MikanCameraRenderTarget_DX.h"
#include "MikanDirectXMath.h"
#include "MikanCameraEvents.h"
#include "MikanCameraRequests.h"
#include "Logger.h"

MikanCameraRenderTarget_DX::MikanCameraRenderTarget_DX(
	IMikanAPIPtr mikanAPI,
	ID3D11Device* d3dDevice,
	MikanCameraID cameraId)
	: MikanCameraRenderTarget(mikanAPI, cameraId)
	, m_d3dDevice(d3dDevice)
{
}

MikanCameraRenderTarget_DX::~MikanCameraRenderTarget_DX()
{
	// We should have already called dispose() and cleaned this stuff up before the destructor is called
	assert(m_renderTargetTexture == nullptr);
	assert(m_renderTargetView == nullptr);
	assert(m_shaderResourceView == nullptr);
	assert(!m_bHasAllocatedRemoteTexture);
}

bool MikanCameraRenderTarget_DX::createGraphicsAPIResources(int textureWidth, int textureHeight)
{
	bool bSuccess = true;

	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
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
	result = m_d3dDevice->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture);
	if (FAILED(result))
	{
		MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources") 
			<< "Failed to create DX Texture2D "
			<<"(format: DXGI_FORMAT_B8G8R8A8_UNORM"
			<< ", size: " << textureWidth << "x" << textureHeight 
			<< ")";
		return false;
	}

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = m_d3dDevice->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &m_renderTargetView);
	if (FAILED(result))
	{
		MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources") << "Failed to create render target view";
		return false;
	}

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = m_d3dDevice->CreateShaderResourceView(m_renderTargetTexture, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(result))
	{
		MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources") << "Failed to create shader resource view";
		return false;
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

	if (m_shaderResourceView)
	{
		m_shaderResourceView->Release();
		m_shaderResourceView = nullptr;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = nullptr;
	}

	if (m_renderTargetTexture)
	{
		m_renderTargetTexture->Release();
		m_renderTargetTexture = nullptr;
	}
}

void* MikanCameraRenderTarget_DX::getGraphicsApiColorTexturePtr() const
{
	return m_renderTargetTexture;
}

void* MikanCameraRenderTarget_DX::getGraphicsApiDepthTexturePtr() const
{
	// TODO: Render depth to a texture
	return nullptr;
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