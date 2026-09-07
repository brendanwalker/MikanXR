#include "SharedTextureWriterBackend.h"
#include "SharedTextureLogger.h"
#include "SpoutDX.h"
#include "SpoutDXDepthTexturePacker.h"

// After the Spout headers: with VK_USE_PLATFORM_WIN32_KHR defined, volk pulls in
// vulkan_win32.h, which needs the windows.h those headers already included.
#include "volk.h"

#include <cstring>
#include <sstream>
#include <string>

// -- SpoutVulkanTextureWriter -----
// A Vulkan client cannot be a Spout sender directly, so each buffer is a D3D11 shared texture
// (the kind Spout already sends) whose memory is imported into the client's VkDevice as a
// "linked" image. A write blits the client's image into the linked image on the client's
// queue, waits the submission's fence, then hands the D3D11 side of the same memory to Spout.
// The fence wait is the synchronization between the two APIs. Both sides must sit on one
// GPU, so the D3D11 device is created on the adapter whose LUID the VkPhysicalDevice reports.
class SpoutVulkanTextureWriter : public ISharedTextureWriterBackend
{
public:
	SpoutVulkanTextureWriter(const SharedTextureWriterContext& context)
		: m_context(context)
		, m_logger(*context.logger)
		, m_deviceTable()
	{
	}

	virtual ~SpoutVulkanTextureWriter() { dispose(); }

	bool init() override
	{
		const SharedTextureDescriptor* descriptor= m_context.descriptor;
		const auto* vulkanInterface= (const SharedVulkanDeviceInterface*)m_context.apiDeviceInterface;

		dispose();

		applySpoutDXLogPolicy();

		if (vulkanInterface == nullptr || vulkanInterface->instance == nullptr
			|| vulkanInterface->physicalDevice == nullptr || vulkanInterface->device == nullptr
			|| vulkanInterface->queue == nullptr)
		{
			m_logger.log(SharedTextureLogLevel::error,
						 "SpoutVulkanTextureWriter::init() - Missing Vulkan device interface "
						 "(instance, physical device, device, and queue are all required)");
			return false;
		}

		m_instance= (VkInstance)vulkanInterface->instance;
		m_physicalDevice= (VkPhysicalDevice)vulkanInterface->physicalDevice;
		m_device= (VkDevice)vulkanInterface->device;
		m_queue= (VkQueue)vulkanInterface->queue;
		m_queueFamilyIndex= vulkanInterface->queueFamilyIndex;

		if (!loadVulkan() || !createCommandState() || !openDirectX11())
		{
			dispose();
			return false;
		}

		// The color spout frame owns the D3D11 class device the other frames and every linked texture share
		DXGI_FORMAT dxgiFormat= DXGI_FORMAT_UNKNOWN;
		VkFormat vkFormat= VK_FORMAT_UNDEFINED;
		if (colorBufferFormats(descriptor->color_buffer_type, dxgiFormat, vkFormat))
		{
			if (!initSpoutFrame(m_spoutColorFrame, m_context.colorSenderName, dxgiFormat)
				|| !createLinkedTexture(m_colorTexture, descriptor->width, descriptor->height, dxgiFormat, vkFormat))
			{
				m_logger.log(SharedTextureLogLevel::error,
							 "SpoutVulkanTextureWriter::init() - Error initializing color spout frame");
				dispose();
				return false;
			}

			m_bIsColorFrameInitialized= true;
		}
		else
		{
			std::stringstream ss;
			ss << "SpoutVulkanTextureWriter::init() - color buffer type not supported: ";
			ss << (int)descriptor->color_buffer_type;
			m_logger.log(SharedTextureLogLevel::error, ss.str());
			dispose();
			return false;
		}

		// Depth: a float depth buffer arrives as an R32 color image and is packed to RGBA8 on the
		// D3D11 side like the DX11 writer's, and an already packed one arrives as RGBA8
		if (descriptor->depth_buffer_type != SharedDepthBufferType::NODEPTH)
		{
			const bool bFloatDepth= descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH
									|| descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH;

			if (!initSpoutFrame(m_spoutDepthFrame, m_context.depthSenderName, DXGI_FORMAT_R8G8B8A8_UNORM))
			{
				m_logger.log(SharedTextureLogLevel::error,
							 "SpoutVulkanTextureWriter::init() - Error initializing depth spout frame");
				dispose();
				return false;
			}

			if (bFloatDepth)
			{
				m_depthTexturePacker= new SpoutDXDepthTexturePacker(m_logger, m_spoutDepthFrame, descriptor);
				if (!m_depthTexturePacker->init())
				{
					m_logger.log(SharedTextureLogLevel::error,
								 "SpoutVulkanTextureWriter::init() - Error initializing float depth packer");
					dispose();
					return false;
				}
			}

			const DXGI_FORMAT depthDxgiFormat= bFloatDepth ? DXGI_FORMAT_R32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
			const VkFormat depthVkFormat= bFloatDepth ? VK_FORMAT_R32_SFLOAT : VK_FORMAT_R8G8B8A8_UNORM;
			if (!createLinkedTexture(m_depthTexture, descriptor->width, descriptor->height, depthDxgiFormat,
									 depthVkFormat))
			{
				dispose();
				return false;
			}

			m_bIsDepthFrameInitialized= true;
		}

		// Shadow: a second color-like buffer
		if (descriptor->shadow_buffer_type != SharedShadowBufferType::NOSHADOW)
		{
			if (!shadowBufferFormats(descriptor->shadow_buffer_type, dxgiFormat, vkFormat)
				|| !initSpoutFrame(m_spoutShadowFrame, m_context.shadowSenderName, dxgiFormat)
				|| !createLinkedTexture(m_shadowTexture, descriptor->width, descriptor->height, dxgiFormat, vkFormat))
			{
				m_logger.log(SharedTextureLogLevel::error,
							 "SpoutVulkanTextureWriter::init() - Error initializing shadow spout frame");
				dispose();
				return false;
			}

			m_bIsShadowFrameInitialized= true;
		}

		return true;
	}

