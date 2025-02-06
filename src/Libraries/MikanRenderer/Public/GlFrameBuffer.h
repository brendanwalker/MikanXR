#pragma once

#include "GlTypesFwd.h"
#include "GlRendererFwd.h"
#include "IGlBindableObject.h"

#include <glm/ext/vector_float4.hpp>

#include <string>
#include <vector>
#include <stdint.h>

class GlFrameBuffer : public IGlBindableObject
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

	GlFrameBuffer()= default;
	GlFrameBuffer(const std::string& name);
	~GlFrameBuffer();

	bool isValid() const { return m_bIsValid; }

	bool createResources();
	void disposeResources();

	void setName(const std::string& name) { m_name = name; }
	void setFrameBufferType(eFrameBufferType frameBufferType);
	void setSize(int width, int height);
	void setColorFormat(eColorFormat colorFormat);
	void setClearColor(const glm::vec4& clearColor) { m_clearColor = clearColor; }
	void setExternalColorTexture(GlTexturePtr texture);

	std::string getName() const { return m_name; }
	GLuint getGlFrameBufferId() const { return m_glFrameBufferId; }
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	eColorFormat getColorFormat() const { return m_colorFormat; }
	GlTexturePtr getColorTexture() const;
	GlTexturePtr getDepthTexture() const;
	GlState* getGlState() const { return m_glState; }

private:
	virtual void bindObject(class GlState& glState) override;
	virtual bool getIsBound() const override { return m_bIsBound; }
	virtual void unbindObject() override;

	bool createColorFrameBuffer();
	bool createDepthFrameBuffer();
	bool createColorAndDepthFrameBuffer();

private:
	GlState* m_glState= nullptr;

	std::string m_name;
	eFrameBufferType m_frameBufferType= eFrameBufferType::COLOR;
	eColorFormat m_colorFormat= eColorFormat::RGB;
	glm::vec4 m_clearColor;

	int m_width= 800;
	int m_height= 600;

	GlTexturePtr m_colorTexture;
	bool m_bIsExternalTexture= false;
	GlTexturePtr m_depthTexture;
	GLuint m_glRenderBufferID= -1;

	// Cached GLState
	GLuint m_glFrameBufferId = -1;
	GLint m_lastGlFrameBufferId = 0;

	bool m_bIsBound= false;
	bool m_bIsValid= false;
};
