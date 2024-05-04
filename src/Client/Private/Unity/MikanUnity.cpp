// Example low level rendering Unity plugin
#include "MikanUnity.h"
#include "IUnityGraphics.h"
#include "Logger.h"
#include "MikanCoreCAPI.h"
#include "assert.h"

// --------------------------------------------------------------------------
// Include headers for the graphics APIs we support

#if SUPPORT_D3D9
#include <d3d9.h>
#include "IUnityGraphicsD3D9.h"
#endif
#if SUPPORT_D3D11
#include <d3d11.h>
#include "IUnityGraphicsD3D11.h"
#endif
#if SUPPORT_D3D12
#include <d3d12.h>
#include "IUnityGraphicsD3D12.h"
#endif

#if SUPPORT_OPENGLES
#if UNITY_IPHONE
#include <OpenGLES/ES2/gl.h>
#elif UNITY_ANDROID
#include <GLES2/gl2.h>
#endif
#elif SUPPORT_OPENGL
#if UNITY_WIN || UNITY_LINUX
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif
#endif

// --------------------------------------------------------------------------
static IUnityInterfaces* s_UnityInterfaces = nullptr;
static IUnityGraphics* s_Graphics = nullptr;
static UnityGfxRenderer s_DeviceType = kUnityGfxRendererNull;
static class MikanRenderAPI* s_CurrentAPI = nullptr;

// --------------------------------------------------------------------------
class MikanRenderAPI
{
public :
	virtual ~MikanRenderAPI() {}

	// Process general event like initialization, shutdown, device loss/reset etc.
	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces) = 0;
};

#if SUPPORT_OPENGLES
class MikanRenderAPI_OpenGLES : public MikanRenderAPI
{
public:
	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces) override;
};
#endif // #if SUPPORT_OPENGLES

#if SUPPORT_D3D9
class MikanRenderAPI_D3D9 : public MikanRenderAPI
{
public:
	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces) override;
};
#endif // #if SUPPORT_D3D9

#if SUPPORT_D3D11
class MikanRenderAPI_D3D11 : public MikanRenderAPI
{
public:
	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces) override;
};
#endif // #if SUPPORT_D3D11

#if SUPPORT_D3D12
class MikanRenderAPI_D3D12 : public MikanRenderAPI
{
public:
	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces) override;
};
#endif // #if SUPPORT_D3D12

#if SUPPORT_METAL
class MikanRenderAPI_Metal : public MikanRenderAPI
{
public:
	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces) override;
};
#endif // #if SUPPORT_METAL

#if SUPPORT_VULKAN
class MikanRenderAPI_Vulkan : public MikanRenderAPI
{
public:
	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces) override;
};
#endif // #if SUPPORT_VULKAN


// --------------------------------------------------------------------------

// Create a graphics API implementation instance for the given API type.
MikanRenderAPI* CreateRenderAPI(UnityGfxRenderer apiType)
{

	#if SUPPORT_OPENGLES
	if (apiType == kUnityGfxRendererOpenGLES30)
	{
		return new MikanRenderAPI_OpenGLES();
	}
	#endif // if SUPPORT_OPENGLES

	#if SUPPORT_D3D11
	if (apiType == kUnityGfxRendererD3D11)
	{
		return new MikanRenderAPI_D3D11();
	}
	#endif // if SUPPORT_D3D11

	#if SUPPORT_D3D12
	if (apiType == kUnityGfxRendererD3D12)
	{
		return new MikanRenderAPI_D3D12();
	}
	#endif // if SUPPORT_D3D12

	#if SUPPORT_METAL
	if (apiType == kUnityGfxRendererMetal)
	{
		return new MikanRenderAPI_Metal();
	}
	#endif // if SUPPORT_METAL

	#if SUPPORT_VULKAN
	if (apiType == kUnityGfxRendererVulkan)
	{
		return new MikanRenderAPI_Vulkan();
	}
	#endif // if SUPPORT_VULKAN

	// Unknown or unsupported graphics API
	return nullptr;
}


// --------------------------------------------------------------------------
// UnitySetInterfaces

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces * unityInterfaces)
{
	s_UnityInterfaces = unityInterfaces;
	s_Graphics = s_UnityInterfaces->Get<IUnityGraphics>();
	s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

#if SUPPORT_VULKAN
	if (s_Graphics->GetRenderer() == kUnityGfxRendererNull)
	{
		extern void RenderAPI_Vulkan_OnPluginLoad(IUnityInterfaces*);
		RenderAPI_Vulkan_OnPluginLoad(unityInterfaces);
	}
#endif // SUPPORT_VULKAN

	// Run OnGraphicsDeviceEvent(initialize) manually on plugin load
	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);

	if (s_CurrentAPI != nullptr)
	{
		delete s_CurrentAPI;
		s_CurrentAPI = nullptr;
	}
}

