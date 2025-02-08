#include "GlCommon.h"
#include "Logger.h"


static inline const char* errorToString(const GLenum errorCode)
{
	switch (errorCode)
	{
	case GL_NO_ERROR: return "GL_NO_ERROR";
	case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
	case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW"; // Legacy; not used on GL3+
	case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";  // Legacy; not used on GL3+
	case GL_FRAMEBUFFER_UNSUPPORTED: return "GL_FRAMEBUFFER_UNSUPPORTED";
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: return "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: return "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
	default: return "Unknown GL error";
	} 
}

bool checkHasAnyMkError(const char* context, const char* file, const int line)
{
	bool bHasAnyError= false;

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		MIKAN_LOG_ERROR("checkGLError") 
			<< context << " - " << file << "(" << line << ") : "
			<< "GL_CORE_ERROR=0x" << std::hex << err << " - " 
			<< errorToString(err);
		bHasAnyError= true;
	}

	return bHasAnyError;
}