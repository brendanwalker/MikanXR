#pragma once

#include <stdint.h>
#include <string>
#include "MikanClientTypes.h"

class InterprocessRenderTargetWriteAccessor
{
public:
	InterprocessRenderTargetWriteAccessor(const std::string& clientName);
	virtual ~InterprocessRenderTargetWriteAccessor();

	bool initialize(const MikanRenderTargetDescriptor* descriptor, void* apiDeviceInterface= nullptr);
	void dispose();
	bool writeRenderTargetMemory(uint64_t frameIndex);
	bool writeRenderTargetTexture(void* ApiTexturePtr, uint64_t frameIndex);
	MikanRenderTargetMemory& getLocalMemory() { return m_localMemory; }
	uint64_t getLocalFrameIndex() const { return m_localFrameIndex; }

private:
	std::string m_clientName;
	MikanRenderTargetMemory m_localMemory;
	uint64_t m_localFrameIndex;
	struct RenderTargetWriterImpl* m_writerImpl;
};