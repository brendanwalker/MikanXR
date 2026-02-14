#pragma once

#include "MkRendererFwd.h"
#include "TestGraphicsContext.h"

#include <d3d11.h>
#include <memory>

class TestGraphicsContext_DX : 
	public TestGraphicsContext,
	public std::enable_shared_from_this<TestGraphicsContext_DX>
{
public:
	TestGraphicsContext_DX(class TestApp* ownerApp);
	
	virtual MikanClientGraphicsApi getGraphicsApi() const override { return MikanClientGraphicsApi_Direct3D11; } 
	virtual struct SDL_Window* getSDLWindow() const override { return m_sdlWindow; }
	virtual TestCameraRenderTargetPtr allocateCameraRenderTarget(int cameraId) override;
	virtual bool create(int windowWidth, int windowHeight) override;
	virtual void recreateMainRenderTarget() override;
	virtual void renderMainTarget() const override;
	virtual bool renderToCameraTarget(class TestCameraRenderTarget* cameraRenderTarget) override;
	virtual void dispose() override;

protected:
	bool createDeviceD3D();
	void cleanupDeviceD3D();
	void createRenderTarget();
	void cleanupRenderTarget();

private:
	struct SDL_Window* m_sdlWindow = nullptr;
	bool m_sdlInitialized= false;

	int m_windowWidth = 0;
	int m_windowHeight = 0;

	ID3D11Device* m_pd3dDevice = nullptr;
	ID3D11DeviceContext* m_pd3dDeviceContext = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;
};