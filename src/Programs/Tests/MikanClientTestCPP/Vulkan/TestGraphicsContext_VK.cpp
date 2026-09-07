#include "TestGraphicsContext_VK.h"
#include "TestApp.h"
#include "TestCameraRenderTarget_VK.h"
#include "Logger.h"

#if defined(_WIN32)
#include <SDL.h>
#include <SDL_vulkan.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#endif

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

namespace
{
const char* k_validationLayerName= "VK_LAYER_KHRONOS_validation";
// The external memory extension's name macro lives in vulkan_win32.h, which the harness leaves to
// SDL's surface handling, so the name is spelled out
const char* k_externalMemoryWin32ExtensionName= "VK_KHR_external_memory_win32";
const char* k_deviceExtensions[]= {VK_KHR_SWAPCHAIN_EXTENSION_NAME, k_externalMemoryWin32ExtensionName};

// Validation is opt-in through MIKAN_VULKAN_VALIDATION, off when unset or "0"
bool isVulkanValidationRequested()
{
	const char* setting= std::getenv("MIKAN_VULKAN_VALIDATION");
	return setting != nullptr && std::strcmp(setting, "0") != 0;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
													  VkDebugUtilsMessageTypeFlagsEXT,
													  const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void*)
{
	if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
	{
		MIKAN_LOG_ERROR("vulkan") << callbackData->pMessage;
	}
	else
	{
		MIKAN_LOG_WARNING("vulkan") << callbackData->pMessage;
	}

	return VK_FALSE;
}

bool hasLayer(const char* layerName)
{
	uint32_t layerCount= 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> layers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

	for (const VkLayerProperties& layer : layers)
	{
		if (std::strcmp(layer.layerName, layerName) == 0)
			return true;
	}

	return false;
}

bool hasDeviceExtensions(VkPhysicalDevice physicalDevice)
{
	uint32_t extensionCount= 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

	for (const char* required : k_deviceExtensions)
	{
		bool bFound= false;
		for (const VkExtensionProperties& extension : extensions)
		{
			if (std::strcmp(extension.extensionName, required) == 0)
			{
				bFound= true;
				break;
			}
		}

		if (!bFound)
			return false;
	}

	return true;
}

VkImageMemoryBarrier makeImageBarrier(VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout,
									  VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
	VkImageMemoryBarrier barrier= {};
	barrier.sType= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask= srcAccess;
	barrier.dstAccessMask= dstAccess;
	barrier.oldLayout= oldLayout;
	barrier.newLayout= newLayout;
	barrier.srcQueueFamilyIndex= VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex= VK_QUEUE_FAMILY_IGNORED;
	barrier.image= image;
	barrier.subresourceRange.aspectMask= aspect;
	barrier.subresourceRange.levelCount= 1;
	barrier.subresourceRange.layerCount= 1;

	return barrier;
}
} // namespace

TestGraphicsContext_VK::TestGraphicsContext_VK(TestApp* ownerApp)
	: TestGraphicsContext(ownerApp)
{
}

TestCameraRenderTargetPtr TestGraphicsContext_VK::allocateCameraRenderTarget(int cameraId)
{
	return std::make_shared<TestCameraRenderTarget_VK>(shared_from_this(), cameraId);
}

