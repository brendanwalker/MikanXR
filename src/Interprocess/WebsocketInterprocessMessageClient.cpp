#include "WebsocketInterprocessMessageClient.h"
#include "JsonUtils.h"
#include "Logger.h"
#include "MikanConstants.h"
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
	inline void setTextResponseHandler(IInterprocessMessageClient::TextResponseHandler handler) { 
		m_textResponseHandler= handler;  
	}
	inline void setBinaryResponseHandler(IInterprocessMessageClient::BinaryResponseHandler handler)
	{
		m_binaryResponseHandler = handler;
	}

	MikanCoreResult connect(
		const std::string& host,
		const std::string& port)
	{
		if (getIsConnected())
		{
			MIKAN_MT_LOG_ERROR("WebsocketConnectionState::connect()") << "Already connected";
			return MikanCoreResult_AlreadyConnected;
		}

		std::string hostAddress= host.empty() ? WEBSOCKET_SERVER_ADDRESS : host;
		std::string hostPort= port.empty() ? WEBSOCKET_SERVER_PORT : port;
		m_websocket->setUrl(hostAddress+":"+hostPort);
		m_websocket->start();

		return MikanCoreResult_Success;
	}

	bool disconnect(uint16_t code = 0, const std::string& reason = "")
	{
		if (getIsConnected())
		{
			if (code != 0)
			{
				m_websocket->close(code, reason);
			}
			else
			{
				m_websocket->close(
					ix::WebSocketCloseConstants::kNormalClosureCode,
					ix::WebSocketCloseConstants::kNormalClosureMessage);
			}

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
					std::stringstream ss;

					ss << WEBSOCKET_CONNECT_EVENT;
					MIKAN_MT_LOG_INFO("handleWebSocketMessage") << "New connection";

					// Append the server version, if available
					{
						auto iter = msg->openInfo.headers.find(MIKAN_SERVER_API_VERSION_KEY);

						if (iter != msg->openInfo.headers.end())
						{
							const std::string serverVersion = iter->second;
							MIKAN_MT_LOG_INFO("handleWebSocketMessage") << "Server API version: " << serverVersion;

							ss << ":" << serverVersion;
						}
						else
						{
							MIKAN_MT_LOG_WARNING("handleWebSocketMessage") << "Server API version: UNKNOWN (assuming 0)";

							ss << ":0";
						}
					}

					// Append the min allowed client version, if available
					{
						auto iter = msg->openInfo.headers.find(MIKAN_MIN_ALLOWED_CLIENT_API_VERSION_KEY);

						if (iter != msg->openInfo.headers.end())
						{
							const std::string minAllowedVersion = iter->second;
							MIKAN_MT_LOG_INFO("handleWebSocketMessage") << "Min Allowed Client API version: " << minAllowedVersion;

							ss << ":" << minAllowedVersion;
						}
						else
						{
							MIKAN_MT_LOG_WARNING("handleWebSocketMessage") << "Min Allowed Client API version: UNKNOWN (assuming 0)";

							ss << ":0";
						}
					}

					m_eventQueue->enqueue(ss.str());
				}
				break;
			case ix::WebSocketMessageType::Close:
				{
					std::stringstream ss;

					ss << WEBSOCKET_DISCONNECT_EVENT;
					MIKAN_MT_LOG_INFO("handleWebSocketMessage") << "Close connection";

					ss << ":" << msg->closeInfo.code << ":" << msg->closeInfo.reason;

					m_eventQueue->enqueue(WEBSOCKET_DISCONNECT_EVENT);
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
							if (m_textResponseHandler != nullptr)
							{
								m_textResponseHandler(msg->str);
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
						// Binary message always assumed to	be a response (and not an event)
						if (m_binaryResponseHandler != nullptr)
						{
							const uint8_t* buffer= reinterpret_cast<const uint8_t*>(msg->str.c_str());
							const size_t bufferSize= msg->wireSize;

							m_binaryResponseHandler(buffer, bufferSize);
						}
						else
						{
							MIKAN_MT_LOG_ERROR("ClientConnectionState::handleClientMessage")
								<< "Received binary message but no handler set";
						}
					}
				} break;
			case ix::WebSocketMessageType::Error:
				{
					MIKAN_MT_LOG_ERROR("handleWebSocketMessage") << "Error: " << msg->errorInfo.reason;

					std::stringstream ss;
					ss << "Error: " << msg->errorInfo.reason;

					m_eventQueue->enqueue(ss.str());
				}
				break;
			case ix::WebSocketMessageType::Ping:
				{
					MIKAN_MT_LOG_INFO("handleWebSocketMessage") << "Ping";

					m_eventQueue->enqueue(WEBSOCKET_PING_EVENT);
				}
				break;
			case ix::WebSocketMessageType::Pong:
				{
					MIKAN_MT_LOG_INFO("handleWebSocketMessage") << "Pong";

					m_eventQueue->enqueue(WEBSOCKET_PONG_EVENT);
				}
				break;
			case ix::WebSocketMessageType::Fragment:
				{
					MIKAN_MT_LOG_INFO("handleWebSocketMessage") << "Fragment";
				}
				break;
		}
	}

