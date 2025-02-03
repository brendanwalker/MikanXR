#pragma once

#include "RendererFwd.h"
#include "SdlFwd.h"

class IGlWindow
{
public:
	virtual ~IGlWindow() {}

	virtual bool startup()= 0;
	virtual void update(float deltaSeconds) = 0;
	virtual void render() = 0;
	virtual void shutdown()= 0;

	virtual float getWidth() const= 0;
	virtual float getHeight() const= 0;
	virtual float getAspectRatio() const= 0;
	virtual bool getIsRenderingStage() const= 0;
	virtual bool getIsRenderingUI() const= 0;

	virtual GlViewportPtr getRenderingViewport() const = 0;
	virtual GlStateStack& getGlStateStack() = 0;
	virtual GlLineRenderer* getLineRenderer() = 0;
	virtual GlTextRenderer* getTextRenderer() = 0;
	virtual GlShaderCache* getShaderCache() = 0;
	virtual GlTextureCache* getTextureCache() = 0;
	virtual GlModelResourceManager* getModelResourceManager() = 0;
	virtual SdlWindow& getSdlWindow() = 0;

	virtual bool onSDLEvent(const SDL_Event* event)= 0;
};