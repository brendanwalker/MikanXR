#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <dxgi.h>

#include <string>
#include <vector>

// Utility functions
const char* DXGIFormatToString(DXGI_FORMAT format);

// Render Target helpers
bool createColorRenderTargetResources(ID3D11Device* m_d3dDevice, int textureWidth, int textureHeight,
									  ID3D11Texture2D** ppColorTargetTexture,
									  ID3D11RenderTargetView** ppColorTargetView,
									  ID3D11ShaderResourceView** ppColorTargetSRV);

bool createDepthRenderTargetResources(ID3D11Device* d3dDevice, int textureWidth, int textureHeight,
									  ID3D11Texture2D** ppFloatDepthTargetTexture,
									  ID3D11DepthStencilView** ppFloatDepthTargetView,
									  ID3D11ShaderResourceView** ppFloatDepthTargetSRV);

// Shader compilation
bool compileShaderFromString(const std::string& shaderCode, const char* szEntryPoint, const char* szShaderModel,
							 ID3DBlob** ppBlobOut);

// Shader creation helpers
bool compileAndCreateShaders(ID3D11Device* d3dDevice, const std::string& shaderCodeString,
							 ID3D11VertexShader** ppVertexShader, ID3D11PixelShader** ppPixelShader,
							 ID3DBlob** ppVertexShaderByteCode);

bool createConstantBuffer(ID3D11Device* d3dDevice, size_t bufferSize, D3D11_USAGE usage, UINT cpuAccessFlags,
						  ID3D11Buffer** ppConstantBuffer);

bool createSamplerState(ID3D11Device* d3dDevice, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode,
						ID3D11SamplerState** ppSamplerState);

bool createInputLayout(ID3D11Device* d3dDevice, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
					   ID3DBlob* vertexShaderByteCode, ID3D11InputLayout** ppInputLayout);
