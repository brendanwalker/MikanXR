#pragma once

#include "SharedTextureFwd.h"
#include "SharedTextureExport.h"
#include "SharedTextureLogger.h"

#include <stdint.h>
#include <string>
#include <memory>
#include <functional>

enum class SharedClientGraphicsApi : int
{
	UNKNOWN = -1,

	Direct3D9,
	Direct3D11,
	Direct3D12,
	OpenGL,
	Metal,
	Vulkan,
};

enum class SharedColorBufferType : int
{
	NOCOLOR,
	RGB24,
	// DXGI_FORMAT_R8G8B8A8_UNORM / DXGI_FORMAT_R8G8B8A8_TYPELESS
	RGBA32,
	// DXGI_FORMAT_B8G8R8A8_UNORM / DXGI_FORMAT_B8G8R8A8_TYPELESS
	BGRA32,
};

enum class SharedDepthBufferType : int
{
	NODEPTH,
	// Raw float non-linear depth values from the z-buffer (in source world units)
	FLOAT_DEVICE_DEPTH,
	// Linearized float distance-from-camera values (in source world units)
	FLOAT_SCENE_DEPTH,
	// DXGI_FORMAT_R8G8B8A8_UNORM / DXGI_FORMAT_R8G8B8A8_TYPELESS
	PACK_DEPTH_RGBA,
};

struct SharedTextureDescriptor
{
	SharedColorBufferType color_buffer_type = SharedColorBufferType::NOCOLOR;
	SharedDepthBufferType depth_buffer_type = SharedDepthBufferType::NODEPTH;
	uint32_t width = 0;
	uint32_t height = 0;
	SharedClientGraphicsApi graphicsAPI = SharedClientGraphicsApi::UNKNOWN;
};

class ISharedTextureWriteAccessor
{
public:
	virtual ~ISharedTextureWriteAccessor() {}

	virtual bool initialize(
		const struct SharedTextureDescriptor* descriptor, 
		bool bEnableFrameCounter, 
		void* apiDeviceInterface= nullptr) = 0;
	virtual void dispose() = 0;

	virtual bool writeColorFrameTexture(void* ApiTexturePtr) = 0;
	virtual bool writeDepthFrameTexture(void* ApiTexturePtr, float zNear, float zFar) = 0;
	virtual void* getPackDepthTextureResourcePtr() const = 0;
	virtual bool getIsInitialized() const = 0;
	virtual const SharedTextureDescriptor* getRenderTargetDescriptor() const = 0;
	virtual void setLogCallback(SharedTextureLogCallback callback) = 0;
};

MIKAN_SHAREDTEXTURE_FUNC(ISharedTextureWriteAccessorPtr) createSharedTextureWriteAccessor(const std::string& clientName);