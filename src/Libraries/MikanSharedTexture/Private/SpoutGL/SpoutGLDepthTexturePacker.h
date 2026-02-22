#pragma once

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLsizei;

class SpoutGLDepthTexturePacker
{
public:
	SpoutGLDepthTexturePacker(
		class SharedTextureLogger& logger,
		struct SPOUTLIBRARY* spout,
		const struct SharedTextureDescriptor* descriptor);
	virtual ~SpoutGLDepthTexturePacker();

	bool init();
	GLuint packDepthTexture(GLuint inDepthTexture, float zNear, float zFar);
	inline GLuint getPackedDepthTextureResourcePtr() const { return m_colorTargetTexture; }

	void dispose();

protected:
	bool initQuadGeometry();
	bool initShader();
	bool initRenderTargetResources(GLuint inDepthTexture);
	void disposeRenderTargetResources();

	GLuint compileShader(const char* shaderSource, GLenum shaderType);
	GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);

private:
	SharedTextureLogger& m_logger;
	SPOUTLIBRARY* m_spout;
	SharedTextureDescriptor m_mikanDescriptor;

	GLuint m_inFloatDepthTexture = 0;
	GLsizei m_inFloatDepthTextureWidth = 0;
	GLsizei m_inFloatDepthTextureHeight = 0;

	GLuint m_vertexShader = 0;
	GLuint m_fragmentShader = 0;
	GLuint m_shaderProgram = 0;

	GLuint m_quadVAO = 0;
	GLuint m_quadVBO = 0;

	GLuint m_colorTargetTexture = 0;
	GLuint m_framebuffer = 0;
};