	void dispose()
	{
		if (m_depthTexturePacker != nullptr)
		{
			delete m_depthTexturePacker;
			m_depthTexturePacker= nullptr;
		}

		// The linked textures and the command state live on the client's device, which outlives this writer
		releaseLinkedTexture(m_colorTexture);
		releaseLinkedTexture(m_depthTexture);
		releaseLinkedTexture(m_shadowTexture);
		destroyCommandState();

		// The depth and shadow frames borrow the color frame's class device, so the color frame closes last
		m_spoutDepthFrame.ReleaseSender();
		m_spoutDepthFrame.CloseDirectX11();
		m_bIsDepthFrameInitialized= false;

		m_spoutShadowFrame.ReleaseSender();
		m_spoutShadowFrame.CloseDirectX11();
		m_bIsShadowFrameInitialized= false;

		m_spoutColorFrame.ReleaseSender();
		m_spoutColorFrame.CloseDirectX11();
		m_bIsColorFrameInitialized= false;

		m_instance= VK_NULL_HANDLE;
		m_physicalDevice= VK_NULL_HANDLE;
		m_device= VK_NULL_HANDLE;
		m_queue= VK_NULL_HANDLE;
		m_queueFamilyIndex= 0;

		DisableSpoutLog();
	}

	bool writeColorFrameTexture(void* apiTexturePtr) override
	{
		const SharedVulkanTexture* texture= (const SharedVulkanTexture*)apiTexturePtr;

		if (!m_bIsColorFrameInitialized || !blitToLinkedTexture(texture, m_colorTexture))
			return false;

		return m_spoutColorFrame.SendTexture(m_colorTexture.d3d11Texture);
	}

