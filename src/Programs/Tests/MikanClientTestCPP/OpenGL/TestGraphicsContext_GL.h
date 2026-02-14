#pragma once

#include "MkRendererFwd.h"
#include "TestGraphicsContext.h"

#include <memory>

class TestGraphicsContext_GL : 
	public TestGraphicsContext,
	public std::enable_shared_from_this<TestGraphicsContext_GL>
{
public:
	TestGraphicsContext_GL(class TestApp* ownerApp);
	
	MkStateStack& getMkStateStack();

	virtual MikanClientGraphicsApi getGraphicsApi() const override { return MikanClientGraphicsApi_OpenGL; } 
	virtual struct SDL_Window* getSDLWindow() const override;
	virtual TestCameraRenderTargetPtr allocateCameraRenderTarget(int cameraId) override;
	virtual bool create(int windowWidth, int windowHeight) override;
	virtual void recreateMainRenderTarget() override;
	virtual void renderMainTarget() const override;
	virtual bool renderToCameraTarget(class TestCameraRenderTarget* cameraRenderTarget) override;
	virtual void dispose() override;

protected:
	bool initializeCubeGeometry();

private:
	IMkWindowPtr m_mkWindow;
	IMkTriangulatedMeshPtr m_viewportQuadMesh;
	IMkTriangulatedMeshPtr m_boxMesh;

	MikanCameraID m_lastRenderedCameraId = INVALID_MIKAN_ID;
};