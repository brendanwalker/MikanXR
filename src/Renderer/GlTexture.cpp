#include "GlTexture.h"
#include "GlCommon.h"
#include "Logger.h"

#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GlTexture::GlTexture()
	: m_width(0)
	, m_height(0)
	, m_textureMapData(nullptr)
	, m_pixelType(GL_UNSIGNED_BYTE)
{}

GlTexture::GlTexture(
	uint16_t width, 
	uint16_t height, 
	const uint8_t* textureMapData,
	uint32_t textureFormat,
	uint32_t bufferFormat)
	: m_width(width)
	, m_height(height)
	, m_textureMapData(textureMapData)
	, m_textureFormat(textureFormat)
	, m_bufferFormat(bufferFormat)
{
	determinePixelType();
}

GlTexture::~GlTexture()
{
	disposeTexture();
}

bool GlTexture::bindTexture(int textureUnit) const
{
	if (m_glTextureId != 0)
	{
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, m_glTextureId);
		return true;
	}

	return false;
}

void GlTexture::clearTexture(int textureUnit) const
{
	if (m_glTextureId != 0)
	{
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

bool GlTexture::createTexture()
{
	if (m_width > 0 && m_height > 0)
	{
		glGenTextures(1, &m_glTextureId);
		glBindTexture(GL_TEXTURE_2D, m_glTextureId);

		if (m_textureFormat == GL_R8)
		{
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
		else
		{
			glPixelStorei(GL_PACK_ALIGNMENT, 4);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}

		glTexImage2D(
			GL_TEXTURE_2D, 0, m_textureFormat,
			m_width, m_height, 0,
			m_bufferFormat, 
			m_pixelType, 
			m_textureMapData);

		if (m_bGenerateMipMap &&
			m_textureMapData != nullptr &&
			m_textureFormat != GL_R8 &&
			m_bufferFormat != GL_DEPTH_COMPONENT)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (m_textureFormat == GL_R16UI || 
			m_textureFormat == GL_R32F || m_textureFormat == GL_RG32F ||  
			m_textureFormat == GL_RGB32F || m_textureFormat == GL_RGBA32F ||
			m_textureFormat == GL_DEPTH_COMPONENT32F)
		{
			// Integer textures only support NEAREST filtering
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		if (m_pboMode != PixelBufferObjectMode::NoPBO)
		{
			// Assumes no extra padding in stride
			m_PBOByteSize = m_width * m_height * getBytesPerPixel(m_bufferFormat, m_pixelType);

			switch (m_pboMode)
			{
			case PixelBufferObjectMode::DoublePBOWrite:
				{
					glGenBuffers(2, m_glPixelBufferObjectIDs);
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_glPixelBufferObjectIDs[0]);
					glBufferData(GL_PIXEL_UNPACK_BUFFER, m_PBOByteSize, 0, GL_STREAM_DRAW);
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_glPixelBufferObjectIDs[1]);
					glBufferData(GL_PIXEL_UNPACK_BUFFER, m_PBOByteSize, 0, GL_STREAM_DRAW);
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				} break;
			case PixelBufferObjectMode::SinglePBOWrite:
				{
					glGenBuffers(1, m_glPixelBufferObjectIDs);
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_glPixelBufferObjectIDs[0]);
					glBufferData(GL_PIXEL_UNPACK_BUFFER, m_PBOByteSize, 0, GL_STREAM_DRAW);
					m_glPixelBufferObjectIDs[1] = 0;
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				} break;
			case PixelBufferObjectMode::DoublePBORead:
				{
					glGenBuffers(2, m_glPixelBufferObjectIDs);
					glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPixelBufferObjectIDs[0]);
					glBufferData(GL_PIXEL_PACK_BUFFER, m_PBOByteSize, 0, GL_STREAM_COPY);
					glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPixelBufferObjectIDs[1]);
					glBufferData(GL_PIXEL_PACK_BUFFER, m_PBOByteSize, 0, GL_STREAM_COPY);
					glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				} break;
			case PixelBufferObjectMode::SinglePBORead:
				{
					glGenBuffers(1, m_glPixelBufferObjectIDs);
					glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPixelBufferObjectIDs[0]);
					glBufferData(GL_PIXEL_PACK_BUFFER, m_PBOByteSize, 0, GL_STREAM_COPY);
					m_glPixelBufferObjectIDs[1] = 0;
					glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				} break;
			}
		}

		return true;
	}

	return false;
}

