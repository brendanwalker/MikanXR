// Mikan API Headers
#include "Logger.h"
#include "MikanAPI.h"
#include "MikanCameraRequests.h"
#include "MikanCameraRenderTarget.h"
#include "MikanVideoSourceEvents.h"
#include "MikanVideoSourceRequests.h"
#include "MikanClientRequests.h"
#include "MikanClientEvents.h"
#include "MikanCameraEvents.h"
#include "MikanMathTypes.h"
#include "MikanStencilTypes.h"
#include "MikanCameraTypes.h"
#include "MikanVideoSourceTypes.h"

// Test App Framework Headers
#include "MikanDirectXUtils.h"
#include "MikanTestApp.h"
#include "MikanCameraRenderTarget_DX.h"

// System Headers
#include <tchar.h>
#include <codecvt>
#include <map>

using namespace DirectX;

#define MIKAN_CLIENT_ID     "MikanClientTestDX"

HINSTANCE g_hInst;
HWND g_hWnd;

struct SimpleVertex
{
    XMFLOAT3 Pos;
};

class MikanTestApp 
{
public:
	MikanTestApp()
	{

	}

	int exec(const char* szClientName, int argc, char** argv)
	{
		if (startup(szClientName, argc, argv))
		{
			m_lastFrameTimestamp = std::chrono::high_resolution_clock::now();

			while (!m_bShutdownRequested)
			{
				// Update the frame rate
				auto now = std::chrono::high_resolution_clock::now();
				const double deltaSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(now - m_lastFrameTimestamp).count();
				m_fps = deltaSeconds > 0.0 ? (float)(1.0 / deltaSeconds) : 0.f;
				m_lastFrameTimestamp = now;

				update(deltaSeconds);
			}
		}
		else
		{
			MIKAN_LOG_ERROR("exec") << "Failed to initialize application!";
		}

		shutdown();

		return m_returnCode;
	}

protected:
	virtual MikanClientGraphicsApi getGraphicsApi() const override 
	{ 
		return MikanClientGraphicsApi_Direct3D11;  
	}
     
    virtual bool startup(const char* szClientName, int argc, char** argv) override 
    {
		bool bSuccess=
			initSwapChain() &&
			initShader() &&
			initGeometry();

		if (!bSuccess)
		{
			return false;
		}

		return Super::startup(szClientName, argc, argv);
    }

