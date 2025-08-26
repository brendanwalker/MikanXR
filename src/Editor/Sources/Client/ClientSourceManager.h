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

	static ClientSourceManager* getInstance() { return m_instance; }

	ClientSourceManager();
	virtual ~ClientSourceManager();

	bool startup(class IMkWindow* ownerWindow);
	void shutdown();

	inline const NamedValueTable<ClientSource*>& getClientSources() const { return m_clientSources; }
	IMkTexturePtr getClientColorSourceTexture(const std::string& clientId, eClientColorTextureType clientTextureType) const;
	IMkTexturePtr getClientDepthSourceTexture(const std::string& clientId, eClientDepthTextureType clientTextureType) const;
	bool getIsSourcePendingRender(const std::string& clientId) const;
	bool markSourceAsPendingRender(const std::string& clientId);

protected:
	bool addClientSource(const std::string& clientId, const MikanClientInfo& clientInfo, class SharedTextureReadAccessor* readAccessor);
	bool removeClientSource(const std::string& clientId, class SharedTextureReadAccessor* readAccessor);

	// MikanServer Events
	void onClientRenderTargetAllocated(const std::string& clientId, const MikanClientInfo& clientInfo, class SharedTextureReadAccessor* readAccessor);
	void onClientRenderTargetReleased(const std::string& clientId, class SharedTextureReadAccessor* readAccessor);
	void onClientRenderTargetUpdated(const std::string& clientId, int64_t frameIndex);

private:
	static ClientSourceManager* m_instance;

	class IMkWindow* m_ownerWindow= nullptr;

	// Data sources used by the compositor layers
	NamedValueTable<ClientSource*> m_clientSources;
};