#include "TestDirectXUtils.h"
#include "Logger.h"

const char* DXGIFormatToString(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_UNKNOWN:
		return "DXGI_FORMAT_UNKNOWN";
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		return "DXGI_FORMAT_R32G32B32A32_TYPELESS";
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return "DXGI_FORMAT_R32G32B32A32_FLOAT";
	case DXGI_FORMAT_R32G32B32A32_UINT:
		return "DXGI_FORMAT_R32G32B32A32_UINT";
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return "DXGI_FORMAT_R32G32B32A32_SINT";
	case DXGI_FORMAT_R32G32B32_TYPELESS:
		return "DXGI_FORMAT_R32G32B32_TYPELESS";
	case DXGI_FORMAT_R32G32B32_FLOAT:
		return "DXGI_FORMAT_R32G32B32_FLOAT";
	case DXGI_FORMAT_R32G32B32_UINT:
		return "DXGI_FORMAT_R32G32B32_UINT";
	case DXGI_FORMAT_R32G32B32_SINT:
		return "DXGI_FORMAT_R32G32B32_SINT";
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		return "DXGI_FORMAT_R16G16B16A16_TYPELESS";
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return "DXGI_FORMAT_R16G16B16A16_FLOAT";
	case DXGI_FORMAT_R16G16B16A16_UNORM:
		return "DXGI_FORMAT_R16G16B16A16_UNORM";
	case DXGI_FORMAT_R16G16B16A16_UINT:
		return "DXGI_FORMAT_R16G16B16A16_UINT";
	case DXGI_FORMAT_R16G16B16A16_SNORM:
		return "DXGI_FORMAT_R16G16B16A16_SNORM";
	case DXGI_FORMAT_R16G16B16A16_SINT:
		return "DXGI_FORMAT_R16G16B16A16_SINT";
	case DXGI_FORMAT_R32G32_TYPELESS:
		return "DXGI_FORMAT_R32G32_TYPELESS";
	case DXGI_FORMAT_R32G32_FLOAT:
		return "DXGI_FORMAT_R32G32_FLOAT";
	case DXGI_FORMAT_R32G32_UINT:
		return "DXGI_FORMAT_R32G32_UINT";
	case DXGI_FORMAT_R32G32_SINT:
		return "DXGI_FORMAT_R32G32_SINT";
	case DXGI_FORMAT_R32G8X24_TYPELESS:
		return "DXGI_FORMAT_R32G8X24_TYPELESS";
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return "DXGI_FORMAT_D32_FLOAT_S8X24_UINT";
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		return "DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS";
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return "DXGI_FORMAT_X32_TYPELESS_G8X24_UINT";
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		return "DXGI_FORMAT_R10G10B10A2_TYPELESS";
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		return "DXGI_FORMAT_R10G10B10A2_UNORM";
	case DXGI_FORMAT_R10G10B10A2_UINT:
		return "DXGI_FORMAT_R10G10B10A2_UINT";
	case DXGI_FORMAT_R11G11B10_FLOAT:
		return "DXGI_FORMAT_R11G11B10_FLOAT";
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return "DXGI_FORMAT_R8G8B8A8_TYPELESS";
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return "DXGI_FORMAT_R8G8B8A8_UNORM";
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return "DXGI_FORMAT_R8G8B8A8_UNORM_SRGB";
	case DXGI_FORMAT_R8G8B8A8_UINT:
		return "DXGI_FORMAT_R8G8B8A8_UINT";
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		return "DXGI_FORMAT_R8G8B8A8_SNORM";
	case DXGI_FORMAT_R8G8B8A8_SINT:
		return "DXGI_FORMAT_R8G8B8A8_SINT";
	case DXGI_FORMAT_R16G16_TYPELESS:
		return "DXGI_FORMAT_R16G16_TYPELESS";
	case DXGI_FORMAT_R16G16_FLOAT:
		return "DXGI_FORMAT_R16G16_FLOAT";
	case DXGI_FORMAT_R16G16_UNORM:
		return "DXGI_FORMAT_R16G16_UNORM";
	case DXGI_FORMAT_R16G16_UINT:
		return "DXGI_FORMAT_R16G16_UINT";
	case DXGI_FORMAT_R16G16_SNORM:
		return "DXGI_FORMAT_R16G16_SNORM";
	case DXGI_FORMAT_R16G16_SINT:
		return "DXGI_FORMAT_R16G16_SINT";
	case DXGI_FORMAT_R32_TYPELESS:
		return "DXGI_FORMAT_R32_TYPELESS";
	case DXGI_FORMAT_D32_FLOAT:
		return "DXGI_FORMAT_D32_FLOAT";
	case DXGI_FORMAT_R32_FLOAT:
		return "DXGI_FORMAT_R32_FLOAT";
	case DXGI_FORMAT_R32_UINT:
		return "DXGI_FORMAT_R32_UINT";
	case DXGI_FORMAT_R32_SINT:
		return "DXGI_FORMAT_R32_SINT";
	case DXGI_FORMAT_R24G8_TYPELESS:
		return "DXGI_FORMAT_R24G8_TYPELESS";
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return "DXGI_FORMAT_D24_UNORM_S8_UINT";
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		return "DXGI_FORMAT_R24_UNORM_X8_TYPELESS";
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return "DXGI_FORMAT_X24_TYPELESS_G8_UINT";
	case DXGI_FORMAT_R8G8_TYPELESS:
		return "DXGI_FORMAT_R8G8_TYPELESS";
	case DXGI_FORMAT_R8G8_UNORM:
		return "DXGI_FORMAT_R8G8_UNORM";
	case DXGI_FORMAT_R8G8_UINT:
		return "DXGI_FORMAT_R8G8_UINT";
	case DXGI_FORMAT_R8G8_SNORM:
		return "DXGI_FORMAT_R8G8_SNORM";
	case DXGI_FORMAT_R8G8_SINT:
		return "DXGI_FORMAT_R8G8_SINT";
	case DXGI_FORMAT_R16_TYPELESS:
		return "DXGI_FORMAT_R16_TYPELESS";
	case DXGI_FORMAT_R16_FLOAT:
		return "DXGI_FORMAT_R16_FLOAT";
	case DXGI_FORMAT_D16_UNORM:
		return "DXGI_FORMAT_D16_UNORM";
	case DXGI_FORMAT_R16_UNORM:
		return "DXGI_FORMAT_R16_UNORM";
	case DXGI_FORMAT_R16_UINT:
		return "DXGI_FORMAT_R16_UINT";
	case DXGI_FORMAT_R16_SNORM:
		return "DXGI_FORMAT_R16_SNORM";
	case DXGI_FORMAT_R16_SINT:
		return "DXGI_FORMAT_R16_SINT";
	case DXGI_FORMAT_R8_TYPELESS:
		return "DXGI_FORMAT_R8_TYPELESS";
	case DXGI_FORMAT_R8_UNORM:
		return "DXGI_FORMAT_R8_UNORM";
	case DXGI_FORMAT_R8_UINT:
		return "DXGI_FORMAT_R8_UINT";
	case DXGI_FORMAT_R8_SNORM:
		return "DXGI_FORMAT_R8_SNORM";
	case DXGI_FORMAT_R8_SINT:
		return "DXGI_FORMAT_R8_SINT";
	case DXGI_FORMAT_A8_UNORM:
		return "DXGI_FORMAT_A8_UNORM";
	case DXGI_FORMAT_R1_UNORM:
		return "DXGI_FORMAT_R1_UNORM";
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		return "DXGI_FORMAT_R9G9B9E5_SHAREDEXP";
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
		return "DXGI_FORMAT_R8G8_B8G8_UNORM";
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
		return "DXGI_FORMAT_G8R8_G8B8_UNORM";
	case DXGI_FORMAT_BC1_TYPELESS:
		return "DXGI_FORMAT_BC1_TYPELESS";
	case DXGI_FORMAT_BC1_UNORM:
		return "DXGI_FORMAT_BC1_UNORM";
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		return "DXGI_FORMAT_BC1_UNORM_SRGB";
	case DXGI_FORMAT_BC2_TYPELESS:
		return "DXGI_FORMAT_BC2_TYPELESS";
	case DXGI_FORMAT_BC2_UNORM:
		return "DXGI_FORMAT_BC2_UNORM";
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		return "DXGI_FORMAT_BC2_UNORM_SRGB";
	case DXGI_FORMAT_BC3_TYPELESS:
		return "DXGI_FORMAT_BC3_TYPELESS";
	case DXGI_FORMAT_BC3_UNORM:
		return "DXGI_FORMAT_BC3_UNORM";
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		return "DXGI_FORMAT_BC3_UNORM_SRGB";
	case DXGI_FORMAT_BC4_TYPELESS:
		return "DXGI_FORMAT_BC4_TYPELESS";
	case DXGI_FORMAT_BC4_UNORM:
		return "DXGI_FORMAT_BC4_UNORM";
	case DXGI_FORMAT_BC4_SNORM:
		return "DXGI_FORMAT_BC4_SNORM";
	case DXGI_FORMAT_BC5_TYPELESS:
		return "DXGI_FORMAT_BC5_TYPELESS";
	case DXGI_FORMAT_BC5_UNORM:
		return "DXGI_FORMAT_BC5_UNORM";
	case DXGI_FORMAT_BC5_SNORM:
		return "DXGI_FORMAT_BC5_SNORM";
	case DXGI_FORMAT_B5G6R5_UNORM:
		return "DXGI_FORMAT_B5G6R5_UNORM";
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return "DXGI_FORMAT_B5G5R5A1_UNORM";
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return "DXGI_FORMAT_B8G8R8A8_UNORM";
	case DXGI_FORMAT_B8G8R8X8_UNORM:
		return "DXGI_FORMAT_B8G8R8X8_UNORM";
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		return "DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM";
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		return "DXGI_FORMAT_B8G8R8A8_TYPELESS";
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB";
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		return "DXGI_FORMAT_B8G8R8X8_TYPELESS";
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return "DXGI_FORMAT_B8G8R8X8_UNORM_SRGB";
	case DXGI_FORMAT_BC6H_TYPELESS:
		return "DXGI_FORMAT_BC6H_TYPELESS";
	case DXGI_FORMAT_BC6H_UF16:
		return "DXGI_FORMAT_BC6H_UF16";
	case DXGI_FORMAT_BC6H_SF16:
		return "DXGI_FORMAT_BC6H_SF16";
	case DXGI_FORMAT_BC7_TYPELESS:
		return "DXGI_FORMAT_BC7_TYPELESS";
	case DXGI_FORMAT_BC7_UNORM:
		return "DXGI_FORMAT_BC7_UNORM";
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return "DXGI_FORMAT_BC7_UNORM_SRGB";
	case DXGI_FORMAT_AYUV:
		return "DXGI_FORMAT_AYUV";
	case DXGI_FORMAT_Y410:
		return "DXGI_FORMAT_Y410";
	case DXGI_FORMAT_Y416:
		return "DXGI_FORMAT_Y416";
	case DXGI_FORMAT_NV12:
		return "DXGI_FORMAT_NV12";
	case DXGI_FORMAT_P010:
		return "DXGI_FORMAT_P010";
	case DXGI_FORMAT_P016:
		return "DXGI_FORMAT_P016";
	case DXGI_FORMAT_420_OPAQUE:
		return "DXGI_FORMAT_420_OPAQUE";
	case DXGI_FORMAT_YUY2:
		return "DXGI_FORMAT_YUY2";
	case DXGI_FORMAT_Y210:
		return "DXGI_FORMAT_Y210";
	case DXGI_FORMAT_Y216:
		return "DXGI_FORMAT_Y216";
	case DXGI_FORMAT_NV11:
		return "DXGI_FORMAT_NV11";
	case DXGI_FORMAT_AI44:
		return "DXGI_FORMAT_AI44";
	case DXGI_FORMAT_IA44:
		return "DXGI_FORMAT_IA44";
	case DXGI_FORMAT_P8:
		return "DXGI_FORMAT_P8";
	case DXGI_FORMAT_A8P8:
		return "DXGI_FORMAT_A8P8";
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return "DXGI_FORMAT_B4G4R4A4_UNORM";
	case DXGI_FORMAT_P208:
		return "DXGI_FORMAT_P208";
	case DXGI_FORMAT_V208:
		return "DXGI_FORMAT_V208";
	case DXGI_FORMAT_V408:
		return "DXGI_FORMAT_V408";
	default:
		return "DXGI_FORMAT_UNKNOWN";
	}
}

