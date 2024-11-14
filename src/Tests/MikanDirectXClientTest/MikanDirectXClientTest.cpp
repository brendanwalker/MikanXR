#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <tchar.h>
#include <codecvt>

#include "Logger.h"
#include "MikanAPI.h"
#include "MikanRenderTargetRequests.h"
#include "MikanVideoSourceRequests.h"
#include "MikanEventTypes.h"
#include "MikanMathTypes.h"
#include "MikanStencilTypes.h"
#include "MikanVideoSourceTypes.h"

using namespace DirectX;

struct SimpleVertex
{
    XMFLOAT3 Pos;
};

HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = nullptr;
ID3D11Device1*          g_pd3dDevice1 = nullptr;
ID3D11DeviceContext*    g_pImmediateContext = nullptr;
ID3D11DeviceContext1*   g_pImmediateContext1 = nullptr;
IDXGISwapChain*         g_pSwapChain = nullptr;
IDXGISwapChain1*        g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pDefaultRenderTargetView = nullptr;
ID3D11VertexShader*     g_pVertexShader = nullptr;
ID3D11PixelShader*      g_pPixelShader = nullptr;
ID3D11InputLayout*      g_pVertexLayout = nullptr;
ID3D11Buffer*           g_pVertexBuffer = nullptr;

ID3D11Texture2D*        g_renderTargetTexture = nullptr;
ID3D11RenderTargetView* g_renderTargetView = nullptr;
ID3D11ShaderResourceView* g_shaderResourceView = nullptr;

IMikanAPIPtr g_mikanAPI;
uint32_t g_lastFrameTimestamp= 0;
uint64_t m_lastReceivedVideoSourceFrame= 0;
bool g_mikanInitialized = true;
uint64_t g_lastReceivedVideoSourceFrame= 0;
float g_mikanReconnectTimeout = 0.f; // seconds
float g_zNear = 0.1f;
float g_zFar = 1000.f;

HRESULT initWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT initDevice();
LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
void cleanupDevice();
bool initMikan();
void updateMikan();
void processNewVideoSourceFrame(const MikanVideoSourceNewFrameEvent& newFrameEvent);
void reallocateRenderBuffers();
void updateCameraProjectionMatrix();
bool createFrameBuffer(uint16_t width, uint16_t height);
void freeFrameBuffer();
void cleanupMikan();
void render();

int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );    

    if( FAILED( initWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( initDevice() ) )
    {
        cleanupDevice();
        return 0;
    }

    if (!initMikan())
    {
        cleanupMikan();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            updateMikan();
        }
    }

    cleanupDevice();
    cleanupMikan();

    return ( int )msg.wParam;
}

void onMikanLog(int log_level, const char* log_message)
{
	switch (log_level)
	{
		case MikanLogLevel_Debug:
			MIKAN_LOG_DEBUG("onMikanLog") << log_message;
			break;
		case MikanLogLevel_Info:
			MIKAN_LOG_INFO("onMikanLog") << log_message;
			break;
		case MikanLogLevel_Warning:
			MIKAN_LOG_WARNING("onMikanLog") << log_message;
			break;
		case MikanLogLevel_Error:
			MIKAN_LOG_ERROR("onMikanLog") << log_message;
			break;
		case MikanLogLevel_Fatal:
			MIKAN_LOG_FATAL("onMikanLog") << log_message;
			break;
		default:
			MIKAN_LOG_INFO("onMikanLog") << log_message;
			break;
	}
}

