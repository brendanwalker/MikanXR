#pragma once

#include <stdint.h>
#include <memory>
#include <string>

#include "MikanCoreTypes.h"

class IMkTexture;
typedef std::shared_ptr<IMkTexture> IMkTexturePtr;

class SharedTextureReadAccessor
{
public:
	SharedTextureReadAccessor(const std::string& clientName);
	~SharedTextureReadAccessor();

	bool initialize(const MikanRenderTargetDescriptor* descriptor);
	void dispose();
	void setColorTexture(IMkTexturePtr texture) { m_colorTexture = texture; }
	void setDepthTexture(IMkTexturePtr texture) { m_depthTexture = texture; }
	inline IMkTexturePtr getColorTexture() { return m_colorTexture; }
	inline IMkTexturePtr getDepthTexture() { return m_depthTexture; }
	inline int64_t getLastFrameRenderedIndex() const { return m_lastFrameRenderedIndex; }
	bool readRenderTargetTextures(const int64_t newFrameIndex);
	
	MikanClientGraphicsApi getClientGraphicsAPI() const { return m_descriptor.graphicsAPI; }
	const MikanRenderTargetDescriptor& getRenderTargetDescriptor() { return m_descriptor; }

private:
	std::string m_clientName;
	MikanRenderTargetDescriptor m_descriptor;
	int64_t m_lastFrameRenderedIndex;
	IMkTexturePtr m_colorTexture;
	IMkTexturePtr m_depthTexture;
	struct RenderTargetReaderImpl* m_readerImpl;
};