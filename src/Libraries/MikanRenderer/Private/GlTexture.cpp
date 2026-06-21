#include "IMkTexture.h"
#include "GlCommon.h"
#include "Logger.h"

#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

class GlTexture : public IMkTexture
{
public:
	GlTexture()
		: m_width(0)
		, m_height(0)
		, m_textureMapData(nullptr)
		, m_pixelType(GL_UNSIGNED_BYTE)
		, m_wrapMode(TextureWrapMode::ClampToEdge)
	{
	}

	GlTexture(uint16_t width, uint16_t height, const uint8_t* textureMapData, uint32_t textureFormat,
			  uint32_t bufferFormat)
		: m_width(width)
		, m_height(height)
		, m_textureMapData(textureMapData)
		, m_textureFormat(textureFormat)
		, m_bufferFormat(bufferFormat)
	{
		determinePixelType();
	}

	virtual ~GlTexture() { disposeTexture(); }

	virtual IMkTexture* setName(const std::string& name) override
	{
		m_name= name;

		return this;
	}

	virtual IMkTexture* setSize(uint16_t width, uint16_t height) override
	{
		m_width= width;
		m_height= height;

		return this;
	}

	virtual IMkTexture* setTextureMapData(const uint8_t* textureMapData) override
	{
		m_textureMapData= textureMapData;

		return this;
	}

	virtual IMkTexture* setTextureFormat(uint32_t textureFormat) override
	{
		m_textureFormat= textureFormat;
		m_bufferFormat= textureFormat;
		determinePixelType();

		return this;
	}

	virtual IMkTexture* setBufferFormat(uint32_t bufferFormat) override
	{
		m_bufferFormat= bufferFormat;

		return this;
	}

	virtual IMkTexture* setPixelType(uint32_t pixelType) override
	{
		m_pixelType= pixelType;

		return this;
	}

	virtual IMkTexture* setGenerateMipMap(bool bFlag) override
	{
		m_bGenerateMipMap= bFlag;

		return this;
	}

	virtual IMkTexture* setPixelBufferObjectMode(PixelBufferObjectMode mode) override
	{
		m_pboMode= mode;

		return this;
	}

	virtual IMkTexture* setTextureWrapMode(TextureWrapMode mode) override
	{
		m_wrapMode= mode;

		return this;
	}

	virtual void setImagePath(const std::filesystem::path& path) override { m_imagePath= path; }

	virtual const std::filesystem::path getImagePath() const override { return m_imagePath; }

	virtual bool reloadTextureFromImagePath() override
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
		unsigned char* data= stbi_load(m_imagePath.string().c_str(), &width, &height, &nrComponents, 0);
		if (data == nullptr)
		{
			MIKAN_LOG_ERROR("reloadTextureFromImagePath") << "Texture failed to load at path: " << m_imagePath;
			stbi_image_free(data);
		}

		GLenum format= 0;

