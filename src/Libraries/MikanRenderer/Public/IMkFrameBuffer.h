#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"
#include "IMkBindableObject.h"

#include <glm/ext/vector_float4.hpp>

#include <string>
#include <vector>
#include <stdint.h>

class IMkFrameBuffer : public IMkBindableObject
{
public:
	enum class eFrameBufferType
	{
		COLOR,
		DEPTH,
		COLOR_AND_DEPTH,
	};

	enum class eColorFormat
	{
		RGB,
		RGBA,
	};

	virtual ~IMkFrameBuffer() {}

	virtual bool isValid() const = 0;

	virtual bool createResources() = 0;
	virtual void disposeResources() = 0;

	virtual void setName(const std::string& name) = 0;
	virtual void setFrameBufferType(eFrameBufferType frameBufferType) = 0;
	virtual void setSize(int width, int height) = 0;
	virtual void setColorFormat(eColorFormat colorFormat) = 0;
	virtual void setClearColor(const glm::vec4& clearColor) = 0;
	virtual void setExternalColorTexture(IMkTexturePtr texture) = 0;

	virtual std::string getName() const = 0;
	virtual uint32_t getMkFrameBufferId() const = 0;
	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;
	virtual eColorFormat getColorFormat() const = 0;
	virtual IMkTexturePtr getColorTexture() const = 0;
	virtual IMkTexturePtr getDepthTexture() const = 0;
	virtual GlState* getGlState() const = 0;
};

MIKAN_RENDERER_FUNC(IMkFrameBufferPtr) createMkFrameBuffer();
MIKAN_RENDERER_FUNC(IMkFrameBufferPtr) createMkFrameBuffer(const std::string& name);
