#pragma once

#include <stdint.h>
#include "MikanClientTypes.h"

struct InterprocessRenderTargetHeader
{
	uint16_t width;
	uint16_t height;
	size_t colorBufferSize;
	size_t depthBufferSize;
	MikanColorBufferType colorBufferType;
	MikanDepthBufferType depthBufferType;
};

class InterprocessRenderTargetView
{
public:
	InterprocessRenderTargetView(const MikanRenderTargetDescriptor* descriptor);
	virtual ~InterprocessRenderTargetView();

	InterprocessRenderTargetHeader& getHeader();
	const uint8_t* getBuffer(MikanBufferType bufferType) const;
	unsigned char* getBufferMutable(MikanBufferType bufferType);

	size_t getTotalSize() const;
	static size_t computeTotalSize(const MikanRenderTargetDescriptor* descriptor);

private:
	InterprocessRenderTargetHeader header;

	uint8_t buffers[1];

	InterprocessRenderTargetView() {}
	InterprocessRenderTargetView(InterprocessRenderTargetView const&);            // undefined
	InterprocessRenderTargetView& operator=(InterprocessRenderTargetView const&); // undefined
};

size_t computeRenderTargetColorBufferSize(MikanColorBufferType colorBufferType, int width, int height);
size_t computeRenderTargetDepthBufferSize(MikanDepthBufferType depthBufferType, int width, int height);
void initializeRenderTargetMemory(const MikanRenderTargetDescriptor* descriptor, MikanRenderTargetMemory* renderTargetMemory);
void disposeRenderTargetMemory(MikanRenderTargetMemory* renderTargetMemory);
