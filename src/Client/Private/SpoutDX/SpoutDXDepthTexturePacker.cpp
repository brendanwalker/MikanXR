#include "SpoutDXDepthTexturePacker.h"
#include "Logger.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#pragma comment(lib, "d3dcompiler.lib")

struct QuadVertex
{
	DirectX::XMFLOAT3 position; // Position of the vertex
	DirectX::XMFLOAT2 texCoord; // Texture coordinates of the vertex
};

SpoutDXDepthTexturePacker::SpoutDXDepthTexturePacker(spoutDX& spout)
	: m_spout(spout)
	, m_inFloatDepthTextureDesc(D3D11_TEXTURE2D_DESC())
{}

SpoutDXDepthTexturePacker::~SpoutDXDepthTexturePacker()
{
	dispose();
}

bool SpoutDXDepthTexturePacker::init()
{
	// Make sure there is a valid DX11 device
	ID3D11Device* d3dDevice = m_spout.GetDX11Device();
	if (d3dDevice == nullptr)
	{
		MIKAN_LOG_ERROR("init") << "Failed to get DX11 device";
		return false;
	}

	if (!initShader(d3dDevice) || !initQuadGeometry(d3dDevice))
	{
		dispose();
		return false;
	}

	return true;
}

ID3D11Texture2D* SpoutDXDepthTexturePacker::packDepthTexture(ID3D11Texture2D* inDepthTexture)
{
	assert(inDepthTexture != nullptr);

	// Make sure there is a valid DX11 device/context
	ID3D11Device* d3dDevice = m_spout.GetDX11Device();
	ID3D11DeviceContext* d3dContext = m_spout.GetDX11Context();
	if (d3dDevice == nullptr || d3dContext == nullptr)
	{
		MIKAN_LOG_ERROR("packDepthTexture") << "Failed to get DX11 device/context";
		return nullptr;
	}

	// Refresh the input float depth texture SRV if the input texture has changed
	if (m_inFloatDepthTextureSRV == nullptr || m_inFloatDepthTexture != inDepthTexture)
	{
		if (!initInputDepthTextureSRV(d3dDevice, inDepthTexture))
		{
			MIKAN_LOG_ERROR("packDepthTexture") << "Failed to initialize input depth texture SRV";
			return nullptr;
		}

		// Cache the input float depth texture
		m_inFloatDepthTexture = inDepthTexture;
	}

	// Make sure the render target resources are initialized
	D3D11_TEXTURE2D_DESC inTextureDesc;
	inDepthTexture->GetDesc(&inTextureDesc);
	if (m_depthTargetTexture == nullptr ||
		memcmp(&m_inFloatDepthTextureDesc, &inTextureDesc, sizeof(D3D11_TEXTURE2D_DESC)) != 0)
	{
		if (!initRenderTargetResources(d3dDevice, inDepthTexture))
		{
			MIKAN_LOG_ERROR("packDepthTexture") << "Failed to initialize render target resources";
			return nullptr;
		}

		// Cache the input texture description
		m_inFloatDepthTextureDesc = inTextureDesc;
	}

	// Convert the float depth texture to a RGBA8 texture using a shader
	// -------

	// Set the output render views
	d3dContext->OMSetRenderTargets(1, &m_colorTargetView, m_depthTargetView);

	// Clear the render targets
	d3dContext->ClearRenderTargetView(m_colorTargetView, DirectX::Colors::Black);
	d3dContext->ClearDepthStencilView(m_depthTargetView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Bind the pack depth shader
	d3dContext->VSSetShader(m_vertexShader, nullptr, 0);
	d3dContext->PSSetShader(m_pixelShader, nullptr, 0);

	// Bind the input float depth texture's shader resource view to the shader
	d3dContext->PSSetShaderResources(0, 1, &m_inFloatDepthTextureSRV);
	d3dContext->PSSetSamplers(0, 1, &m_samplerState);

	// Bind the vertex buffer
	UINT quadStride = sizeof(QuadVertex);
	UINT quadffset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &m_quadVertexBuffer, &quadStride, &quadffset);

	// Bind the input layout
	d3dContext->IASetInputLayout(m_quadInputLayout);

	// Set the topology to a triangle list (2 triangles)
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw the quad (2 triangles)
	d3dContext->Draw(6, 0);

	// The RGBA8 texture contains the packed float depth date
	return m_colorTargetTexture;

}

