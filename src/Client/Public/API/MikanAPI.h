#pragma once

#include "MikanAPITypes.h"

#include <memory>
#include <string>

using IMikanAPIPtr = std::shared_ptr<class IMikanAPI>;

// -- Render Target API -----
class MIKAN_PUBLIC_CLASS IMikanRenderTargetAPI
{
public:
	IMikanRenderTargetAPI() {}
	virtual ~IMikanRenderTargetAPI() {}

	virtual MikanResult setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface) = 0;
	virtual MikanResult getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface) = 0;
	virtual MikanResponseFuture allocateRenderTargetTextures(const MikanRenderTargetDescriptor& descriptor) = 0;
	virtual MikanResult writeColorRenderTargetTexture(void* apiColorTexturePtr) = 0;
	virtual MikanResult writeDepthRenderTargetTexture(void* apiDepthTexturePtr, float zNear, float zFar) = 0;
	virtual MikanResult publishRenderTargetTextures(MikanClientFrameRendered& frameInfo) = 0;
	virtual MikanResponseFuture freeRenderTargetTextures() = 0;
};

// -- Script API -----
class MIKAN_PUBLIC_CLASS IMikanScriptAPI
{
public:
	IMikanScriptAPI() {}
	virtual ~IMikanScriptAPI() {}

	virtual MikanResponseFuture sendScriptMessage(const std::string& mesg) = 0; // returns MikanResult
};

// -- Spatial Anchor API -----
class MIKAN_PUBLIC_CLASS IMikanSpatialAnchorAPI
{
public:
	IMikanSpatialAnchorAPI() {}
	virtual ~IMikanSpatialAnchorAPI() {}

	virtual MikanResponseFuture getSpatialAnchorList() = 0; // returns MikanSpatialAnchorList
	virtual MikanResponseFuture getSpatialAnchorInfo(MikanSpatialAnchorID anchorId) = 0; // returns MikanSpatialAnchorInfo
	virtual MikanResponseFuture findSpatialAnchorInfoByName(const std::string& anchorName) = 0; // returns MikanSpatialAnchorInfo
};

// -- Stencil API -----
class MIKAN_PUBLIC_CLASS IMikanStencilAPI
{
public:
	IMikanStencilAPI() {}
	virtual ~IMikanStencilAPI() {}

	virtual MikanResponseFuture getQuadStencilList() = 0; // returns MikanStencilList
	virtual MikanResponseFuture getQuadStencil(MikanStencilID stencilId) = 0; // returns MikanStencilQuad
	virtual MikanResponseFuture getBoxStencilList() = 0; // returns MikanStencilList
	virtual MikanResponseFuture getBoxStencil(MikanStencilID stencilId) = 0; // returns MikanStencilBox
	virtual MikanResponseFuture getModelStencilList() = 0; // returns MikanStencilList
	virtual MikanResponseFuture getModelStencil(MikanStencilID stencilId) = 0; // returns MikanStencilModel
	virtual MikanResponseFuture getModelStencilRenderGeometry(MikanStencilID stencilId) = 0; // returns MikanStencilModel
};

// -- Video Source API -----
class MIKAN_PUBLIC_CLASS IMikanVideoSourceAPI
{
public:
	IMikanVideoSourceAPI() {}
	virtual ~IMikanVideoSourceAPI() {}

	virtual MikanResponseFuture getVideoSourceIntrinsics() = 0; // returns MikanVideoSourceIntrinsics
	virtual MikanResponseFuture getVideoSourceMode() = 0; // returns MikanVideoSourceMode
	virtual MikanResponseFuture getVideoSourceAttachment() = 0; // returns MikanVideoSourceAttachmentInfo
};

// -- VRDevice API -----
class MIKAN_PUBLIC_CLASS IMikanVRDeviceAPI
{
public:
	IMikanVRDeviceAPI() {}
	virtual ~IMikanVRDeviceAPI() {}

	virtual MikanResponseFuture getVRDeviceList() = 0; // returns MikanVRDeviceList
	virtual MikanResponseFuture getVRDeviceInfo(MikanVRDeviceID deviceId) = 0; // returns MikanVRDeviceInfo
	virtual MikanResponseFuture subscribeToVRDevicePoseUpdates(MikanVRDeviceID deviceId) = 0; // returns MikanResponse
	virtual MikanResponseFuture unsubscribeFromVRDevicePoseUpdates(MikanVRDeviceID deviceId) = 0; // returns MikanResponse
};

// -- Mikan API -----
class MIKAN_PUBLIC_CLASS IMikanAPI
{
public:
	IMikanAPI() = default;
	virtual ~IMikanAPI() = default;

	// Initialize the Mikan API
	static IMikanAPIPtr createMikanAPI();
	virtual MikanResult init(MikanLogLevel min_log_level, MikanLogCallback log_callback) = 0;
	virtual bool getIsInitialized() = 0;

	// Sub API accessors
	virtual int getCoreSDKVersion() const = 0;
	virtual std::string getClientUniqueID() const = 0;
	virtual IMikanRenderTargetAPI* getRenderTargetAPI() const = 0;
	virtual IMikanVideoSourceAPI* getVideoSourceAPI() const = 0;
	virtual IMikanVRDeviceAPI* getVRDeviceAPI() const = 0;
	virtual IMikanScriptAPI* getScriptAPI() const = 0;
	virtual IMikanStencilAPI* getStencilAPI() const = 0;
	virtual IMikanSpatialAnchorAPI* getSpatialAnchorAPI() const = 0;

	// Set client properties before calling connect
	virtual MikanResult setClientInfo(const MikanClientInfo& clientInfo) = 0;
	virtual MikanResult connect(const std::string& host="", const std::string& port="") = 0;
	virtual bool getIsConnected() = 0;
	virtual MikanResult fetchNextEvent(MikanEventPtr& out_event) = 0;
	virtual MikanResult disconnect() = 0;
	virtual MikanResult shutdown() = 0;
};