#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include "MikanClientTypes.h"

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
	GlTexturePtr getColorTexture() { return m_colorTexture; }
	GlTexturePtr getDepthTexture() { return m_depthTexture; }
	bool readRenderTargetMemory();
	
	MikanClientGraphicsApi getClientGraphicsAPI() const { return m_descriptor.graphicsAPI; }
	MikanRenderTargetDescriptor& getRenderTargetDescriptor() { return m_descriptor; }

private:
	std::string m_clientName;
	MikanRenderTargetDescriptor m_descriptor;
	GlTexturePtr m_colorTexture;
	GlTexturePtr m_depthTexture;
	struct RenderTargetReaderImpl* m_readerImpl;
};