void GlTexture::determinePixelType()
{
	GLenum pixelType;
	switch (m_textureFormat)
	{
		case GL_R32F:
		case GL_RG32F:
		case GL_RGB32F:
		case GL_RGBA32F:
		case GL_DEPTH_COMPONENT32F:
			m_pixelType = GL_FLOAT;
			break;
		case GL_DEPTH_COMPONENT16:
			m_pixelType = GL_UNSIGNED_SHORT;
			break;
		default:
			m_pixelType = GL_UNSIGNED_BYTE;
	}
}

size_t GlTexture::getBytesPerPixel(uint32_t format, uint32_t pixelType)
{
	size_t bytesPerChannel = 0;
	switch (pixelType)
	{
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			bytesPerChannel = 1;
			break;
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
			bytesPerChannel = 2;
			break;
		case GL_FLOAT:
		case GL_UNSIGNED_INT_24_8:
			bytesPerChannel = 4;
			break;
		default:
			{
				MIKAN_LOG_ERROR("getBytesPerPixel") << "Unknown pixelType: " << format;
				assert(false);
			}
	}

	size_t bytesPerPixel = 0;
	switch (format)
	{
	case GL_RED:
		{
			bytesPerPixel = bytesPerChannel;
		} break;
	case GL_RG:
		{
			bytesPerPixel = bytesPerChannel * 2;
		} break;
	case GL_RGB:
	case GL_BGR:
		{
			bytesPerPixel = bytesPerChannel * 3;
		} break;
	case GL_RGBA:
	case GL_BGRA:
		{
			bytesPerPixel = bytesPerChannel * 4;
		} break;
	case GL_DEPTH_COMPONENT:
		{
			bytesPerPixel = bytesPerChannel;
		} break;
	case GL_DEPTH_STENCIL:
		{
			assert(pixelType == GL_UNSIGNED_INT_24_8);
			bytesPerPixel = bytesPerChannel;
		} break;
	default:
		{
			MIKAN_LOG_ERROR("getBytesPerPixel") << "Unknown format: " << format;
			assert(false);
		}
	}

	return bytesPerPixel;
}

bool GlTexture::reloadTextureFromImagePath()
{
	if (m_imagePath.empty())
	{
		MIKAN_LOG_ERROR("reloadTextureFromImagePath") << "Image filename is empty";
		return false;
	}

	if (!std::filesystem::exists(m_imagePath))
	{
		MIKAN_LOG_ERROR("reloadTextureFromImagePath") << "Given filename does not exist at path: " << m_imagePath;
		return false;
	}

	// Free any existing texture data
	disposeTexture();

	int width, height, nrComponents;
	unsigned char* data = stbi_load(m_imagePath.string().c_str(), &width, &height, &nrComponents, 0);
	if (data == nullptr)
	{
		MIKAN_LOG_ERROR("reloadTextureFromImagePath") << "Texture failed to load at path: " << m_imagePath;
		stbi_image_free(data);
	}

	GLenum format = 0;

	if (nrComponents == 1)
		format = GL_RED;
	else if (nrComponents == 3)
		format = GL_RGB;
	else if (nrComponents == 4)
		format = GL_RGBA;

	if (format != 0)
	{
		m_width= width;
		m_height= height;
		m_textureMapData= data;
		m_textureFormat= format;
		m_bufferFormat= format;
		m_pixelType= GL_UNSIGNED_BYTE;

		if (!createTexture())
		{
			MIKAN_LOG_ERROR("reloadTextureFromImagePath") << "Failed to create GL Texture from image at path: " << m_imagePath;
		}
	}

	stbi_image_free(data);

	return true;
}

