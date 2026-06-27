#pragma once

//-- includes -----
#include "SharedTextureFwd.h"
#include "MikanCoreTypes.h"
#include "MikanClientLogger.h"

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
	const std::string& getClientName() const;

	// -- ClientMikanAPI System -----
	MikanCoreResult startup(const char* client_name, ClientLogSeverityLevel log_level, t_logCallback log_callback);
	MikanCoreResult connect(const std::string& host, const std::string& port);
	MikanCoreResult disconnect(uint16_t code, const std::string& reason);
	MikanCoreResult fetchNextEvent(size_t utf8_buffer_size, char* out_utf8_buffer, size_t* out_utf8_bytes_written);
	MikanCoreResult setTextResponseCallback(MikanTextResponseCallback callback, void* callback_userdata);
	MikanCoreResult setBinaryResponseCallback(MikanBinaryResponseCallback callback, void* callback_userdata);
	MikanCoreResult sendRequestJSON(const char* utf8_request_json);
	MikanCoreResult shutdown();

	MikanCoreResult allocateCameraRenderTargetTextures(MikanCameraID cameraId,
													   const MikanRenderTargetDescriptor& descriptor);
	MikanCoreResult getCameraRenderTargetDescriptor(MikanCameraID cameraId, MikanRenderTargetDescriptor& outDescriptor);
	MikanCoreResult freeCameraRenderTargetTextures(MikanCameraID cameraId);
	MikanCoreResult freeAllCameraRenderTargetTextures();
	MikanCoreResult writeCameraColorRenderTargetTexture(MikanCameraID cameraId, void* ApiColorTexturePtr);
	MikanCoreResult writeCameraDepthRenderTargetTexture(MikanCameraID cameraId, void* ApiDepthTexturePtr, float zNear,
														float zFar);
	void* getCameraPackDepthTextureResourcePtr(MikanCameraID cameraId) const;
	MikanCoreResult setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface);
	MikanCoreResult getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface);
	MikanCoreResult setGraphicsCommandQueueInterface(MikanClientGraphicsApi api, void* graphicsCommandQueueInterface);
	MikanCoreResult getGraphicsCommandQueueInterface(MikanClientGraphicsApi api, void** outGraphicsCommandQueueInterface);

protected:
	ISharedTextureWriteAccessorPtr getSharedTextureWriteAccessor(MikanCameraID camera_id) const;
	ISharedTextureWriteAccessorPtr addSharedTextureWriteAccessor(MikanCameraID camera_id);
	void textResponseHandler(const std::string& utf8ResponseString);
	void binaryResponseHandler(const uint8_t* buffer, size_t bufferSize);

private:
	std::array<void*, MikanClientGraphicsApi_COUNT> m_graphicsDeviceInterfaces;
	std::array<void*, MikanClientGraphicsApi_COUNT> m_graphicsCommandQueueInterfaces;

	MikanTextResponseCallback m_textResponseCallback= nullptr;
	void* m_textResponseCallbackUserData= nullptr;
	MikanBinaryResponseCallback m_binaryResponseCallback= nullptr;
	void* m_binaryResponseCallbackUserData= nullptr;

	std::string m_clientName;
	std::map<MikanCameraID, ISharedTextureWriteAccessorPtr> m_renderTargetWriterCameraMap;
	class IInterprocessMessageClient* m_messageClient;
	bool m_bIsConnected;
};