bool initMikan()
{
    g_lastFrameTimestamp = GetTickCount();

	LoggerSettings settings = {};
	settings.min_log_level = LogSeverityLevel::info;
	settings.enable_console = true;

	log_init(settings);

    g_mikanAPI= IMikanAPI::createMikanAPI();

	if (g_mikanAPI->init(MikanLogLevel_Info, onMikanLog) == MikanResult_Success)
	{
		MikanClientInfo ClientInfo = {};
		ClientInfo.engineName = "MikanXR Test";
		ClientInfo.engineVersion= "1.0";
		ClientInfo.applicationName= "MikanXR Test";
		ClientInfo.applicationVersion= "1.0";
		ClientInfo.graphicsAPI = MikanClientGraphicsApi_OpenGL;
		ClientInfo.supportsRGB24 = true;
		g_mikanAPI->setClientInfo(ClientInfo);
        g_mikanAPI->setGraphicsDeviceInterface(MikanClientGraphicsApi_Direct3D11, g_pd3dDevice);
		g_mikanInitialized = true;
	}
	else
	{
		MIKAN_LOG_ERROR("startup") << "Failed to initialize Mikan Client API";
        return false;
	}

    return true;
}

void updateMikan()
{
	// Update the frame rate
	const uint32_t now = GetTickCount();
	const float deltaSeconds = fminf((float)(now - g_lastFrameTimestamp) / 1000.f, 0.1f);
	//m_fps = deltaSeconds > 0.f ? (1.0f / deltaSeconds) : 0.f;
    g_lastFrameTimestamp = now;

	if (g_mikanAPI->getIsConnected())
	{
		MikanEventPtr event;
		while (g_mikanAPI->fetchNextEvent(event) == MikanResult_Success)
		{
            const std::string& eventType = event->eventType.getValue();

			if (eventType == MikanConnectedEvent::k_typeName)
            {
				reallocateRenderBuffers();
				updateCameraProjectionMatrix();
            }
			else if (eventType == MikanVideoSourceOpenedEvent::k_typeName)
            {
				reallocateRenderBuffers();
				updateCameraProjectionMatrix();
            }
            else if (eventType == MikanVideoSourceNewFrameEvent::k_typeName)
            {
				auto newFrameEvent = std::static_pointer_cast<MikanVideoSourceNewFrameEvent>(event);
				processNewVideoSourceFrame(*newFrameEvent.get());
            }
			else if (eventType == MikanVideoSourceModeChangedEvent::k_typeName ||
					 eventType == MikanVideoSourceIntrinsicsChangedEvent::k_typeName)
            {
				reallocateRenderBuffers();
				updateCameraProjectionMatrix();
            }
		}
	}
	else
	{
		if (g_mikanReconnectTimeout <= 0.f)
		{
			if (g_mikanAPI->connect() != MikanResult_Success || !g_mikanAPI->getIsConnected())
			{
				// timeout between reconnect attempts
				g_mikanReconnectTimeout = 1.0f;
			}
		}
		else
		{
			g_mikanReconnectTimeout -= deltaSeconds;
		}

		// Just render the scene using the last applied camera pose
		render();
	}
}

void processNewVideoSourceFrame(const MikanVideoSourceNewFrameEvent& newFrameEvent)
{
	if (newFrameEvent.frame == m_lastReceivedVideoSourceFrame)
		return;

	// Apply the camera pose received
	//setCameraPose(
	//	MikanVector3f_to_glm_vec3(newFrameEvent.cameraForward),
	//	MikanVector3f_to_glm_vec3(newFrameEvent.cameraUp),
	//	MikanVector3f_to_glm_vec3(newFrameEvent.cameraPosition));

	// Render out a new frame
	render();

	// Write the color texture to the shared texture
	{
		WriteColorRenderTargetTexture writeTextureRequest;

		writeTextureRequest.apiColorTexturePtr = g_renderTargetTexture;
		g_mikanAPI->sendRequest(writeTextureRequest);
	}

	// Publish the new video frame back to Mikan
	{
		PublishRenderTargetTextures frameRendered;

		frameRendered.frameIndex = newFrameEvent.frame;
		g_mikanAPI->sendRequest(frameRendered);
	}

	// Remember the frame index of the last frame we published
    g_lastReceivedVideoSourceFrame = newFrameEvent.frame;
}

