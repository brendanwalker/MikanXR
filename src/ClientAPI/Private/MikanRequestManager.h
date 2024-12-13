#pragma once

#include "MikanCoreTypes.h"
#include "MikanAPITypes.h"
#include "MikanTypeFwd.h"
#include "BinaryUtility.h"
#include "JsonSerializer.h"

#include <future>
#include <mutex>
#include <string>

struct MikanRequest;
typedef void* MikanContext;

class MikanRequestManager
{
public:
	MikanRequestManager() = default;

	MikanAPIResult init(MikanContext context);
	MikanContext getContext() const { return m_context; }

	MikanResponseFuture sendRequest(MikanRequest& request);
	MikanResponseFuture addResponseHandler(MikanRequestID requestId, MikanAPIResult result);
	MikanResponseFuture makeImmediateResponse(MikanAPIResult result);

protected:
	static void textResponseHandlerStatic(MikanRequestID requestId, const char* utf8ResponseString, void* userdata);
	void textResponseHander(MikanRequestID requestId, const char* utf8ResponseString);
	MikanResponsePtr parseResponseString(const char* utf8ResponseString);

	static void binaryResponseHandlerStatic(const uint8_t* buffer, size_t bufferSize, void* userdata);
	void binaryResponseHander(const uint8_t* buffer, size_t bufferSize);
	MikanResponsePtr parseResponseBinaryReader(
		const MikanResponse& requestHeader,
		const uint8_t* buffer, 
		size_t bufferSize);

private:
	struct PendingRequest
	{
		MikanRequestID id;
		MikanResponsePromise promise;
	};
	using PendingRequestPtr = std::shared_ptr<PendingRequest>;

	MikanContext m_context= nullptr;
	std::map<MikanRequestID, PendingRequestPtr> m_pendingRequests;
	std::mutex m_pending_request_map_mutex;
	MikanRequestID m_nextRequestID= 0;
};