void SpoutDXDepthTexturePacker::dispose()
{
	if (m_vertexShaderByteCode)
	{
		m_vertexShaderByteCode->Release();
		m_vertexShaderByteCode = nullptr;
	}

	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}

	if (m_pixelShaderByteCode)
	{
		m_pixelShaderByteCode->Release();
		m_pixelShaderByteCode = nullptr;
	}

	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}

	if (m_quadVertexBuffer)
	{
		m_quadVertexBuffer->Release();
		m_quadVertexBuffer = nullptr;
	}

	if (m_quadInputLayout)
	{
		m_quadInputLayout->Release();
		m_quadInputLayout = nullptr;
	}

	if (m_samplerState)
	{
		m_samplerState->Release();
		m_samplerState = nullptr;
	}

	disposeRenderTargetResouces();
}

bool SpoutDXDepthTexturePacker::initQuadGeometry(ID3D11Device* d3dDevice)
{
	QuadVertex vertices[] = {
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },

		{ DirectX::XMFLOAT3(1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
	};

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(QuadVertex) * 6; // 6 vertices
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;

	// Create the vertex buffer
	HRESULT hr = d3dDevice->CreateBuffer(&bufferDesc, &initData, &m_quadVertexBuffer);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("initQuadGeometry") << "Failed to create quad geometry";
		return false;
	}

	return true;
}

bool SpoutDXDepthTexturePacker::initShader(ID3D11Device* d3dDevice)
{
	const std::string shaderCodeString = R""""(
		Texture2D<float> InputTexture : register(t0);
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
			// https://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
			float floatValue = InputTexture.Sample(samLinear, input.uv).r;
			float4 encodedValue = float4(1.0, 255.0, 65025.0, 16581375.0) * floatValue;
			encodedValue = frac(encodedValue);
			encodedValue -= encodedValue.yzww * float4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);

			return encodedValue;
		}
	)"""";

	// Compile vertex shader
	HRESULT hr = compileShaderFromString(shaderCodeString, "vs_main", "vs_4_0", &m_vertexShaderByteCode);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXDepthTexturePacker") << "Failed to compile vertex shader";
		return false;
	}

	// Compile pixel shader
	hr = compileShaderFromString(shaderCodeString, "ps_main", "ps_4_0", &m_pixelShaderByteCode);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXDepthTexturePacker") << "Failed to compile pixel shader";
		return false;
	}

	// Create vertex shader
	hr = d3dDevice->CreateVertexShader(
		m_vertexShaderByteCode->GetBufferPointer(),
		m_vertexShaderByteCode->GetBufferSize(),
		nullptr,
		&m_vertexShader);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXDepthTexturePacker") << "Failed to create vertex shader";
		return false;
	}

	// Create pixel shader
	hr = d3dDevice->CreatePixelShader(
		m_pixelShaderByteCode->GetBufferPointer(),
		m_pixelShaderByteCode->GetBufferSize(),
		nullptr,
		&m_pixelShader);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXDepthTexturePacker") << "Failed to create vertex shader";
		return false;
	}

	// Create input layout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(DirectX::XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	d3dDevice->CreateInputLayout(
		layout,
		ARRAYSIZE(layout),
		m_vertexShaderByteCode->GetBufferPointer(),
		m_vertexShaderByteCode->GetBufferSize(),
		&m_quadInputLayout);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXDepthTexturePacker") << "Failed to fetch vertex input layout";
		return false;
	}

	// Create the float depth texture sampler state
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = d3dDevice->CreateSamplerState(&samplerDesc, &m_samplerState);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXDepthTexturePacker") << "Failed to create sampler state";
		return false;
	}

	return true;
}

