#include "WebsocketInterprocessMessageClient.h"
#include "JsonUtils.h"
#include "Logger.h"
#include "StringUtils.h"

#include "readerwriterqueue.h"

#include <ixwebsocket/IXWebSocket.h>


using LockFreeEventQueue = moodycamel::ReaderWriterQueue<std::string>;
using LockFreeEventQueuePtr = std::shared_ptr<LockFreeEventQueue>;

using WebSocketWeakPtr = std::weak_ptr<ix::WebSocket>;
using WebSocketPtr = std::shared_ptr<ix::WebSocket>;

//-- WebsocketConnectionState -----
class WebsocketConnectionState
{
public:
	WebsocketConnectionState()
		: m_websocket(std::make_shared<ix::WebSocket>())
		, m_eventQueue(std::make_shared<LockFreeEventQueue>())
	{		
		m_websocket->setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
			handleWebSocketMessage(msg);
		});
	}

	~WebsocketConnectionState()
	{
		disconnect();
	}

	//const std::string& getClientId() const { return m_clientId; }
	const bool getIsConnected() const { 
		auto readyState= m_websocket->getReadyState();
		return readyState == ix::ReadyState::Open || readyState == ix::ReadyState::Connecting; 
	}
	
	inline WebSocketPtr getWebSocket() { return m_websocket; }

	inline LockFreeEventQueuePtr getServerEventQueue() { return m_eventQueue; }
	inline void setResponseHandler(IInterprocessMessageClient::ResponseHandler handler) { 
		m_responseHandler= handler;  
	}

	void setClientProperty(const std::string& key, const std::string& value)
	{
		m_headers[key]= value;
	}

	MikanResult connect(const std::string& host, const std::string& port)
	{
		if (getIsConnected())
		{
			MIKAN_MT_LOG_ERROR("WebsocketConnectionState::connect()") << "Already connected";
			return MikanResult_AlreadyConnected;
		}

		std::string hostAddress= host.empty() ? WEBSOCKET_SERVER_ADDRESS : host;
		std::string hostPort= port.empty() ? WEBSOCKET_SERVER_PORT : port;
		m_websocket->setUrl(hostAddress+":"+hostPort);

		m_websocket->setExtraHeaders(m_headers);
		m_websocket->start();

		return MikanResult_Success;
	}

	bool disconnect()
	{
		if (getIsConnected())
		{
			m_websocket->close();
			return true;
		}

		return false;
	}

	void handleWebSocketMessage(const ix::WebSocketMessagePtr& msg)
	{
		switch (msg->type)
		{
			case ix::WebSocketMessageType::Open:
				{
					MIKAN_MT_LOG_ERROR("handleWebSocketMessage") << "New connection";
				}
				break;
			case ix::WebSocketMessageType::Close:
				{
					MIKAN_MT_LOG_ERROR("handleWebSocketMessage") << "Close connection";
				}
				break;
			case ix::WebSocketMessageType::Message:
				{
					if (!msg->binary)
					{
						JsonSaxStringValueSearcher searcher;

						if (searcher.hasKey(msg->str, "eventType"))
						{
							m_eventQueue->enqueue(msg->str);
						}
						else if (searcher.hasKey(msg->str, "responseType"))
						{
							if (m_responseHandler != nullptr)
							{
								m_responseHandler(msg->str);
							}
							else
							{
								MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage")
									<< "Received response message but no handler set: " << msg->str;
							}
						}
						else
						{
							MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage")
								<< "Received unsupported message: " << msg->str;
						}
					}
					else
					{
						MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage")
							<< "Received unsupported binary message";
					}
				} break;
			case ix::WebSocketMessageType::Error:
				{
					MIKAN_MT_LOG_ERROR("handleWebSocketMessage") << "Error: " << msg->errorInfo.reason;
				}
				break;
			case ix::WebSocketMessageType::Ping:
				{
					MIKAN_MT_LOG_ERROR("handleWebSocketMessage") << "Ping";
				}
				break;
			case ix::WebSocketMessageType::Pong:
				{
					MIKAN_MT_LOG_ERROR("handleWebSocketMessage") << "Pong";
				}
				break;
			case ix::WebSocketMessageType::Fragment:
				{
					MIKAN_MT_LOG_ERROR("handleWebSocketMessage") << "Fragment";
				}
				break;
		}
	}

