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
	bool writeDepthFrameTexture(void* ApiTexturePtr, float zNear, float zFar);
	void* getPackDepthTextureResourcePtr() const;
	bool getIsInitialized() const { return m_bIsInitialized; }
	const struct MikanRenderTargetDescriptor* getRenderTargetDescriptor() const;

private:
	bool m_bIsInitialized= false;
	std::string m_clientName;
	struct RenderTargetWriterImpl* m_writerImpl;
};