HRESULT SpoutDXDepthTexturePacker::compileShaderFromString(
	const std::string& shaderCode,
	const char* szEntryPoint,       // Entry point function name in the shader
	const char* szShaderModel,      // Shader model (e.g., "vs_5_0" for vertex shader, "ps_5_0" for pixel shader)
	ID3DBlob** ppBlobOut)           // Pointer to store compiled shader code
{
	HRESULT hr = S_OK;
	ID3DBlob* pErrorBlob = nullptr;

	hr = D3DCompile(
		shaderCode.c_str(),         // HLSL shader code
		shaderCode.length(),        // Length of the shader code
		nullptr,                    // Optional source name
		nullptr,                    // Optional macro definitions
		nullptr,                    // Optional include handler
		szEntryPoint,               // Entry point function name
		szShaderModel,              // Shader model
		D3DCOMPILE_DEBUG,           // Shader compile options
		0,                          // More compile options
		ppBlobOut,                  // Pointer to store compiled shader code
		&pErrorBlob                 // Pointer to store error messages
	);

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			const char* szErrorString = (char*)pErrorBlob->GetBufferPointer();
			MIKAN_LOG_ERROR("CompileShaderFromMemory") << szErrorString;
			pErrorBlob->Release();
		}

		return hr;
	}

	if (pErrorBlob)
	{
		pErrorBlob->Release();
	}

	return S_OK;
}

bool SpoutDXDepthTexturePacker::initInputDepthTextureSRV(ID3D11Device* d3dDevice, ID3D11Texture2D* inDepthTexture)
{
	disposeInputDepthTextureSRV();

	D3D11_TEXTURE2D_DESC inTextureDesc;
	inDepthTexture->GetDesc(&inTextureDesc);

	// Create a shader resource view for the input float depth texture
	D3D11_SHADER_RESOURCE_VIEW_DESC inSrvDesc;
	ZeroMemory(&inSrvDesc, sizeof(inSrvDesc));
	inSrvDesc.Format = GetDepthSRVFormat(inTextureDesc.Format);
	inSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	inSrvDesc.Texture2D.MipLevels = inTextureDesc.MipLevels;
	inSrvDesc.Texture2D.MostDetailedMip = 0;

	HRESULT hr = d3dDevice->CreateShaderResourceView(inDepthTexture, &inSrvDesc, &m_inFloatDepthTextureSRV);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXcolorTexturePacker") << "Failed to create SRV for input depth texture";
		return false;
	}

	return true;
}

void SpoutDXDepthTexturePacker::disposeInputDepthTextureSRV()
{
	m_inFloatDepthTextureDesc = D3D11_TEXTURE2D_DESC();
	m_inFloatDepthTexture = nullptr;

	if (m_inFloatDepthTextureSRV)
	{
		m_inFloatDepthTextureSRV->Release();
		m_inFloatDepthTextureSRV = nullptr;
	}
}

