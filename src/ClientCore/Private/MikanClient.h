#pragma once

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
	const std::string& getClientUniqueID() const;

    // -- ClientMikanAPI System -----
    MikanResult startup(LogSeverityLevel log_level, t_logCallback log_callback);
	MikanResult setClientInfo(const std::string& clientinfo);
	MikanResult connect(const std::string& host, const std::string& port);
	MikanResult disconnect();
	MikanResult fetchNextEvent(size_t utf8_buffer_size, char* out_utf8_buffer, size_t* out_utf8_bytes_written);
	MikanResult setTextResponseCallback(MikanTextResponseCallback callback, void* callback_userdata);
	MikanResult setBinaryResponseCallback(MikanBinaryResponseCallback callback, void* callback_userdata);
	MikanResult sendRequestJSON(const char* utf8_request_json);
	MikanResult shutdown();

	MikanResult allocateRenderTargetTextures(const MikanRenderTargetDescriptor& descriptor);
	MikanResult getRenderTargetDescriptor(MikanRenderTargetDescriptor& outDescriptor);
	MikanResult freeRenderTargetTextures();
	MikanResult writeColorRenderTargetTexture(void* ApiColorTexturePtr);
	MikanResult writeDepthRenderTargetTexture(void* ApiDepthTexturePtr, float zNear, float zFar);
	void* getPackDepthTextureResourcePtr() const;
	MikanResult setGraphicsDeviceInterface(
		MikanClientGraphicsApi api,
		void* graphicsDeviceInterface);
	MikanResult getGraphicsDeviceInterface(
		MikanClientGraphicsApi api,
		void** outGraphicsDeviceInterface);

protected:
	void textResponseHandler(const std::string& utf8ResponseString);
	void binaryResponseHandler(const uint8_t* buffer, size_t bufferSize);

private:
	std::array<void*, MikanClientGraphicsApi_COUNT> m_graphicsDeviceInterfaces;

	MikanTextResponseCallback m_textResponseCallback= nullptr;
	void* m_textResponseCallbackUserData= nullptr;
	MikanBinaryResponseCallback m_binaryResponseCallback = nullptr;
	void* m_binaryResponseCallbackUserData= nullptr;

	std::string m_clientUniqueID;
	class InterprocessRenderTargetWriteAccessor* m_renderTargetWriter;
	class IInterprocessMessageClient* m_messageClient;
	bool m_bIsConnected;
};