private:
	WebSocketPtr m_websocket;
	ix::WebSocketHttpHeaders m_headers;
	LockFreeEventQueuePtr m_eventQueue;
	IInterprocessMessageClient::ResponseHandler m_responseHandler;
	//std::string m_clientId;
};

//-- WebsocketInterprocessMessageClient -----
WebsocketInterprocessMessageClient::WebsocketInterprocessMessageClient()
	: m_connectionState(std::make_shared<WebsocketConnectionState>())
{
}

WebsocketInterprocessMessageClient::~WebsocketInterprocessMessageClient()
{
	dispose();
}

MikanResult WebsocketInterprocessMessageClient::initialize()
{
	return MikanResult_Success;
}

void WebsocketInterprocessMessageClient::dispose()
{
	disconnect();
}

//const std::string& WebsocketInterprocessMessageClient::getClientId() const
//{ 
//	return m_connectionState->getClientId(); 
//}

const bool WebsocketInterprocessMessageClient::getIsConnected() const
{ 
	return m_connectionState->getIsConnected(); 
}

MikanResult WebsocketInterprocessMessageClient::setClientProperty(const std::string& key, const std::string& value)
{
	m_connectionState->setClientProperty(key, value);

	return MikanResult_Success;
}

void WebsocketInterprocessMessageClient::setResponseHandler(IInterprocessMessageClient::ResponseHandler handler) 
{ 
	m_connectionState->setResponseHandler(handler);
}

MikanResult WebsocketInterprocessMessageClient::connect(const std::string& host, const std::string& port)
{
	return m_connectionState->connect(host, port);
}

void WebsocketInterprocessMessageClient::disconnect()
{
	m_connectionState->disconnect();
}

MikanResult WebsocketInterprocessMessageClient::fetchNextEvent(
	size_t utf8BufferSize, 
	char* outUtf8Buffer, 
	size_t* outUtf8BufferSizeNeeded)
{
	auto eventQueue= m_connectionState->getServerEventQueue();
	const std::string* nextEvent = eventQueue->peek();

	if (nextEvent == nullptr)
		return MikanResult_NoData;

	const size_t eventSize= nextEvent->size();
	const size_t bytesNeeded= eventSize + 1; // Include null terminator

	if (outUtf8Buffer != nullptr)
	{
		if (bytesNeeded > utf8BufferSize)
			return MikanResult_BufferTooSmall;

		// Copy the utf-8 buffer from the event queue into the output buffer
		memcpy(outUtf8Buffer, nextEvent->c_str(), eventSize);

		// Null terminate the end of the string
		outUtf8Buffer[eventSize]= '\0';

		// Remove the event string from the queue
		eventQueue->pop();

		if (outUtf8BufferSizeNeeded != nullptr)
			*outUtf8BufferSizeNeeded= bytesNeeded;

		return MikanResult_Success;
	}
	else 
	{
		if (outUtf8BufferSizeNeeded == nullptr)
			return MikanResult_NullParam;

		*outUtf8BufferSizeNeeded= bytesNeeded;
		return MikanResult_Success;
	}
}

MikanResult WebsocketInterprocessMessageClient::sendRequest(const std::string& utf8RequestString)
{
	ix::WebSocketSendInfo sendInfo= m_connectionState->getWebSocket()->sendText(utf8RequestString);

	if (!sendInfo.success)
	{
		MIKAN_LOG_ERROR("WebsocketInterprocessMessageClient::sendRequest()") 
			<< "Failed to send request: " << utf8RequestString;
		return MikanResult_SocketError;
	}

	return MikanResult_Success;
}