		if (nrComponents == 1)
			format= GL_RED;
		else if (nrComponents == 3)
			format= GL_RGB;
		else if (nrComponents == 4)
			format= GL_RGBA;

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
				MIKAN_LOG_ERROR("reloadTextureFromImagePath")
					<< "Failed to create GL Texture from image at path: " << m_imagePath;
			}
		}

		stbi_image_free(data);

		return true;
	}

	virtual bool createTexture() override
	{
		if (m_width > 0 && m_height > 0)
		{
			glGenTextures(1, &m_glTextureId);
			glBindTexture(GL_TEXTURE_2D, m_glTextureId);

			if (!m_name.empty())
			{
				glObjectLabel(GL_TEXTURE, m_glTextureId, -1, m_name.c_str());
			}

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

			glTexImage2D(GL_TEXTURE_2D, 0, m_textureFormat, m_width, m_height, 0, m_bufferFormat, m_pixelType,
						 m_textureMapData);

			bool bMipmapGenerated= false;
			if (m_bGenerateMipMap && m_textureMapData != nullptr && m_textureFormat != GL_R8
				&& m_bufferFormat != GL_DEPTH_COMPONENT)
			{
				glGenerateMipmap(GL_TEXTURE_2D);
				bMipmapGenerated= true;
			}

			GLenum glWrapMode= GL_CLAMP_TO_EDGE;
			switch (m_wrapMode)
			{
			case TextureWrapMode::ClampToEdge:
				glWrapMode= GL_CLAMP_TO_EDGE;
				break;
			case TextureWrapMode::Repeat:
				glWrapMode= GL_REPEAT;
				break;
			case TextureWrapMode::MirroredRepeat:
				glWrapMode= GL_MIRRORED_REPEAT;
				break;
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrapMode);

			if (m_textureFormat == GL_R16UI || m_textureFormat == GL_R32F || m_textureFormat == GL_RG32F
				|| m_textureFormat == GL_RGB32F || m_textureFormat == GL_RGBA32F
				|| m_textureFormat == GL_DEPTH_COMPONENT32F)
			{
				// Integer textures only support NEAREST filtering
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				// Use trilinear filtering if mipmaps were generated
				if (bMipmapGenerated)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				}
			}

			glBindTexture(GL_TEXTURE_2D, 0);

			if (m_pboMode != PixelBufferObjectMode::NoPBO)
			{
				// Assumes no extra padding in stride
				m_PBOByteSize= m_width * m_height * getBytesPerPixel(m_bufferFormat, m_pixelType);

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
				}
				break;
				case PixelBufferObjectMode::SinglePBOWrite:
				{
					glGenBuffers(1, m_glPixelBufferObjectIDs);
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_glPixelBufferObjectIDs[0]);
					glBufferData(GL_PIXEL_UNPACK_BUFFER, m_PBOByteSize, 0, GL_STREAM_DRAW);
					m_glPixelBufferObjectIDs[1]= 0;
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				}
				break;
				case PixelBufferObjectMode::DoublePBORead:
				{
					glGenBuffers(2, m_glPixelBufferObjectIDs);
					glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPixelBufferObjectIDs[0]);
					glBufferData(GL_PIXEL_PACK_BUFFER, m_PBOByteSize, 0, GL_STREAM_COPY);
					glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPixelBufferObjectIDs[1]);
					glBufferData(GL_PIXEL_PACK_BUFFER, m_PBOByteSize, 0, GL_STREAM_COPY);
					glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				}
				break;
				case PixelBufferObjectMode::SinglePBORead:
				{
					glGenBuffers(1, m_glPixelBufferObjectIDs);
					glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPixelBufferObjectIDs[0]);
					glBufferData(GL_PIXEL_PACK_BUFFER, m_PBOByteSize, 0, GL_STREAM_COPY);
					m_glPixelBufferObjectIDs[1]= 0;
					glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				}
				break;
				}
			}

			return true;
		}

		return false;
	}

	virtual void copyBufferIntoTexture(const uint8_t* buffer, size_t bufferSize) override
	{
		if (m_glTextureId != 0)
		{
			// TODO: Move this to a GLStateModifier
			class ScopedPixelStoreSetting
			{
			public:
				ScopedPixelStoreSetting(GLenum pname, GLint param)
					: m_pname(pname)
					, m_param(param)
				{
					glGetIntegerv(m_pname, &m_prevParam);
				}

				void apply()
				{
					if (m_prevParam != m_param)
					{
						glPixelStorei(m_pname, m_param);
					}
				}

				void revert()
				{
					if (m_prevParam != m_param)
					{
						glPixelStorei(m_pname, m_prevParam);
					}
				}

			protected:
				GLenum m_pname;
				GLint m_param;
				GLint m_prevParam;
			};
			std::vector<ScopedPixelStoreSetting> scopedPixelStoreSettings;

			if (m_pixelType == GL_UNSIGNED_BYTE)
			{
				scopedPixelStoreSettings.push_back(ScopedPixelStoreSetting(GL_UNPACK_SWAP_BYTES, GL_FALSE));
				scopedPixelStoreSettings.push_back(ScopedPixelStoreSetting(GL_UNPACK_LSB_FIRST, GL_TRUE));
				scopedPixelStoreSettings.push_back(ScopedPixelStoreSetting(GL_UNPACK_ROW_LENGTH, 0));
				scopedPixelStoreSettings.push_back(ScopedPixelStoreSetting(GL_UNPACK_SKIP_PIXELS, 0));
				scopedPixelStoreSettings.push_back(ScopedPixelStoreSetting(GL_UNPACK_SKIP_ROWS, 0));
				scopedPixelStoreSettings.push_back(ScopedPixelStoreSetting(GL_UNPACK_ALIGNMENT, 1));

				for (auto& setting : scopedPixelStoreSettings)
				{
					setting.apply();
				}
			}

			glBindTexture(GL_TEXTURE_2D, m_glTextureId);

			// See http://www.songho.ca/opengl/gl_pbo.html#create
			if (m_pboMode == PixelBufferObjectMode::SinglePBOWrite
				|| m_pboMode == PixelBufferObjectMode::DoublePBOWrite)
			{
				// Make sure buffer size matches the PBO size
				assert(bufferSize == m_PBOByteSize);

				int nextPBOIndex= 0;

				if (m_pboMode == PixelBufferObjectMode::SinglePBOWrite)
				{
					// In single PBO mode, the index and nextIndex are set to 0
					m_pboWriteIndex= nextPBOIndex= 0;
				}
				else if (m_pboMode == PixelBufferObjectMode::DoublePBOWrite)
				{
					// In double PBO mode, increment current index first then get the next index
					m_pboWriteIndex= (m_pboWriteIndex + 1) % 2;
					nextPBOIndex= (m_pboWriteIndex + 1) % 2;
				}

				// Bind the the current PBO to write to the texture
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_glPixelBufferObjectIDs[m_pboWriteIndex]);

				// Copy pixels from PBO to texture object
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, m_bufferFormat, m_pixelType,
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
				GLubyte* writePointer= (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
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
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, m_bufferFormat, m_pixelType, buffer);
			}

			glBindTexture(GL_TEXTURE_2D, 0);

			// Restore the previous pixel store settings
			for (auto& setting : scopedPixelStoreSettings)
			{
				setting.revert();
			}
		}
	}

	virtual void copyTextureIntoBuffer(uint8_t* outBuffer, size_t bufferSize) override
	{
		if (m_pboMode == PixelBufferObjectMode::SinglePBORead || m_pboMode == PixelBufferObjectMode::DoublePBORead)
		{
			// Make sure buffer size matches the PBO size
			assert(bufferSize == m_PBOByteSize);

			int nextPBOIndex= 0;

			if (m_pboMode == PixelBufferObjectMode::SinglePBORead)
			{
				// In single PBO mode, the index and nextIndex are set to 0
				m_pboWriteIndex= nextPBOIndex= 0;
			}
			else if (m_pboMode == PixelBufferObjectMode::DoublePBORead)
			{
				// In double PBO mode, increment current index first then get the next index
				m_pboWriteIndex= (m_pboWriteIndex + 1) % 2;
				nextPBOIndex= (m_pboWriteIndex + 1) % 2;
			}

			// Bind the the current PBO to write to the texture
			glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPixelBufferObjectIDs[m_pboWriteIndex]);

			// Bind tge texture to read from
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_glTextureId);

			// Copy pixels from texture object to PBO
			glGetTexImage(GL_TEXTURE_2D, 0, m_bufferFormat, m_pixelType,
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
			GLubyte* readPointer= (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
			if (readPointer)
			{
				// update data directly on the mapped buffer
				std::memcpy(outBuffer, readPointer, bufferSize);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			}

			// ensure we don't try and read data before the transfer is complete
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
			// #ifdef _MSC_VER
			//				Sleep(1);
			// #else
			//				struct timespec req = { 0 };
			//				req.tv_sec = 0;
			//				req.tv_nsec = 1 * MILLISECONDS_TO_NANOSECONDS;
			//				nanosleep(&req, (struct timespec*)NULL);
			// #endif
			//			}
			//		}

			// it is good idea to release PBOs with ID 0 after use.
			// Once bound with 0, all pixel operations behave normal ways.
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_glTextureId);
			glGetTexImage(GL_TEXTURE_2D, 0, m_bufferFormat, m_pixelType, outBuffer);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	virtual void disposeTexture() override
	{
		if (m_glTextureId != 0)
		{
			glDeleteTextures(1, &m_glTextureId);
			m_glTextureId= 0;
		}

		if (m_pboMode == PixelBufferObjectMode::DoublePBOWrite)
		{
			glDeleteBuffers(2, m_glPixelBufferObjectIDs);
		}
		else if (m_pboMode == PixelBufferObjectMode::SinglePBOWrite)
		{
			glDeleteBuffers(1, m_glPixelBufferObjectIDs);
		}
		m_glPixelBufferObjectIDs[0]= 0;
		m_glPixelBufferObjectIDs[1]= 0;
	}

	virtual bool bindTexture(int textureUnit) const override
	{
		if (m_glTextureId != 0)
		{
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(GL_TEXTURE_2D, m_glTextureId);
			return true;
		}

		return false;
	}

	virtual void clearTexture(int textureUnit) const override
	{
		if (m_glTextureId != 0)
		{
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	virtual const std::string getName() const override { return m_name; }
	virtual bool getIsValid() const override { return m_glTextureId != 0; }
	virtual void* getPlatformTexture() override { return &m_glTextureId; }
	virtual uint32_t getGlTextureId() const override { return m_glTextureId; }
	virtual uint16_t getTextureWidth() const override { return m_width; }
	virtual uint16_t getTextureHeight() const override { return m_height; }
	virtual uint32_t getTextureFormat() const override { return m_textureFormat; }
	virtual uint32_t getBufferFormat() const override { return m_bufferFormat; }

protected:
	void determinePixelType()
	{
		GLenum pixelType;
		switch (m_textureFormat)
		{
		case GL_R32F:
		case GL_RG32F:
		case GL_RGB32F:
		case GL_RGBA32F:
		case GL_DEPTH_COMPONENT32F:
			m_pixelType= GL_FLOAT;
			break;
		case GL_DEPTH_COMPONENT16:
			m_pixelType= GL_UNSIGNED_SHORT;
			break;
		default:
			m_pixelType= GL_UNSIGNED_BYTE;
		}
	}

	size_t getBytesPerPixel(uint32_t format, uint32_t pixelType)
	{
		size_t bytesPerChannel= 0;
		switch (pixelType)
		{
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			bytesPerChannel= 1;
			break;
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
			bytesPerChannel= 2;
			break;
		case GL_FLOAT:
		case GL_UNSIGNED_INT_24_8:
			bytesPerChannel= 4;
			break;
		default:
		{
			MIKAN_LOG_ERROR("getBytesPerPixel") << "Unknown pixelType: " << format;
			assert(false);
		}
		}

		size_t bytesPerPixel= 0;
		switch (format)
		{
		case GL_RED:
		{
			bytesPerPixel= bytesPerChannel;
		}
		break;
		case GL_RG:
		{
			bytesPerPixel= bytesPerChannel * 2;
		}
		break;
		case GL_RGB:
		case GL_BGR:
		{
			bytesPerPixel= bytesPerChannel * 3;
		}
		break;
		case GL_RGBA:
		case GL_BGRA:
		{
			bytesPerPixel= bytesPerChannel * 4;
		}
		break;
		case GL_DEPTH_COMPONENT:
		{
			bytesPerPixel= bytesPerChannel;
		}
		break;
		case GL_DEPTH_STENCIL:
		{
			assert(pixelType == GL_UNSIGNED_INT_24_8);
			bytesPerPixel= bytesPerChannel;
		}
		break;
		default:
		{
			MIKAN_LOG_ERROR("getBytesPerPixel") << "Unknown format: " << format;
			assert(false);
		}
		}

		return bytesPerPixel;
	}

protected:
	std::string m_name;
	uint32_t m_glTextureId= 0;
	uint16_t m_width= 0;
	uint16_t m_height= 0;
	uint32_t m_textureFormat= 0;
	uint32_t m_bufferFormat= 0;
	uint32_t m_pixelType= 0;
	uint32_t m_glPixelBufferObjectIDs[2]= {0, 0};
	uint16_t m_pboWriteIndex= 0;
	PixelBufferObjectMode m_pboMode= PixelBufferObjectMode::NoPBO;
	size_t m_PBOByteSize= 0;
	bool m_bGenerateMipMap= true;
	TextureWrapMode m_wrapMode= TextureWrapMode::ClampToEdge;
	const uint8_t* m_textureMapData;
	std::filesystem::path m_imagePath;
};

class GlExternalTexture : public IMkExternalTexture
{
public:
	GlExternalTexture() {}

	GlExternalTexture(const void* platformTexture) { setExternalPlatformTexture(const_cast<void*>(platformTexture)); }

	virtual void setExternalPlatformTexture(void* platformTexture) override
	{
		auto* glTextureId= reinterpret_cast<const uint32_t*>(platformTexture);

		if (glTextureId != nullptr)
		{
			if (*glTextureId != m_glTextureId)
			{
				m_glTextureId= *glTextureId;

				glBindTexture(GL_TEXTURE_2D, m_glTextureId);

				// Width and height
				GLint width= 0, height= 0;
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
				m_width= static_cast<uint16_t>(width);
				m_height= static_cast<uint16_t>(height);

				// Internal (texture) format
				GLint internalFormat= 0;
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
				m_textureFormat= static_cast<uint32_t>(internalFormat);

				switch (internalFormat)
				{
				case GL_RGBA8:
					m_bufferFormat= GL_RGBA;
					m_pixelType= GL_UNSIGNED_BYTE;
					break;
				case GL_RGB8:
					m_bufferFormat= GL_RGB;
					m_pixelType= GL_UNSIGNED_BYTE;
					break;
				case GL_RG8:
					m_bufferFormat= GL_RG;
					m_pixelType= GL_UNSIGNED_BYTE;
					break;
				case GL_R8:
					m_bufferFormat= GL_RED;
					m_pixelType= GL_UNSIGNED_BYTE;
					break;
				case GL_RGBA16F:
					m_bufferFormat= GL_RGBA;
					m_pixelType= GL_HALF_FLOAT;
					break;
				case GL_RGBA32F:
					m_bufferFormat= GL_RGBA;
					m_pixelType= GL_FLOAT;
					break;
				case GL_DEPTH_COMPONENT24:
				case GL_DEPTH_COMPONENT32F:
					m_bufferFormat= GL_DEPTH_COMPONENT;
					m_pixelType= GL_FLOAT;
					break;
					// Add others as needed
				default:
					m_bufferFormat= GL_RGBA;
					m_pixelType= GL_UNSIGNED_BYTE;
					break;
				}

				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		else
		{
			m_glTextureId= 0;
			m_width= 0;
			m_height= 0;
			m_textureFormat= 0;
			m_bufferFormat= 0;
			m_pixelType= 0;
		}
	}

	virtual IMkTexture* setName(const std::string& name) override
	{
		m_name= name;

		return this;
	}

	virtual IMkTexture* setSize(uint16_t width, uint16_t height) override
	{
		// Do nothing since this class is for external textures that are created outside of this class

		return this;
	}

	virtual IMkTexture* setTextureMapData(const uint8_t* textureMapData) override
	{
		// Do nothing since this class is for external textures that are created outside of this class

		return this;
	}

	virtual IMkTexture* setTextureFormat(uint32_t textureFormat) override
	{
		// Do nothing since this class is for external textures that are created outside of this class

		return this;
	}

	virtual IMkTexture* setBufferFormat(uint32_t bufferFormat) override
	{
		// Do nothing since this class is for external textures that are created outside of this class

		return this;
	}

	virtual IMkTexture* setPixelType(uint32_t pixelType) override
	{
		// Do nothing since this class is for external textures that are created outside of this class

		return this;
	}

	virtual IMkTexture* setGenerateMipMap(bool bFlag) override
	{
		// Do nothing since this class is for external textures that are created outside of this class

		return this;
	}

	virtual IMkTexture* setPixelBufferObjectMode(PixelBufferObjectMode mode) override
	{
		// Do nothing since this class is for external textures that are created outside of this class

		return this;
	}

	virtual IMkTexture* setTextureWrapMode(TextureWrapMode mode) override
	{
		// Do nothing since this class is for external textures that are created outside of this class

		return this;
	}

	virtual void setImagePath(const std::filesystem::path& path) override
	{
		// Do nothing since this class is for external textures that are created outside of this class
	}

	virtual const std::filesystem::path getImagePath() const override
	{
		// Do nothing since this class is for external textures that are created outside of this class
		return {};
	}

	virtual bool reloadTextureFromImagePath() override
	{
		// Do nothing since this class is for external textures that are created outside of this class
		return false;
	}

	virtual bool createTexture() override
	{
		// Do nothing since this class is for external textures that are created outside of this class
		return false;
	}

	virtual void copyBufferIntoTexture(const uint8_t* buffer, size_t bufferSize) override
	{
		// Do nothing
		return;
	}

	virtual void copyTextureIntoBuffer(uint8_t* outBuffer, size_t bufferSize) override
	{
		// Do nothing
		return;
	}

	virtual void disposeTexture() override
	{
		// Forget about the external texture, but don't delete it since we don't own it
		m_glTextureId= 0;
	}

	virtual bool bindTexture(int textureUnit) const override
	{
		if (m_glTextureId != 0)
		{
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(GL_TEXTURE_2D, m_glTextureId);
			return true;
		}

		return false;
	}

	virtual void clearTexture(int textureUnit) const override
	{
		if (m_glTextureId != 0)
		{
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	virtual const std::string getName() const override { return m_name; }
	virtual bool getIsValid() const override { return m_glTextureId != 0; }
	virtual void* getPlatformTexture() override { return &m_glTextureId; }
	virtual uint32_t getGlTextureId() const override { return m_glTextureId; }
	virtual uint16_t getTextureWidth() const override { return m_width; }
	virtual uint16_t getTextureHeight() const override { return m_height; }
	virtual uint32_t getTextureFormat() const override { return m_textureFormat; }
	virtual uint32_t getBufferFormat() const override { return m_bufferFormat; }

protected:
	std::string m_name;
	uint32_t m_glTextureId= 0;
	uint16_t m_width= 0;
	uint16_t m_height= 0;
	uint32_t m_textureFormat= 0;
	uint32_t m_bufferFormat= 0;
	uint32_t m_pixelType= 0;
};

IMkTexturePtr CreateMkTexture() { return std::make_shared<GlTexture>(); }

IMkTexturePtr CreateMkTexture(uint16_t width, uint16_t height, const uint8_t* textureMapData, uint32_t textureFormat,
							  uint32_t bufferFormat)
{
	return std::make_shared<GlTexture>(width, height, textureMapData, textureFormat, bufferFormat);
}

IMkExternalTexturePtr CreateMkExternalTexture() { return std::make_shared<GlExternalTexture>(); }

IMkExternalTexturePtr CreateMkExternalTexture(void* platformTexture)
{
	return std::make_shared<GlExternalTexture>(platformTexture);
}

bool saveMkTextureToPNG(IMkTexturePtr texture, const char* filename)
{
	int channels= 0;
	bool bIsBGR= false;
	switch (texture->getBufferFormat())
	{
	case MK_RGB:
		channels= 3;
		break;
	case MK_BGR:
		channels= 3;
		bIsBGR= true;
		break;
	case MK_RGBA:
		channels= 4;
		break;
	case MK_BGRA:
		channels= 4;
		bIsBGR= true;
		break;
	default:
		break;
	}

	const int width= texture->getTextureWidth();
	const int height= texture->getTextureHeight();

	if (width == 0 || height == 0 || channels == 0)
	{
		return false;
	}

	const size_t bufferSize= (size_t)width * height * channels;
	uint8_t* buffer= new uint8_t[bufferSize];

	texture->copyTextureIntoBuffer(buffer, bufferSize);

	if (bIsBGR)
	{
		// Swap R and B channels so stbi_write_png outputs standard RGB(A)
		for (size_t i= 0; i < bufferSize; i+= channels)
		{
			std::swap(buffer[i], buffer[i + 2]);
		}
	}

	const int stride= width * channels;
	const bool bSuccess= stbi_write_png(filename, width, height, channels, buffer, stride) != 0;

	delete[] buffer;

	return bSuccess;
}