// --------------------------------------------------------------------------
// GraphicsDeviceEvent

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	// Create graphics API implementation upon initialization
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		assert(s_CurrentAPI == nullptr);
		s_DeviceType = s_Graphics->GetRenderer();
		s_CurrentAPI = CreateRenderAPI(s_DeviceType);
	}

	// Let the implementation process the device related events
	if (s_CurrentAPI)
	{
		s_CurrentAPI->ProcessDeviceEvent(eventType, s_UnityInterfaces);
	}

	// Cleanup graphics API implementation upon shutdown
	if (eventType == kUnityGfxDeviceEventShutdown)
	{
		if (s_CurrentAPI != nullptr)
		{
			delete s_CurrentAPI;
			s_CurrentAPI = nullptr;
		}
		s_DeviceType = kUnityGfxRendererNull;
	}
}

#if SUPPORT_OPENGLES
void MikanRenderAPI_OpenGLES::ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces)
{
	// Create or release a small dynamic vertex buffer depending on the event type.
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		MIKAN_LOG_INFO("Unity") << "Binding OpenGL Device";
		// No Device handle required for OpenGL
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_OpenGL, nullptr);
	}
	else if (eventType == kUnityGfxDeviceEventShutdown)
	{
		MIKAN_LOG_INFO("Unity") << "Unbinding OpenGL Device";
		// No Device handle required for OpenGL
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_OpenGL, nullptr);
	}
}
#endif // #if SUPPORT_OPENGLES

#if SUPPORT_D3D9
void MikanRenderAPI_D3D9::ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces)
{
	// Create or release a small dynamic vertex buffer depending on the event type.
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		IUnityGraphicsD3D9* d3d9 = s_UnityInterfaces->Get<IUnityGraphicsD3D9>();
		IDirect3DDevice9* d3d9Device = d3d9->GetDevice();

		MIKAN_LOG_INFO("Unity") << "Binding D3D9 Device: 0x" << (void*)d3d9Device;
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_Direct3D9, d3d9Device);
	}
	else if (eventType == kUnityGfxDeviceEventShutdown)
	{
		MIKAN_LOG_INFO("Unity") << "Unbinding D3D9 Device";
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_Direct3D9, nullptr);
	}
}
#endif // #if SUPPORT_D3D9

#if SUPPORT_D3D11
void MikanRenderAPI_D3D11::ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces)
{
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		IUnityGraphicsD3D11* d3d11 = s_UnityInterfaces->Get<IUnityGraphicsD3D11>();
		ID3D11Device* d3d11Device = d3d11->GetDevice();

		MIKAN_LOG_INFO("Unity") << "Binding D3D11 Device: 0x" << (void*)d3d11Device;
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_Direct3D11, d3d11Device);
	}
	else if (eventType == kUnityGfxDeviceEventShutdown)
	{
		MIKAN_LOG_INFO("Unity") << "Unbinding D3D11 Device";
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_Direct3D11, nullptr);
	}
}
#endif // #if SUPPORT_D3D11

#if SUPPORT_D3D12
void MikanRenderAPI_D3D12::ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		IUnityGraphicsD3D12* d3dD12 = s_UnityInterfaces->Get<IUnityGraphicsD3D12>();
		ID3D12Device* d3d12Device = d3dD12->GetDevice();

		MIKAN_LOG_INFO("Unity") << "Binding D3D12 Device: 0x" << (void*)d3d12Device;
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_Direct3D12, d3d12Device);
	}
	else if (eventType == kUnityGfxDeviceEventShutdown)
	{
		MIKAN_LOG_INFO("Unity") << "Unbinding D3D12 Device";
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_Direct3D12, nullptr);
	}
}
#endif // #if SUPPORT_D3D12

#if SUPPORT_METAL
void MikanRenderAPI_Metal::ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		MIKAN_LOG_INFO("Unity") << "Binding Metal Device";
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_Metal, nullptr);
	}
	else if (eventType == kUnityGfxDeviceEventShutdown)
	{
		MIKAN_LOG_INFO("Unity") << "Unbinding Metal Device";
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_Metal, nullptr);
	}
}
#endif // #if SUPPORT_METAL

#if SUPPORT_VULKAN
void MikanRenderAPI_Vulkan::ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		IUnityGraphicsVulkan* unityVulkan = interfaces->Get<IUnityGraphicsVulkan>();
		VkInstance instance = unityVulkan->Instance();

		MIKAN_LOG_INFO("Unity") << "Binding Vulkan Device";
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_Vulkan, instance);
	}
	else if (eventType == kUnityGfxDeviceEventShutdown)
	{
		MIKAN_LOG_INFO("Unity") << "Unbinding Vulkan Device";
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_Vulkan, nullptr);
	}
}
#endif // #if SUPPORT_VULKAN