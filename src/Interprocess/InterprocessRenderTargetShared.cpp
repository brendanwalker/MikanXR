#include "InterprocessRenderTargetShared.h"
#include "Logger.h"

InterprocessRenderTargetView::InterprocessRenderTargetView(const MikanRenderTargetDescriptor* descriptor)
	: mutex()
{
	header.width = descriptor->width;
	header.height = descriptor->height;
	header.colorBufferType = descriptor->color_buffer_type;
	header.depthBufferType = descriptor->depth_buffer_type;

	header.colorBufferSize = computeRenderTargetColorBufferSize(
		descriptor->color_buffer_type, descriptor->width, descriptor->height);
	if (header.colorBufferSize > 0)
	{
		std::memset(getBufferMutable(MikanRenderTarget_COLOR), 0, header.colorBufferSize);
	}

	header.depthBufferSize = computeRenderTargetDepthBufferSize(
		descriptor->depth_buffer_type, descriptor->width, descriptor->height);
	if (header.depthBufferSize > 0)
	{
		std::memset(getBufferMutable(MikanRenderTarget_DEPTH), 0, header.depthBufferSize);
	}
}

InterprocessRenderTargetView::~InterprocessRenderTargetView()
{
}

boost::interprocess::interprocess_mutex& InterprocessRenderTargetView::getMutex()
{
	return mutex;
}

InterprocessRenderTargetHeader& InterprocessRenderTargetView::getHeader()
{
	return header;
}

const uint8_t* InterprocessRenderTargetView::getBuffer(MikanBufferType bufferType) const
{
	size_t offset = 0;

	switch (bufferType)
	{
	case MikanRenderTarget_COLOR:
		offset = 0;
		break;
	case MikanRenderTarget_DEPTH:
		// depth buffer comes after the color buffer
		offset = computeRenderTargetColorBufferSize(header.colorBufferType, header.width, header.height);
		break;
	}

	return buffers + offset;
}

unsigned char* InterprocessRenderTargetView::getBufferMutable(MikanBufferType bufferType)
{
	return const_cast<unsigned char*>(getBuffer(bufferType));
}

size_t InterprocessRenderTargetView::getTotalSize() const
{
	return
		offsetof(InterprocessRenderTargetView, buffers)
		+ computeRenderTargetColorBufferSize(header.colorBufferType, header.width, header.height)
		+ computeRenderTargetDepthBufferSize(header.depthBufferType, header.width, header.height);
}

size_t InterprocessRenderTargetView::computeTotalSize(const MikanRenderTargetDescriptor* descriptor)
{
	return
		offsetof(InterprocessRenderTargetView, buffers)
		+ computeRenderTargetColorBufferSize(descriptor->color_buffer_type, descriptor->width, descriptor->height)
		+ computeRenderTargetDepthBufferSize(descriptor->depth_buffer_type, descriptor->width, descriptor->height);
}

size_t computeRenderTargetColorBufferSize(MikanColorBufferType colorBufferType, int width, int height)
{
	switch (colorBufferType)
	{
	case MikanColorBuffer_RGB24:
		return width * height * 3;
	case MikanColorBuffer_RGBA32:
		return width * height * 4;
	case MikanColorBuffer_NOCOLOR:
	default:
		return 0;
	}
}

size_t computeRenderTargetDepthBufferSize(MikanDepthBufferType depthBufferType, int width, int height)
{
	switch (depthBufferType)
	{
	case MikanDepthBuffer_DEPTH16:
		return width * height * 2;
	case MikanDepthBuffer_DEPTH32:
		return width * height * 4;
	case MikanDepthBuffer_NODEPTH:
	default:
		return 0;
	}
}

void initializeRenderTargetMemory(
	const MikanRenderTargetDescriptor* descriptor,
	MikanRenderTargetMemory* renderTargetMemory)
{
	// Allocate local memory used to copy render target data from shared memory
	renderTargetMemory->width = descriptor->width;
	renderTargetMemory->height = descriptor->height;
	renderTargetMemory->color_buffer_size =
		computeRenderTargetColorBufferSize(
			descriptor->color_buffer_type, descriptor->width, descriptor->height);
	if (renderTargetMemory->color_buffer_size > 0)
	{
		renderTargetMemory->color_buffer = new uint8_t[renderTargetMemory->color_buffer_size];
		memset(renderTargetMemory->color_buffer, 0, renderTargetMemory->color_buffer_size);
	}
	renderTargetMemory->depth_buffer_size =
		computeRenderTargetDepthBufferSize(
			descriptor->depth_buffer_type, descriptor->width, descriptor->height);
	if (renderTargetMemory->depth_buffer_size > 0)
	{
		renderTargetMemory->depth_buffer = new uint8_t[renderTargetMemory->depth_buffer_size];
		memset(renderTargetMemory->depth_buffer, 0, renderTargetMemory->depth_buffer_size);
	}
}

void disposeRenderTargetMemory(MikanRenderTargetMemory* renderTargetMemory)
{
	// Clean up the local memory
	if (renderTargetMemory->color_buffer != nullptr)
	{
		delete[] renderTargetMemory->color_buffer;
	}
	if (renderTargetMemory->depth_buffer != nullptr)
	{
		delete[] renderTargetMemory->depth_buffer;
	}
	memset(renderTargetMemory, 0, sizeof(MikanRenderTargetMemory));
}