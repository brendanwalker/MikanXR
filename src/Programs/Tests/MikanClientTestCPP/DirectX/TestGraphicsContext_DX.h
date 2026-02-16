#pragma once

#include "MkRendererFwd.h"
#include "TestGraphicsContext.h"

#include <d3d11.h>
#include <directxmath.h>
#include <memory>

class TestGraphicsContext_DX : 
	public TestGraphicsContext,
	public std::enable_shared_from_this<TestGraphicsContext_DX>
{
public:
	TestGraphicsContext_DX(class TestApp* ownerApp);
	
	virtual MikanClientGraphicsApi getGraphicsApi() const override { return MikanClientGraphicsApi_Direct3D11; } 
	virtual void* getGraphicsDeviceInterface() const override;
	virtual struct SDL_Window* getSDLWindow() const override { return m_sdlWindow; }
	virtual TestCameraRenderTargetPtr allocateCameraRenderTarget(int cameraId) override;
	virtual bool create(int windowWidth, int windowHeight) override;
	virtual void recreateMainRenderTarget() override;
	virtual void renderMainTarget() const override;
	virtual bool renderToCameraTarget(class TestCameraRenderTarget* cameraRenderTarget) override;
	virtual void dispose() override;

protected:
	bool createDeviceD3D();
	void cleanupDeviceD3D();
	void createRenderTarget();
	void cleanupRenderTarget();

	bool initializeQuadGeometry();
	bool initializeCubeGeometry();
	bool initializeCubeShader();
	bool initializeDepthNormalizeShader();
	bool initializeQuadTextureShader();

	void renderNormalizedDepthTexture(class TestCameraRenderTarget_DX* dxRenderTarget);
	void renderColorTexture(ID3D11ShaderResourceView* textureSRV) const;
	void renderCube(
		const DirectX::XMMATRIX& viewProj,
		const DirectX::XMFLOAT3& cameraPosition,
		const DirectX::XMFLOAT3& cameraForward,
		const DirectX::XMFLOAT3& cameraUp,
		const DirectX::XMFLOAT3& cameraRight) const;

private:
	struct SDL_Window* m_sdlWindow = nullptr;
	bool m_sdlInitialized= false;

	int m_windowWidth = 0;
	int m_windowHeight = 0;

	ID3D11Device* m_pd3dDevice = nullptr;
	ID3D11DeviceContext* m_pd3dDeviceContext = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;

	// Cube shader state
	ID3D11VertexShader* m_cubeVertexShader = nullptr;
	ID3D11PixelShader* m_cubePixelShader = nullptr;
	ID3D11Buffer* m_cubeConstantBuffer = nullptr;
	ID3D11InputLayout* m_cubeInputLayout = nullptr;
	ID3D11Buffer* m_cubeVertexBuffer = nullptr;
	int m_cubeVertexCount = 0;

	// Depth normalize shader state
	ID3D11VertexShader* m_depthNormalizeVertexShader = nullptr;
	ID3D11PixelShader* m_depthNormalizePixelShader = nullptr;
	ID3D11Buffer* m_depthNormalizeConstantBuffer = nullptr;
	ID3D11SamplerState* m_depthNormalizeSamplerState = nullptr;

	// Depth Normalize Color Target
	int m_depthNormalizeTargetWidth = 0;
	int m_depthNormalizeTargetHeight = 0;
	ID3D11Texture2D* m_depthNormalizeColorTargetTexture = nullptr;
	ID3D11RenderTargetView* m_depthNormalizeColorTargetView = nullptr;
	ID3D11ShaderResourceView* m_depthNormalizeColorTargetSRV = nullptr;

	// Quad texture shader state
	ID3D11VertexShader* m_quadTextureVertexShader = nullptr;
	ID3D11PixelShader* m_quadTexturePixelShader = nullptr;
	ID3D11SamplerState* m_quadTextureSamplerState = nullptr;
	ID3D11InputLayout* m_quadInputLayout = nullptr;
	ID3D11Buffer* m_quadVertexBuffer = nullptr;

	// Animation state
	DirectX::XMMATRIX m_defaultViewProjectionMatrix;
	MikanCameraID m_lastRenderedCameraId = INVALID_MIKAN_ID;

	static const float kDefaultZNear;
	static const float kDefaultZFar;
	static const DirectX::XMFLOAT3 kDefaultCameraPosition;
	static const DirectX::XMFLOAT3 kDefaultCameraFocusPoint;
	static const DirectX::XMFLOAT3 kDefaultCameraRight;
	static const DirectX::XMFLOAT3 kDefaultCameraUp;
	static const DirectX::XMFLOAT3 kDefaultCameraForward;
};