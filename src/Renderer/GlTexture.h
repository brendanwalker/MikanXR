#pragma once

#include <filesystem>
#include <stdint.h>

class GlTexture
{
public:
	enum class PixelBufferObjectMode
	{
		NoPBO,
		SinglePBOWrite,
		DoublePBOWrite,
		SinglePBORead,
		DoublePBORead,
	};

	GlTexture();
	GlTexture(
		uint16_t width, 
		uint16_t height, 
		const uint8_t* textureMapData,
		uint32_t textureFormat,
		uint32_t bufferFormat);
	virtual ~GlTexture();

	GlTexture* setName(const std::string& name)
	{ m_name= name; return this; }
	GlTexture* setSize(uint16_t width, uint16_t height)
	{ m_width= width; m_height= height; return this; }
	GlTexture* setTextureMapData(const uint8_t* textureMapData)
	{ m_textureMapData= textureMapData; return this; }
	GlTexture* setTextureFormat(uint32_t textureFormat)
	{ m_textureFormat= textureFormat; m_bufferFormat= textureFormat; return this; }
	GlTexture* setBufferFormat(uint32_t bufferFormat)
	{ m_bufferFormat = bufferFormat; return this; }
	GlTexture* setPixelType(uint32_t pixelType)
	{ m_pixelType = pixelType; return this; }
	GlTexture* setGenerateMipMap(bool bFlag)
	{ m_bGenerateMipMap= bFlag; return this; }
	GlTexture* setPixelBufferObjectMode(PixelBufferObjectMode mode)
	{ m_pboMode = mode; return this; }

	void setImagePath(const std::filesystem::path& path) { m_imagePath = path; }
	const std::filesystem::path getImagePath() const { return m_imagePath; }
	bool reloadTextureFromImagePath();

	bool createTexture();
	void copyBufferIntoTexture(const uint8_t* buffer, size_t bufferSize);
	void copyTextureIntoBuffer(uint8_t* outBuffer, size_t bufferSize);
	void disposeTexture();

	bool bindTexture(int textureUnit = 0) const;
	void renderFullscreen() const;
	void clearTexture(int textureUnit = 0) const;

	const std::string getName() const { return m_name; }
	uint32_t getGlTextureId() const { return m_glTextureId; }
	uint16_t getTextureWidth() const { return m_width; }
	uint16_t getTextureHeight() const { return m_height; }
	uint32_t getTextureFormat() const { return m_textureFormat; }
	uint32_t getBufferFormat() const { return m_textureFormat; }

private:
	static size_t getBytesPerPixel(uint32_t format, uint32_t type);

	std::string m_name;
	uint32_t m_glTextureId = 0;
	uint16_t m_width = 0;
	uint16_t m_height = 0;
	uint32_t m_textureFormat = 0;
	uint32_t m_bufferFormat = 0;
	uint32_t m_pixelType = 0;
	uint32_t m_glPixelBufferObjectIDs[2] = {0, 0};
	uint16_t m_pboWriteIndex = 0;
	PixelBufferObjectMode m_pboMode= PixelBufferObjectMode::NoPBO;	
	size_t m_PBOByteSize = 0;
	bool m_bGenerateMipMap= true;
	const uint8_t* m_textureMapData;
	std::filesystem::path m_imagePath;
};

