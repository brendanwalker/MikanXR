#pragma once

#include "MikanAPI.h"
#include "MikanCameraRenderTarget.h"

#include <d3d11_1.h>
#include <directxmath.h>

class MikanCameraRenderTarget_DX : public MikanCameraRenderTarget
{
public:
	MikanCameraRenderTarget_DX(IMikanAPIPtr mikanAPI, ID3D11Device* d3dDevice, int cameraId);
	virtual ~MikanCameraRenderTarget_DX();

	ID3D11Texture2D* getRenderTarget() const { return m_renderTargetTexture; }
	ID3D11RenderTargetView* getRenderTargetView() const { return m_renderTargetView; }
	ID3D11ShaderResourceView* getShaderResourceView() const { return m_shaderResourceView; }

protected:
	virtual bool createGraphicsAPIResources(int textureWidth, int textureHeight) override;
	virtual void freeGraphicsAPIResources() override;
	virtual void* getGraphicsApiColorTexturePtr() const override;
	virtual void* getGraphicsApiDepthTexturePtr() const override;

	virtual void updateCameraViewMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) override;
	virtual void updateCameraProjectionMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) override;

private:
	ID3D11Device* m_d3dDevice= nullptr;
	
	ID3D11Texture2D* m_renderTargetTexture = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11ShaderResourceView* m_shaderResourceView = nullptr;

	DirectX::XMMATRIX m_projMatrix;
	DirectX::XMMATRIX m_viewMatrix;
};