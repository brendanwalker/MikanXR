#pragma once

//-- includes -----
#include "MikanCoreTypes.h"

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
    MikanCoreResult startup(ClientLogSeverityLevel log_level, t_logCallback log_callback);
	MikanCoreResult connect(const std::string& host, const std::string& port);
	MikanCoreResult disconnect(uint16_t code, const std::string& reason);
	MikanCoreResult fetchNextEvent(size_t utf8_buffer_size, char* out_utf8_buffer, size_t* out_utf8_bytes_written);
	MikanCoreResult setTextResponseCallback(MikanTextResponseCallback callback, void* callback_userdata);
	MikanCoreResult setBinaryResponseCallback(MikanBinaryResponseCallback callback, void* callback_userdata);
	MikanCoreResult sendRequestJSON(const char* utf8_request_json);
	MikanCoreResult shutdown();

	MikanCoreResult allocateRenderTargetTextures(const MikanRenderTargetDescriptor& descriptor);
	MikanCoreResult getRenderTargetDescriptor(MikanRenderTargetDescriptor& outDescriptor);
	MikanCoreResult freeRenderTargetTextures();
	MikanCoreResult writeColorRenderTargetTexture(void* ApiColorTexturePtr);
	MikanCoreResult writeDepthRenderTargetTexture(void* ApiDepthTexturePtr, float zNear, float zFar);
	void* getPackDepthTextureResourcePtr() const;
	MikanCoreResult setGraphicsDeviceInterface(
		MikanClientGraphicsApi api,
		void* graphicsDeviceInterface);
	MikanCoreResult getGraphicsDeviceInterface(
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