#include "SpoutDXDepthTexturePacker.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <sstream>

#pragma comment(lib, "d3dcompiler.lib")

struct QuadVertex
{
	DirectX::XMFLOAT3 position; // Position of the vertex
	DirectX::XMFLOAT2 texCoord; // Texture coordinates of the vertex
};

struct DepthNormalizeConstantBuffer
{
	float zNear;
	float zFar;
	float padding[2];
};

namespace SpoutDXDepthPackerShaderCode
{
	const std::string pacDeviceDepthShaderCode = R""""(
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
			// Read the raw depth value from the input float depth texture
			float deviceDepth = InputTexture.Sample(samLinear, input.uv).r;

			// Convert the depth value to a linear [0, 1) value (0 = near, 1 = far)
			// 1.0 is not encoded property, so we need to clamp it to 0.999999
			float eyeDepth = zFar * zNear / ((zNear - zFar) * deviceDepth + zFar);
			float zNorm = min((eyeDepth - zNear) / (zFar - zNear), 0.999999);

			// Encode the linear depth value to a RGBA8 texture
			// https://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
			float4 encodedValue = float4(1.0, 255.0, 65025.0, 16581375.0) * zNorm;
			encodedValue = frac(encodedValue);
			encodedValue -= encodedValue.yzww * float4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);

			return encodedValue;
		}
	)"""";

	const std::string packSceneDepthShaderCode = R""""(
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
			// Read the linear scene depth value from the input float depth texture
			float eyeDepth = InputTexture.Sample(samLinear, input.uv).r;

			// Convert the eyeDepth value to a normalized [0, 1) value (0 = near, 1 = far)
			// 1.0 is not encoded property, so we need to clamp it to 0.999999
			float zNorm = min((eyeDepth - zNear) / (zFar - zNear), 0.999999);

			// Encode the linear depth value to a RGBA8 texture
			// https://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
			float4 encodedValue = float4(1.0, 255.0, 65025.0, 16581375.0) * zNorm;
			encodedValue = frac(encodedValue);
			encodedValue -= encodedValue.yzww * float4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);

			return encodedValue;
		}
	)"""";
}

SpoutDXDepthTexturePacker::SpoutDXDepthTexturePacker(
	SharedTextureLogger& logger,
	spoutDX& spout, 
	const SharedTextureDescriptor* descriptor)
	: m_logger(logger)
	, m_spout(spout)
	, m_mikanDescriptor(*descriptor)
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
		m_logger.log(SharedTextureLogLevel::error, "init - Failed to get DX11 device");
		return false;
	}

	if (!initShader(d3dDevice) || !initQuadGeometry(d3dDevice))
	{
		dispose();
		return false;
	}

	return true;
}

ID3D11Texture2D* SpoutDXDepthTexturePacker::packDepthTexture(
	ID3D11Texture2D* inDepthTexture, 
	float zNear, 
	float zFar)
{
	assert(inDepthTexture != nullptr);

	// Make sure there is a valid DX11 device/context
	ID3D11Device* d3dDevice = m_spout.GetDX11Device();
	ID3D11DeviceContext* d3dContext = m_spout.GetDX11Context();
	if (d3dDevice == nullptr || d3dContext == nullptr)
	{
		m_logger.log(SharedTextureLogLevel::error, "packDepthTexture - Failed to get DX11 device/context");
		return nullptr;
	}

	// Refresh the input float depth texture SRV if the input texture has changed
	if (m_inFloatDepthTextureSRV == nullptr || m_inFloatDepthTexture != inDepthTexture)
	{
		if (!initInputDepthTextureSRV(d3dDevice, inDepthTexture))
		{
			m_logger.log(SharedTextureLogLevel::error, "packDepthTexture - Failed to initialize input depth texture SRV");
			return nullptr;
		}

		// Cache the input float depth texture
		m_inFloatDepthTexture = inDepthTexture;
	}

	// Make sure the render target resources are initialized
	D3D11_TEXTURE2D_DESC inTextureDesc;
	inDepthTexture->GetDesc(&inTextureDesc);
	if (m_colorTargetTexture == nullptr ||
		memcmp(&m_inFloatDepthTextureDesc, &inTextureDesc, sizeof(D3D11_TEXTURE2D_DESC)) != 0)
	{
		if (!initRenderTargetResources(d3dDevice, inDepthTexture))
		{
			m_logger.log(SharedTextureLogLevel::error, "packDepthTexture - Failed to initialize render target resources");
			return nullptr;
		}

		// Cache the input texture description
		m_inFloatDepthTextureDesc = inTextureDesc;
	}

	// Convert the float depth texture to a RGBA8 texture using a shader
	// -------

	// Set the output render views
	//d3dContext->OMSetRenderTargets(1, &m_colorTargetView, m_depthTargetView);
	d3dContext->OMSetRenderTargets(1, &m_colorTargetView, nullptr);

	// Clear the render targets
	d3dContext->ClearRenderTargetView(m_colorTargetView, DirectX::Colors::Black);
	//d3dContext->ClearDepthStencilView(m_depthTargetView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Bind the pack depth shader
	d3dContext->VSSetShader(m_vertexShader, nullptr, 0);
	d3dContext->PSSetShader(m_pixelShader, nullptr, 0);

	// Bind the input float depth texture's shader resource view to the shader
	d3dContext->PSSetShaderResources(0, 1, &m_inFloatDepthTextureSRV);
	d3dContext->PSSetSamplers(0, 1, &m_samplerState);

	// Bind the constant buffer
	DepthNormalizeConstantBuffer constants;
	constants.zNear = zNear;
	constants.zFar = zFar;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	d3dContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &constants, sizeof(DepthNormalizeConstantBuffer));
	d3dContext->Unmap(m_constantBuffer, 0);
	d3dContext->PSSetConstantBuffers(0, 1, &m_constantBuffer);

	// Set the viewport to the size of the input texture
	D3D11_VIEWPORT viewport= {};
	viewport.Width = (float)inTextureDesc.Width;
	viewport.Height = (float)inTextureDesc.Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	d3dContext->RSSetViewports(1, &viewport);

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
		m_logger.log(SharedTextureLogLevel::error, "initQuadGeometry - Failed to create quad vertex buffer");
		return false;
	}

	return true;
}

