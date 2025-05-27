#pragma once

#include "MikanCoreTypes.h"
#include "MulticastDelegate.h"
#include "IServerRequestHandler.h"

class RenderTargetClientState
{
public:
	RenderTargetClientState() = default;
	RenderTargetClientState(class MikanClientConnectionState* owner);
	virtual ~RenderTargetClientState();

	MikanClientGraphicsApi getClientGraphicsAPI() const;

	void allocateRenderTargetAccessor();
	void disposeRenderTargetAccessor();

	inline class SharedTextureReadAccessor* getRenderTargetReadAccessor() const
	{ return m_renderTargetReadAccessor; }
	bool hasAllocatedRenderTarget() const;
	bool allocateRenderTargetTextures(const MikanRenderTargetDescriptor& desc);
	void freeRenderTargetTexturesHandler();
	bool readRenderTargetTextures(const int64_t newFrameIndex);

private:
	class MikanClientConnectionState* m_owner= nullptr;
	class SharedTextureReadAccessor* m_renderTargetReadAccessor = nullptr;
};

class RenderTargetRequestHandler : public IServerRequestHandler
{
public:
	RenderTargetRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override {}

	MulticastDelegate<void(const std::string& clientId, const struct MikanClientInfo& clientInfo, class SharedTextureReadAccessor* readAccessor) > OnClientRenderTargetAllocated;
	MulticastDelegate<void(const std::string& clientId, class SharedTextureReadAccessor* readAccessor)> OnClientRenderTargetReleased;
	MulticastDelegate<void(const std::string& clientId, int64_t frameIndex)> OnClientRenderTargetUpdated;

protected:
	void allocateRenderTargetTexturesHandler(const ClientRequest& request, ClientResponse& response);
	void freeRenderTargetTexturesHandler(const ClientRequest& request, ClientResponse& response);
	void frameRenderedHandler(const ClientRequest& request, ClientResponse& response);
};