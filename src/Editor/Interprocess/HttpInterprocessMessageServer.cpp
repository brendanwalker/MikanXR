#include "HttpInterprocessMessageServer.h"
#include "Logger.h"

#include "IxWebSocket/IXHttpServer.h"
#include "IxWebSocket/IXConnectionState.h"
#include "IxWebSocket/IxNetSystem.h"

#include <chrono>
#include <future>

namespace
{
constexpr int k_httpRequestTimeoutMs= 1000;

std::string stripQueryString(const std::string& uri)
{
	const size_t queryPos= uri.find('?');
	return queryPos != std::string::npos ? uri.substr(0, queryPos) : uri;
}

std::string httpStatusDescription(int statusCode)
{
	switch (statusCode)
	{
	case 200:
		return "OK";
	case 400:
		return "Bad Request";
	case 404:
		return "Not Found";
	case 422:
		return "Unprocessable Entity";
	case 504:
		return "Gateway Timeout";
	default:
		return "Internal Server Error";
	}
}
} // namespace

struct PendingHttpRequest
{
	std::string method;
	std::string path;
	std::string body;
	std::shared_ptr<std::promise<HttpRouteResponse>> responsePromise;
};

HttpInterprocessMessageServer::HttpInterprocessMessageServer()
	: m_server(nullptr)
{
}

HttpInterprocessMessageServer::~HttpInterprocessMessageServer() { dispose(); }

bool HttpInterprocessMessageServer::initialize(int port)
{
	if (!ix::initNetSystem())
	{
		MIKAN_LOG_WARNING("HttpInterprocessMessageServer::initialize()") << "Failed to initialize net system";
		return false;
	}

	HttpInterprocessMessageServer* ownerServer= this;

	m_server= std::make_shared<ix::HttpServer>(port);
	m_server->setOnConnectionCallback(
		[ownerServer](ix::HttpRequestPtr request,
					  std::shared_ptr<ix::ConnectionState> connectionState) -> ix::HttpResponsePtr
		{ return ownerServer->handleIncomingRequest(request); });

	std::pair<bool, std::string> result= m_server->listen();
	if (!result.first)
	{
		MIKAN_LOG_WARNING("HttpInterprocessMessageServer::initialize()") << "Listen error: " << result.second;
		return false;
	}

	m_server->start();
	return true;
}

void HttpInterprocessMessageServer::dispose()
{
	if (m_server)
	{
		m_server->stop();
		m_server= nullptr;

		ix::uninitNetSystem();
	}

	{
		std::lock_guard<std::mutex> lock(m_pendingRequestsMutex);
		m_pendingRequests.clear();
	}
}

bool HttpInterprocessMessageServer::setRouteHandler(const std::string& path, HttpRouteHandler handler)
{
	std::lock_guard<std::mutex> lock(m_routeHandlersMutex);

	if (m_routeHandlers.find(path) != m_routeHandlers.end())
	{
		MIKAN_LOG_WARNING("HttpInterprocessMessageServer::setRouteHandler") << "Route already registered: " << path;
		return false;
	}

	m_routeHandlers[path]= handler;
	return true;
}

void HttpInterprocessMessageServer::removeRouteHandler(const std::string& path)
{
	std::lock_guard<std::mutex> lock(m_routeHandlersMutex);

	m_routeHandlers.erase(path);
}

ix::HttpResponsePtr HttpInterprocessMessageServer::handleIncomingRequest(ix::HttpRequestPtr request)
{
	auto pendingRequest= std::make_shared<PendingHttpRequest>();
	pendingRequest->method= request->method;
	pendingRequest->path= stripQueryString(request->uri);
	pendingRequest->body= request->body;
	pendingRequest->responsePromise= std::make_shared<std::promise<HttpRouteResponse>>();

	std::future<HttpRouteResponse> future= pendingRequest->responsePromise->get_future();

	{
		std::lock_guard<std::mutex> lock(m_pendingRequestsMutex);
		m_pendingRequests.push_back(pendingRequest);
	}

	HttpRouteResponse routeResponse;
	try
	{
		if (future.wait_for(std::chrono::milliseconds(k_httpRequestTimeoutMs)) == std::future_status::ready)
		{
			routeResponse= future.get();
		}
		else
		{
			// The main thread may still fulfill this promise later; that's harmless since
			// std::promise::set_value() doesn't require a waiter to still be listening.
			MIKAN_MT_LOG_WARNING("HttpInterprocessMessageServer::handleIncomingRequest")
				<< "Timed out waiting for main thread to process request: " << pendingRequest->path;
			routeResponse.statusCode= 504;
			routeResponse.body= R"({"error":"Timed out waiting for request to be processed"})";
		}
	}
	catch (const std::future_error&)
	{
		// The promise was dropped (e.g. server shutting down) without being fulfilled.
		MIKAN_MT_LOG_WARNING("HttpInterprocessMessageServer::handleIncomingRequest")
			<< "Request abandoned before it could be processed: " << pendingRequest->path;
		routeResponse.statusCode= 503;
		routeResponse.body= R"({"error":"Server shutting down"})";
	}

	ix::WebSocketHttpHeaders headers;
	headers["Content-Type"]= routeResponse.contentType;

	return std::make_shared<ix::HttpResponse>(routeResponse.statusCode, httpStatusDescription(routeResponse.statusCode),
											  ix::HttpErrorCode::Ok, headers, routeResponse.body);
}

void HttpInterprocessMessageServer::processRequests()
{
	std::vector<PendingHttpRequestPtr> pendingRequests;
	{
		std::lock_guard<std::mutex> lock(m_pendingRequestsMutex);
		pendingRequests.swap(m_pendingRequests);
	}

	for (const PendingHttpRequestPtr& pendingRequest : pendingRequests)
	{
		HttpRouteHandler handler;
		{
			std::lock_guard<std::mutex> lock(m_routeHandlersMutex);

			auto handler_it= m_routeHandlers.find(pendingRequest->path);
			if (handler_it != m_routeHandlers.end())
			{
				handler= handler_it->second;
			}
		}

		HttpRouteResponse response;
		if (handler)
		{
			response= handler(pendingRequest->method, pendingRequest->path, pendingRequest->body);
		}
		else
		{
			MIKAN_LOG_WARNING("HttpInterprocessMessageServer::processRequests")
				<< "No route registered for path: " << pendingRequest->path;
			response.statusCode= 404;
			response.body= R"({"error":"No route registered for path"})";
		}

		pendingRequest->responsePromise->set_value(response);
	}
}
