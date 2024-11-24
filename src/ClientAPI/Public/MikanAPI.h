#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"

#include <memory>
#include <string>

using IMikanAPIPtr = std::shared_ptr<class IMikanAPI>;

// -- Mikan API -----
class MIKAN_API IMikanAPI
{
public:
	IMikanAPI() = default;
	virtual ~IMikanAPI() = default;

	// API Lifecycle
	static IMikanAPIPtr createMikanAPI();
	virtual MikanResult init(MikanLogLevel min_log_level, MikanLogCallback log_callback) = 0;
	virtual bool getIsInitialized() = 0;
	virtual MikanResult shutdown() = 0;

	// Client Info
	virtual int getCoreSDKVersion() const = 0;
	virtual std::string getClientUniqueID() const = 0;
	virtual MikanResult setClientInfo(const MikanClientInfo& clientInfo) = 0;
	virtual MikanResult setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface) = 0;
	virtual MikanResult getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface) = 0;

	// Connection Management
	// NOTE: Call setClientInfo() before calling connect
	virtual MikanResult connect(const std::string& host = "", const std::string& port = "") = 0;
	virtual bool getIsConnected() = 0;
	virtual MikanResult disconnect() = 0;

	// Messaging
	virtual MikanResponseFuture sendRequest(const MikanRequest& request) = 0;
	virtual MikanResult fetchNextEvent(MikanEventPtr& out_event) = 0;
};
