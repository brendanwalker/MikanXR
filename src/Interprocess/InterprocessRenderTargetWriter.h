#pragma once

#include <stdint.h>
#include <string>
#include "MikanClientTypes.h"

class InterprocessRenderTargetWriteAccessor
{
public:
	InterprocessRenderTargetWriteAccessor(const std::string& clientName);
	virtual ~InterprocessRenderTargetWriteAccessor();

	bool initialize(
		const MikanRenderTargetDescriptor* descriptor, 
		bool bEnableFrameCounter, 
		void* apiDeviceInterface= nullptr);
	void dispose();
	bool writeRenderTargetMemory();
	bool writeRenderTargetTexture(void* ApiTexturePtr);
	MikanRenderTargetMemory& getLocalMemory() { return m_localMemory; }
	bool getIsInitialized() const { return m_bIsInitialized; }

private:
	bool m_bIsInitialized= false;
	std::string m_clientName;
	MikanRenderTargetMemory m_localMemory;
	struct RenderTargetWriterImpl* m_writerImpl;
};