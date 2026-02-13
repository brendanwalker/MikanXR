#pragma once

#include "MikanAPI.h"
#include "MikanCameraRenderTarget.h"

using IMkTexturePtr= std::shared_ptr<class IMkTexture>;
using TestGraphicsContext_OpenGLPtr = std::shared_ptr<class TestGraphicsContext_OpenGL>;

class MikanCameraRenderTarget_GL : public TestCameraRenderTarget
{
public:
	MikanCameraRenderTarget_GL(ITestGraphicsContextWeakPtr ownerContext, IMikanAPIPtr mikanAPI, int cameraId);
	virtual ~MikanCameraRenderTarget_GL();

	inline IMkTexturePtr getColorTexture() const { return m_colorTargetTexture; }
	inline IMkTexturePtr getDepthTexture() const { return m_floatDepthTargetTexture; }

	inline bool getIsInitialized() const { m_colorTargetTexture != nullptr && m_floatDepthTargetTexture != nullptr; }

protected:
	TestGraphicsContext_OpenGLPtr getOpenGLOwnerContext() const;
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
};