#pragma once

#include "volk.h"
#include "MikanCoreVulkanTypes.h"
#include "TestGraphicsContext.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

// The Vulkan render path of the test harness: a Vulkan 1.3 device over an SDL window, a cube
// pass into each camera's offscreen color target through dynamic rendering, and a blit of the
// last rendered camera onto the swapchain. The device is what Mikan's Vulkan writer imports
// its shared textures into, so it carries VK_KHR_external_memory_win32.
class TestGraphicsContext_VK : public TestGraphicsContext, public std::enable_shared_from_this<TestGraphicsContext_VK>
{
public:
	static constexpr VkFormat k_colorFormat= VK_FORMAT_B8G8R8A8_UNORM;
	static constexpr VkFormat k_depthFormat= VK_FORMAT_D32_SFLOAT;

	TestGraphicsContext_VK(class TestApp* ownerApp);

	virtual MikanClientGraphicsApi getGraphicsApi() const override { return MikanClientGraphicsApi_Vulkan; }
	virtual void* getGraphicsDeviceInterface() const override { return (void*)&m_deviceInterface; }
	virtual struct SDL_Window* getSDLWindow() const override { return m_sdlWindow; }
	virtual TestCameraRenderTargetPtr allocateCameraRenderTarget(int cameraId) override;
	virtual bool create(int windowWidth, int windowHeight) override;
	virtual void recreateMainRenderTarget() override;
	virtual void renderMainTarget() const override;
	virtual bool renderToCameraTarget(class TestCameraRenderTarget* cameraRenderTarget) override;
	virtual void dispose() override;
	virtual MikanCameraID getLastRenderedCameraId() const override { return m_lastRenderedCameraId; }
	virtual bool readCameraTargetPixels(class TestCameraRenderTarget* cameraRenderTarget,
										std::vector<uint8_t>& outRgbaPixels, int& outWidth, int& outHeight) override;

	// Resource helpers the camera render targets build their targets with
	bool createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
					 VkImageAspectFlags aspect, VkImage& outImage, VkDeviceMemory& outMemory,
					 VkImageView& outView) const;
	void destroyImage(VkImage& image, VkDeviceMemory& memory, VkImageView& view) const;
	bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
					  VkBuffer& outBuffer, VkDeviceMemory& outMemory) const;
	void destroyBuffer(VkBuffer& buffer, VkDeviceMemory& memory) const;

protected:
	bool createInstance();
	bool createSurfaceAndDevice();
	bool createCommandState();
	void destroyCommandState();
	bool createSwapchain();
	void destroySwapchain();
	bool createCubeGeometry();
	bool createCubePipeline();
	bool loadShaderModule(const char* fileName, VkShaderModule& outModule) const;
	bool findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, uint32_t& outMemoryType) const;

	bool waitForFence(VkFence fence) const;
	bool beginCommands(VkCommandBuffer commandBuffer, VkFence fence) const;
	bool submitCommands(VkCommandBuffer commandBuffer, VkFence fence, VkSemaphore waitSemaphore,
						VkPipelineStageFlags waitStage, VkSemaphore signalSemaphore) const;

	void renderCube(VkCommandBuffer commandBuffer, const glm::mat4& viewProj, const glm::vec3& cameraPosition,
					const glm::vec3& cameraForward, const glm::vec3& cameraUp, const glm::vec3& cameraRight) const;

private:
	struct SDL_Window* m_sdlWindow= nullptr;
	int m_windowWidth= 0;
	int m_windowHeight= 0;

	// Instance and device
	VkInstance m_instance= VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger= VK_NULL_HANDLE;
	VkSurfaceKHR m_surface= VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice= VK_NULL_HANDLE;
	VkDevice m_device= VK_NULL_HANDLE;
	VkQueue m_queue= VK_NULL_HANDLE;
	uint32_t m_queueFamilyIndex= 0;
	MikanVulkanDeviceInterface m_deviceInterface= {};

	// Swapchain, one render-finished semaphore per image so a presented image never reuses one
	VkSwapchainKHR m_swapchain= VK_NULL_HANDLE;
	VkFormat m_swapchainFormat= VK_FORMAT_UNDEFINED;
	VkExtent2D m_swapchainExtent= {};
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	VkSemaphore m_imageAvailableSemaphore= VK_NULL_HANDLE;
	// Set when a present reports the swapchain out of date, consumed at the next main-target render
	mutable bool m_bSwapchainDirty= false;

	// One command buffer and fence for the camera pass and one for the present, each waited before reuse
	VkCommandPool m_commandPool= VK_NULL_HANDLE;
	VkCommandBuffer m_cameraCommandBuffer= VK_NULL_HANDLE;
	VkCommandBuffer m_presentCommandBuffer= VK_NULL_HANDLE;
	VkFence m_cameraFence= VK_NULL_HANDLE;
	VkFence m_presentFence= VK_NULL_HANDLE;

	// Cube pipeline state
	VkPipelineLayout m_cubePipelineLayout= VK_NULL_HANDLE;
	VkPipeline m_cubePipeline= VK_NULL_HANDLE;
	VkBuffer m_cubeVertexBuffer= VK_NULL_HANDLE;
	VkDeviceMemory m_cubeVertexMemory= VK_NULL_HANDLE;
	uint32_t m_cubeVertexCount= 0;

	mutable bool m_bWarnedUnsupportedRenderMode= false;
	MikanCameraID m_lastRenderedCameraId= INVALID_MIKAN_ID;
};
