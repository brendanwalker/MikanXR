#pragma once

#include <stdint.h>
#include <string>
#include "MikanClientTypes.h"

class InterprocessRenderTargetReadAccessor
{
public:
	InterprocessRenderTargetReadAccessor(const std::string& clientName);
	~InterprocessRenderTargetReadAccessor();

	bool initialize(const MikanRenderTargetDescriptor* descriptor);
	void dispose();
	void setColorTexture(class GlTexture* texture) { m_colorTexture = texture; }
	void setDepthTexture(class GlTexture* texture) { m_depthTexture = texture; }
	class GlTexture* getColorTexture() { return m_colorTexture; }
	class GlTexture* getDepthTexture() { return m_depthTexture; }
	bool readRenderTargetMemory();
	
	MikanClientGraphicsAPI getClientGraphicsAPI() const { return m_descriptor.graphicsAPI; }
	MikanRenderTargetDescriptor& getRenderTargetDescriptor() { return m_descriptor; }
	const MikanRenderTargetMemory& getLocalMemory() { return m_localMemory; }
	uint64_t getLocalFrameIndex() const { return m_localFrameIndex; }

private:
	std::string m_clientName;
	MikanRenderTargetDescriptor m_descriptor;
	MikanRenderTargetMemory m_localMemory;
	class GlTexture* m_colorTexture;
	class GlTexture* m_depthTexture;
	uint64_t m_localFrameIndex;
	struct RenderTargetReaderImpl* m_readerImpl;
};