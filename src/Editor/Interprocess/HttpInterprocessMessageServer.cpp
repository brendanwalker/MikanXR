#include "HttpInterprocessMessageServer.h"
#include "Logger.h"

#include "IxWebSocket/IXHttpServer.h"
#include "IxWebSocket/IXConnectionState.h"
#include "IxWebSocket/IxNetSystem.h"

#include <algorithm>
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

int hexDigitValue(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return -1;
}

// Percent-decodes one query string token, treating '+' as a space per the
// application/x-www-form-urlencoded convention. A malformed escape is left verbatim.
std::string percentDecode(const std::string& text)
{
	std::string decoded;
	decoded.reserve(text.size());

	for (size_t i= 0; i < text.size(); ++i)
	{
		const char c= text[i];

		if (c == '+')
		{
			decoded.push_back(' ');
		}
		else if (c == '%' && i + 2 < text.size())
		{
			const int high= hexDigitValue(text[i + 1]);
			const int low= hexDigitValue(text[i + 2]);

			if (high >= 0 && low >= 0)
			{
				decoded.push_back(static_cast<char>((high << 4) | low));
				i+= 2;
			}
			else
			{
				decoded.push_back(c);
			}
		}
		else
		{
			decoded.push_back(c);
		}
	}

	return decoded;
}

// Splits a request URI's "?key=value&..." tail into decoded pairs. A key with no '=' maps to
// an empty value, an empty key is dropped, and a repeated key keeps the last occurrence.
std::map<std::string, std::string> parseQueryString(const std::string& uri)
{
	std::map<std::string, std::string> queryArgs;

	const size_t queryPos= uri.find('?');
	if (queryPos == std::string::npos)
	{
		return queryArgs;
	}

	size_t pairStart= queryPos + 1;
	while (pairStart <= uri.size())
	{
		const size_t pairEnd= std::min(uri.find('&', pairStart), uri.size());
		const std::string pair= uri.substr(pairStart, pairEnd - pairStart);
		pairStart= pairEnd + 1;

		if (pair.empty())
		{
			continue;
		}

		const size_t equalsPos= pair.find('=');
		const std::string key= percentDecode(pair.substr(0, equalsPos));
		if (key.empty())
		{
			continue;
		}

		queryArgs[key]= equalsPos != std::string::npos ? percentDecode(pair.substr(equalsPos + 1)) : std::string();
	}

	return queryArgs;
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
	HttpRouteRequest routeRequest;
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

std::vector<std::string> HttpInterprocessMessageServer::getRegisteredRoutePaths() const
{
	std::vector<std::string> paths;

	std::lock_guard<std::mutex> lock(m_routeHandlersMutex);
	paths.reserve(m_routeHandlers.size());
	for (const auto& entry : m_routeHandlers)
	{
		paths.push_back(entry.first);
	}

	return paths;
}

bool HttpInterprocessMessageServer::invokeRouteHandler(const HttpRouteRequest& request, HttpRouteResponse& outResponse)
{
	HttpRouteHandler handler;
	{
		std::lock_guard<std::mutex> lock(m_routeHandlersMutex);

		auto handler_it= m_routeHandlers.find(request.path);
		if (handler_it == m_routeHandlers.end())
		{
			return false;
		}

		handler= handler_it->second;
	}

	outResponse= handler(request);
	return true;
}

ix::HttpResponsePtr HttpInterprocessMessageServer::handleIncomingRequest(ix::HttpRequestPtr request)
{
	auto pendingRequest= std::make_shared<PendingHttpRequest>();
	pendingRequest->routeRequest.method= request->method;
	pendingRequest->routeRequest.path= stripQueryString(request->uri);
	pendingRequest->routeRequest.queryArgs= parseQueryString(request->uri);
	pendingRequest->routeRequest.body= request->body;
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
				<< "Timed out waiting for main thread to process request: " << pendingRequest->routeRequest.path;
			routeResponse.statusCode= 504;
			routeResponse.body= R"({"error":"Timed out waiting for request to be processed"})";
		}
	}
	catch (const std::future_error&)
	{
		// The promise was dropped (e.g. server shutting down) without being fulfilled.
		MIKAN_MT_LOG_WARNING("HttpInterprocessMessageServer::handleIncomingRequest")
			<< "Request abandoned before it could be processed: " << pendingRequest->routeRequest.path;
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

			auto handler_it= m_routeHandlers.find(pendingRequest->routeRequest.path);
			if (handler_it != m_routeHandlers.end())
			{
				handler= handler_it->second;
			}
		}

		HttpRouteResponse response;
		if (handler)
		{
			response= handler(pendingRequest->routeRequest);
		}
		else
		{
			MIKAN_LOG_WARNING("HttpInterprocessMessageServer::processRequests")
				<< "No route registered for path: " << pendingRequest->routeRequest.path;
			response.statusCode= 404;
			response.body= R"({"error":"No route registered for path"})";
		}

		pendingRequest->responsePromise->set_value(response);
	}
}