//void setCameraPose(
//	const glm::vec3& cameraForward,
//	const glm::vec3& cameraUp,
//	const glm::vec3& cameraPosition)
//{
//	m_viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);
//}

void reallocateRenderBuffers()
{
	freeFrameBuffer();

    FreeRenderTargetTextures freeRequest;
	g_mikanAPI->sendRequest(freeRequest).wait();

    GetVideoSourceMode getModeRequest;
	auto future = g_mikanAPI->sendRequest(getModeRequest);
	auto response = future.get();
	if (response->resultCode == MikanResult_Success)
	{
        auto mode = std::static_pointer_cast<MikanVideoSourceMode>(response);

		MikanRenderTargetDescriptor desc;
		memset(&desc, 0, sizeof(MikanRenderTargetDescriptor));
		desc.width = mode->resolution_x;
		desc.height = mode->resolution_y;
		desc.color_buffer_type = MikanColorBuffer_BGRA32;
		desc.depth_buffer_type = MikanDepthBuffer_NODEPTH;
		desc.graphicsAPI = MikanClientGraphicsApi_Direct3D11;

		// Tell the server to allocate new render target buffers
        AllocateRenderTargetTextures allocateRequest;
        allocateRequest.descriptor = desc;
		g_mikanAPI->sendRequest(allocateRequest).wait();

        // Create a new frame buffer to render to
		createFrameBuffer(mode->resolution_x, mode->resolution_y);
	}
}

void updateCameraProjectionMatrix()
{
    GetVideoSourceIntrinsics getIntrinsicsRequest;
	auto future = g_mikanAPI->sendRequest(getIntrinsicsRequest);
	auto response = future.get();

	if (response->resultCode == MikanResult_Success)
	{
        auto videoSourceIntrinsics= std::static_pointer_cast<MikanVideoSourceIntrinsics>(response);
        auto cameraIntrinsics= videoSourceIntrinsics->intrinsics_ptr.getSharedPointer();

		const float videoSourcePixelWidth = cameraIntrinsics->pixel_width;
		const float videoSourcePixelHeight = cameraIntrinsics->pixel_height;

        g_zNear = (float)cameraIntrinsics->znear;
        g_zFar = (float)cameraIntrinsics->zfar;
		//m_projectionMatrix =
		//	glm::perspective(
		//		(float)degrees_to_radians(monoIntrinsics.vfov),
		//		videoSourcePixelWidth / videoSourcePixelHeight,
		//		(float)monoIntrinsics.znear,
		//		(float)monoIntrinsics.zfar);
	}
}

bool createFrameBuffer(
    uint16_t textureWidth, 
    uint16_t textureHeight)
{
	bool bSuccess = true;

	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_TYPELESS; //  DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	result = g_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_renderTargetTexture);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = g_pd3dDevice->CreateRenderTargetView(g_renderTargetTexture, &renderTargetViewDesc, &g_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = g_pd3dDevice->CreateShaderResourceView(g_renderTargetTexture, &shaderResourceViewDesc, &g_shaderResourceView);
	if (FAILED(result))
	{
		return false;
	}

	return true;

	return bSuccess;
}

void freeFrameBuffer()
{
	if (g_shaderResourceView)
	{
		g_shaderResourceView->Release();
		g_shaderResourceView = 0;
	}

	if (g_renderTargetView)
	{
		g_renderTargetView->Release();
		g_renderTargetView = 0;
	}

	if (g_renderTargetTexture)
	{
		g_renderTargetTexture->Release();
		g_renderTargetTexture = 0;
	}
}

void cleanupMikan()
{
	if (g_mikanInitialized)
	{
		g_mikanAPI->shutdown();
        g_mikanInitialized = false;
	}

	log_dispose();
}

HRESULT initWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_APPLICATION);
    wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = _T("TutorialWindowClass");
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_APPLICATION);
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 800, 600 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( _T("TutorialWindowClass"), _T("Direct3D 11 Tutorial 2: Rendering a Triangle"),
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                           nullptr );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


