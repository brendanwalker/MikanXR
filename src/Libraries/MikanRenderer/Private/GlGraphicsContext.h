#pragma once

#include "IMkGraphicsContext.h"
#include "MkRendererFwd.h"

class GlGraphicsContext : public IMkGraphicsContext
{
public:
	GlGraphicsContext() = delete;
	GlGraphicsContext(class IMkFontManager* fontManager);
	virtual ~GlGraphicsContext();

	// -- IMkGraphicsContext --
	virtual bool startup() override;
	virtual bool renderBegin() override;
	virtual bool renderEnd() override;
	virtual void shutdown() override;

	virtual eGraphicsAPI getGraphicsAPI() const override { return eGraphicsAPI::OpenGL; }

	virtual void onNativeContextCreated(void* nativeContext) override;
	virtual void* getNativeGraphicsContext() const override { return m_glContext; }

	virtual float getWidth() const override { return m_width; }
	virtual float getHeight() const override { return m_height; }
	virtual void onWindowSizeChanged(int width, int height) override;

	virtual IMkViewportPtr getRenderingViewport() const override { return m_renderingViewport; }
	virtual void setRenderingViewport(IMkViewportPtr viewport) override { m_renderingViewport = viewport; }

	virtual MkStateStack& getMkStateStack() override;
	virtual class IMkLineRenderer* getLineRenderer() override;
	virtual class IMkTextRenderer* getTextRenderer() override;
	virtual IMkShaderCache* getShaderCache() override;
	virtual IMkTextureCache* getTextureCache() override;

private:
	// Native GL context handle (void* from SDL_GL_CreateContext)
	void* m_glContext = nullptr;

	// Window dimensions (updated on resize)
	float m_width = 0.f;
	float m_height = 0.f;

	// Rendering viewport
	IMkViewportPtr m_renderingViewport;

	// Font manager needed for text renderer creation (not owned)
	class IMkFontManager* m_fontManager = nullptr;

	// GL rendering resources (created during startup)
	MkStateStackUniquePtr m_stateStack;
	IMkLineRendererPtr m_lineRenderer;
	IMkTextRendererPtr m_textRenderer;
	IMkShaderCachePtr m_shaderCache;
	IMkTextureCachePtr m_textureCache;
};
