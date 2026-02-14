#include "TestGraphicsContext.h"
#include "Logger.h"
#include "MikanCameraEvents.h"
#include "MikanCameraRequests.h"
#include "TestCameraRenderTarget.h"

TestCameraRenderTarget::TestCameraRenderTarget(
	TestGraphicsContextPtr ownerContext,
	MikanCameraID cameraId)
	: m_ownerContext(ownerContext)
	, m_cameraId(cameraId)
{
}

TestCameraRenderTarget::~TestCameraRenderTarget()
{
	// We should have already called dispose() and cleaned this stuff up before the destructor is called
	assert(!m_bHasAllocatedRemoteTexture);
}

bool TestCameraRenderTarget::processCameraNewFrameEvent(
	IMikanAPIPtr mikanAPI,
	const MikanCameraNewFrameEvent& newFrameEvent)
{
	if (newFrameEvent.frame == m_lastReceivedFrameIndex)
	{
		MIKAN_LOG_INFO("MikanCameraRenderTarget::allocateRenderTarget")
			<< "Ignoring duplicate frame event "
			<< "(camera_id: " << m_cameraId
			<< ", frame: " << m_lastReceivedFrameIndex
			<< ").";
		return true;
	}

	// Remember the frame index of the last frame we published
	m_lastReceivedFrameIndex = newFrameEvent.frame;

	// Update the camera view matrix using the latest camera extrinsics
	updateCameraViewMatrix(newFrameEvent);

	// Update the camera perspective matrix using the latest camera intrinsics
	updateCameraProjectionMatrix(newFrameEvent);

	// Recreate the render target if the texture size changed
	const int newWidth = newFrameEvent.pixel_size.x;
	const int newHeight = newFrameEvent.pixel_size.y;
	if (m_width != newWidth || m_height != newHeight)
	{
		if (reallocateRenderTarget(mikanAPI, newWidth, newHeight))
		{
			MIKAN_LOG_INFO("MikanCameraRenderTarget::allocateRenderTarget")
				<< "Update frame size "
				<< "(camera_id: " << m_cameraId
				<< ", new size: " << newWidth << "x" << newHeight
				<< ", frame: " << m_lastReceivedFrameIndex
				<< ").";
		}
		else
		{
			MIKAN_LOG_ERROR("MikanCameraRenderTarget::allocateRenderTarget")
				<< "Failed to update frame size"
				<< "(camera_id: " << m_cameraId
				<< ", new size: " << newWidth << "x" << newHeight
				<< ", frame: " << m_lastReceivedFrameIndex
				<< "). Invalidating renderMainTarget target.";
			return false;
		}
	}

	// Bind the graphics API specific render target for this camera
	bindGraphicsAPIResource();

	// Render the scene from the PoV of this camera
	if (!m_ownerContext.lock()->renderToCameraTarget(this))
	{
		MIKAN_LOG_INFO("MikanCameraRenderTarget::allocateRenderTarget")
			<< "Failed to renderMainTarget camera frame "
			<< "(camera_id: " << m_cameraId
			<< ", frame: " << m_lastReceivedFrameIndex
			<< ").";
	}

	// Unbind the graphics API specific render target for this camera
	unbindGraphicsAPIResource();

	// Write the color texture, if any, to the shared texture
	void* colorTexture= getGraphicsApiColorTexturePtr();
	if (colorTexture)
	{
		WriteCameraColorRenderTargetTexture writeTextureRequest = {};

		writeTextureRequest.camera_id= m_cameraId;
		writeTextureRequest.api_color_texture_ptr= colorTexture;

		mikanAPI->sendRequest(writeTextureRequest);
	}

	// Write the depth texture, if any, to the shared texture
	void* depthTexture = getGraphicsApiDepthTexturePtr();
	if (depthTexture)
	{
		WriteCameraDepthRenderTargetTexture writeTextureRequest = {};

		writeTextureRequest.camera_id = m_cameraId;
		writeTextureRequest.api_depth_texture_ptr = depthTexture;
		writeTextureRequest.z_near= newFrameEvent.z_bounds.x;
		writeTextureRequest.z_far= newFrameEvent.z_bounds.y;

		mikanAPI->sendRequest(writeTextureRequest);
	}

	// Publish the new video frame back to Mikan
	if (colorTexture || depthTexture)
	{
		PublishCameraRenderTargetTextures frameRendered= {};

		frameRendered.camera_id = m_cameraId;
		frameRendered.frame_index = newFrameEvent.frame;

		mikanAPI->sendRequest(frameRendered);
	}

	return true;
}

