#ifndef MIKAN_CLIENT_H
#define MIKAN_CLIENT_H

//-- includes -----
#include "MikanCoreTypes.h"
#include "Logger.h"

#include <map>
#include <mutex>

//-- definitions -----
class MikanClient
{
public:
    MikanClient();
    virtual ~MikanClient();

	// -- State Queries ----
	bool getIsConnected() const;

    // -- ClientMikanAPI System -----
    MikanResult startup(LogSeverityLevel log_level, t_logCallback log_callback);
	MikanResult setClientProperty(const std::string& key, const std::string& value);
	MikanResult connect(const std::string& host, const std::string& port);
	MikanResult disconnect();
	MikanResult fetchNextEvent(size_t utf8_buffer_size, char* out_utf8_buffer, size_t* out_utf8_bytes_written);
	MikanResult setResponseCallback(MikanResponseCallback callback, void* callback_userdata);
	MikanResult sendRequest(const char* utf8_request_name, const char* utf8_payload, int request_version, MikanRequestID* out_request_id);
	MikanResult shutdown();

	MikanResult allocateRenderTargetBuffers(const MikanRenderTargetDescriptor& descriptor, MikanRequestID* out_request_id);
	MikanResult publishRenderTargetTexture(void* ApiTexturePtr, const MikanClientFrameRendered& frameInfo);
	MikanResult freeRenderTargetBuffers(MikanRequestID* out_request_id);

protected:
	void responseHandler(const std::string& utf8ResponseString);

private:
	MikanResponseCallback m_responseCallback= nullptr;
	void* m_responseCallbackUserData= nullptr;
	MikanRequestID m_next_request_id = 0;

	std::string m_clientName;
	class InterprocessRenderTargetWriteAccessor* m_renderTargetWriter;
	class IInterprocessMessageClient* m_messageClient;
	bool m_bIsConnected;
};


#endif // MIKAN_CLIENT_H