	bool writeDepthFrameTexture(void* apiTexturePtr, float zNear, float zFar) override
	{
		const SharedVulkanTexture* texture= (const SharedVulkanTexture*)apiTexturePtr;

		if (!m_bIsDepthFrameInitialized || !blitToLinkedTexture(texture, m_depthTexture))
			return false;

		if (m_depthTexturePacker != nullptr)
		{
			ID3D11Texture2D* packedDepthTexture=
				m_depthTexturePacker->packDepthTexture(m_depthTexture.d3d11Texture, zNear, zFar);

			return packedDepthTexture != nullptr && m_spoutDepthFrame.SendTexture(packedDepthTexture);
		}

		return m_spoutDepthFrame.SendTexture(m_depthTexture.d3d11Texture);
	}

	bool writeShadowFrameTexture(void* apiTexturePtr) override
	{
		const SharedVulkanTexture* texture= (const SharedVulkanTexture*)apiTexturePtr;

		if (!m_bIsShadowFrameInitialized || !blitToLinkedTexture(texture, m_shadowTexture))
			return false;

		return m_spoutShadowFrame.SendTexture(m_shadowTexture.d3d11Texture);
	}

	// The packed depth texture is a D3D11 resource on the writer's own device, which a Vulkan client cannot read
	void* getPackDepthTextureResourcePtr() const override { return nullptr; }

private:
	// One buffer's shared memory: the D3D11 side Spout sends and the Vulkan side the client's blit fills
	struct LinkedTexture
	{
		ID3D11Texture2D* d3d11Texture= nullptr;
		HANDLE shareHandle= nullptr;
		VkImage image= VK_NULL_HANDLE;
		VkDeviceMemory memory= VK_NULL_HANDLE;
		DXGI_FORMAT dxgiFormat= DXGI_FORMAT_UNKNOWN;
		VkFormat format= VK_FORMAT_UNDEFINED;
		uint32_t width= 0;
		uint32_t height= 0;
	};

	static bool colorBufferFormats(SharedColorBufferType type, DXGI_FORMAT& outDxgiFormat, VkFormat& outVkFormat)
	{
		switch (type)
		{
		case SharedColorBufferType::RGBA32:
			outDxgiFormat= DXGI_FORMAT_R8G8B8A8_UNORM;
			outVkFormat= VK_FORMAT_R8G8B8A8_UNORM;
			return true;
		case SharedColorBufferType::BGRA32:
			outDxgiFormat= DXGI_FORMAT_B8G8R8A8_UNORM;
			outVkFormat= VK_FORMAT_B8G8R8A8_UNORM;
			return true;
		case SharedColorBufferType::RGBA16F:
			outDxgiFormat= DXGI_FORMAT_R16G16B16A16_FLOAT;
			outVkFormat= VK_FORMAT_R16G16B16A16_SFLOAT;
			return true;
		default:
			return false;
		}
	}

	static bool shadowBufferFormats(SharedShadowBufferType type, DXGI_FORMAT& outDxgiFormat, VkFormat& outVkFormat)
	{
		switch (type)
		{
		case SharedShadowBufferType::RGBA32:
			return colorBufferFormats(SharedColorBufferType::RGBA32, outDxgiFormat, outVkFormat);
		case SharedShadowBufferType::BGRA32:
			return colorBufferFormats(SharedColorBufferType::BGRA32, outDxgiFormat, outVkFormat);
		case SharedShadowBufferType::RGBA16F:
			return colorBufferFormats(SharedColorBufferType::RGBA16F, outDxgiFormat, outVkFormat);
		default:
			return false;
		}
	}

	void logVulkanFailure(const char* what, VkResult result)
	{
		std::stringstream ss;
		ss << "SpoutVulkanTextureWriter - " << what << " failed (VkResult " << (int)result << ")";
		m_logger.log(SharedTextureLogLevel::error, ss.str());
	}