bool SpoutDXDepthTexturePacker::initShader(ID3D11Device* d3dDevice)
{
	const std::string shaderCodeString = 
		m_mikanDescriptor.depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH 
		? SpoutDXDepthPackerShaderCode::packSceneDepthShaderCode 
		: SpoutDXDepthPackerShaderCode::pacDeviceDepthShaderCode;

	// Compile vertex shader
	HRESULT hr = compileShaderFromString(shaderCodeString, "vs_main", "vs_4_0", &m_vertexShaderByteCode);
	if (FAILED(hr))
	{
		m_logger.log(SharedTextureLogLevel::error, "initShader - Failed to compile vertex shader");
		return false;
	}

	// Compile pixel shader
	hr = compileShaderFromString(shaderCodeString, "ps_main", "ps_4_0", &m_pixelShaderByteCode);
	if (FAILED(hr))
	{
		m_logger.log(SharedTextureLogLevel::error, "initShader - Failed to compile pixel shader");
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
		m_logger.log(SharedTextureLogLevel::error, "initShader - Failed to create vertex shader");
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
		m_logger.log(SharedTextureLogLevel::error, "initShader - Failed to create pixel shader");
		return false;
	}

	// Create Constant Buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(DepthNormalizeConstantBuffer);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr= d3dDevice->CreateBuffer(&bufferDesc, nullptr, &m_constantBuffer);
	if (FAILED(hr))
	{
		m_logger.log(SharedTextureLogLevel::error, "initShader - Failed to create constants buffer");
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
		m_logger.log(SharedTextureLogLevel::error, "initShader - Failed to fetch vertex input layout");
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
		m_logger.log(SharedTextureLogLevel::error, "initShader - Failed to create sampler state");
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
			std::stringstream ss;
			ss << "CompileShaderFromMemory failed with error: " << szErrorString;
			m_logger.log(SharedTextureLogLevel::error, ss.str());

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
		m_logger.log(SharedTextureLogLevel::error, "initInputDepthTextureSRV - Failed to create SRV for input depth texture");
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
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
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
		m_logger.log(SharedTextureLogLevel::error, "initRenderTargetResources - Failed to create color target texture");
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc= {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	hr = d3dDevice->CreateRenderTargetView(m_colorTargetTexture, &rtvDesc, &m_colorTargetView);
	if (FAILED(hr))
	{
		m_logger.log(SharedTextureLogLevel::error, "initRenderTargetResources - Failed to create color target view");
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC colorSrvDesc= {};
	colorSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	colorSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	colorSrvDesc.Texture2D.MostDetailedMip = 0;
	colorSrvDesc.Texture2D.MipLevels = 1;
	hr = d3dDevice->CreateShaderResourceView(m_colorTargetTexture, &colorSrvDesc, &m_colorTargetSRV);
	if (FAILED(hr))
	{
		m_logger.log(SharedTextureLogLevel::error, "initRenderTargetResources - Failed to create color target SRV");
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
		case DXGI_FORMAT_R32_FLOAT:
			srvFormat = DXGI_FORMAT_R32_FLOAT;
			break;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			srvFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
			break;
	}

	return srvFormat;
}