#pragma once

#include "MkRendererExport.h"
#include "MkRendererFwd.h"

enum class eGraphicsAPI { 
	INVALID= -1,

	OpenGL= 0,
	Vulkan = 1,
	Direct3D11 = 2,
	Metal = 3,

	COUNT
};

class IMkGraphicsContext
{
public:
	virtual ~IMkGraphicsContext() {}

	virtual bool startup() = 0;
	virtual bool renderBegin() = 0;
	virtual bool renderEnd() = 0;
	virtual void shutdown() = 0;

	virtual eGraphicsAPI getGraphicsAPI() const = 0;

	// Called by the windowing system after the native graphics context is created,
	// before startup() is called. Allows the graphics context to store the handle
	// for getNativeGraphicsContext().
	virtual void onNativeContextCreated(void* nativeContext) = 0;
	virtual void* getNativeGraphicsContext() const = 0;

	// Window dimensions - updated by the windowing system on resize
	virtual float getWidth() const = 0;
	virtual float getHeight() const = 0;
	virtual void onWindowSizeChanged(int width, int height) = 0;

	// Rendering viewport - set/queried by window and used by renderers
	virtual IMkViewportPtr getRenderingViewport() const = 0;
	virtual void setRenderingViewport(IMkViewportPtr viewport) = 0;

	virtual MkStateStack& getMkStateStack() = 0;
	virtual class IMkLineRenderer* getLineRenderer() = 0;
	virtual class IMkTextRenderer* getTextRenderer() = 0;
	virtual IMkShaderCache* getShaderCache() = 0;
	virtual IMkTextureCache* getTextureCache() = 0;
};

MIKAN_RENDERER_FUNC(IMkGraphicsContextPtr) createMkGraphicsContext(
	eGraphicsAPI api,
	class IMkFontManager* fontManager);