	// volk resolves the loader once per process and the instance entry points into its globals;
	// device entry points go into this writer's own table so two writers on two devices coexist.
	bool loadVulkan()
	{
		static VkResult s_loaderResult= VK_RESULT_MAX_ENUM;
		if (s_loaderResult == VK_RESULT_MAX_ENUM)
			s_loaderResult= volkInitialize();

		if (s_loaderResult != VK_SUCCESS)
		{
			logVulkanFailure("loading vulkan-1.dll", s_loaderResult);
			return false;
		}

		volkLoadInstanceOnly(m_instance);
		volkLoadDeviceTable(&m_deviceTable, m_device);

		// The import path is Vulkan 1.1 core (external memory, dedicated allocation, memory requirements 2)
		if (vkGetPhysicalDeviceProperties2 == nullptr || vkGetPhysicalDeviceImageFormatProperties2 == nullptr
			|| vkGetPhysicalDeviceMemoryProperties == nullptr || m_deviceTable.vkGetImageMemoryRequirements2 == nullptr
			|| m_deviceTable.vkCmdBlitImage == nullptr)
		{
			m_logger.log(SharedTextureLogLevel::error,
						 "SpoutVulkanTextureWriter - The Vulkan instance and device must be at least version 1.1");
			return false;
		}

		return true;
	}

	bool createCommandState()
	{
		VkCommandPoolCreateInfo poolInfo= {};
		poolInfo.sType= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex= m_queueFamilyIndex;
		VkResult result= m_deviceTable.vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
		if (result != VK_SUCCESS)
		{
			logVulkanFailure("vkCreateCommandPool", result);
			return false;
		}

		VkCommandBufferAllocateInfo allocInfo= {};
		allocInfo.sType= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool= m_commandPool;
		allocInfo.level= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount= 1;
		result= m_deviceTable.vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer);
		if (result != VK_SUCCESS)
		{
			logVulkanFailure("vkAllocateCommandBuffers", result);
			return false;
		}

		VkFenceCreateInfo fenceInfo= {};
		fenceInfo.sType= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		result= m_deviceTable.vkCreateFence(m_device, &fenceInfo, nullptr, &m_fence);
		if (result != VK_SUCCESS)
		{
			logVulkanFailure("vkCreateFence", result);
			return false;
		}

