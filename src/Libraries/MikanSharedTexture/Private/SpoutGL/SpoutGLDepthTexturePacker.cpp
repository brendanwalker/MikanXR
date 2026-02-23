#include <GL/glew.h>

#include "SharedTextureLogger.h"
#include "SharedTextureWriter.h"
#include "SpoutLibrary.h"
#include "SpoutGLDepthTexturePacker.h"

#include <sstream>

struct QuadVertex
{
	float position[2];
	float texCoord[2];
};

namespace SpoutGLDepthPackerShaderCode
{
	const char* packDeviceDepthVertexShader = R""""(
		#version 330 core
		
		layout(location = 0) in vec2 position;
		layout(location = 1) in vec2 texCoord;
		
		out vec2 uv;
		
		void main()
		{
			gl_Position = vec4(position, 0.0, 1.0);
			uv = texCoord;
		}
	)"""";

	const char* packDeviceDepthFragmentShader = R""""(
		#version 330 core
		
		uniform sampler2D InputTexture;
		uniform float zNear;
		uniform float zFar;
		
		in vec2 uv;
		out vec4 FragColor;
		
		void main()
		{
			// Read the raw depth value from the input float depth texture
			float deviceDepth = texture(InputTexture, uv).r;
			
			// Convert from [0,1] depth buffer range to [-1,1] NDC range
			float z_ndc = deviceDepth * 2.0 - 1.0;
    
			// Convert the depth value to a linear eye-space depth [zNear, zFar]
			float eyeDepth = (2.0 * zNear * zFar) / (zFar + zNear - z_ndc * (zFar - zNear));

			// Normalize to [0, 1) range (0 = near, 1 = far)
			// 1.0 is not encoded properly, so we need to clamp it to 0.999999
			float zNorm = min((eyeDepth - zNear) / (zFar - zNear), 0.999999);
			
			// Encode the linear depth value to a RGBA8 texture
			// https://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
			vec4 encodedValue = vec4(1.0, 255.0, 65025.0, 16581375.0) * zNorm;
			encodedValue = fract(encodedValue);
			encodedValue -= encodedValue.yzww * vec4(1.0/255.0, 1.0/255.0, 1.0/255.0, 0.0);
			
			FragColor = encodedValue;
		}
	)"""";

	const char* packSceneDepthVertexShader = R""""(
		#version 330 core
		
		layout(location = 0) in vec2 position;
		layout(location = 1) in vec2 texCoord;
		
		out vec2 uv;
		
		void main()
		{
			gl_Position = vec4(position, 0.0, 1.0);
			uv = texCoord;
		}
	)"""";

	const char* packSceneDepthFragmentShader = R""""(
		#version 330 core
		
		uniform sampler2D InputTexture;
		uniform float zNear;
		uniform float zFar;
		
		in vec2 uv;
		out vec4 FragColor;
		
		void main()
		{
			// Read the linear scene depth value from the input float depth texture
			float eyeDepth = texture(InputTexture, uv).r;
			
			// Convert the eyeDepth value to a normalized [0, 1) value (0 = near, 1 = far)
			// 1.0 is not encoded properly, so we need to clamp it to 0.999999
			float zNorm = min((eyeDepth - zNear) / (zFar - zNear), 0.999999);
			
			// Encode the linear depth value to a RGBA8 texture
			// https://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
			vec4 encodedValue = vec4(1.0, 255.0, 65025.0, 16581375.0) * zNorm;
			encodedValue = fract(encodedValue);
			encodedValue -= encodedValue.yzww * vec4(1.0/255.0, 1.0/255.0, 1.0/255.0, 0.0);
			
			FragColor = encodedValue;
		}
	)"""";
}

SpoutGLDepthTexturePacker::SpoutGLDepthTexturePacker(
	SharedTextureLogger& logger,
	SPOUTLIBRARY* spout,
	const SharedTextureDescriptor* descriptor)
	: m_logger(logger)
	, m_spout(spout)
	, m_mikanDescriptor(*descriptor)
{
}

SpoutGLDepthTexturePacker::~SpoutGLDepthTexturePacker()
{
	dispose();
}

