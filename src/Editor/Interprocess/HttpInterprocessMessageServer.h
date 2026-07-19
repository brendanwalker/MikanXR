#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ix
{
class HttpServer;
struct HttpRequest;
struct HttpResponse;
using HttpRequestPtr= std::shared_ptr<HttpRequest>;
using HttpResponsePtr= std::shared_ptr<HttpResponse>;
} // namespace ix

#define HTTP_SERVER_ADDRESS "http://127.0.0.1"
#define HTTP_SERVER_PORT 8090

struct HttpRouteResponse
{
	int statusCode= 200;
	std::string body;
	std::string contentType= "application/json";
};

// method is the HTTP verb (e.g. "GET"/"POST"), path is the request URI with any "?query" stripped,
// body is the raw request body (may be empty).
using HttpRouteHandler=
	std::function<HttpRouteResponse(const std::string& method, const std::string& path, const std::string& body)>;

struct PendingHttpRequest;
using PendingHttpRequestPtr= std::shared_ptr<PendingHttpRequest>;

// Minimal stateless HTTP request/response server, distinct from IInterprocessMessageServer
// (which models persistent, bidirectional websocket connections). Wraps ix::HttpServer, which
// spawns one dedicated thread per accepted connection and expects a synchronous response back
// from that thread. Since route handlers may need to invoke Lua script triggers (not thread-safe),
// incoming requests are queued and only actually dispatched on the main thread inside
// processRequests(), with the connection thread blocking on a promise/future pair until that
// happens (or a timeout elapses).
class HttpInterprocessMessageServer
{
public:
	HttpInterprocessMessageServer();
	~HttpInterprocessMessageServer();

	bool initialize(int port= HTTP_SERVER_PORT);
	void dispose();

	// Registers a handler for an exact route path (e.g. "/trigger/toggle_light").
	// Returns false if the path is already registered.
	bool setRouteHandler(const std::string& path, HttpRouteHandler handler);
	void removeRouteHandler(const std::string& path);

	// Pumps queued HTTP requests on the calling thread (expected to be the main thread),
	// invoking any matched route handler and fulfilling the corresponding connection thread's promise.
	void processRequests();

	// Returns the currently registered route paths (e.g. for a UI that lists them).
	// Main-thread-only, same as processRequests() (route handlers may invoke Lua script triggers).
	std::vector<std::string> getRegisteredRoutePaths() const;

	// Invokes a registered route's handler directly, in-process, bypassing the HTTP
	// request queue entirely (e.g. for a UI button that fires a trigger without a real HTTP
	// round-trip). Returns false if no handler is registered for path. Main-thread-only.
	bool invokeRouteHandler(const std::string& path, HttpRouteResponse& outResponse, const std::string& method= "GET",
							const std::string& body= "");

protected:
	// Runs on a per-connection background thread spawned by ix::SocketServer.
	ix::HttpResponsePtr handleIncomingRequest(ix::HttpRequestPtr request);

private:
	std::shared_ptr<ix::HttpServer> m_server;

	mutable std::mutex m_routeHandlersMutex;
	std::map<std::string, HttpRouteHandler> m_routeHandlers;

	std::mutex m_pendingRequestsMutex;
	std::vector<PendingHttpRequestPtr> m_pendingRequests;
};