static DXGI_FORMAT computeCompatibleDepthResourceFormat(DXGI_FORMAT depthFormat)
{
	DXGI_FORMAT resultFormat= DXGI_FORMAT_UNKNOWN;

	switch (depthFormat)
	{
	case DXGI_FORMAT_D16_UNORM:
		resultFormat= DXGI_FORMAT_R16G16_TYPELESS;
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		resultFormat= DXGI_FORMAT_R24G8_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT:
		resultFormat= DXGI_FORMAT_R32_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		resultFormat= DXGI_FORMAT_R32G8X24_TYPELESS;
		break;
	}

	return resultFormat;
}

static DXGI_FORMAT computeCompatibleDepthSRVFormat(DXGI_FORMAT depthFormat)
{
	DXGI_FORMAT resultFormat= DXGI_FORMAT_UNKNOWN;

	switch (depthFormat)
	{
	case DXGI_FORMAT_D16_UNORM:
		resultFormat= DXGI_FORMAT_R16_FLOAT;
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		resultFormat= DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT:
		resultFormat= DXGI_FORMAT_R32_FLOAT;
		break;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		resultFormat= DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		break;
	}

	return resultFormat;
}

bool createColorRenderTargetResources(
	ID3D11Device* d3dDevice,
	int textureWidth,
	int textureHeight,
	ID3D11Texture2D** ppColorTargetTexture,
	ID3D11RenderTargetView** ppColorTargetView,
	ID3D11ShaderResourceView** ppColorTargetSRV)
{
	HRESULT result;

	// Setup the color render target texture description.
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width= (UINT)textureWidth;
	textureDesc.Height= (UINT)textureHeight;
	textureDesc.MipLevels= 1;
	textureDesc.ArraySize= 1;
	textureDesc.Format= DXGI_FORMAT_B8G8R8A8_TYPELESS; //  DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count= 1;
	textureDesc.Usage= D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags= 0;
	textureDesc.MiscFlags= 0;

	// Create the render target texture.
	result= d3dDevice->CreateTexture2D(&textureDesc, NULL, ppColorTargetTexture);
	if (FAILED(result))
	{
		MIKAN_LOG_ERROR("createColorRenderTargetResources")
			<< "Failed to create Color DX Texture2D "
			<< "(format: DXGI_FORMAT_B8G8R8A8_UNORM"
			<< ", size: " << textureWidth << "x" << textureHeight
			<< ")";
		return false;
	}

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
	renderTargetViewDesc.Format= DXGI_FORMAT_B8G8R8A8_UNORM; // textureDesc.Format;
	renderTargetViewDesc.ViewDimension= D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice= 0;

	// Create the render target view.
	result= d3dDevice->CreateRenderTargetView(*ppColorTargetTexture, &renderTargetViewDesc, ppColorTargetView);
	if (FAILED(result))
	{
		MIKAN_LOG_ERROR("createColorRenderTargetResources") << "Failed to create color render target view";
		return false;
	}

	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format= DXGI_FORMAT_B8G8R8A8_UNORM; // textureDesc.Format;
	shaderResourceViewDesc.ViewDimension= D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip= 0;
	shaderResourceViewDesc.Texture2D.MipLevels= 1;

	// Create the shader resource view.
	result= d3dDevice->CreateShaderResourceView(*ppColorTargetTexture, &shaderResourceViewDesc, ppColorTargetSRV);
	if (FAILED(result))
	{
		MIKAN_LOG_ERROR("createColorRenderTargetResources") << "Failed to create color shader resource view";
		return false;
	}

	return true;
}