bool SpoutGLDepthTexturePacker::init()
{
	if (!initShader() || !initQuadGeometry())
	{
		dispose();
		return false;
	}

	return true;
}

GLuint SpoutGLDepthTexturePacker::packDepthTexture(
	GLuint inDepthTexture,
	float zNear,
	float zFar)
{
	if (inDepthTexture == 0)
	{
		m_logger.log(SharedTextureLogLevel::error, "packDepthTexture - Invalid input texture");
		return 0;
	}

	// Get the input texture dimensions
	GLsizei inWidth = 0, inHeight = 0;
	glBindTexture(GL_TEXTURE_2D, inDepthTexture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &inWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &inHeight);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Refresh the render target resources if the input texture has changed
	if (m_colorTargetTexture == 0 ||
		m_inFloatDepthTexture != inDepthTexture ||
		m_inFloatDepthTextureWidth != inWidth ||
		m_inFloatDepthTextureHeight != inHeight)
	{
		if (!initRenderTargetResources(inDepthTexture))
		{
			m_logger.log(SharedTextureLogLevel::error, "packDepthTexture - Failed to initialize render target resources");
			return 0;
		}

		// Cache the input texture info
		m_inFloatDepthTexture = inDepthTexture;
		m_inFloatDepthTextureWidth = inWidth;
		m_inFloatDepthTextureHeight = inHeight;
	}

	// Save current OpenGL state
	GLint prevFramebuffer = 0;
	GLint prevViewport[4];
	GLint prevProgram = 0;
	GLboolean prevDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
	GLboolean prevCullFaceEnabled = glIsEnabled(GL_CULL_FACE);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
	glGetIntegerv(GL_VIEWPORT, prevViewport);
	glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);

	// Bind the framebuffer for rendering
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

	// Disable face culling for the fullscreen quad
	glDisable(GL_CULL_FACE);

	// Disable depth testing since we're rendering to a color-only framebuffer 
	glDisable(GL_DEPTH_TEST);

	// Clear the render target
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Set the viewport to match the texture size
	glViewport(0, 0, inWidth, inHeight);

	// Use the depth packing shader
	glUseProgram(m_shaderProgram);

	// Bind the input depth texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inDepthTexture);
	glUniform1i(glGetUniformLocation(m_shaderProgram, "InputTexture"), 0);

	// Set the zNear and zFar uniforms
	glUniform1f(glGetUniformLocation(m_shaderProgram, "zNear"), zNear);
	glUniform1f(glGetUniformLocation(m_shaderProgram, "zFar"), zFar);

	// Draw the fullscreen quad
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	// Restore cull face state
	if (prevCullFaceEnabled) glEnable(GL_CULL_FACE);

	// Restore previous OpenGL state
	if (prevDepthTestEnabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);
	glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
	glUseProgram(prevProgram);

	// The RGBA8 texture contains the packed float depth data
	return m_colorTargetTexture;
}

void SpoutGLDepthTexturePacker::dispose()
{
	if (m_vertexShader != 0)
	{
		glDeleteShader(m_vertexShader);
		m_vertexShader = 0;
	}

	if (m_fragmentShader != 0)
	{
		glDeleteShader(m_fragmentShader);
		m_fragmentShader = 0;
	}

	if (m_shaderProgram != 0)
	{
		glDeleteProgram(m_shaderProgram);
		m_shaderProgram = 0;
	}

	if (m_quadVBO != 0)
	{
		glDeleteBuffers(1, &m_quadVBO);
		m_quadVBO = 0;
	}

	if (m_quadVAO != 0)
	{
		glDeleteVertexArrays(1, &m_quadVAO);
		m_quadVAO = 0;
	}

	disposeRenderTargetResources();
}

