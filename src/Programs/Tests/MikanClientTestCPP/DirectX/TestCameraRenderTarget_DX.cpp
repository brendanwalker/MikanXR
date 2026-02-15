#include "MikanCameraEvents.h"
#include "MikanCameraRequests.h"
#include "TestCameraRenderTarget_DX.h"
#include "TestDirectXMath.h"
#include "TestDirectXUtils.h"
#include "Logger.h"

TestCameraRenderTarget_DX::TestCameraRenderTarget_DX(
	TestGraphicsContextPtr ownerContext,
	ID3D11Device* d3dDevice,
	MikanCameraID cameraId)
	: TestCameraRenderTarget(ownerContext, cameraId)
	, m_d3dDevice(d3dDevice)
	, m_projMatrix(DirectX::XMMatrixIdentity())
	, m_viewMatrix(DirectX::XMMatrixIdentity())
	, m_cameraPosition(0.0f, 0.0f, 0.0f)
	, m_cameraForward(0.0f, 0.0f, 1.0f)
	, m_cameraUp(0.0f, 1.0f, 0.0f)
	, m_cameraRight(1.0f, 0.0f, 0.0f)
{
}

TestCameraRenderTarget_DX::~TestCameraRenderTarget_DX()
{
	// We should have already called dispose() and cleaned this stuff up before the destructor is called
	assert(m_colorTargetTexture == nullptr);
	assert(m_colorTargetView == nullptr);
	assert(m_colorTargetSRV == nullptr);
	assert(!m_bHasAllocatedRemoteTexture);
}

bool TestCameraRenderTarget_DX::createGraphicsAPIResources(int textureWidth, int textureHeight)
{
	// Create the color render target texture.
	if (!createColorRenderTargetResources(
		m_d3dDevice,
		textureWidth,
		textureHeight,
		&m_colorTargetTexture,
		&m_colorTargetView,
		&m_colorTargetSRV))
	{
		MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources") << "Failed to create color render target";
		return false;
	}

	// Create the depth render target texture
	if (!createDepthRenderTargetResources(
		m_d3dDevice,
		textureWidth,
		textureHeight,
		&m_floatDepthTargetTexture,
		&m_floatDepthTargetView,
		&m_floatDepthTargetSRV))
	{
		MIKAN_LOG_ERROR("MikanCameraRenderTarget::createDirectXResources") << "Failed to create depth render target";
		return false;
	}

	return true;
}

void TestCameraRenderTarget_DX::freeGraphicsAPIResources()
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

void* TestCameraRenderTarget_DX::getGraphicsApiColorTexturePtr() const
{
	return m_colorTargetTexture;
}

void* TestCameraRenderTarget_DX::getGraphicsApiDepthTexturePtr() const
{
	return m_floatDepthTargetTexture;
}

void TestCameraRenderTarget_DX::updateCameraViewMatrix(const MikanCameraNewFrameEvent& newFrameEvent)
{
	const MikanVector3f& mkCameraForward = newFrameEvent.camera_forward;
	const MikanVector3f& mkCameraUp = newFrameEvent.camera_up;
	const MikanVector3f& mkCameraPosition = newFrameEvent.camera_position;

	DirectX::XMVECTOR xmPosition = mikan_position_to_directx_xmvector(mkCameraPosition);
	DirectX::XMVECTOR xmForward = mikan_vec3_to_directx_xmvector(mkCameraForward);
	DirectX::XMVECTOR xmUp = mikan_vec3_to_directx_xmvector(mkCameraUp);
	DirectX::XMVECTOR xmRight = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(xmForward, xmUp));

	DirectX::XMStoreFloat3(&m_cameraPosition, xmPosition);
	DirectX::XMStoreFloat3(&m_cameraForward, xmForward);
	DirectX::XMStoreFloat3(&m_cameraUp, xmUp);
	DirectX::XMStoreFloat3(&m_cameraRight, xmRight);

	m_viewMatrix = mikan_camera_pose_to_directx_view_matrix(mkCameraForward, mkCameraUp, mkCameraPosition);
}

void TestCameraRenderTarget_DX::updateCameraProjectionMatrix(const MikanCameraNewFrameEvent& newFrameEvent)
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

void TestCameraRenderTarget_DX::bindGraphicsAPIResource()
{
	// Get the immediate context from the device
	ID3D11DeviceContext* deviceContext = nullptr;
	m_d3dDevice->GetImmediateContext(&deviceContext);

	if (deviceContext == nullptr)
	{
		MIKAN_LOG_ERROR("TestCameraRenderTarget_DX::bindGraphicsAPIResource")
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

void TestCameraRenderTarget_DX::unbindGraphicsAPIResource()
{
	// Get the immediate context from the device
	ID3D11DeviceContext* deviceContext = nullptr;
	m_d3dDevice->GetImmediateContext(&deviceContext);

	if (deviceContext != nullptr)
	{
		// Unbind the render target and depth stencil views by setting them to null
		ID3D11RenderTargetView* nullRTV = nullptr;
		ID3D11DepthStencilView* nullDSV = nullptr;
		deviceContext->OMSetRenderTargets(1, &nullRTV, nullDSV);

		// Release the device context reference
		deviceContext->Release();
	}
}