HRESULT CompileShaderFromFile( const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile( szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
    if( FAILED(hr) )
    {
        if( pErrorBlob )
        {
            OutputDebugStringA( reinterpret_cast<const char*>( pErrorBlob->GetBufferPointer() ) );
            pErrorBlob->Release();
        }
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


HRESULT initDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice( nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );

        if ( hr == E_INVALIDARG )
        {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice( nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                                    D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        }

        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = g_pd3dDevice->QueryInterface( __uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice) );
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                DXGI_ADAPTER_DESC adapterDesc;
                adapter->GetDesc(&adapterDesc);

				using convert_typeX = std::codecvt_utf8<wchar_t>;
				std::wstring_convert<convert_typeX, wchar_t> converterX;
				std::string description= converterX.to_bytes(adapterDesc.Description);

                MIKAN_LOG_ERROR("startup") << "Selected DirextX Adapter: " << description;
                hr = adapter->GetParent( __uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory) );
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;

    // Create swap chain
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface( __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2) );
    if ( dxgiFactory2 )
    {
        // DirectX 11.1 or later
        hr = g_pd3dDevice->QueryInterface( __uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1) );
        if (SUCCEEDED(hr))
        {
            (void) g_pImmediateContext->QueryInterface( __uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1) );
        }

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd( g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1 );
        if (SUCCEEDED(hr))
        {
            hr = g_pSwapChain1->QueryInterface( __uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain) );
        }

        dxgiFactory2->Release();
    }
    else
    {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = g_hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain( g_pd3dDevice, &sd, &g_pSwapChain );
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation( g_hWnd, DXGI_MWA_NO_ALT_ENTER );

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &pBackBuffer ) );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, nullptr, &g_pDefaultRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pDefaultRenderTargetView, nullptr );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile( L"shader.fxh", "VS", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( nullptr,
                    _T("The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file."), _T("Error"), MB_OK );
        return hr;
    }

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        return hr;
	}

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
	hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
	pVSBlob->Release();
	if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile( L"shader.fxh", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( nullptr,
                    _T("The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file."), _T("Error"), MB_OK );
        return hr;
    }

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader );
	pPSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        XMFLOAT3( 0.0f, 0.5f, 0.5f ),
        XMFLOAT3( 0.5f, -0.5f, 0.5f ),
        XMFLOAT3( -0.5f, -0.5f, 0.5f ),
    };
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 3;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    return S_OK;
}


void cleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pDefaultRenderTargetView ) g_pDefaultRenderTargetView->Release();
    if( g_pSwapChain1 ) g_pSwapChain1->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext1 ) g_pImmediateContext1->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice1 ) g_pd3dDevice1->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();
}


LRESULT CALLBACK wndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

        // Note that this tutorial does not handle resizing (WM_SIZE) requests,
        // so we created the window without the resize border.

    default:
        return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


void render()
{
    if (g_renderTargetView != nullptr)
    {
		// Set the render target to be the render to texture.
		g_pImmediateContext->OMSetRenderTargets(1, &g_renderTargetView, nullptr);

		// Clear the render to texture.
		float color[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
		g_pImmediateContext->ClearRenderTargetView(g_renderTargetView, color);

		// Render a triangle
		g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
		g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
		g_pImmediateContext->Draw(3, 0);

		g_pImmediateContext->OMSetRenderTargets(1, &g_pDefaultRenderTargetView, nullptr);
    }

    // Clear the back buffer 
    g_pImmediateContext->ClearRenderTargetView( g_pDefaultRenderTargetView, Colors::MidnightBlue );

	// Render a triangle
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->Draw(3, 0);


    // Present the information rendered to the back buffer to the front buffer (the screen)
    g_pSwapChain->Present( 0, 0 );
}