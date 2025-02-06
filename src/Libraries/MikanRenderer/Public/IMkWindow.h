#pragma once

#include "MkRendererFwd.h"

class IMkWindow
{
public:
	virtual ~IMkWindow() {}

	virtual bool startup()= 0;
	virtual void update(float deltaSeconds) = 0;
	virtual void render() = 0;
	virtual void shutdown()= 0;

	virtual float getWidth() const= 0;
	virtual float getHeight() const= 0;
	virtual float getAspectRatio() const= 0;
	virtual bool getIsRenderingStage() const= 0;
	virtual bool getIsRenderingUI() const= 0;

	virtual IMkViewportPtr getRenderingViewport() const = 0;
	virtual GlStateStack& getGlStateStack() = 0;
	virtual class IMkLineRenderer* getLineRenderer() = 0;
	virtual class IMkTextRenderer* getTextRenderer() = 0;
	virtual GlShaderCache* getShaderCache() = 0;
	virtual GlTextureCache* getTextureCache() = 0;
	virtual GlModelResourceManager* getModelResourceManager() = 0;
};