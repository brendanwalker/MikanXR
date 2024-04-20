#pragma once

#include "SpoutDX.h"

class SpoutDXDepthTexturePacker
{
public:
	SpoutDXDepthTexturePacker(spoutDX& spout);
	virtual ~SpoutDXDepthTexturePacker();

	bool init();
	ID3D11Texture2D* packDepthTexture(ID3D11Texture2D* inDepthTexture);

	void dispose();

protected:
	bool initQuadGeometry(ID3D11Device* d3dDevice);
	bool initShader(ID3D11Device* d3dDevice);
	HRESULT compileShaderFromString(
		const std::string& shaderCode,
		const char* szEntryPoint,
		const char* szShaderModel,
		ID3DBlob** ppBlobOut);
	bool initInputDepthTextureSRV(ID3D11Device* d3dDevice, ID3D11Texture2D* inDepthTexture);
	void disposeInputDepthTextureSRV();
	bool initRenderTargetResources(ID3D11Device* d3dDevice, ID3D11Texture2D* inDepthTexture);
	void disposeRenderTargetResouces();

	static DXGI_FORMAT GetDepthResourceFormat(DXGI_FORMAT depthformat);
	static DXGI_FORMAT GetDepthSRVFormat(DXGI_FORMAT depthformat);

private:
	spoutDX& m_spout;

	ID3D11Texture2D* m_inFloatDepthTexture = nullptr;
	ID3D11ShaderResourceView* m_inFloatDepthTextureSRV = nullptr;
	D3D11_TEXTURE2D_DESC m_inFloatDepthTextureDesc;

	ID3DBlob* m_vertexShaderByteCode = nullptr;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3DBlob* m_pixelShaderByteCode = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11SamplerState* m_samplerState = nullptr;

	ID3D11Buffer* m_quadVertexBuffer = nullptr;
	ID3D11InputLayout* m_quadInputLayout = nullptr;

	ID3D11Texture2D* m_colorTargetTexture = nullptr;
	ID3D11RenderTargetView* m_colorTargetView = nullptr;
	ID3D11ShaderResourceView* m_colorTargetSRV = nullptr;

	ID3D11Texture2D* m_depthTargetTexture = nullptr;
	ID3D11DepthStencilView* m_depthTargetView = nullptr;
	ID3D11ShaderResourceView* m_depthTargetSRV = nullptr;
};
