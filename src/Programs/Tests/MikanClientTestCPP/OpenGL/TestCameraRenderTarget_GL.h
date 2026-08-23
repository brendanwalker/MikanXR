#pragma once

#include "MikanAPI.h"
#include "MkRendererFwd.h"
#include "TestCameraRenderTarget.h"

#include <glm/glm.hpp>

using IMkTexturePtr= std::shared_ptr<class IMkTexture>;
using TestGraphicsContext_GLPtr= std::shared_ptr<class TestGraphicsContext_GL>;

class TestCameraRenderTarget_GL : public TestCameraRenderTarget
{
public:
	TestCameraRenderTarget_GL(TestGraphicsContextPtr ownerContext, int cameraId);
	virtual ~TestCameraRenderTarget_GL();

	inline bool getIsInitialized() const { return (bool)m_frameBuffer; }
	IMkTexturePtr getColorTexture() const;
	IMkTexturePtr getDepthTexture() const;

	const glm::vec3& getCameraPosition() const { return m_cameraPosition; }
	const glm::vec3& getCameraForward() const { return m_cameraForward; }
	const glm::vec3& getCameraUp() const { return m_cameraUp; }
	const glm::vec3& getCameraRight() const { return m_cameraRight; }
	const glm::mat4 getViewProjectionMatrix() const { return m_projMatrix * m_viewMatrix; }

protected:
	TestGraphicsContext_GLPtr getOpenGLOwnerContext() const;
	virtual bool createGraphicsAPIResources(int textureWidth, int textureHeight) override;
	virtual void bindGraphicsAPIResource() override;
	virtual void unbindGraphicsAPIResource() override;
	virtual void freeGraphicsAPIResources() override;
	virtual void* getGraphicsApiColorTexturePtr() const override;
	virtual void* getGraphicsApiDepthTexturePtr() const override;

	virtual void updateCameraViewMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) override;
	virtual void updateCameraProjectionMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) override;

private:
	std::string m_renderTargetName;

	// Mikan Color and Depth Target
	IMkFrameBufferPtr m_frameBuffer;
	IMkState* m_mkState= nullptr;

	glm::mat4 m_projMatrix;
	glm::mat4 m_viewMatrix;
	glm::vec3 m_cameraPosition;
	glm::vec3 m_cameraForward;
	glm::vec3 m_cameraUp;
	glm::vec3 m_cameraRight;
};