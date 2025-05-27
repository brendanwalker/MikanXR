#pragma once

#include "MikanCoreTypes.h"
#include "MikanClientTypes.h"
#include "MikanTypeFwd.h"

#include <string>

class MikanClientConnectionState
{
public:
	MikanClientConnectionState(
		const std::string& connectionId,
		class IInterprocessMessageServer* messageServer);
	virtual ~MikanClientConnectionState();

	inline const std::string& getConnectionId() const { return m_connectionId; }
	inline const struct MikanClientInfo& getMikanClientInfo() const { return m_clientInfo; }
	const std::string& getClientId() const;
	bool isClientInfoValid() const { return !getClientId().empty(); }
	inline class RenderTargetClientState* getRenderTargetClientState() const { return m_renderTargetClientState; }
	inline class VRDeviceClientState* getVRDeviceClientState() const { return m_vrDeviceClientState; }

	void setMikanClientInfo(const struct MikanClientInfo& clientInfo);
	void clearMikanClientInfo();

	// Event Publishing
	void publishMikanJsonEvent(const std::string& mikanJsonEvent);

private:
	std::string m_connectionId;
	MikanClientInfo m_clientInfo;
	class IInterprocessMessageServer* m_messageServer = nullptr;
	class RenderTargetClientState* m_renderTargetClientState = nullptr;
	class VRDeviceClientState* m_vrDeviceClientState = nullptr;
};
