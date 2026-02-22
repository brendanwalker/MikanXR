#pragma once

#include "MkRendererFwd.h"
#include "TestGraphicsContext.h"

#include <memory>

#include <glm/glm.hpp>

class TestGraphicsContext_GL : 
	public TestGraphicsContext,
	public std::enable_shared_from_this<TestGraphicsContext_GL>
{
public:
	TestGraphicsContext_GL(class TestApp* ownerApp);
	
	MkStateStack& getMkStateStack();

	virtual MikanClientGraphicsApi getGraphicsApi() const override { return MikanClientGraphicsApi_OpenGL; }
	virtual void* getGraphicsDeviceInterface() const override;
	virtual struct SDL_Window* getSDLWindow() const override;
	virtual TestCameraRenderTargetPtr allocateCameraRenderTarget(int cameraId) override;
	virtual bool create(int windowWidth, int windowHeight) override;
	virtual void recreateMainRenderTarget() override;
	virtual void renderMainTarget() const override;
	virtual bool renderToCameraTarget(class TestCameraRenderTarget* cameraRenderTarget) override;
	virtual void dispose() override;

protected:
	class TestMkWindow* getTestMkWindow() const;
	bool initializeCubeGeometry();
	void renderColorTexture(class TestCameraRenderTarget_GL* glRenderTarget) const;
	void renderNormalizedDepthTexture(class TestCameraRenderTarget_GL* glRenderTarget) const;
	void renderPackedDepthTexture(class TestCameraRenderTarget_GL* glRenderTarget, IMikanAPIPtr mikanApi) const;
	void renderCube(
		const glm::mat4& viewProj,
		const glm::vec3& cameraPosition,
		const glm::vec3& cameraForward,
		const glm::vec3& cameraUp,
		const glm::vec3& cameraRight) const;

private:
	IMkWindowPtr m_mkWindow;
	IMkTriangulatedMeshPtr m_viewportQuadMesh;
	IMkTriangulatedMeshPtr m_depthNormalizeQuadMesh;
	IMkTriangulatedMeshPtr m_boxMesh;
	IMkExternalTexturePtr m_depthPackExternalTexture;

	MikanCameraID m_lastRenderedCameraId = INVALID_MIKAN_ID;
};