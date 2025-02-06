#pragma once

#include "MkRendererExport.h"
#include "MkRendererFwd.h"

#include <filesystem>
#include <stdint.h>

class IMkTexture
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

	virtual ~IMkTexture() {}

	virtual IMkTexture* setName(const std::string& name) = 0;
	virtual IMkTexture* setSize(uint16_t width, uint16_t height) = 0;
	virtual IMkTexture* setTextureMapData(const uint8_t* textureMapData) = 0;
	virtual IMkTexture* setTextureFormat(uint32_t textureFormat) = 0;
	virtual IMkTexture* setBufferFormat(uint32_t bufferFormat) = 0;
	virtual IMkTexture* setPixelType(uint32_t pixelType) = 0;
	virtual IMkTexture* setGenerateMipMap(bool bFlag) = 0;
	virtual IMkTexture* setPixelBufferObjectMode(PixelBufferObjectMode mode) = 0;

	virtual void setImagePath(const std::filesystem::path& path) = 0;
	virtual const std::filesystem::path getImagePath() const = 0;
	virtual bool reloadTextureFromImagePath() = 0;

	virtual bool createTexture() = 0;
	virtual void copyBufferIntoTexture(const uint8_t* buffer, size_t bufferSize) = 0;
	virtual void copyTextureIntoBuffer(uint8_t* outBuffer, size_t bufferSize) = 0;
	virtual void disposeTexture() = 0;

	virtual bool bindTexture(int textureUnit = 0) const = 0;
	virtual void clearTexture(int textureUnit = 0) const = 0;

	virtual const std::string getName() const = 0;
	virtual uint32_t getGlTextureId() const = 0;
	virtual uint16_t getTextureWidth() const = 0;
	virtual uint16_t getTextureHeight() const = 0;
	virtual uint32_t getTextureFormat() const = 0;
	virtual uint32_t getBufferFormat() const = 0;
};

MIKAN_RENDERER_FUNC(IMkTexturePtr) CreateMkTexture();
MIKAN_RENDERER_FUNC(IMkTexturePtr) CreateMkTexture(
	uint16_t width, 
	uint16_t height, 
	const uint8_t* textureMapData,
	uint32_t textureFormat,
	uint32_t bufferFormat);