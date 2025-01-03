#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include "MikanCoreTypes.h"

class GlTexture;
typedef std::shared_ptr<GlTexture> GlTexturePtr;

class InterprocessRenderTargetReadAccessor
{
public:
	InterprocessRenderTargetReadAccessor(const std::string& clientName);
	~InterprocessRenderTargetReadAccessor();

	bool initialize(const MikanRenderTargetDescriptor* descriptor);
	void dispose();
	void setColorTexture(GlTexturePtr texture) { m_colorTexture = texture; }
	void setDepthTexture(GlTexturePtr texture) { m_depthTexture = texture; }
	inline GlTexturePtr getColorTexture() { return m_colorTexture; }
	inline GlTexturePtr getDepthTexture() { return m_depthTexture; }
	inline int64_t getLastFrameRenderedIndex() const { return m_lastFrameRenderedIndex; }
	bool readRenderTargetTextures(const int64_t newFrameIndex);
	
	MikanClientGraphicsApi getClientGraphicsAPI() const { return m_descriptor.graphicsAPI; }
	const MikanRenderTargetDescriptor& getRenderTargetDescriptor() { return m_descriptor; }

private:
	std::string m_clientName;
	MikanRenderTargetDescriptor m_descriptor;
	int64_t m_lastFrameRenderedIndex;
	GlTexturePtr m_colorTexture;
	GlTexturePtr m_depthTexture;
	struct RenderTargetReaderImpl* m_readerImpl;
};