bool createDepthRenderTargetResources(
	ID3D11Device* d3dDevice,
	int textureWidth,
	int textureHeight,
	ID3D11Texture2D** ppFloatDepthTargetTexture,
	ID3D11DepthStencilView** ppFloatDepthTargetView,
	ID3D11ShaderResourceView** ppFloatDepthTargetSRV)
{
	HRESULT result;

	DXGI_FORMAT depthViewFormat= DXGI_FORMAT_D32_FLOAT;
	DXGI_FORMAT resourceFormat= computeCompatibleDepthResourceFormat(depthViewFormat);
	DXGI_FORMAT srvFormat= computeCompatibleDepthSRVFormat(depthViewFormat);

	// Setup the depth render target texture description.
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width= (UINT)textureWidth;
	textureDesc.Height= (UINT)textureHeight;
	textureDesc.MipLevels= 1;
	textureDesc.ArraySize= 1;
	textureDesc.Format= resourceFormat;
	textureDesc.SampleDesc.Count= 1;
	textureDesc.Usage= D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags= D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags= 0;
	textureDesc.MiscFlags= 0;

	// Create the render target texture.
	result= d3dDevice->CreateTexture2D(&textureDesc, NULL, ppFloatDepthTargetTexture);
	if (FAILED(result))
	{
		MIKAN_LOG_ERROR("createDepthRenderTargetResources")
			<< "Failed to create Float Depth DX Texture2D "
			<< "(format: " << DXGIFormatToString(resourceFormat)
			<< ", size: " << textureWidth << "x" << textureHeight
			<< ")";
		return false;
	}

	// Create the depth stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format= depthViewFormat;
	depthStencilViewDesc.ViewDimension= D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice= 0;

	result= d3dDevice->CreateDepthStencilView(*ppFloatDepthTargetTexture, &depthStencilViewDesc, ppFloatDepthTargetView);
	if (FAILED(result))
	{
		MIKAN_LOG_ERROR("createDepthRenderTargetResources")
			<< "Failed to create depth stencil view"
			<< "(format: " << DXGIFormatToString(depthViewFormat)
			<< ", size: " << textureWidth << "x" << textureHeight
			<< ")";
		return false;
	}

	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format= srvFormat;
	shaderResourceViewDesc.ViewDimension= D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip= 0;
	shaderResourceViewDesc.Texture2D.MipLevels= 1;

	// Create the shader resource view.
	result= d3dDevice->CreateShaderResourceView(*ppFloatDepthTargetTexture, &shaderResourceViewDesc, ppFloatDepthTargetSRV);
	if (FAILED(result))
	{
		MIKAN_LOG_ERROR("createDepthRenderTargetResources")
			<< "Failed to create depth shader resource view"
			<< "(format: " << DXGIFormatToString(srvFormat)
			<< ", size: " << textureWidth << "x" << textureHeight
			<< ")";
		return false;
	}

	return true;
}

HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr= S_OK;

	DWORD dwShaderFlags= D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows
	// the shaders to be optimized and to run exactly the way they will run in
	// the release configuration of this program.
	dwShaderFlags|= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags|= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob= nullptr;
	hr= D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
						   dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob)
		pErrorBlob->Release();

	return S_OK;
}

bool compileShaderFromString(
	const std::string& shaderCode,
	const char* szEntryPoint,  // Entry point function name in the shader
	const char* szShaderModel, // Shader model (e.g., "vs_5_0" for vertex shader, "ps_5_0" for pixel shader)
	ID3DBlob** ppBlobOut)      // Pointer to store compiled shader code
{
	HRESULT hr= S_OK;
	ID3DBlob* pErrorBlob= nullptr;

	DWORD dwShaderFlags= D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows
	// the shaders to be optimized and to run exactly the way they will run in
	// the release configuration of this program.
	dwShaderFlags|= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags|= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	hr= D3DCompile(
		shaderCode.c_str(),  // HLSL shader code
		shaderCode.length(), // Length of the shader code
		nullptr,             // Optional source name
		nullptr,             // Optional macro definitions
		nullptr,             // Optional include handler
		szEntryPoint,        // Entry point function name
		szShaderModel,       // Shader model
		dwShaderFlags,       // Shader compile options
		0,                   // More compile options
		ppBlobOut,           // Pointer to store compiled shader code
		&pErrorBlob          // Pointer to store error messages
	);

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			const char* szErrorString= (char*)pErrorBlob->GetBufferPointer();

			MIKAN_LOG_ERROR("compileShaderFromString")
				<< "D3DCompile failed with error: "
				<< szErrorString;

			pErrorBlob->Release();
		}

		return false;
	}

	if (pErrorBlob)
	{
		pErrorBlob->Release();
	}

	return true;
}

