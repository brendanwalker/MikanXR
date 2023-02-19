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
	const MikanRenderTargetMemory& getLocalMemory() { return m_localMemory; }
	uint64_t getLocalFrameIndex() const { return m_localFrameIndex; }

private:
	std::string m_clientName;
	MikanRenderTargetDescriptor m_descriptor;
	MikanRenderTargetMemory m_localMemory;
	GlTexturePtr m_colorTexture;
	GlTexturePtr m_depthTexture;
	uint64_t m_localFrameIndex;
	struct RenderTargetReaderImpl* m_readerImpl;
};