bool SpoutGLDepthTexturePacker::initQuadGeometry()
{
	// Define a fullscreen quad (2 triangles)
	QuadVertex vertices[] = {
		{ {-1.0f,  1.0f}, {0.0f, 1.0f} },
		{ {-1.0f, -1.0f}, {0.0f, 0.0f} },
		{ { 1.0f, -1.0f}, {1.0f, 0.0f} },

		{ {-1.0f,  1.0f}, {0.0f, 1.0f} },
		{ { 1.0f, -1.0f}, {1.0f, 0.0f} },
		{ { 1.0f,  1.0f}, {1.0f, 1.0f} },
	};

	// Create and bind VAO
	glGenVertexArrays(1, &m_quadVAO);
	glBindVertexArray(m_quadVAO);

	// Create and bind VBO
	glGenBuffers(1, &m_quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Set up vertex attributes
	// Position attribute (location = 0)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)0);

	// TexCoord attribute (location = 1)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)(2 * sizeof(float)));

	// Unbind VAO
	glBindVertexArray(0);

	return true;
}

bool SpoutGLDepthTexturePacker::initShader()
{
	const char* vertexShaderSource;
	const char* fragmentShaderSource;

	if (m_mikanDescriptor.depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
	{
		vertexShaderSource = SpoutGLDepthPackerShaderCode::packSceneDepthVertexShader;
		fragmentShaderSource = SpoutGLDepthPackerShaderCode::packSceneDepthFragmentShader;
	}
	else
	{
		vertexShaderSource = SpoutGLDepthPackerShaderCode::packDeviceDepthVertexShader;
		fragmentShaderSource = SpoutGLDepthPackerShaderCode::packDeviceDepthFragmentShader;
	}

	// Compile vertex shader
	m_vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
	if (m_vertexShader == 0)
	{
		m_logger.log(SharedTextureLogLevel::error, "initShader - Failed to compile vertex shader");
		return false;
	}

	// Compile fragment shader
	m_fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
	if (m_fragmentShader == 0)
	{
		m_logger.log(SharedTextureLogLevel::error, "initShader - Failed to compile fragment shader");
		return false;
	}

	// Link shader program
	m_shaderProgram = linkProgram(m_vertexShader, m_fragmentShader);
	if (m_shaderProgram == 0)
	{
		m_logger.log(SharedTextureLogLevel::error, "initShader - Failed to link shader program");
		return false;
	}

	return true;
}

GLuint SpoutGLDepthTexturePacker::compileShader(const char* shaderSource, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, nullptr);
	glCompileShader(shader);

	// Check for compilation errors
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);

		std::stringstream ss;
		ss << "compileShader - Shader compilation failed: " << infoLog;
		m_logger.log(SharedTextureLogLevel::error, ss.str());

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint SpoutGLDepthTexturePacker::linkProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	// Check for linking errors
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);

		std::stringstream ss;
		ss << "linkProgram - Program linking failed: " << infoLog;
		m_logger.log(SharedTextureLogLevel::error, ss.str());

		glDeleteProgram(program);
		return 0;
	}

	return program;
}

bool SpoutGLDepthTexturePacker::initRenderTargetResources(GLuint inDepthTexture)
{
	disposeRenderTargetResources();

	// Get the input texture dimensions
	GLsizei inWidth = 0, inHeight = 0;
	glBindTexture(GL_TEXTURE_2D, inDepthTexture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &inWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &inHeight);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Create the color render target texture
	glGenTextures(1, &m_colorTargetTexture);
	glBindTexture(GL_TEXTURE_2D, m_colorTargetTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, inWidth, inHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Create the framebuffer
	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

	// Attach the color texture to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTargetTexture, 0);

	// Check framebuffer completeness
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::stringstream ss;
		ss << "initRenderTargetResources - Framebuffer is not complete: " << status;
		m_logger.log(SharedTextureLogLevel::error, ss.str());

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		disposeRenderTargetResources();
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void SpoutGLDepthTexturePacker::disposeRenderTargetResources()
{
	if (m_colorTargetTexture != 0)
	{
		glDeleteTextures(1, &m_colorTargetTexture);
		m_colorTargetTexture = 0;
	}

	if (m_framebuffer != 0)
	{
		glDeleteFramebuffers(1, &m_framebuffer);
		m_framebuffer = 0;
	}

	m_inFloatDepthTexture = 0;
	m_inFloatDepthTextureWidth = 0;
	m_inFloatDepthTextureHeight = 0;
}