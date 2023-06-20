#pragma once

#include <stdint.h>
#include "MikanClientTypes.h"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#ifdef WIN32
#define BOOST_INTERPROCESS_SHARED_DIR_PATH "shared_mem"
#endif // WIN32

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

	boost::interprocess::interprocess_mutex& getMutex();
	InterprocessRenderTargetHeader& getHeader();
	const uint8_t* getBuffer(MikanBufferType bufferType) const;
	unsigned char* getBufferMutable(MikanBufferType bufferType);

	size_t getTotalSize() const;
	static size_t computeTotalSize(const MikanRenderTargetDescriptor* descriptor);

private:
	//Mutex to protect access to the shared memory
	boost::interprocess::interprocess_mutex mutex;

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