		return true;
	}

	void destroyCommandState()
	{
		if (m_device == VK_NULL_HANDLE)
			return;

		if (m_fence != VK_NULL_HANDLE)
		{
			m_deviceTable.vkDestroyFence(m_device, m_fence, nullptr);
			m_fence= VK_NULL_HANDLE;
		}

		// Destroying the pool frees its command buffer
		if (m_commandPool != VK_NULL_HANDLE)
		{
			m_deviceTable.vkDestroyCommandPool(m_device, m_commandPool, nullptr);
			m_commandPool= VK_NULL_HANDLE;
		}
		m_commandBuffer= VK_NULL_HANDLE;
	}

	// Create the D3D11 class device on the adapter the Vulkan physical device lives on, then
	// share it with the depth and shadow frames
	bool openDirectX11()
	{
		VkPhysicalDeviceIDProperties idProperties= {};
		idProperties.sType= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
		VkPhysicalDeviceProperties2 properties= {};
		properties.sType= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		properties.pNext= &idProperties;
		vkGetPhysicalDeviceProperties2(m_physicalDevice, &properties);

		spoutDirectX& spoutdx= m_spoutColorFrame.spoutdx;
		bool bMatchedAdapter= false;
		if (idProperties.deviceLUIDValid)
		{
			const int adapterCount= spoutdx.GetNumAdapters();
			for (int adapterIndex= 0; adapterIndex < adapterCount && !bMatchedAdapter; ++adapterIndex)
			{
				IDXGIAdapter* adapter= spoutdx.GetAdapterPointer(adapterIndex);
				if (adapter == nullptr)
					continue;

				DXGI_ADAPTER_DESC adapterDesc= {};
				const HRESULT hr= adapter->GetDesc(&adapterDesc);
				adapter->Release();

				static_assert(sizeof(LUID) == VK_LUID_SIZE, "LUID size mismatch");
				if (SUCCEEDED(hr) && std::memcmp(&adapterDesc.AdapterLuid, idProperties.deviceLUID, VK_LUID_SIZE) == 0)
				{
					bMatchedAdapter= spoutdx.SetAdapter(adapterIndex);
				}
			}
		}

		if (!bMatchedAdapter)
		{
			m_logger.log(SharedTextureLogLevel::info,
						 "SpoutVulkanTextureWriter::openDirectX11() - No DXGI adapter matches the Vulkan device LUID, "
						 "using the default adapter");
		}

		if (!m_spoutColorFrame.OpenDirectX11(nullptr) || m_spoutColorFrame.GetDX11Device() == nullptr)
		{
			m_logger.log(SharedTextureLogLevel::error,
						 "SpoutVulkanTextureWriter::openDirectX11() - Failed to create the D3D11 device");
			return false;
		}

		return true;
	}

	bool initSpoutFrame(spoutDX& frame, const std::string& senderName, DXGI_FORMAT senderFormat)
	{
		// The color frame already holds the class device; the others open on the same device
		ID3D11Device* d3d11Device= m_spoutColorFrame.GetDX11Device();
		if (&frame != &m_spoutColorFrame && !frame.OpenDirectX11(d3d11Device))
			return false;

		if (!frame.SetSenderName(senderName.c_str()))
			return false;

		frame.SetSenderFormat(senderFormat);

		if (!m_context.bEnableFrameCounter)
			frame.DisableFrameCount();

		return true;
	}

	bool findMemoryType(uint32_t typeBits, uint32_t& outMemoryType) const
	{
		VkPhysicalDeviceMemoryProperties memoryProperties= {};
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

		// Prefer device-local memory, and otherwise take any type the image allows
		bool bFound= false;
		for (uint32_t typeIndex= 0; typeIndex < memoryProperties.memoryTypeCount; ++typeIndex)
		{
			if ((typeBits & (1u << typeIndex)) == 0)
				continue;

			const bool bDeviceLocal=
				(memoryProperties.memoryTypes[typeIndex].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0;
			if (!bFound || bDeviceLocal)
			{
				outMemoryType= typeIndex;
				bFound= true;
			}
			if (bDeviceLocal)
				break;
		}

		return bFound;
	}

	bool createLinkedTexture(LinkedTexture& linked, uint32_t width, uint32_t height, DXGI_FORMAT dxgiFormat,
							 VkFormat vkFormat)
	{
		releaseLinkedTexture(linked);

		linked.dxgiFormat= dxgiFormat;
		linked.format= vkFormat;
		linked.width= width;
		linked.height= height;

		// The D3D11 side: a MISC_SHARED texture with a legacy KMT share handle, the only kind
		// a Spout sender can carry
		ID3D11Device* d3d11Device= m_spoutColorFrame.GetDX11Device();
		if (!m_spoutColorFrame.spoutdx.CreateSharedDX11Texture(d3d11Device, width, height, dxgiFormat,
															   &linked.d3d11Texture, linked.shareHandle))
		{
			m_logger.log(SharedTextureLogLevel::error,
						 "SpoutVulkanTextureWriter::createLinkedTexture() - Failed to create the shared D3D11 texture");
			releaseLinkedTexture(linked);
			return false;
		}

		// The Vulkan side has to be able to import that handle for this format and usage
		VkPhysicalDeviceExternalImageFormatInfo externalFormatInfo= {};
		externalFormatInfo.sType= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
		externalFormatInfo.handleType= VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT;
		VkPhysicalDeviceImageFormatInfo2 formatInfo= {};
		formatInfo.sType= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
		formatInfo.pNext= &externalFormatInfo;
		formatInfo.format= vkFormat;
		formatInfo.type= VK_IMAGE_TYPE_2D;
		formatInfo.tiling= VK_IMAGE_TILING_OPTIMAL;
		formatInfo.usage= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		VkExternalImageFormatProperties externalFormatProperties= {};
		externalFormatProperties.sType= VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;
		VkImageFormatProperties2 formatProperties= {};
		formatProperties.sType= VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
		formatProperties.pNext= &externalFormatProperties;
		VkResult result= vkGetPhysicalDeviceImageFormatProperties2(m_physicalDevice, &formatInfo, &formatProperties);
		const VkExternalMemoryFeatureFlags externalFeatures=
			externalFormatProperties.externalMemoryProperties.externalMemoryFeatures;
		if (result != VK_SUCCESS || (externalFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) == 0)
		{
			std::stringstream ss;
			ss << "SpoutVulkanTextureWriter::createLinkedTexture() - The Vulkan device cannot import a D3D11 texture "
			   << "of VkFormat " << (int)vkFormat << " (VkResult " << (int)result << ")";
			m_logger.log(SharedTextureLogLevel::error, ss.str());
			releaseLinkedTexture(linked);
			return false;
		}

		VkExternalMemoryImageCreateInfo externalImageInfo= {};
		externalImageInfo.sType= VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
		externalImageInfo.handleTypes= VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT;
		VkImageCreateInfo imageInfo= {};
		imageInfo.sType= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext= &externalImageInfo;
		imageInfo.imageType= VK_IMAGE_TYPE_2D;
		imageInfo.format= vkFormat;
		imageInfo.extent= {width, height, 1};
		imageInfo.mipLevels= 1;
		imageInfo.arrayLayers= 1;
		imageInfo.samples= VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling= VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.sharingMode= VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout= VK_IMAGE_LAYOUT_UNDEFINED;
		result= m_deviceTable.vkCreateImage(m_device, &imageInfo, nullptr, &linked.image);
		if (result != VK_SUCCESS)
		{
			logVulkanFailure("vkCreateImage", result);
			releaseLinkedTexture(linked);
			return false;
		}

		VkMemoryDedicatedRequirements dedicatedRequirements= {};
		dedicatedRequirements.sType= VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
		VkMemoryRequirements2 memoryRequirements= {};
		memoryRequirements.sType= VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		memoryRequirements.pNext= &dedicatedRequirements;
		VkImageMemoryRequirementsInfo2 requirementsInfo= {};
		requirementsInfo.sType= VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
		requirementsInfo.image= linked.image;
		m_deviceTable.vkGetImageMemoryRequirements2(m_device, &requirementsInfo, &memoryRequirements);

		uint32_t memoryType= 0;
		if (!findMemoryType(memoryRequirements.memoryRequirements.memoryTypeBits, memoryType))
		{
			m_logger.log(SharedTextureLogLevel::error,
						 "SpoutVulkanTextureWriter::createLinkedTexture() - No memory type accepts the imported image");
			releaseLinkedTexture(linked);
			return false;
		}

		// An imported D3D11 texture is always a dedicated allocation
		VkMemoryDedicatedAllocateInfo dedicatedAllocateInfo= {};
		dedicatedAllocateInfo.sType= VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
		dedicatedAllocateInfo.image= linked.image;
		VkImportMemoryWin32HandleInfoKHR importInfo= {};
		importInfo.sType= VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
		importInfo.pNext= &dedicatedAllocateInfo;
		importInfo.handleType= VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT;
		importInfo.handle= linked.shareHandle;
		VkMemoryAllocateInfo allocateInfo= {};
		allocateInfo.sType= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.pNext= &importInfo;
		allocateInfo.allocationSize= memoryRequirements.memoryRequirements.size;
		allocateInfo.memoryTypeIndex= memoryType;
		result= m_deviceTable.vkAllocateMemory(m_device, &allocateInfo, nullptr, &linked.memory);
		if (result != VK_SUCCESS)
		{
			logVulkanFailure("vkAllocateMemory (D3D11 KMT import)", result);
			releaseLinkedTexture(linked);
			return false;
		}

		result= m_deviceTable.vkBindImageMemory(m_device, linked.image, linked.memory, 0);
		if (result != VK_SUCCESS)
		{
			logVulkanFailure("vkBindImageMemory", result);
			releaseLinkedTexture(linked);
			return false;
		}

		return true;
	}

	void releaseLinkedTexture(LinkedTexture& linked)
	{
		if (m_device != VK_NULL_HANDLE)
		{
			if (linked.image != VK_NULL_HANDLE)
				m_deviceTable.vkDestroyImage(m_device, linked.image, nullptr);
			if (linked.memory != VK_NULL_HANDLE)
				m_deviceTable.vkFreeMemory(m_device, linked.memory, nullptr);
		}
		linked.image= VK_NULL_HANDLE;
		linked.memory= VK_NULL_HANDLE;

		if (linked.d3d11Texture != nullptr)
		{
			linked.d3d11Texture->Release();
			linked.d3d11Texture= nullptr;
		}
		linked.shareHandle= nullptr;
		linked.width= 0;
		linked.height= 0;
	}

	// Copy the client's image into the linked image on the client's queue and wait for it.
	// The image is read in the layout the client names and returned to it; a layout that cannot
	// be returned to (undefined, preinitialized) is returned as general.
	bool blitToLinkedTexture(const SharedVulkanTexture* texture, LinkedTexture& linked)
	{
		if (texture == nullptr || texture->image == nullptr || texture->width == 0 || texture->height == 0)
			return false;

		// An external memory binding cannot be resized, so a size change re-links
		if (linked.width != texture->width || linked.height != texture->height)
		{
			if (!createLinkedTexture(linked, texture->width, texture->height, linked.dxgiFormat, linked.format))
				return false;
		}

		const VkImage sourceImage= (VkImage)texture->image;
		const VkImageLayout sourceLayout= (VkImageLayout)texture->layout;
		const VkImageLayout restoreLayout=
			(sourceLayout == VK_IMAGE_LAYOUT_UNDEFINED || sourceLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
				? VK_IMAGE_LAYOUT_GENERAL
				: sourceLayout;

		VkCommandBufferBeginInfo beginInfo= {};
		beginInfo.sType= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VkResult result= m_deviceTable.vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
		if (result != VK_SUCCESS)
		{
			logVulkanFailure("vkBeginCommandBuffer", result);
			return false;
		}

		VkImageSubresourceRange colorRange= {};
		colorRange.aspectMask= VK_IMAGE_ASPECT_COLOR_BIT;
		colorRange.levelCount= 1;
		colorRange.layerCount= 1;

		// Into the transfer layouts. The source barrier waits on every stage, which also orders
		// this copy after whatever the client submitted to the same queue before the write.
		VkImageMemoryBarrier toTransfer[2]= {};
		toTransfer[0].sType= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		toTransfer[0].srcAccessMask= VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
		toTransfer[0].dstAccessMask= VK_ACCESS_TRANSFER_READ_BIT;
		toTransfer[0].oldLayout= sourceLayout;
		toTransfer[0].newLayout= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		toTransfer[0].srcQueueFamilyIndex= VK_QUEUE_FAMILY_IGNORED;
		toTransfer[0].dstQueueFamilyIndex= VK_QUEUE_FAMILY_IGNORED;
		toTransfer[0].image= sourceImage;
		toTransfer[0].subresourceRange= colorRange;
		toTransfer[1]= toTransfer[0];
		toTransfer[1].srcAccessMask= 0;
		toTransfer[1].dstAccessMask= VK_ACCESS_TRANSFER_WRITE_BIT;
		toTransfer[1].oldLayout= VK_IMAGE_LAYOUT_UNDEFINED;
		toTransfer[1].newLayout= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		toTransfer[1].image= linked.image;
		m_deviceTable.vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
										   VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2, toTransfer);

		VkImageBlit region= {};
		region.srcSubresource.aspectMask= VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.layerCount= 1;
		region.srcOffsets[1]= {(int32_t)texture->width, (int32_t)texture->height, 1};
		region.dstSubresource= region.srcSubresource;
		region.dstOffsets[1]= {(int32_t)linked.width, (int32_t)linked.height, 1};
		m_deviceTable.vkCmdBlitImage(m_commandBuffer, sourceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, linked.image,
									 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_NEAREST);

		// Back out: the client's image to its layout, the linked image to general for the D3D11 side
		VkImageMemoryBarrier fromTransfer[2]= {};
		fromTransfer[0]= toTransfer[0];
		fromTransfer[0].srcAccessMask= VK_ACCESS_TRANSFER_READ_BIT;
		fromTransfer[0].dstAccessMask= VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
		fromTransfer[0].oldLayout= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		fromTransfer[0].newLayout= restoreLayout;
		fromTransfer[1]= toTransfer[1];
		fromTransfer[1].srcAccessMask= VK_ACCESS_TRANSFER_WRITE_BIT;
		fromTransfer[1].dstAccessMask= VK_ACCESS_MEMORY_READ_BIT;
		fromTransfer[1].oldLayout= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		fromTransfer[1].newLayout= VK_IMAGE_LAYOUT_GENERAL;
		m_deviceTable.vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
										   VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 2,
										   fromTransfer);

		result= m_deviceTable.vkEndCommandBuffer(m_commandBuffer);
		if (result != VK_SUCCESS)
		{
			logVulkanFailure("vkEndCommandBuffer", result);
			return false;
		}

		VkSubmitInfo submitInfo= {};
		submitInfo.sType= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount= 1;
		submitInfo.pCommandBuffers= &m_commandBuffer;
		m_deviceTable.vkResetFences(m_device, 1, &m_fence);
		result= m_deviceTable.vkQueueSubmit(m_queue, 1, &submitInfo, m_fence);
		if (result != VK_SUCCESS)
		{
			logVulkanFailure("vkQueueSubmit", result);
			return false;
		}

		result= m_deviceTable.vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);
		if (result != VK_SUCCESS)
		{
			logVulkanFailure("vkWaitForFences", result);
			return false;
		}

		return true;
	}

