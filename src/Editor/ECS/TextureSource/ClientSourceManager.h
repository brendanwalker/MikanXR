#pragma once

#include "CompositorConstants.h"
#include "MikanAPITypes.h"
#include "MikanClientTypes.h"
#include "MulticastDelegate.h"
#include "MikanRendererFwd.h"
#include "NamedValueTable.h"

#include <string>

class ClientSourceManager
{
public:
	struct ClientSource
	{
		std::string clientId;
		MikanClientInfo clientInfo;
		MikanRenderTargetDescriptor desc;
		IMkTexturePtr colorTexture;
		IMkTexturePtr depthTexture;
		int64_t frameIndex = 0;
		bool bIsPendingRender = false;
	};

	ClientSourceManager() = default;
	virtual ~ClientSourceManager() = default;

	bool startup(class IMkWindow* ownerWindow);
	void shutdown();

	inline const NamedValueTable<ClientSource*>& getClientSources() const { return m_clientSources; }
	bool hasClientSource(const std::string& clientId, MikanCameraID cameraId) const;
	bool getClientSourceDimensions(const std::string& clientId, MikanCameraID cameraId, int& outWidth, int& outHeight) const;
	IMkTexturePtr getClientColorSourceTexture(const std::string& clientId, MikanCameraID cameraId, eTextureSourceColorType textureSourceColorType) const;
	IMkTexturePtr getClientDepthSourceTexture(const std::string& clientId, MikanCameraID cameraId, eTextureSourceDepthType textureSourceColorType) const;
	bool getIsSourcePendingRender(const std::string& clientId, MikanCameraID cameraId) const;
	bool markSourceAsPendingRender(const std::string& clientId, MikanCameraID cameraId);

	MulticastDelegate<void(const std::string& clientId) > OnClientSourceConnected;
	MulticastDelegate<void(const std::string& clientId) > OnClientSourceUpdated;
	MulticastDelegate<void(const std::string& clientId)> OnClientSourceDisconnected;

protected:
	static std::string makeClientSourceTableKey(const std::string& clientId, MikanCameraID cameraId);
	ClientSource* getClientSource(const std::string& clientId, MikanCameraID cameraId) const;
	bool addClientSource(const std::string& clientId, const MikanClientInfo& clientInfo, class SharedTextureReadAccessor* readAccessor);
	bool removeClientSource(const std::string& clientId, class SharedTextureReadAccessor* readAccessor);

	// MikanServer Events
	void onClientRenderTargetAllocated(const std::string& clientId, const MikanClientInfo& clientInfo, class SharedTextureReadAccessor* readAccessor);
	void onClientRenderTargetReleased(const std::string& clientId, class SharedTextureReadAccessor* readAccessor);
	void onClientRenderTargetUpdated(const std::string& clientId, MikanCameraID cameraId, int64_t frameIndex);

private:
	class IMkWindow* m_ownerWindow= nullptr;

	// Data sources used by the compositor layers
	NamedValueTable<ClientSource*> m_clientSources;
};