bool TestGraphicsContext_VK::create(int windowWidth, int windowHeight)
{
	if (SDL_Vulkan_LoadLibrary(nullptr) != 0)
	{
		MIKAN_LOG_ERROR("startup") << "Unable to load the Vulkan loader: " << SDL_GetError();
		return false;
	}

	const SDL_WindowFlags window_flags=
		(SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	m_sdlWindow= SDL_CreateWindow("Mikan Client Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth,
								  windowHeight, window_flags);
	m_windowWidth= windowWidth;
	m_windowHeight= windowHeight;

	if (m_sdlWindow == nullptr)
	{
		MIKAN_LOG_ERROR("startup") << "Unable to initialize window: " << SDL_GetError();
		return false;
	}

	// Share SDL's loader with volk so both dispatch through the same vulkan-1.dll
	volkInitializeCustom((PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr());

	if (!createInstance() || !createSurfaceAndDevice() || !createCommandState() || !createSwapchain())
	{
		MIKAN_LOG_ERROR("startup") << "Failed to create the Vulkan device!";
		return false;
	}

	if (!createCubeGeometry() || !createCubePipeline())
	{
		MIKAN_LOG_ERROR("startup") << "Failed to initialize the cube pipeline!";
		return false;
	}

	return true;
}

bool TestGraphicsContext_VK::createInstance()
{
	unsigned int extensionCount= 0;
	if (!SDL_Vulkan_GetInstanceExtensions(m_sdlWindow, &extensionCount, nullptr))
	{
		MIKAN_LOG_ERROR("createInstance") << "SDL_Vulkan_GetInstanceExtensions failed: " << SDL_GetError();
		return false;
	}
	std::vector<const char*> extensions(extensionCount);
	SDL_Vulkan_GetInstanceExtensions(m_sdlWindow, &extensionCount, extensions.data());

	std::vector<const char*> layers;
	const bool bValidation= isVulkanValidationRequested() && hasLayer(k_validationLayerName);
	if (bValidation)
	{
		layers.push_back(k_validationLayerName);
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		MIKAN_LOG_INFO("createInstance") << "Vulkan validation layer enabled";
	}

	VkApplicationInfo applicationInfo= {};
	applicationInfo.sType= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName= "MikanXR Client Test C++";
	applicationInfo.pEngineName= "MikanXR Test";
	applicationInfo.apiVersion= VK_API_VERSION_1_3;

	VkInstanceCreateInfo instanceInfo= {};
	instanceInfo.sType= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo= &applicationInfo;
	instanceInfo.enabledLayerCount= (uint32_t)layers.size();
	instanceInfo.ppEnabledLayerNames= layers.data();
	instanceInfo.enabledExtensionCount= (uint32_t)extensions.size();
	instanceInfo.ppEnabledExtensionNames= extensions.data();

	const VkResult result= vkCreateInstance(&instanceInfo, nullptr, &m_instance);
	if (result != VK_SUCCESS)
	{
		MIKAN_LOG_ERROR("createInstance") << "vkCreateInstance failed (VkResult " << (int)result << ")";
		return false;
	}
	volkLoadInstance(m_instance);

	if (bValidation)
	{
		VkDebugUtilsMessengerCreateInfoEXT messengerInfo= {};
		messengerInfo.sType= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		messengerInfo.messageSeverity=
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		messengerInfo.messageType= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
								   | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
								   | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		messengerInfo.pfnUserCallback= debugMessengerCallback;
		vkCreateDebugUtilsMessengerEXT(m_instance, &messengerInfo, nullptr, &m_debugMessenger);
	}

	return true;
}

bool TestGraphicsContext_VK::createSurfaceAndDevice()
{
	if (!SDL_Vulkan_CreateSurface(m_sdlWindow, m_instance, &m_surface))
	{
		MIKAN_LOG_ERROR("createSurfaceAndDevice") << "SDL_Vulkan_CreateSurface failed: " << SDL_GetError();
		return false;
	}

	// Pick the best device with a queue that can both draw and present and the extensions Mikan needs
	uint32_t physicalDeviceCount= 0;
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());

	int bestScore= 0;
	for (VkPhysicalDevice candidate : physicalDevices)
	{
		VkPhysicalDeviceProperties properties= {};
		vkGetPhysicalDeviceProperties(candidate, &properties);
		if (properties.apiVersion < VK_API_VERSION_1_3 || !hasDeviceExtensions(candidate))
			continue;

		uint32_t queueFamilyCount= 0;
		vkGetPhysicalDeviceQueueFamilyProperties(candidate, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(candidate, &queueFamilyCount, queueFamilies.data());

		for (uint32_t familyIndex= 0; familyIndex < queueFamilyCount; ++familyIndex)
		{
			VkBool32 bPresentSupported= VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(candidate, familyIndex, m_surface, &bPresentSupported);
			if ((queueFamilies[familyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 || !bPresentSupported)
				continue;

			const int score= properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU     ? 3
							 : properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 2
																							   : 1;
			if (score > bestScore)
			{
				bestScore= score;
				m_physicalDevice= candidate;
				m_queueFamilyIndex= familyIndex;
				MIKAN_LOG_INFO("createSurfaceAndDevice") << "Vulkan device candidate: " << properties.deviceName;
			}
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
	{
		MIKAN_LOG_ERROR("createSurfaceAndDevice")
			<< "No Vulkan 1.3 device with a graphics+present queue and " << k_externalMemoryWin32ExtensionName;
		return false;
	}

	const float queuePriority= 1.0f;
	VkDeviceQueueCreateInfo queueInfo= {};
	queueInfo.sType= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex= m_queueFamilyIndex;
	queueInfo.queueCount= 1;
	queueInfo.pQueuePriorities= &queuePriority;

	// Dynamic rendering keeps the cube pass free of render pass and framebuffer objects
	VkPhysicalDeviceVulkan13Features features13= {};
	features13.sType= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	features13.dynamicRendering= VK_TRUE;
	VkPhysicalDeviceFeatures2 features2= {};
	features2.sType= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.pNext= &features13;

	VkDeviceCreateInfo deviceInfo= {};
	deviceInfo.sType= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext= &features2;
	deviceInfo.queueCreateInfoCount= 1;
	deviceInfo.pQueueCreateInfos= &queueInfo;
	deviceInfo.enabledExtensionCount= (uint32_t)(sizeof(k_deviceExtensions) / sizeof(k_deviceExtensions[0]));
	deviceInfo.ppEnabledExtensionNames= k_deviceExtensions;

	const VkResult result= vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_device);
	if (result != VK_SUCCESS)
	{
		MIKAN_LOG_ERROR("createSurfaceAndDevice") << "vkCreateDevice failed (VkResult " << (int)result << ")";
		return false;
	}
	volkLoadDevice(m_device);
	vkGetDeviceQueue(m_device, m_queueFamilyIndex, 0, &m_queue);

	// What Mikan's Vulkan writer is handed
	m_deviceInterface.instance= m_instance;
	m_deviceInterface.physicalDevice= m_physicalDevice;
	m_deviceInterface.device= m_device;
	m_deviceInterface.queue= m_queue;
	m_deviceInterface.queueFamilyIndex= m_queueFamilyIndex;

	return true;
}

bool TestGraphicsContext_VK::createCommandState()
{
	VkCommandPoolCreateInfo poolInfo= {};
	poolInfo.sType= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex= m_queueFamilyIndex;
	if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
		return false;

	VkCommandBuffer commandBuffers[2]= {};
	VkCommandBufferAllocateInfo allocInfo= {};
	allocInfo.sType= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool= m_commandPool;
	allocInfo.level= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount= 2;
	if (vkAllocateCommandBuffers(m_device, &allocInfo, commandBuffers) != VK_SUCCESS)
		return false;
	m_cameraCommandBuffer= commandBuffers[0];
	m_presentCommandBuffer= commandBuffers[1];

	// Fences start signaled so the first wait before reuse passes
	VkFenceCreateInfo fenceInfo= {};
	fenceInfo.sType= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags= VK_FENCE_CREATE_SIGNALED_BIT;
	if (vkCreateFence(m_device, &fenceInfo, nullptr, &m_cameraFence) != VK_SUCCESS
		|| vkCreateFence(m_device, &fenceInfo, nullptr, &m_presentFence) != VK_SUCCESS)
		return false;

	VkSemaphoreCreateInfo semaphoreInfo= {};
	semaphoreInfo.sType= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS)
		return false;

	return true;
}

void TestGraphicsContext_VK::destroyCommandState()
{
	if (m_imageAvailableSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr);
		m_imageAvailableSemaphore= VK_NULL_HANDLE;
	}
	if (m_cameraFence != VK_NULL_HANDLE)
	{
		vkDestroyFence(m_device, m_cameraFence, nullptr);
		m_cameraFence= VK_NULL_HANDLE;
	}
	if (m_presentFence != VK_NULL_HANDLE)
	{
		vkDestroyFence(m_device, m_presentFence, nullptr);
		m_presentFence= VK_NULL_HANDLE;
	}
	if (m_commandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		m_commandPool= VK_NULL_HANDLE;
	}
	m_cameraCommandBuffer= VK_NULL_HANDLE;
	m_presentCommandBuffer= VK_NULL_HANDLE;
}

bool TestGraphicsContext_VK::createSwapchain()
{
	VkSurfaceCapabilitiesKHR capabilities= {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);

	// Match the camera targets' format where the surface allows it
	uint32_t formatCount= 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());
	VkSurfaceFormatKHR surfaceFormat= formats.empty() ? VkSurfaceFormatKHR{} : formats[0];
	for (const VkSurfaceFormatKHR& format : formats)
	{
		if (format.format == k_colorFormat)
		{
			surfaceFormat= format;
			break;
		}
	}

	// Present without vsync like the DirectX path, falling back to the always-available FIFO
	uint32_t presentModeCount= 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data());
	VkPresentModeKHR presentMode= VK_PRESENT_MODE_FIFO_KHR;
	if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != presentModes.end())
		presentMode= VK_PRESENT_MODE_IMMEDIATE_KHR;

	VkExtent2D extent= capabilities.currentExtent;
	if (extent.width == UINT32_MAX)
	{
		int drawableWidth= 0;
		int drawableHeight= 0;
		SDL_Vulkan_GetDrawableSize(m_sdlWindow, &drawableWidth, &drawableHeight);
		extent.width=
			std::clamp((uint32_t)drawableWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height= std::clamp((uint32_t)drawableHeight, capabilities.minImageExtent.height,
								  capabilities.maxImageExtent.height);
	}

	uint32_t imageCount= capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0)
		imageCount= (std::min)(imageCount, capabilities.maxImageCount);

	VkSwapchainCreateInfoKHR swapchainInfo= {};
	swapchainInfo.sType= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface= m_surface;
	swapchainInfo.minImageCount= imageCount;
	swapchainInfo.imageFormat= surfaceFormat.format;
	swapchainInfo.imageColorSpace= surfaceFormat.colorSpace;
	swapchainInfo.imageExtent= extent;
	swapchainInfo.imageArrayLayers= 1;
	swapchainInfo.imageUsage= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchainInfo.imageSharingMode= VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.preTransform= capabilities.currentTransform;
	swapchainInfo.compositeAlpha= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode= presentMode;
	swapchainInfo.clipped= VK_TRUE;

	const VkResult result= vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &m_swapchain);
	if (result != VK_SUCCESS)
	{
		MIKAN_LOG_ERROR("createSwapchain") << "vkCreateSwapchainKHR failed (VkResult " << (int)result << ")";
		return false;
	}
	m_swapchainFormat= surfaceFormat.format;
	m_swapchainExtent= extent;

	uint32_t swapchainImageCount= 0;
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, nullptr);
	m_swapchainImages.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, m_swapchainImages.data());

	VkSemaphoreCreateInfo semaphoreInfo= {};
	semaphoreInfo.sType= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	m_renderFinishedSemaphores.resize(swapchainImageCount, VK_NULL_HANDLE);
	for (VkSemaphore& semaphore : m_renderFinishedSemaphores)
	{
		if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
			return false;
	}

	m_bSwapchainDirty= false;

	return true;
}

