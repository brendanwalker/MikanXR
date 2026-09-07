#pragma once

#include "volk.h"
#include "MikanAPI.h"
#include "MikanCoreVulkanTypes.h"
#include "TestCameraRenderTarget.h"

#include <glm/glm.hpp>

class TestCameraRenderTarget_VK : public TestCameraRenderTarget
{
public:
	TestCameraRenderTarget_VK(TestGraphicsContextPtr ownerContext, int cameraId);
	virtual ~TestCameraRenderTarget_VK();

	inline VkImage getColorImage() const { return m_colorImage; }
	inline VkImageView getColorImageView() const { return m_colorImageView; }
	inline VkImage getDepthImage() const { return m_depthImage; }
	inline VkImageView getDepthImageView() const { return m_depthImageView; }
	inline VkImage getDepthColorImage() const { return m_depthColorImage; }
	inline VkBuffer getDepthTransferBuffer() const { return m_depthTransferBuffer; }
	inline VkExtent2D getExtent() const { return {m_mikanColorTexture.width, m_mikanColorTexture.height}; }
	inline bool getIsInitialized() const
	{
		return m_colorImage != VK_NULL_HANDLE && m_depthImage != VK_NULL_HANDLE && m_depthColorImage != VK_NULL_HANDLE;
	}

	const glm::vec3& getCameraPosition() const { return m_cameraPosition; }
	const glm::vec3& getCameraForward() const { return m_cameraForward; }
	const glm::vec3& getCameraUp() const { return m_cameraUp; }
	const glm::vec3& getCameraRight() const { return m_cameraRight; }
	glm::mat4 getViewProjectionMatrix() const { return m_projMatrix * m_viewMatrix; }

protected:
	virtual bool createGraphicsAPIResources(int textureWidth, int textureHeight) override;
	virtual void bindGraphicsAPIResource() override;
	virtual void unbindGraphicsAPIResource() override;
	virtual void freeGraphicsAPIResources() override;
	virtual void* getGraphicsApiColorTexturePtr() const override;
	virtual void* getGraphicsApiDepthTexturePtr() const override;

	virtual void updateCameraViewMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) override;
	virtual void updateCameraProjectionMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) override;

private:
	// Color target: what the cube renders into and what Mikan receives.
	// Held in TRANSFER_SRC_OPTIMAL between frames, which is the layout handed to Mikan.
	VkImage m_colorImage= VK_NULL_HANDLE;
	VkDeviceMemory m_colorMemory= VK_NULL_HANDLE;
	VkImageView m_colorImageView= VK_NULL_HANDLE;

	// Depth target for the cube pass
	VkImage m_depthImage= VK_NULL_HANDLE;
	VkDeviceMemory m_depthMemory= VK_NULL_HANDLE;
	VkImageView m_depthImageView= VK_NULL_HANDLE;

	// Device depth as an R32 color image, the form Mikan's depth write takes: the camera pass copies
	// the depth attachment into the transfer buffer and the buffer into this image every frame
	VkBuffer m_depthTransferBuffer= VK_NULL_HANDLE;
	VkDeviceMemory m_depthTransferMemory= VK_NULL_HANDLE;
	VkImage m_depthColorImage= VK_NULL_HANDLE;
	VkDeviceMemory m_depthColorMemory= VK_NULL_HANDLE;
	VkImageView m_depthColorImageView= VK_NULL_HANDLE;

	// The color and depth images as the client API describes them
	MikanVulkanTexture m_mikanColorTexture= {};
	MikanVulkanTexture m_mikanDepthTexture= {};

	glm::mat4 m_projMatrix= glm::mat4(1.0f);
	glm::mat4 m_viewMatrix= glm::mat4(1.0f);
	glm::vec3 m_cameraPosition= glm::vec3(0.0f);
	glm::vec3 m_cameraForward= glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_cameraUp= glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_cameraRight= glm::vec3(1.0f, 0.0f, 0.0f);
};
