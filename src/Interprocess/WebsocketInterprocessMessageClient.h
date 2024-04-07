#pragma once

#include "InterprocessMessages.h"

#include <memory>

class WebsocketConnectionState;
using WebsocketConnectionStatePtr = std::shared_ptr<WebsocketConnectionState>;

class WebsocketInterprocessMessageClient : public IInterprocessMessageClient
{
public:
	WebsocketInterprocessMessageClient();
	virtual ~WebsocketInterprocessMessageClient();

	virtual MikanResult initialize() override;
	virtual void dispose() override;

	virtual MikanResult setClientProperty(const std::string& key, const std::string& value) override;
	virtual void setResponseHandler(ResponseHandler handler) override;

	MikanResult connect(const std::string& host, const std::string& port) override;
	void disconnect() override;

	virtual MikanResult fetchNextEvent(
		size_t utf8BufferSize,
		char* outUtf8Buffer,
		size_t* outUtf8BufferSizeNeeded) override;
	virtual MikanResult sendRequest(const std::string& utf8RequestString) override;

	const bool getIsConnected() const override;

private:
	WebsocketConnectionStatePtr m_connectionState;
};