#include "TestCameraRenderTarget_VK.h"
#include "TestGraphicsContext_VK.h"
#include "TestOpenGLMath.h"
#include "TestVulkanMath.h"
#include "MikanCameraEvents.h"
#include "Logger.h"

#include <cassert>

TestCameraRenderTarget_VK::TestCameraRenderTarget_VK(TestGraphicsContextPtr ownerContext, MikanCameraID cameraId)
	: TestCameraRenderTarget(ownerContext, cameraId)
{
}

TestCameraRenderTarget_VK::~TestCameraRenderTarget_VK()
{
	// We should have already called dispose() and cleaned this stuff up before the destructor is called
	assert(m_colorImage == VK_NULL_HANDLE);
	assert(m_depthImage == VK_NULL_HANDLE);
	assert(!m_bHasAllocatedRemoteTexture);
}

bool TestCameraRenderTarget_VK::createGraphicsAPIResources(int textureWidth, int textureHeight)
{
	auto vulkanContext= std::static_pointer_cast<TestGraphicsContext_VK>(m_ownerContext.lock());
	if (!vulkanContext)
		return false;

	// The color target is rendered to, blitted to the window, and handed to Mikan's writer
	const VkImageUsageFlags colorUsage=
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	if (!vulkanContext->createImage(textureWidth, textureHeight, TestGraphicsContext_VK::k_colorFormat, colorUsage,
									VK_IMAGE_ASPECT_COLOR_BIT, m_colorImage, m_colorMemory, m_colorImageView))
	{
		MIKAN_LOG_ERROR("TestCameraRenderTarget_VK::createGraphicsAPIResources") << "Failed to create color target";
		return false;
	}

	// The depth target is also copied out each frame, so it needs transfer source usage
	if (!vulkanContext->createImage(textureWidth, textureHeight, TestGraphicsContext_VK::k_depthFormat,
									VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
									VK_IMAGE_ASPECT_DEPTH_BIT, m_depthImage, m_depthMemory, m_depthImageView))
	{
		MIKAN_LOG_ERROR("TestCameraRenderTarget_VK::createGraphicsAPIResources") << "Failed to create depth target";
		return false;
	}

	// The depth copy chain: one float per texel through a device-local buffer into an R32 image
	const VkDeviceSize depthByteCount= (VkDeviceSize)textureWidth * textureHeight * sizeof(float);
	const VkImageUsageFlags depthColorUsage=
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	if (!vulkanContext->createBuffer(depthByteCount,
									 VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
									 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthTransferBuffer, m_depthTransferMemory)
		|| !vulkanContext->createImage(textureWidth, textureHeight, VK_FORMAT_R32_SFLOAT, depthColorUsage,
									   VK_IMAGE_ASPECT_COLOR_BIT, m_depthColorImage, m_depthColorMemory,
									   m_depthColorImageView))
	{
		MIKAN_LOG_ERROR("TestCameraRenderTarget_VK::createGraphicsAPIResources")
			<< "Failed to create the depth copy resources";
		return false;
	}

	m_mikanColorTexture.image= m_colorImage;
	m_mikanColorTexture.layout= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	m_mikanColorTexture.format= TestGraphicsContext_VK::k_colorFormat;
	m_mikanColorTexture.width= (uint32_t)textureWidth;
	m_mikanColorTexture.height= (uint32_t)textureHeight;

	m_mikanDepthTexture= m_mikanColorTexture;
	m_mikanDepthTexture.image= m_depthColorImage;
	m_mikanDepthTexture.format= VK_FORMAT_R32_SFLOAT;

	return true;
}

void TestCameraRenderTarget_VK::freeGraphicsAPIResources()
{
	m_width= 0;
	m_height= 0;
	m_mikanColorTexture= {};
	m_mikanDepthTexture= {};

	auto vulkanContext= std::static_pointer_cast<TestGraphicsContext_VK>(m_ownerContext.lock());
	if (vulkanContext)
	{
		// destroyImage waits the device idle, so the buffer destroy after them is safe
		vulkanContext->destroyImage(m_colorImage, m_colorMemory, m_colorImageView);
		vulkanContext->destroyImage(m_depthImage, m_depthMemory, m_depthImageView);
		vulkanContext->destroyImage(m_depthColorImage, m_depthColorMemory, m_depthColorImageView);
		vulkanContext->destroyBuffer(m_depthTransferBuffer, m_depthTransferMemory);
	}
}

// The whole camera pass records inside renderToCameraTarget, so there is nothing to bind here
void TestCameraRenderTarget_VK::bindGraphicsAPIResource() {}

void TestCameraRenderTarget_VK::unbindGraphicsAPIResource() {}

void* TestCameraRenderTarget_VK::getGraphicsApiColorTexturePtr() const
{
	return m_colorImage != VK_NULL_HANDLE ? (void*)&m_mikanColorTexture : nullptr;
}

void* TestCameraRenderTarget_VK::getGraphicsApiDepthTexturePtr() const
{
	return m_depthColorImage != VK_NULL_HANDLE ? (void*)&m_mikanDepthTexture : nullptr;
}

void TestCameraRenderTarget_VK::updateCameraViewMatrix(const MikanCameraNewFrameEvent& newFrameEvent)
{
	m_cameraPosition= MikanVector3f_to_glm_vec3(newFrameEvent.camera_position);
	m_cameraForward= MikanVector3f_to_glm_vec3(newFrameEvent.camera_forward);
	m_cameraUp= MikanVector3f_to_glm_vec3(newFrameEvent.camera_up);
	m_cameraRight= glm::normalize(glm::cross(m_cameraForward, m_cameraUp));

	m_viewMatrix= mikan_camera_pose_to_glm_view_matrix(newFrameEvent.camera_forward, newFrameEvent.camera_up,
													   newFrameEvent.camera_position);
}

void TestCameraRenderTarget_VK::updateCameraProjectionMatrix(const MikanCameraNewFrameEvent& newFrameEvent)
{
	m_projMatrix= mikan_camera_intrinsics_to_vulkan_projection_matrix(
		newFrameEvent.focal_length.x, newFrameEvent.focal_length.y, newFrameEvent.principal_point.x,
		newFrameEvent.principal_point.y, newFrameEvent.pixel_size.x, newFrameEvent.pixel_size.y,
		newFrameEvent.z_bounds.x, newFrameEvent.z_bounds.y);
}