private:
	const SharedTextureWriterContext& m_context;
	SharedTextureLogger& m_logger;

	VkInstance m_instance= VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice= VK_NULL_HANDLE;
	VkDevice m_device= VK_NULL_HANDLE;
	VkQueue m_queue= VK_NULL_HANDLE;
	uint32_t m_queueFamilyIndex= 0;
	VolkDeviceTable m_deviceTable;
	VkCommandPool m_commandPool= VK_NULL_HANDLE;
	VkCommandBuffer m_commandBuffer= VK_NULL_HANDLE;
	VkFence m_fence= VK_NULL_HANDLE;

	spoutDX m_spoutColorFrame;
	spoutDX m_spoutDepthFrame;
	spoutDX m_spoutShadowFrame;
	LinkedTexture m_colorTexture;
	LinkedTexture m_depthTexture;
	LinkedTexture m_shadowTexture;
	SpoutDXDepthTexturePacker* m_depthTexturePacker= nullptr;
	bool m_bIsColorFrameInitialized= false;
	bool m_bIsDepthFrameInitialized= false;
	bool m_bIsShadowFrameInitialized= false;
};

ISharedTextureWriterBackendPtr createVulkanTextureWriter(const SharedTextureWriterContext& context)
{
	return std::make_unique<SpoutVulkanTextureWriter>(context);
}
