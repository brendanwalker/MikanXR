#pragma once

#include <stdint.h>
#include <string>

class InterprocessRenderTargetWriteAccessor
{
public:
	InterprocessRenderTargetWriteAccessor(const std::string& clientName);
	virtual ~InterprocessRenderTargetWriteAccessor();

	bool initialize(
		const struct MikanRenderTargetDescriptor* descriptor, 
		bool bEnableFrameCounter, 
		void* apiDeviceInterface= nullptr);
	void dispose();
	bool writeColorFrameTexture(void* ApiTexturePtr);
	bool writeDepthFrameTexture(void* ApiTexturePtr);
	bool getIsInitialized() const { return m_bIsInitialized; }
	bool getWantsDepth() const { return m_bWantsDepth; }

private:
	bool m_bIsInitialized= false;
	bool m_bWantsDepth= false;
	std::string m_clientName;
	struct RenderTargetWriterImpl* m_writerImpl;
};