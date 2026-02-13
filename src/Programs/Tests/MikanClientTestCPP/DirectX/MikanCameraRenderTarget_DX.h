#pragma once

#include "MikanAPI.h"
#include "MikanCameraRenderTarget.h"

#include <d3d11_1.h>
#include <directxmath.h>
//dxgiformat.h

class MikanCameraRenderTarget_DX : public TestCameraRenderTarget
{
public:
	MikanCameraRenderTarget_DX(IMikanAPIPtr mikanAPI, ID3D11Device* d3dDevice, int cameraId);
	virtual ~MikanCameraRenderTarget_DX();

	inline ID3D11Texture2D* getColorTexture() const { return m_colorTargetTexture; }
	inline ID3D11Texture2D* getDepthTexture() const { return m_floatDepthTargetTexture; }

	inline ID3D11RenderTargetView* getColorTargetView() const { return m_colorTargetView; }
	inline ID3D11DepthStencilView* getDepthTargetView() const { return m_floatDepthTargetView; }

	inline ID3D11ShaderResourceView* getColorTextureSRV() const { return m_colorTargetSRV; }
	inline ID3D11ShaderResourceView* getFloatDepthTextureSRV() const { return m_floatDepthTargetSRV; }

	inline bool getIsInitialized() const { m_colorTargetTexture != nullptr && m_floatDepthTargetTexture != nullptr; }

	DXGI_FORMAT getDepthResourceFormat(DXGI_FORMAT depthFormat);
	DXGI_FORMAT getDepthSRVFormat(DXGI_FORMAT depthFormat);

protected:
	virtual bool createGraphicsAPIResources(int textureWidth, int textureHeight) override;
	virtual void bindGraphicsAPIResource() override;
	virtual void unbindGraphicsAPIResource() override;
	virtual void freeGraphicsAPIResources() override;
	virtual void* getGraphicsApiColorTexturePtr() const override;
	virtual void* getGraphicsApiDepthTexturePtr() const override;

	virtual void updateCameraViewMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) override;
	virtual void updateCameraProjectionMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) override;

private:
	ID3D11Device* m_d3dDevice= nullptr;
	
	// Direct3D11 Color Target
	ID3D11Texture2D* m_colorTargetTexture = nullptr;
	ID3D11RenderTargetView* m_colorTargetView = nullptr;
	ID3D11ShaderResourceView* m_colorTargetSRV = nullptr;

	// Direct3D11 Depth Target
	ID3D11Texture2D* m_floatDepthTargetTexture = nullptr;
	ID3D11DepthStencilView* m_floatDepthTargetView = nullptr;
	ID3D11ShaderResourceView* m_floatDepthTargetSRV = nullptr;

	DirectX::XMMATRIX m_projMatrix;
	DirectX::XMMATRIX m_viewMatrix;
};