bool compileAndCreateShaders(
	ID3D11Device* d3dDevice,
	const std::string& shaderCodeString,
	ID3D11VertexShader** ppVertexShader,
	ID3D11PixelShader** ppPixelShader,
	ID3DBlob** ppVertexShaderByteCode)
{
	ID3DBlob* vertexShaderBlob= nullptr;
	ID3DBlob* pixelShaderBlob= nullptr;

	// Compile vertex shader
	if (!compileShaderFromString(shaderCodeString, "vs_main", "vs_4_0", &vertexShaderBlob))
	{
		MIKAN_LOG_ERROR("compileAndCreateShaders") << "Failed to compile vertex shader";
		return false;
	}

	// Compile pixel shader
	if (!compileShaderFromString(shaderCodeString, "ps_main", "ps_4_0", &pixelShaderBlob))
	{
		MIKAN_LOG_ERROR("compileAndCreateShaders") << "Failed to compile pixel shader";
		vertexShaderBlob->Release();
		return false;
	}

	// Create vertex shader
	HRESULT hr= d3dDevice->CreateVertexShader(
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize(),
		nullptr,
		ppVertexShader);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("compileAndCreateShaders") << "Failed to create vertex shader";
		vertexShaderBlob->Release();
		pixelShaderBlob->Release();
		return false;
	}

	// Create pixel shader
	hr= d3dDevice->CreatePixelShader(
		pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize(),
		nullptr,
		ppPixelShader);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("compileAndCreateShaders") << "Failed to create pixel shader";
		(*ppVertexShader)->Release();
		*ppVertexShader= nullptr;
		vertexShaderBlob->Release();
		pixelShaderBlob->Release();
		return false;
	}

	// Return vertex shader bytecode for input layout creation
	if (ppVertexShaderByteCode)
	{
		*ppVertexShaderByteCode= vertexShaderBlob;
	}
	else
	{
		vertexShaderBlob->Release();
	}

	// Release pixel shader bytecode (not needed after shader creation)
	pixelShaderBlob->Release();

	return true;
}

