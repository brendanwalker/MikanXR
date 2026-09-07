#pragma once

#include "SharedTextureWriter.h"

#include <memory>
#include <string>

// -- Writer backends -----
// One graphics API's shared texture writer lives in its own file behind this interface, so the
// accessor that owns it neither names an API type nor branches on one, and a new API is a file
// rather than another arm of five if-else chains. A texture pointer stays opaque here and the
// backend casts it to its own API's type, which is where that knowledge belongs.

// What a backend reads from the accessor that owns it: the render target it was created for, the
// client's device, and the Spout sender names. The accessor owns this and outlives its backend,
// so a backend holds it by reference and re-reads it per frame rather than copying at init.
struct SharedTextureWriterContext
{
	const SharedTextureDescriptor* descriptor= nullptr;
	void* apiDeviceInterface= nullptr;
	// Optional native command queue (e.g. ID3D12CommandQueue*) the client renders on. When provided
	// for D3D12, the D3D11On12 device is created against this queue so the shared-texture copy is
	// serialized on the GPU after the client's rendering, instead of racing it on a separate queue.
	void* apiCommandQueueInterface= nullptr;
	std::string colorSenderName;
	std::string depthSenderName;
	std::string shadowSenderName;
	bool bEnableFrameCounter= false;
	SharedTextureLogger* logger= nullptr;
};

class ISharedTextureWriterBackend
{
public:
	virtual ~ISharedTextureWriterBackend()= default;

	// Opens the Spout senders and the API resources behind them. A backend that fails here is
	// still destroyed normally, so init cleans up whatever it managed to create.
	virtual bool init()= 0;

	virtual bool writeColorFrameTexture(void* apiTexturePtr)= 0;
	virtual bool writeDepthFrameTexture(void* apiTexturePtr, float zNear, float zFar)= 0;
	virtual bool writeShadowFrameTexture(void* apiTexturePtr)= 0;
	virtual void* getPackDepthTextureResourcePtr() const= 0;
};
using ISharedTextureWriterBackendPtr= std::unique_ptr<ISharedTextureWriterBackend>;

// One per API, each defined beside the backend it builds. The context must outlive the backend.
ISharedTextureWriterBackendPtr createOpenGLTextureWriter(const SharedTextureWriterContext& context);
ISharedTextureWriterBackendPtr createDX11TextureWriter(const SharedTextureWriterContext& context);
ISharedTextureWriterBackendPtr createDX12TextureWriter(const SharedTextureWriterContext& context);
ISharedTextureWriterBackendPtr createVulkanTextureWriter(const SharedTextureWriterContext& context);

// -- Spout log policy -----
// Spout writes its diagnostics to a console it allocates itself, which pops a second
// window over the host process. Nothing is enabled unless MIKAN_SPOUT_LOG asks for it:
//   unset, "0", "off"        no Spout logging (default)
//   "console", "1", "on"     Spout's own console window
//   "file"                   MikanSpoutSender.log in the Spout log folder (%APPDATA%\Spout)
// The editor drives its own Spout logging through SpoutLogRelay, which routes the same
// output into the log panel instead, and stands down whenever this variable is set.
enum class SpoutLogTarget
{
	none,
	console,
	file,
};

extern const char* const k_spoutSenderLogFileName;

SpoutLogTarget getSpoutLogTarget();

// The Direct3D and Vulkan writers link the in-tree SpoutDX sources, which carry their own copy
// of the Spout log globals separate from the ones inside SpoutLibrary. Applied once per process.
void applySpoutDXLogPolicy();