    virtual void update(const double deltaSeconds) override
    {
		MSG msg = { 0 };
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                requestShutdown((int)msg.wParam);
                return;
            }
		}

        Super::update(deltaSeconds);
    }
    
    virtual bool renderToCameraTarget(TestCameraRenderTarget* cameraRenderTarget) const override
    {
		auto* cameraRenderTargetDX= static_cast<MikanCameraRenderTarget_DX *>(cameraRenderTarget);

		ID3D11RenderTargetView* renderTargetView= cameraRenderTargetDX->getColorTargetView();
		if (renderTargetView == nullptr)
            return false;

		// Set the render target to be the render to texture.
		m_pImmediateContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

		// Clear the render to texture.
		float color[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
		m_pImmediateContext->ClearRenderTargetView(renderTargetView, color);

		// Render a triangle
		m_pImmediateContext->VSSetShader(m_pVertexShader, nullptr, 0);
		m_pImmediateContext->PSSetShader(m_pPixelShader, nullptr, 0);
		m_pImmediateContext->Draw(3, 0);

        return true;
    }

    virtual bool renderToViewport() const override
    {
		m_pImmediateContext->OMSetRenderTargets(1, &m_pDefaultRenderTargetView, nullptr);

		// Clear the back buffer 
		m_pImmediateContext->ClearRenderTargetView(m_pDefaultRenderTargetView, Colors::MidnightBlue);

		// Render a triangle
		m_pImmediateContext->VSSetShader(m_pVertexShader, nullptr, 0);
		m_pImmediateContext->PSSetShader(m_pPixelShader, nullptr, 0);
		m_pImmediateContext->Draw(3, 0);

		// Present the information rendered to the back buffer to the front buffer (the screen)
		m_pSwapChain->Present(0, 0);

		return true;
    }

    virtual void shutdown() override
    {
		Super::shutdown();

		cleanupDevice();
    }

	// Camera Render Target Helpers
    virtual TestCameraRenderTargetPtr allocateCameraRenderTarget(IMikanAPIPtr mikanAPI, int cameraId) override 
    {
        return std::make_shared<MikanCameraRenderTarget_DX>(m_mikanApi, m_pd3dDevice, cameraId);
    }

	bool initSwapChain()
	{
		HRESULT hr = S_OK;

		RECT rc;
		GetClientRect(m_hWnd, &rc);
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
		UINT numDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			m_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);

			if (hr == E_INVALIDARG)
			{
				// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
				hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
					D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
			}

			if (SUCCEEDED(hr))
				break;
		}

		if (FAILED(hr))
			return false;

		// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
		IDXGIFactory1* dxgiFactory = nullptr;
		{
			IDXGIDevice* dxgiDevice = nullptr;
			hr = m_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
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
					std::string description = converterX.to_bytes(adapterDesc.Description);

					MIKAN_LOG_ERROR("startup") << "Selected DirextX Adapter: " << description;
					hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
					adapter->Release();
				}
				dxgiDevice->Release();
			}
		}

		if (FAILED(hr))
			return false;

		// Create swap chain
		IDXGIFactory2* dxgiFactory2 = nullptr;
		hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
		if (dxgiFactory2)
		{
			// DirectX 11.1 or later
			hr = m_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&m_pd3dDevice1));
			if (SUCCEEDED(hr))
			{
				(void)m_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&m_pImmediateContext1));
			}

			DXGI_SWAP_CHAIN_DESC1 sd = {};
			sd.Width = width;
			sd.Height = height;
			sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 1;

			hr = dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice, m_hWnd, &sd, nullptr, nullptr, &m_pSwapChain1);
			if (SUCCEEDED(hr))
			{
				hr = m_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&m_pSwapChain));
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
			sd.OutputWindow = m_hWnd;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = TRUE;

			hr = dxgiFactory->CreateSwapChain(m_pd3dDevice, &sd, &m_pSwapChain);
		}

		// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
		dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

		dxgiFactory->Release();

		if (FAILED(hr))
			return false;

		// Create a render target view
		ID3D11Texture2D* pBackBuffer = nullptr;
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
		if (FAILED(hr))
			return false;

		hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pDefaultRenderTargetView);
		pBackBuffer->Release();
		if (FAILED(hr))
			return false;

		m_pImmediateContext->OMSetRenderTargets(1, &m_pDefaultRenderTargetView, nullptr);

		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_pImmediateContext->RSSetViewports(1, &vp);

		return true;
	}

	bool initShader()
	{
		HRESULT hr = S_OK;

		// Compile the vertex shader
		ID3DBlob* pVSBlob = nullptr;
		hr = CompileShaderFromFile(L"shader.fxh", "VS", "vs_4_0", &pVSBlob);
		if (FAILED(hr))
		{
			MessageBox(nullptr,
				_T("The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file."), _T("Error"), MB_OK);
			return false;
		}

		// Create the vertex shader
		hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);
		if (FAILED(hr))
		{
			pVSBlob->Release();
			return false;
		}

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);

		// Create the input layout
		hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
			pVSBlob->GetBufferSize(), &m_pVertexLayout);
		pVSBlob->Release();
		if (FAILED(hr))
			return false;

		// Set the input layout
		m_pImmediateContext->IASetInputLayout(m_pVertexLayout);

		// Compile the pixel shader
		ID3DBlob* pPSBlob = nullptr;
		hr = CompileShaderFromFile(L"shader.fxh", "PS", "ps_4_0", &pPSBlob);
		if (FAILED(hr))
		{
			MessageBox(nullptr,
				_T("The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file."), _T("Error"), MB_OK);
			return false;
		}

		// Create the pixel shader
		hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader);
		pPSBlob->Release();
		if (FAILED(hr))
			return false;

		return true;
	}

	bool initGeometry()
	{
		HRESULT hr = S_OK;

		// Create vertex buffer
		SimpleVertex vertices[] =
		{
			XMFLOAT3(0.0f, 0.5f, 0.5f),
			XMFLOAT3(0.5f, -0.5f, 0.5f),
			XMFLOAT3(-0.5f, -0.5f, 0.5f),
		};
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * 3;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = vertices;
		hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
		if (FAILED(hr))
			return false;

		// Set vertex buffer
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

		// Set primitive topology
		m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		return true;
	}

	void cleanupDevice()
	{
		if (m_pImmediateContext) m_pImmediateContext->ClearState();
		if (m_pVertexBuffer) m_pVertexBuffer->Release();
		if (m_pVertexLayout) m_pVertexLayout->Release();
		if (m_pVertexShader) m_pVertexShader->Release();
		if (m_pPixelShader) m_pPixelShader->Release();
		if (m_pDefaultRenderTargetView) m_pDefaultRenderTargetView->Release();
		if (m_pSwapChain1) m_pSwapChain1->Release();
		if (m_pSwapChain) m_pSwapChain->Release();
		if (m_pImmediateContext1) m_pImmediateContext1->Release();
		if (m_pImmediateContext) m_pImmediateContext->Release();
		if (m_pd3dDevice1) m_pd3dDevice1->Release();
		if (m_pd3dDevice) m_pd3dDevice->Release();
	}

private:
	HINSTANCE m_hInst = nullptr;
	HWND m_hWnd = nullptr;
	D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* m_pd3dDevice = nullptr;
	ID3D11Device1* m_pd3dDevice1 = nullptr;
	ID3D11DeviceContext* m_pImmediateContext = nullptr;
	ID3D11DeviceContext1* m_pImmediateContext1 = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	IDXGISwapChain1* m_pSwapChain1 = nullptr;
	ID3D11RenderTargetView* m_pDefaultRenderTargetView = nullptr;
	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;
	ID3D11InputLayout* m_pVertexLayout = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
};

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

HRESULT initWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = _T("MikanDirectXTestClass");
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_APPLICATION);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(_T("MikanDirectXTestClass"), _T("Mikan DirectX 11 Test rendering"),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );    

    if( FAILED( initWindow( hInstance, nCmdShow ) ) )
        return 1;

	int argc;
	LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (!argvW) {
		return 1;
	}

	// Convert to UTF-8 char**
	std::vector<std::string> argStrings;
	std::vector<char*> argv;

	for (int i = 0; i < argc; ++i) 
    {
		int size = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
		std::string str(size, 0);

		WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, &str[0], size, nullptr, nullptr);
		str.resize(size - 1); // Remove null terminator
		argStrings.push_back(str);
		argv.push_back(&argStrings.back()[0]);
	}
	argv.push_back(nullptr); // Null-terminate argv

	LocalFree(argvW);

    MikanTestApp_DX app;
    int retCode= app.exec(MIKAN_CLIENT_ID, argc, argv.data());

    return retCode;
}