bool createConstantBuffer(
	ID3D11Device* d3dDevice,
	size_t bufferSize,
	D3D11_USAGE usage,
	UINT cpuAccessFlags,
	ID3D11Buffer** ppConstantBuffer)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage= usage;
	bufferDesc.ByteWidth= (UINT)bufferSize;
	bufferDesc.BindFlags= D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags= cpuAccessFlags;

	HRESULT hr= d3dDevice->CreateBuffer(&bufferDesc, nullptr, ppConstantBuffer);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("createConstantBuffer") << "Failed to create constant buffer";
		return false;
	}

	return true;
}

bool createSamplerState(
	ID3D11Device* d3dDevice,
	D3D11_FILTER filter,
	D3D11_TEXTURE_ADDRESS_MODE addressMode,
	ID3D11SamplerState** ppSamplerState)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter= filter;
	samplerDesc.AddressU= addressMode;
	samplerDesc.AddressV= addressMode;
	samplerDesc.AddressW= addressMode;
	samplerDesc.ComparisonFunc= D3D11_COMPARISON_NEVER;
	samplerDesc.MaxAnisotropy= 1;
	samplerDesc.MaxLOD= D3D11_FLOAT32_MAX;

	HRESULT hr= d3dDevice->CreateSamplerState(&samplerDesc, ppSamplerState);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("createSamplerState") << "Failed to create sampler state";
		return false;
	}

	return true;
}

bool createInputLayout(
	ID3D11Device* d3dDevice,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
	ID3DBlob* vertexShaderByteCode,
	ID3D11InputLayout** ppInputLayout)
{
	HRESULT hr= d3dDevice->CreateInputLayout(
		inputElements.data(),
		(UINT)inputElements.size(),
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		ppInputLayout);
	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("createInputLayout") << "Failed to create input layout";
		return false;
	}

	return true;
}