void GlTexture::copyBufferIntoTexture(const uint8_t* buffer, size_t bufferSize)
{
	if (m_glTextureId != 0)
	{
		glPushAttrib(GL_CLIENT_PIXEL_STORE_BIT);
		if (m_pixelType == GL_UNSIGNED_BYTE)
		{
			glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
			glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}

		glBindTexture(GL_TEXTURE_2D, m_glTextureId);

		// See http://www.songho.ca/opengl/gl_pbo.html#create
		if (m_pboMode == PixelBufferObjectMode::SinglePBOWrite || 
			m_pboMode == PixelBufferObjectMode::DoublePBOWrite)
		{
			// Make sure buffer size matches the PBO size
			assert(bufferSize == m_PBOByteSize);

			int nextPBOIndex = 0;

			if (m_pboMode == PixelBufferObjectMode::SinglePBOWrite)
			{
				// In single PBO mode, the index and nextIndex are set to 0
				m_pboWriteIndex = nextPBOIndex = 0;
			}
			else if (m_pboMode == PixelBufferObjectMode::DoublePBOWrite)
			{
				// In double PBO mode, increment current index first then get the next index
				m_pboWriteIndex = (m_pboWriteIndex + 1) % 2;
				nextPBOIndex = (m_pboWriteIndex + 1) % 2;
			}

			// Bind the the current PBO to write to the texture
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_glPixelBufferObjectIDs[m_pboWriteIndex]);

			// Copy pixels from PBO to texture object
			glTexSubImage2D(
				GL_TEXTURE_2D, 
				0, 
				0, 
				0, 
				m_width, 
				m_height, 
				m_bufferFormat,
				m_pixelType, 
				0); // Treated as offset instead of pointer to pixel data

			// Bind PBO to fill in from main memory
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_glPixelBufferObjectIDs[nextPBOIndex]);

			// Map the buffer object into system memory
			// Note that glMapBuffer() causes sync issue.
			// If GPU is working with this buffer, glMapBuffer() will wait(stall)
			// for GPU to finish its job. To avoid waiting (stall), you can call
			// first glBufferData() with NULL pointer before glMapBuffer().
			// If you do that, the previous data in PBO will be discarded and
			// glMapBuffer() returns a new allocated pointer immediately
			// even if GPU is still working with the previous data.
			glBufferData(GL_PIXEL_UNPACK_BUFFER, bufferSize, 0, GL_STREAM_DRAW);
			GLubyte* writePointer = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
			if (writePointer)
			{
				// update data directly on the mapped buffer
				std::memcpy(writePointer, buffer, bufferSize);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
			}

			// it is good idea to release PBOs with ID 0 after use.
			// Once bound with 0, all pixel operations behave normal ways.
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		}
		else
		{
			// Copy pixels from system memory to GPU memory
			glTexSubImage2D(
				GL_TEXTURE_2D,
				0,
				0,
				0,
				m_width,
				m_height,
				m_bufferFormat,
				m_pixelType,
				buffer);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glPopAttrib();
	}
}

