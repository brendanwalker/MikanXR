#pragma once

#include "RendererFwd.h"

class IGlWindow
{
public:
	virtual float getWidth() const= 0;
	virtual float getHeight() const= 0;
	virtual float getAspectRatio() const= 0;
	virtual bool getIsRenderingStage() const= 0;
	virtual bool getIsRenderingUI() const= 0;
	virtual GlViewportConstPtr getRenderingViewport() const = 0;
	virtual GlStateStack& getGlStateStack() = 0;
	virtual GlLineRenderer* getLineRenderer() = 0;
	virtual GlTextRenderer* getTextRenderer() = 0;
};