void TestGraphicsContext_VK::destroySwapchain()
{
	for (VkSemaphore semaphore : m_renderFinishedSemaphores)
	{
		if (semaphore != VK_NULL_HANDLE)
			vkDestroySemaphore(m_device, semaphore, nullptr);
	}
	m_renderFinishedSemaphores.clear();
	m_swapchainImages.clear();

	if (m_swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
		m_swapchain= VK_NULL_HANDLE;
	}
}

void TestGraphicsContext_VK::recreateMainRenderTarget()
{
	if (m_device == VK_NULL_HANDLE)
		return;

	vkDeviceWaitIdle(m_device);
	destroySwapchain();
	if (!createSwapchain())
	{
		MIKAN_LOG_ERROR("recreateMainRenderTarget") << "Failed to recreate the swapchain";
	}
}

bool TestGraphicsContext_VK::createCubeGeometry()
{
	struct CubeVertex
	{
		glm::vec4 position;
		glm::vec4 color;
	};

	// Cube vertex data (position + color), the same cube the DirectX path draws
	const CubeVertex vertices[]= {
		// Front face (Red)
		{{-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{-1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},

		// Back face (Green)
		{{-1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{-1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{-1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},

		// Top face (Blue)
		{{-1.0f, 1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
		{{-1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
		{{-1.0f, 1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},

		// Bottom face (Yellow)
		{{-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
		{{-1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
		{{-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},

		// Left face (Magenta)
		{{-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
		{{-1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
		{{-1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
		{{-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
		{{-1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
		{{-1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},

		// Right face (Cyan)
		{{1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
		{{1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
	};
	m_cubeVertexCount= (uint32_t)(sizeof(vertices) / sizeof(vertices[0]));

	// A host-visible vertex buffer, written once
	if (!createBuffer(sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_cubeVertexBuffer,
					  m_cubeVertexMemory))
	{
		MIKAN_LOG_ERROR("createCubeGeometry") << "Failed to create the cube vertex buffer";
		return false;
	}

	void* mapped= nullptr;
	if (vkMapMemory(m_device, m_cubeVertexMemory, 0, sizeof(vertices), 0, &mapped) != VK_SUCCESS)
	{
		MIKAN_LOG_ERROR("createCubeGeometry") << "Failed to map cube vertex memory";
		return false;
	}
	std::memcpy(mapped, vertices, sizeof(vertices));
	vkUnmapMemory(m_device, m_cubeVertexMemory);

	return true;
}

bool TestGraphicsContext_VK::loadShaderModule(const char* fileName, VkShaderModule& outModule) const
{
	// The SPIR-V lands beside the executable at build time
	char* basePath= SDL_GetBasePath();
	const std::string path= std::string(basePath != nullptr ? basePath : "") + "shaders/" + fileName;
	SDL_free(basePath);

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		MIKAN_LOG_ERROR("loadShaderModule") << "Missing shader " << path << " (was dxc found at configure time?)";
		return false;
	}
	const std::streamsize size= file.tellg();
	std::vector<uint32_t> code((size_t)(size + 3) / 4);
	file.seekg(0);
	file.read((char*)code.data(), size);

	VkShaderModuleCreateInfo moduleInfo= {};
	moduleInfo.sType= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.codeSize= (size_t)size;
	moduleInfo.pCode= code.data();
	if (vkCreateShaderModule(m_device, &moduleInfo, nullptr, &outModule) != VK_SUCCESS)
	{
		MIKAN_LOG_ERROR("loadShaderModule") << "vkCreateShaderModule failed for " << path;
		return false;
	}

	return true;
}

bool TestGraphicsContext_VK::createCubePipeline()
{
	VkShaderModule vertexModule= VK_NULL_HANDLE;
	VkShaderModule pixelModule= VK_NULL_HANDLE;
	if (!loadShaderModule("cube_vs.spv", vertexModule) || !loadShaderModule("cube_ps.spv", pixelModule))
	{
		if (vertexModule != VK_NULL_HANDLE)
			vkDestroyShaderModule(m_device, vertexModule, nullptr);
		return false;
	}

	// The world-view-projection matrix rides a push constant
	VkPushConstantRange pushConstantRange= {};
	pushConstantRange.stageFlags= VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.size= sizeof(glm::mat4);
	VkPipelineLayoutCreateInfo layoutInfo= {};
	layoutInfo.sType= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.pushConstantRangeCount= 1;
	layoutInfo.pPushConstantRanges= &pushConstantRange;
	if (vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &m_cubePipelineLayout) != VK_SUCCESS)
	{
		MIKAN_LOG_ERROR("createCubePipeline") << "vkCreatePipelineLayout failed";
		return false;
	}

	VkPipelineShaderStageCreateInfo stages[2]= {};
	stages[0].sType= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage= VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module= vertexModule;
	stages[0].pName= "vs_main";
	stages[1].sType= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage= VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module= pixelModule;
	stages[1].pName= "ps_main";

	// position (float4) + color (float4)
	VkVertexInputBindingDescription binding= {};
	binding.binding= 0;
	binding.stride= sizeof(glm::vec4) * 2;
	binding.inputRate= VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription attributes[2]= {};
	attributes[0].location= 0;
	attributes[0].binding= 0;
	attributes[0].format= VK_FORMAT_R32G32B32A32_SFLOAT;
	attributes[0].offset= 0;
	attributes[1].location= 1;
	attributes[1].binding= 0;
	attributes[1].format= VK_FORMAT_R32G32B32A32_SFLOAT;
	attributes[1].offset= sizeof(glm::vec4);
	VkPipelineVertexInputStateCreateInfo vertexInput= {};
	vertexInput.sType= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexBindingDescriptionCount= 1;
	vertexInput.pVertexBindingDescriptions= &binding;
	vertexInput.vertexAttributeDescriptionCount= 2;
	vertexInput.pVertexAttributeDescriptions= attributes;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly= {};
	inputAssembly.sType= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Viewport and scissor are dynamic, set per camera target
	VkPipelineViewportStateCreateInfo viewportState= {};
	viewportState.sType= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount= 1;
	viewportState.scissorCount= 1;
	const VkDynamicState dynamicStates[]= {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState= {};
	dynamicState.sType= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount= 2;
	dynamicState.pDynamicStates= dynamicStates;

	// The projection's y flip reverses the winding the DirectX cube was authored in, and the
	// cube is closed and depth tested, so no face culling is the simplest correct choice
	VkPipelineRasterizationStateCreateInfo rasterization= {};
	rasterization.sType= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization.polygonMode= VK_POLYGON_MODE_FILL;
	rasterization.cullMode= VK_CULL_MODE_NONE;
	rasterization.frontFace= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization.lineWidth= 1.0f;

	VkPipelineMultisampleStateCreateInfo multisample= {};
	multisample.sType= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.rasterizationSamples= VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depthStencil= {};
	depthStencil.sType= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable= VK_TRUE;
	depthStencil.depthWriteEnable= VK_TRUE;
	depthStencil.depthCompareOp= VK_COMPARE_OP_LESS;

	VkPipelineColorBlendAttachmentState blendAttachment= {};
	blendAttachment.colorWriteMask=
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendStateCreateInfo colorBlend= {};
	colorBlend.sType= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlend.attachmentCount= 1;
	colorBlend.pAttachments= &blendAttachment;

	// Dynamic rendering: the attachment formats stand in for a render pass
	const VkFormat colorFormat= k_colorFormat;
	VkPipelineRenderingCreateInfo renderingInfo= {};
	renderingInfo.sType= VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingInfo.colorAttachmentCount= 1;
	renderingInfo.pColorAttachmentFormats= &colorFormat;
	renderingInfo.depthAttachmentFormat= k_depthFormat;

	VkGraphicsPipelineCreateInfo pipelineInfo= {};
	pipelineInfo.sType= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext= &renderingInfo;
	pipelineInfo.stageCount= 2;
	pipelineInfo.pStages= stages;
	pipelineInfo.pVertexInputState= &vertexInput;
	pipelineInfo.pInputAssemblyState= &inputAssembly;
	pipelineInfo.pViewportState= &viewportState;
	pipelineInfo.pRasterizationState= &rasterization;
	pipelineInfo.pMultisampleState= &multisample;
	pipelineInfo.pDepthStencilState= &depthStencil;
	pipelineInfo.pColorBlendState= &colorBlend;
	pipelineInfo.pDynamicState= &dynamicState;
	pipelineInfo.layout= m_cubePipelineLayout;

	const VkResult result=
		vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_cubePipeline);
	vkDestroyShaderModule(m_device, vertexModule, nullptr);
	vkDestroyShaderModule(m_device, pixelModule, nullptr);
	if (result != VK_SUCCESS)
	{
		MIKAN_LOG_ERROR("createCubePipeline") << "vkCreateGraphicsPipelines failed (VkResult " << (int)result << ")";
		return false;
	}

	return true;
}

bool TestGraphicsContext_VK::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
										  VkBuffer& outBuffer, VkDeviceMemory& outMemory) const
{
	VkBufferCreateInfo bufferInfo= {};
	bufferInfo.sType= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size= size;
	bufferInfo.usage= usage;
	bufferInfo.sharingMode= VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &outBuffer) != VK_SUCCESS)
		return false;

	VkMemoryRequirements requirements= {};
	vkGetBufferMemoryRequirements(m_device, outBuffer, &requirements);
	uint32_t memoryType= 0;
	if (!findMemoryType(requirements.memoryTypeBits, properties, memoryType))
		return false;

	VkMemoryAllocateInfo allocInfo= {};
	allocInfo.sType= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize= requirements.size;
	allocInfo.memoryTypeIndex= memoryType;

	return vkAllocateMemory(m_device, &allocInfo, nullptr, &outMemory) == VK_SUCCESS
		   && vkBindBufferMemory(m_device, outBuffer, outMemory, 0) == VK_SUCCESS;
}

void TestGraphicsContext_VK::destroyBuffer(VkBuffer& buffer, VkDeviceMemory& memory) const
{
	if (m_device == VK_NULL_HANDLE)
		return;

	if (buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_device, buffer, nullptr);
		buffer= VK_NULL_HANDLE;
	}
	if (memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_device, memory, nullptr);
		memory= VK_NULL_HANDLE;
	}
}

bool TestGraphicsContext_VK::findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties,
											uint32_t& outMemoryType) const
{
	VkPhysicalDeviceMemoryProperties memoryProperties= {};
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

	for (uint32_t typeIndex= 0; typeIndex < memoryProperties.memoryTypeCount; ++typeIndex)
	{
		if ((typeBits & (1u << typeIndex)) != 0
			&& (memoryProperties.memoryTypes[typeIndex].propertyFlags & properties) == properties)
		{
			outMemoryType= typeIndex;
			return true;
		}
	}

	return false;
}

bool TestGraphicsContext_VK::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
										 VkImageAspectFlags aspect, VkImage& outImage, VkDeviceMemory& outMemory,
										 VkImageView& outView) const
{
	VkImageCreateInfo imageInfo= {};
	imageInfo.sType= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType= VK_IMAGE_TYPE_2D;
	imageInfo.format= format;
	imageInfo.extent= {width, height, 1};
	imageInfo.mipLevels= 1;
	imageInfo.arrayLayers= 1;
	imageInfo.samples= VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling= VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage= usage;
	imageInfo.sharingMode= VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout= VK_IMAGE_LAYOUT_UNDEFINED;
	if (vkCreateImage(m_device, &imageInfo, nullptr, &outImage) != VK_SUCCESS)
		return false;

	VkMemoryRequirements requirements= {};
	vkGetImageMemoryRequirements(m_device, outImage, &requirements);
	uint32_t memoryType= 0;
	if (!findMemoryType(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryType))
		return false;

	VkMemoryAllocateInfo allocInfo= {};
	allocInfo.sType= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize= requirements.size;
	allocInfo.memoryTypeIndex= memoryType;
	if (vkAllocateMemory(m_device, &allocInfo, nullptr, &outMemory) != VK_SUCCESS
		|| vkBindImageMemory(m_device, outImage, outMemory, 0) != VK_SUCCESS)
		return false;

	VkImageViewCreateInfo viewInfo= {};
	viewInfo.sType= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image= outImage;
	viewInfo.viewType= VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format= format;
	viewInfo.subresourceRange.aspectMask= aspect;
	viewInfo.subresourceRange.levelCount= 1;
	viewInfo.subresourceRange.layerCount= 1;

	return vkCreateImageView(m_device, &viewInfo, nullptr, &outView) == VK_SUCCESS;
}

void TestGraphicsContext_VK::destroyImage(VkImage& image, VkDeviceMemory& memory, VkImageView& view) const
{
	if (m_device == VK_NULL_HANDLE)
		return;

	// A camera target dies rarely (resize, teardown), so an idle wait is the simplest safety
	vkDeviceWaitIdle(m_device);

	if (view != VK_NULL_HANDLE)
	{
		vkDestroyImageView(m_device, view, nullptr);
		view= VK_NULL_HANDLE;
	}
	if (image != VK_NULL_HANDLE)
	{
		vkDestroyImage(m_device, image, nullptr);
		image= VK_NULL_HANDLE;
	}
	if (memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_device, memory, nullptr);
		memory= VK_NULL_HANDLE;
	}
}

bool TestGraphicsContext_VK::waitForFence(VkFence fence) const
{
	return vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX) == VK_SUCCESS;
}

bool TestGraphicsContext_VK::beginCommands(VkCommandBuffer commandBuffer, VkFence fence) const
{
	vkResetFences(m_device, 1, &fence);

	VkCommandBufferBeginInfo beginInfo= {};
	beginInfo.sType= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	return vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS;
}

bool TestGraphicsContext_VK::submitCommands(VkCommandBuffer commandBuffer, VkFence fence, VkSemaphore waitSemaphore,
											VkPipelineStageFlags waitStage, VkSemaphore signalSemaphore) const
{
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		return false;

	VkSubmitInfo submitInfo= {};
	submitInfo.sType= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount= waitSemaphore != VK_NULL_HANDLE ? 1 : 0;
	submitInfo.pWaitSemaphores= &waitSemaphore;
	submitInfo.pWaitDstStageMask= &waitStage;
	submitInfo.commandBufferCount= 1;
	submitInfo.pCommandBuffers= &commandBuffer;
	submitInfo.signalSemaphoreCount= signalSemaphore != VK_NULL_HANDLE ? 1 : 0;
	submitInfo.pSignalSemaphores= &signalSemaphore;

	return vkQueueSubmit(m_queue, 1, &submitInfo, fence) == VK_SUCCESS;
}

bool TestGraphicsContext_VK::renderToCameraTarget(TestCameraRenderTarget* cameraRenderTarget)
{
	auto* vkRenderTarget= static_cast<TestCameraRenderTarget_VK*>(cameraRenderTarget);
	if (!vkRenderTarget->getIsInitialized())
		return false;

	// Depth publishes to Mikan on every frame; only the window's depth preview modes (keys 2 and 3)
	// are absent on this path, so they show color
	if (m_ownerApp->getRenderMode() != TestRenderMode::Color && !m_bWarnedUnsupportedRenderMode)
	{
		MIKAN_LOG_WARNING("renderToCameraTarget")
			<< "The depth preview modes are not implemented on the Vulkan path; the window shows color";
		m_bWarnedUnsupportedRenderMode= true;
	}

	if (!waitForFence(m_cameraFence) || !beginCommands(m_cameraCommandBuffer, m_cameraFence))
		return false;

	const VkExtent2D extent= vkRenderTarget->getExtent();
	const VkImage colorImage= vkRenderTarget->getColorImage();

	// Into attachment layouts; the previous contents are discarded
	VkImageMemoryBarrier toAttachments[2]= {
		makeImageBarrier(colorImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
						 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT),
		makeImageBarrier(vkRenderTarget->getDepthImage(), VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
						 VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),
	};
	vkCmdPipelineBarrier(m_cameraCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0,
						 0, nullptr, 0, nullptr, 2, toAttachments);

	VkRenderingAttachmentInfo colorAttachment= {};
	colorAttachment.sType= VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView= vkRenderTarget->getColorImageView();
	colorAttachment.imageLayout= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp= VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue.color= {{0.0f, 0.0f, 0.0f, 0.0f}};
	VkRenderingAttachmentInfo depthAttachment= {};
	depthAttachment.sType= VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachment.imageView= vkRenderTarget->getDepthImageView();
	depthAttachment.imageLayout= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	depthAttachment.loadOp= VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.clearValue.depthStencil= {1.0f, 0};
	VkRenderingInfo renderingInfo= {};
	renderingInfo.sType= VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea= {{0, 0}, extent};
	renderingInfo.layerCount= 1;
	renderingInfo.colorAttachmentCount= 1;
	renderingInfo.pColorAttachments= &colorAttachment;
	renderingInfo.pDepthAttachment= &depthAttachment;
	vkCmdBeginRendering(m_cameraCommandBuffer, &renderingInfo);

	VkViewport viewport= {0.0f, 0.0f, (float)extent.width, (float)extent.height, 0.0f, 1.0f};
	VkRect2D scissor= {{0, 0}, extent};
	vkCmdSetViewport(m_cameraCommandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(m_cameraCommandBuffer, 0, 1, &scissor);
	vkCmdBindPipeline(m_cameraCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_cubePipeline);
	const VkDeviceSize vertexOffset= 0;
	vkCmdBindVertexBuffers(m_cameraCommandBuffer, 0, 1, &m_cubeVertexBuffer, &vertexOffset);

	renderCube(m_cameraCommandBuffer, vkRenderTarget->getViewProjectionMatrix(), vkRenderTarget->getCameraPosition(),
			   vkRenderTarget->getCameraForward(), vkRenderTarget->getCameraUp(), vkRenderTarget->getCameraRight());

	vkCmdEndRendering(m_cameraCommandBuffer);

	// Device depth for Mikan: a depth image cannot be copied to a color format, but its depth aspect
	// copies to a buffer as 32-bit floats and the buffer copies into an R32 color image
	const VkImage depthImage= vkRenderTarget->getDepthImage();
	const VkImage depthColorImage= vkRenderTarget->getDepthColorImage();
	const VkBuffer depthTransferBuffer= vkRenderTarget->getDepthTransferBuffer();
	VkImageMemoryBarrier depthToTransfer[2]= {
		makeImageBarrier(depthImage, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
						 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
						 VK_ACCESS_TRANSFER_READ_BIT),
		makeImageBarrier(depthColorImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
						 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT),
	};
	vkCmdPipelineBarrier(m_cameraCommandBuffer, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
						 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2, depthToTransfer);

	VkBufferImageCopy depthCopy= {};
	depthCopy.imageSubresource.aspectMask= VK_IMAGE_ASPECT_DEPTH_BIT;
	depthCopy.imageSubresource.layerCount= 1;
	depthCopy.imageExtent= {extent.width, extent.height, 1};
	vkCmdCopyImageToBuffer(m_cameraCommandBuffer, depthImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, depthTransferBuffer,
						   1, &depthCopy);

	VkBufferMemoryBarrier depthBufferWritten= {};
	depthBufferWritten.sType= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	depthBufferWritten.srcAccessMask= VK_ACCESS_TRANSFER_WRITE_BIT;
	depthBufferWritten.dstAccessMask= VK_ACCESS_TRANSFER_READ_BIT;
	depthBufferWritten.srcQueueFamilyIndex= VK_QUEUE_FAMILY_IGNORED;
	depthBufferWritten.dstQueueFamilyIndex= VK_QUEUE_FAMILY_IGNORED;
	depthBufferWritten.buffer= depthTransferBuffer;
	depthBufferWritten.size= VK_WHOLE_SIZE;
	vkCmdPipelineBarrier(m_cameraCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
						 nullptr, 1, &depthBufferWritten, 0, nullptr);

	VkBufferImageCopy depthColorCopy= depthCopy;
	depthColorCopy.imageSubresource.aspectMask= VK_IMAGE_ASPECT_COLOR_BIT;
	vkCmdCopyBufferToImage(m_cameraCommandBuffer, depthTransferBuffer, depthColorImage,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &depthColorCopy);

	VkImageMemoryBarrier depthColorToSource= makeImageBarrier(
		depthColorImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
	vkCmdPipelineBarrier(m_cameraCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
						 nullptr, 0, nullptr, 1, &depthColorToSource);

	// Leave the color target in the layout Mikan's writer and the window blit read it in
	VkImageMemoryBarrier toTransferSource= makeImageBarrier(
		colorImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
	vkCmdPipelineBarrier(m_cameraCommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toTransferSource);

	if (!submitCommands(m_cameraCommandBuffer, m_cameraFence, VK_NULL_HANDLE, 0, VK_NULL_HANDLE))
	{
		MIKAN_LOG_ERROR("renderToCameraTarget") << "Failed to submit the camera pass";
		return false;
	}

	// Remember the last rendered camera ID
	// We will render this camera in renderMainTarget
	m_lastRenderedCameraId= cameraRenderTarget->getCameraId();

	return true;
}

void TestGraphicsContext_VK::renderCube(VkCommandBuffer commandBuffer, const glm::mat4& viewProj,
										const glm::vec3& cameraPosition, const glm::vec3& cameraForward,
										const glm::vec3& cameraUp, const glm::vec3& cameraRight) const
{
	const float time= m_ownerApp->getTimeSeconds();
	const MikanVector3f& cubeOffset= m_ownerApp->getCubeOffset();

	// Compute cube position
	const glm::vec3 cubePosition=
		cameraPosition + cameraForward * cubeOffset.z + cameraUp * cubeOffset.y + cameraRight * cubeOffset.x;

	// Build cube transformation matrix
	const glm::mat4 cubeXform= glm::translate(glm::mat4(1.0f), cubePosition)
							   * glm::rotate(glm::mat4(1.0f), time * 0.7f, glm::vec3(0.0f, 0.0f, 1.0f))
							   * glm::rotate(glm::mat4(1.0f), time * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f))
							   * glm::rotate(glm::mat4(1.0f), time, glm::vec3(1.0f, 0.0f, 0.0f))
							   * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
	const glm::mat4 worldViewProj= viewProj * cubeXform;

	vkCmdPushConstants(commandBuffer, m_cubePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4),
					   &worldViewProj);
	vkCmdDraw(commandBuffer, m_cubeVertexCount, 1, 0, 0);
}

void TestGraphicsContext_VK::renderMainTarget() const
{
	if (m_swapchain == VK_NULL_HANDLE)
		return;

	// The base class declares this const while a present is a mutation of the swapchain state,
	// so the out-of-date recovery casts that away in this one place
	if (m_bSwapchainDirty)
	{
		const_cast<TestGraphicsContext_VK*>(this)->recreateMainRenderTarget();
		if (m_swapchain == VK_NULL_HANDLE)
			return;
	}

	if (!waitForFence(m_presentFence))
		return;

	uint32_t imageIndex= 0;
	VkResult result= vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE,
										   &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		m_bSwapchainDirty= true;
		return;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		return;

	if (!beginCommands(m_presentCommandBuffer, m_presentFence))
		return;

	const VkImage swapchainImage= m_swapchainImages[imageIndex];
	VkImageMemoryBarrier toTransferDestination=
		makeImageBarrier(swapchainImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
						 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
	vkCmdPipelineBarrier(m_presentCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
						 0, nullptr, 0, nullptr, 1, &toTransferDestination);

	// Draw the most recently rendered camera texture to the back buffer, or clear to blue without one
	TestCameraRenderTargetPtr renderTarget= getCameraRenderTarget(m_lastRenderedCameraId);
	auto vkRenderTarget= std::static_pointer_cast<TestCameraRenderTarget_VK>(renderTarget);
	if (vkRenderTarget && vkRenderTarget->getIsInitialized())
	{
		// The camera pass (and Mikan's copy of it) left the image in TRANSFER_SRC; make its writes visible
		VkImageMemoryBarrier cameraReady= makeImageBarrier(
			vkRenderTarget->getColorImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
		vkCmdPipelineBarrier(m_presentCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
							 0, 0, nullptr, 0, nullptr, 1, &cameraReady);

		const VkExtent2D cameraExtent= vkRenderTarget->getExtent();
		VkImageBlit region= {};
		region.srcSubresource.aspectMask= VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.layerCount= 1;
		region.srcOffsets[1]= {(int32_t)cameraExtent.width, (int32_t)cameraExtent.height, 1};
		region.dstSubresource= region.srcSubresource;
		region.dstOffsets[1]= {(int32_t)m_swapchainExtent.width, (int32_t)m_swapchainExtent.height, 1};
		vkCmdBlitImage(m_presentCommandBuffer, vkRenderTarget->getColorImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					   swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);
	}
	else
	{
		const VkClearColorValue clearColor= {{0.0f, 0.0f, 1.0f, 1.0f}};
		VkImageSubresourceRange range= {};
		range.aspectMask= VK_IMAGE_ASPECT_COLOR_BIT;
		range.levelCount= 1;
		range.layerCount= 1;
		vkCmdClearColorImage(m_presentCommandBuffer, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor,
							 1, &range);
	}

	VkImageMemoryBarrier toPresent=
		makeImageBarrier(swapchainImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_TRANSFER_WRITE_BIT, 0);
	vkCmdPipelineBarrier(m_presentCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
						 0, 0, nullptr, 0, nullptr, 1, &toPresent);

	const VkSemaphore renderFinished= m_renderFinishedSemaphores[imageIndex];
	if (!submitCommands(m_presentCommandBuffer, m_presentFence, m_imageAvailableSemaphore,
						VK_PIPELINE_STAGE_TRANSFER_BIT, renderFinished))
	{
		MIKAN_LOG_ERROR("renderMainTarget") << "Failed to submit the present pass";
		return;
	}

	VkPresentInfoKHR presentInfo= {};
	presentInfo.sType= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount= 1;
	presentInfo.pWaitSemaphores= &renderFinished;
	presentInfo.swapchainCount= 1;
	presentInfo.pSwapchains= &m_swapchain;
	presentInfo.pImageIndices= &imageIndex;
	result= vkQueuePresentKHR(m_queue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		m_bSwapchainDirty= true;
	}
}

bool TestGraphicsContext_VK::readCameraTargetPixels(TestCameraRenderTarget* cameraRenderTarget,
													std::vector<uint8_t>& outRgbaPixels, int& outWidth, int& outHeight)
{
	auto* vkRenderTarget= static_cast<TestCameraRenderTarget_VK*>(cameraRenderTarget);
	if (!vkRenderTarget->getIsInitialized())
		return false;

	const VkExtent2D extent= vkRenderTarget->getExtent();
	const VkDeviceSize byteCount= (VkDeviceSize)extent.width * extent.height * 4;

	// A host-visible buffer the camera image copies into; the image already sits in TRANSFER_SRC
	VkBuffer readbackBuffer= VK_NULL_HANDLE;
	VkDeviceMemory readbackMemory= VK_NULL_HANDLE;
	bool bSuccess= createBuffer(byteCount, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
								VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								readbackBuffer, readbackMemory);

	bSuccess= bSuccess && waitForFence(m_cameraFence) && beginCommands(m_cameraCommandBuffer, m_cameraFence);
	if (bSuccess)
	{
		VkImageMemoryBarrier cameraReady= makeImageBarrier(
			vkRenderTarget->getColorImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
		vkCmdPipelineBarrier(m_cameraCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
							 0, 0, nullptr, 0, nullptr, 1, &cameraReady);

		VkBufferImageCopy region= {};
		region.imageSubresource.aspectMask= VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.layerCount= 1;
		region.imageExtent= {extent.width, extent.height, 1};
		vkCmdCopyImageToBuffer(m_cameraCommandBuffer, vkRenderTarget->getColorImage(),
							   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readbackBuffer, 1, &region);

		bSuccess= submitCommands(m_cameraCommandBuffer, m_cameraFence, VK_NULL_HANDLE, 0, VK_NULL_HANDLE)
				  && waitForFence(m_cameraFence);
	}

	void* mapped= nullptr;
	if (bSuccess && vkMapMemory(m_device, readbackMemory, 0, byteCount, 0, &mapped) == VK_SUCCESS)
	{
		outWidth= (int)extent.width;
		outHeight= (int)extent.height;
		outRgbaPixels.resize((size_t)byteCount);

		// The target is BGRA; swizzle to RGBA
		const uint8_t* source= (const uint8_t*)mapped;
		for (size_t pixelIndex= 0; pixelIndex < (size_t)extent.width * extent.height; ++pixelIndex)
		{
			outRgbaPixels[pixelIndex * 4 + 0]= source[pixelIndex * 4 + 2];
			outRgbaPixels[pixelIndex * 4 + 1]= source[pixelIndex * 4 + 1];
			outRgbaPixels[pixelIndex * 4 + 2]= source[pixelIndex * 4 + 0];
			outRgbaPixels[pixelIndex * 4 + 3]= source[pixelIndex * 4 + 3];
		}
		vkUnmapMemory(m_device, readbackMemory);
	}
	else
	{
		bSuccess= false;
	}

	if (readbackBuffer != VK_NULL_HANDLE)
		vkDestroyBuffer(m_device, readbackBuffer, nullptr);
	if (readbackMemory != VK_NULL_HANDLE)
		vkFreeMemory(m_device, readbackMemory, nullptr);

	return bSuccess;
}

void TestGraphicsContext_VK::dispose()
{
	if (m_device != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle(m_device);

		if (m_cubePipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(m_device, m_cubePipeline, nullptr);
			m_cubePipeline= VK_NULL_HANDLE;
		}
		if (m_cubePipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(m_device, m_cubePipelineLayout, nullptr);
			m_cubePipelineLayout= VK_NULL_HANDLE;
		}
		if (m_cubeVertexBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_device, m_cubeVertexBuffer, nullptr);
			m_cubeVertexBuffer= VK_NULL_HANDLE;
		}
		if (m_cubeVertexMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_device, m_cubeVertexMemory, nullptr);
			m_cubeVertexMemory= VK_NULL_HANDLE;
		}

		destroySwapchain();
		destroyCommandState();

		vkDestroyDevice(m_device, nullptr);
		m_device= VK_NULL_HANDLE;
		m_queue= VK_NULL_HANDLE;
		m_deviceInterface= {};
	}

	if (m_instance != VK_NULL_HANDLE)
	{
		if (m_surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
			m_surface= VK_NULL_HANDLE;
		}
		if (m_debugMessenger != VK_NULL_HANDLE)
		{
			vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
			m_debugMessenger= VK_NULL_HANDLE;
		}
		vkDestroyInstance(m_instance, nullptr);
		m_instance= VK_NULL_HANDLE;
	}

	if (m_sdlWindow != nullptr)
	{
		SDL_DestroyWindow(m_sdlWindow);
		m_sdlWindow= nullptr;
		SDL_Vulkan_UnloadLibrary();
	}
}