private:
	WebSocketPtr m_websocket;
	ix::WebSocketHttpHeaders m_headers;
	LockFreeEventQueuePtr m_eventQueue;
	IInterprocessMessageClient::TextResponseHandler m_textResponseHandler;
	IInterprocessMessageClient::BinaryResponseHandler m_binaryResponseHandler;
	std::string m_connectionRequestJson;
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

MikanCoreResult WebsocketInterprocessMessageClient::initialize()
{
	return MikanCoreResult_Success;
}

void WebsocketInterprocessMessageClient::dispose()
{
	disconnect(0, "");
}

const bool WebsocketInterprocessMessageClient::getIsConnected() const
{ 
	return m_connectionState->getIsConnected(); 
}

void WebsocketInterprocessMessageClient::setTextResponseHandler(
	IInterprocessMessageClient::TextResponseHandler handler) 
{ 
	m_connectionState->setTextResponseHandler(handler);
}

void WebsocketInterprocessMessageClient::setBinaryResponseHandler(
	IInterprocessMessageClient::BinaryResponseHandler handler)
{
	m_connectionState->setBinaryResponseHandler(handler);
}

MikanCoreResult WebsocketInterprocessMessageClient::connect(
	const std::string& host, 
	const std::string& port)
{
	return m_connectionState->connect(host, port);
}

void WebsocketInterprocessMessageClient::disconnect(uint16_t code, const std::string& reason)
{
	m_connectionState->disconnect(code, reason);
}

MikanCoreResult WebsocketInterprocessMessageClient::fetchNextEvent(
	size_t utf8BufferSize, 
	char* outUtf8Buffer, 
	size_t* outUtf8BufferSizeNeeded)
{
	auto eventQueue= m_connectionState->getServerEventQueue();
	const std::string* nextEvent = eventQueue->peek();

	if (nextEvent == nullptr)
		return MikanCoreResult_NoData;

	const size_t eventSize= nextEvent->size();
	const size_t bytesNeeded= eventSize + 1; // Include null terminator

	if (outUtf8Buffer != nullptr)
	{
		if (bytesNeeded > utf8BufferSize)
			return MikanCoreResult_BufferTooSmall;

		// Copy the utf-8 buffer from the event queue into the output buffer
		memcpy(outUtf8Buffer, nextEvent->c_str(), eventSize);

		// Null terminate the end of the string
		outUtf8Buffer[eventSize]= '\0';

		// Remove the event string from the queue
		eventQueue->pop();

		if (outUtf8BufferSizeNeeded != nullptr)
			*outUtf8BufferSizeNeeded= bytesNeeded;

		return MikanCoreResult_Success;
	}
	else 
	{
		if (outUtf8BufferSizeNeeded == nullptr)
			return MikanCoreResult_NullParam;

		*outUtf8BufferSizeNeeded= bytesNeeded;
		return MikanCoreResult_Success;
	}
}

MikanCoreResult WebsocketInterprocessMessageClient::sendRequest(const std::string& utf8RequestString)
{
	ix::WebSocketSendInfo sendInfo= m_connectionState->getWebSocket()->sendText(utf8RequestString);

	if (!sendInfo.success)
	{
		MIKAN_LOG_ERROR("WebsocketInterprocessMessageClient::sendRequest()") 
			<< "Failed to send request: " << utf8RequestString;
		return MikanCoreResult_SocketError;
	}

	return MikanCoreResult_Success;
}