void GlTexture::copyTextureIntoBuffer(uint8_t* outBuffer, size_t bufferSize)
{
	if (m_pboMode == PixelBufferObjectMode::SinglePBORead ||
		m_pboMode == PixelBufferObjectMode::DoublePBORead)
	{
		// Make sure buffer size matches the PBO size
		assert(bufferSize == m_PBOByteSize);

		int nextPBOIndex = 0;

		if (m_pboMode == PixelBufferObjectMode::SinglePBORead)
		{
			// In single PBO mode, the index and nextIndex are set to 0
			m_pboWriteIndex = nextPBOIndex = 0;
		}
		else if (m_pboMode == PixelBufferObjectMode::DoublePBORead)
		{
			// In double PBO mode, increment current index first then get the next index
			m_pboWriteIndex = (m_pboWriteIndex + 1) % 2;
			nextPBOIndex = (m_pboWriteIndex + 1) % 2;
		}

		// Bind the the current PBO to write to the texture
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPixelBufferObjectIDs[m_pboWriteIndex]);

		// Bind tge texture to read from
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_glTextureId);

		// Copy pixels from texture object to PBO
		glGetTexImage(GL_TEXTURE_2D,
			0,
			m_bufferFormat,
			m_pixelType,
			0); // Treated as offset instead of pointer to pixel data

		// Bind PBO to fill in from main memory
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPixelBufferObjectIDs[nextPBOIndex]);

		// Map the buffer object into system memory
		// Note that glMapBuffer() causes sync issue.
		// If GPU is working with this buffer, glMapBuffer() will wait(stall)
		// for GPU to finish its job. To avoid waiting (stall), you can call
		// first glBufferData() with NULL pointer before glMapBuffer().
		// If you do that, the previous data in PBO will be discarded and
		// glMapBuffer() returns a new allocated pointer immediately
		// even if GPU is still working with the previous data.
		glBufferData(GL_PIXEL_PACK_BUFFER, bufferSize, 0, GL_STREAM_READ);
		GLubyte* readPointer = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		if (readPointer)
		{
			// update data directly on the mapped buffer
			std::memcpy(outBuffer, readPointer, bufferSize);
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		}

		//ensure we don't try and read data before the transfer is complete
//		GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
//
//		// then regularly check for completion
//		bool bCompleted= false;
//		while (!bCompleted)
//		{
//			GLint result;
//			glGetSynciv(sync, GL_SYNC_STATUS, sizeof(result), NULL, &result);
//			if (result == GL_SIGNALED)
//			{
//				glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPixelBufferObjectIDs[nextPBOIndex]);
//				void* readPointer = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
//
//				if (readPointer != nullptr)
//				{
//					std::memcpy(outBuffer, readPointer, bufferSize);
//					glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
//					bCompleted= true;
//				}
//			}
//			else
//			{
//#ifdef _MSC_VER
//				Sleep(1);
//#else
//				struct timespec req = { 0 };
//				req.tv_sec = 0;
//				req.tv_nsec = 1 * MILLISECONDS_TO_NANOSECONDS;
//				nanosleep(&req, (struct timespec*)NULL);
//#endif
//			}
//		}

		// it is good idea to release PBOs with ID 0 after use.
		// Once bound with 0, all pixel operations behave normal ways.
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, m_glTextureId);
		glGetTexImage(GL_TEXTURE_2D,
			0,
			m_bufferFormat,
			m_pixelType,
			outBuffer);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void GlTexture::renderFullscreen() const
{
	if (m_glTextureId == 0)
		return;

	// Save a backup of the projection matrix and replace with the identity matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// Save a backup of the modelview matrix and replace with the identity matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Clear the screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind the texture to draw
	glBindTexture(GL_TEXTURE_2D, m_glTextureId);

	// Fill the screen with the texture
	glColor3f(1.f, 1.f, 1.f);
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 1.f); glVertex2f(-1.f, -1.f);
	glTexCoord2f(1.f, 1.f); glVertex2f(1.f, -1.f);
	glTexCoord2f(1.f, 0.f); glVertex2f(1.f, 1.f);
	glTexCoord2f(0.f, 0.f); glVertex2f(-1.f, 1.f);
	glEnd();

	// rebind the default texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Clear the depth buffer to allow overdraw 
	glClear(GL_DEPTH_BUFFER_BIT);

	// Restore the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Restore the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void GlTexture::disposeTexture()
{
	if (m_glTextureId != 0)
	{
		glDeleteTextures(1, &m_glTextureId);
		m_glTextureId = 0;
	}

	if (m_pboMode == PixelBufferObjectMode::DoublePBOWrite)
	{
		glDeleteBuffers(2, m_glPixelBufferObjectIDs);
	}
	else if (m_pboMode == PixelBufferObjectMode::SinglePBOWrite)
	{
		glDeleteBuffers(1, m_glPixelBufferObjectIDs);
	}
	m_glPixelBufferObjectIDs[0] = 0;
	m_glPixelBufferObjectIDs[1] = 0;
}