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

	virtual void setTextResponseHandler(TextResponseHandler handler) override;
	virtual void setBinaryResponseHandler(BinaryResponseHandler handler) override;

	MikanResult connect(
		const std::string& host, 
		const std::string& port) override;
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