void TestCameraRenderTarget::dispose(IMikanAPIPtr mikanAPI)
{
	// Camera matrices aren't valid until we receive the 
	m_bHasValidProjMatrix = false;
	m_bHasValidViewMatrix = false;

	// Clean up shared texture first
	freeSharedTexture(mikanAPI);

	// Clean up locally allocated direct x resources
	freeGraphicsAPIResources();
}

bool TestCameraRenderTarget::reallocateRenderTarget(
	IMikanAPIPtr mikanAPI,
	int textureWidth, 
	int textureHeight)
{
	// Clean up anything we had 
	dispose(mikanAPI);

	// Create a new render texture to render frames too
	bool bSuccess= false; 
	if (createGraphicsAPIResources(textureWidth, textureHeight))
	{
		MIKAN_LOG_INFO("MikanCameraRenderTarget::allocateRenderTarget") 
			<< "Successfully created DirectX texture resources "
			<< "(size: " << textureWidth << "x" << textureHeight << ")";

		if (createSharedTexture(mikanAPI, textureWidth, textureHeight))
		{
			MIKAN_LOG_INFO("MikanCameraRenderTarget::allocateRenderTarget") 
				<< "Successfully created shared texture " 
				<< " (camera id: " << m_cameraId << ")";
			bSuccess= true;
		}
		else
		{
			// Clean back up the DirectX resources, since there is no point to making them if we can't allocate the shared texture 
			freeGraphicsAPIResources();
		}
	}

	return bSuccess;
}

bool TestCameraRenderTarget::createSharedTexture(
	IMikanAPIPtr mikanAPI, 
	int textureWidth, 
	int textureHeight)
{
	assert(!m_bHasAllocatedRemoteTexture);

	// Tell Mikan to create a shared BGRA32 texture for us to render frames to 
	MikanRenderTargetDescriptor desc = {};
	desc.width = textureWidth;
	desc.height = textureHeight;
	desc.color_buffer_type = MikanColorBuffer_BGRA32;
	desc.depth_buffer_type = MikanDepthBuffer_FLOAT_DEVICE_DEPTH;
	desc.graphicsAPI = MikanClientGraphicsApi_Direct3D11;

	AllocateCameraRenderTargetTextures allocateRequest;
	allocateRequest.camera_id = m_cameraId;
	allocateRequest.descriptor = desc;

	auto response = mikanAPI->sendRequest(allocateRequest).fetchResponse();
	if (response->resultCode == MikanAPIResult::Success)
	{
		MIKAN_LOG_INFO("MikanCameraRenderTarget::allocateRenderTarget") << "Successfully allocated shared texture";

		m_bHasAllocatedRemoteTexture= true;
	}
	else
	{
		MIKAN_LOG_WARNING("MikanCameraRenderTarget::allocateRenderTarget") 
			<< "Failed to allocate remote texture " 
			<<"(error_code: " << (int)response->resultCode << ")";
		return false;
	}

	return true;
}

void TestCameraRenderTarget::freeSharedTexture(IMikanAPIPtr mikanAPI)
{
	if (m_bHasAllocatedRemoteTexture)
	{
		// Tell mikan to clean up the shared texture on its side first
		FreeCameraRenderTargetTextures freeRequest = {};
		freeRequest.camera_id = m_cameraId;

		auto response = mikanAPI->sendRequest(freeRequest).fetchResponse();
		if (response->resultCode != MikanAPIResult::Success)
		{
			MIKAN_LOG_WARNING("MikanCameraRenderTarget::freeRenderTarget") << "Failed to free remote texture";
		}

		m_bHasAllocatedRemoteTexture= false;
	}
}