bool SpoutDXDepthTexturePacker::initRenderTargetResources(ID3D11Device* d3dDevice, ID3D11Texture2D* inDepthTexture)
{
	disposeRenderTargetResouces();

	D3D11_TEXTURE2D_DESC inTextureDesc;
	inDepthTexture->GetDesc(&inTextureDesc);

	// Create the color render target resources
	D3D11_TEXTURE2D_DESC textureDesc= {};
	textureDesc.Width = inTextureDesc.Width;
	textureDesc.Height = inTextureDesc.Height;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_TYPELESS;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	HRESULT hr = d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_colorTargetTexture);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXcolorTexturePacker") << "Failed to create color target texture";
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc= {};
	rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	hr = d3dDevice->CreateRenderTargetView(m_colorTargetTexture, &rtvDesc, &m_colorTargetView);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXcolorTexturePacker") << "Failed to create color target view";
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC colorSrvDesc= {};
	colorSrvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	colorSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	colorSrvDesc.Texture2D.MostDetailedMip = 0;
	colorSrvDesc.Texture2D.MipLevels = 1;
	hr = d3dDevice->CreateShaderResourceView(m_colorTargetTexture, &colorSrvDesc, &m_colorTargetSRV);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXcolorTexturePacker") << "Failed to create color target SRV";
		return false;
	}

	// Create the depth render target resources
	textureDesc.Format = inTextureDesc.Format;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	hr = d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_depthTargetTexture);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXDepthTexturePacker") << "Failed to create depth target texture";
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc= {};
	dsvDesc.Format = GetDepthStencilViewFormat(inTextureDesc.Format);
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = d3dDevice->CreateDepthStencilView(m_depthTargetTexture, &dsvDesc, &m_depthTargetView);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXDepthTexturePacker") << "Failed to create depth target view";
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC depthSrvDesc= {};
	depthSrvDesc.Format = GetDepthSRVFormat(inTextureDesc.Format);
	depthSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthSrvDesc.Texture2D.MostDetailedMip = 0;
	depthSrvDesc.Texture2D.MipLevels = 1;
	hr = d3dDevice->CreateShaderResourceView(m_depthTargetTexture, &depthSrvDesc, &m_depthTargetSRV);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("SpoutDXDepthTexturePacker") << "Failed to create depth target SRV";
		return false;
	}

	return true;
}

void SpoutDXDepthTexturePacker::disposeRenderTargetResouces()
{
	// Free color target resources
	if (m_colorTargetTexture)
	{
		m_colorTargetTexture->Release();
		m_colorTargetTexture = nullptr;
	}

	if (m_colorTargetView)
	{
		m_colorTargetView->Release();
		m_colorTargetView = nullptr;
	}

	if (m_colorTargetSRV)
	{
		m_colorTargetSRV->Release();
		m_colorTargetSRV = nullptr;
	}

	// Free depth target resources
	if (m_depthTargetTexture)
	{
		m_depthTargetTexture->Release();
		m_depthTargetTexture = nullptr;
	}

	if (m_depthTargetView)
	{
		m_depthTargetView->Release();
		m_depthTargetView = nullptr;
	}

	if (m_depthTargetSRV)
	{
		m_depthTargetSRV->Release();
		m_depthTargetSRV = nullptr;
	}
}

DXGI_FORMAT SpoutDXDepthTexturePacker::GetDepthStencilViewFormat(DXGI_FORMAT resFormat)
{
	DXGI_FORMAT viewFormat = DXGI_FORMAT_UNKNOWN;

	switch (resFormat)
	{
		case DXGI_FORMAT_R16G16_TYPELESS:
			viewFormat = DXGI_FORMAT_D16_UNORM;
			break;
		case DXGI_FORMAT_R24G8_TYPELESS:
			viewFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		case DXGI_FORMAT_R32_TYPELESS:
			viewFormat = DXGI_FORMAT_D32_FLOAT;
			break;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			viewFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			break;
	}

	return viewFormat;
}

DXGI_FORMAT SpoutDXDepthTexturePacker::GetDepthSRVFormat(DXGI_FORMAT resFormat)
{
	DXGI_FORMAT srvFormat = DXGI_FORMAT_UNKNOWN;

	switch (resFormat)
	{
		case DXGI_FORMAT_R16G16_TYPELESS:
			srvFormat = DXGI_FORMAT_R16_FLOAT;
			break;
		case DXGI_FORMAT_R24G8_TYPELESS:
			srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;
		case DXGI_FORMAT_R32_TYPELESS:
			srvFormat = DXGI_FORMAT_R32_FLOAT;
			break;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			srvFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
			break;
	}

	return srvFormat;
}