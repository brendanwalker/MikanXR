#pragma once

#include "SharedTextureLogger.h"
#include "SharedTextureWriter.h"
#include "SpoutDX.h"

class SpoutDXDepthTexturePacker
{
public:
	SpoutDXDepthTexturePacker(
		SharedTextureLogger& logger,
		spoutDX& spout, 
		const struct SharedTextureDescriptor* descriptor);
	virtual ~SpoutDXDepthTexturePacker();

	bool init();
	ID3D11Texture2D* packDepthTexture(ID3D11Texture2D* inDepthTexture, float zNear, float zFar);
	inline ID3D11ShaderResourceView* getPackedDepthTextureResourcePtr() const { return m_colorTargetSRV; }

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

	static DXGI_FORMAT GetDepthStencilViewFormat(DXGI_FORMAT depthformat);
	static DXGI_FORMAT GetDepthSRVFormat(DXGI_FORMAT depthformat);

private:
	SharedTextureLogger& m_logger;
	spoutDX& m_spout;
	SharedTextureDescriptor m_mikanDescriptor;

	ID3D11Texture2D* m_inFloatDepthTexture = nullptr;
	ID3D11ShaderResourceView* m_inFloatDepthTextureSRV = nullptr;
	D3D11_TEXTURE2D_DESC m_inFloatDepthTextureDesc;

	ID3DBlob* m_vertexShaderByteCode = nullptr;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3DBlob* m_pixelShaderByteCode = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11SamplerState* m_samplerState = nullptr;
	ID3D11Buffer* m_constantBuffer = nullptr;

	ID3D11Buffer* m_quadVertexBuffer = nullptr;
	ID3D11InputLayout* m_quadInputLayout = nullptr;

	ID3D11Texture2D* m_colorTargetTexture = nullptr;
	ID3D11RenderTargetView* m_colorTargetView = nullptr;
	ID3D11ShaderResourceView* m_colorTargetSRV = nullptr;
};
