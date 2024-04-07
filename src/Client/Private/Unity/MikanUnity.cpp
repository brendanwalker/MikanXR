// Example low level rendering Unity plugin
#include "MikanUnity.h"
#include "IUnityApplication.h"
#include "IUnityGraphics.h"
#include "Logger.h"
#include "MikanCoreCAPI.h"

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
// UnitySetInterfaces

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

static IUnityInterfaces* s_UnityInterfaces = NULL;
static IUnityApplication* s_UnityApp = NULL;
static IUnityGraphics* s_Graphics = NULL;
static UnityGfxRenderer s_DeviceType = kUnityGfxRendererNull;

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces * unityInterfaces)
{
	s_UnityInterfaces = unityInterfaces;
	s_UnityApp = s_UnityInterfaces->Get<IUnityApplication>();
	s_Graphics = s_UnityInterfaces->Get<IUnityGraphics>();
	s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

	// Run OnGraphicsDeviceEvent(initialize) manually on plugin load
	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

// --------------------------------------------------------------------------
// GraphicsDeviceEvent

// Actual setup/teardown functions defined below
#if SUPPORT_D3D9
static void DoEventGraphicsDeviceD3D9(UnityGfxDeviceEventType eventType);
#endif
#if SUPPORT_D3D11
static void DoEventGraphicsDeviceD3D11(UnityGfxDeviceEventType eventType);
#endif
#if SUPPORT_D3D12
static void DoEventGraphicsDeviceD3D12(UnityGfxDeviceEventType eventType);
#endif
#if SUPPORT_OPENGLES
static void DoEventGraphicsDeviceGLES(UnityGfxDeviceEventType eventType);
#endif

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	UnityGfxRenderer currentDeviceType = s_DeviceType;

	switch (eventType)
	{
	case kUnityGfxDeviceEventInitialize:
	{
		MIKAN_LOG_INFO("Unity") << "OnGraphicsDeviceEvent(Initialize)";
		s_DeviceType = s_Graphics->GetRenderer();
		currentDeviceType = s_DeviceType;
		break;
	}

	case kUnityGfxDeviceEventShutdown:
	{
		MIKAN_LOG_INFO("Unity") << "OnGraphicsDeviceEvent(Shutdown)";
		s_DeviceType = kUnityGfxRendererNull;
		break;
	}

	case kUnityGfxDeviceEventBeforeReset:
	{
		MIKAN_LOG_INFO("Unity") << "OnGraphicsDeviceEvent(BeforeReset).";
		break;
	}

	case kUnityGfxDeviceEventAfterReset:
	{
		MIKAN_LOG_INFO("Unity") << "OnGraphicsDeviceEvent(AfterReset).";
		break;
	}
	};

#if SUPPORT_D3D9
	if (currentDeviceType == kUnityGfxRendererD3D9)
		DoEventGraphicsDeviceD3D9(eventType);
#endif

#if SUPPORT_D3D11
	if (currentDeviceType == kUnityGfxRendererD3D11)
		DoEventGraphicsDeviceD3D11(eventType);
#endif

#if SUPPORT_D3D12
	if (currentDeviceType == kUnityGfxRendererD3D12)
		DoEventGraphicsDeviceD3D12(eventType);
#endif

#if SUPPORT_OPENGLES
	if (currentDeviceType == kUnityGfxRendererOpenGLES20 ||
		currentDeviceType == kUnityGfxRendererOpenGLES30)
		DoEventGraphicsDeviceGLES(eventType);
#endif
}

// --------------------------------------------------------------------------
// GetRenderEventFunc, an example function we export which is used to get a rendering event callback function.
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
{
	return nullptr;
}

// -------------------------------------------------------------------
//  Direct3D 9 setup/teardown code
#if SUPPORT_D3D9
static void DoEventGraphicsDeviceD3D9(UnityGfxDeviceEventType eventType)
{
	// Create or release a small dynamic vertex buffer depending on the event type.
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		IUnityGraphicsD3D9* d3d9 = s_UnityInterfaces->Get<IUnityGraphicsD3D9>();
		IDirect3DDevice9* d3d9Device= d3d9->GetDevice();

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

// -------------------------------------------------------------------
//  Direct3D 11 setup/teardown code
#if SUPPORT_D3D11
static void DoEventGraphicsDeviceD3D11(UnityGfxDeviceEventType eventType)
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

// -------------------------------------------------------------------
// Direct3D 12 setup/teardown code
#if SUPPORT_D3D12
static void DoEventGraphicsDeviceD3D12(UnityGfxDeviceEventType eventType)
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

// -------------------------------------------------------------------
// GLES setup/teardown code
#if SUPPORT_OPENGLES
static void DoEventGraphicsDeviceGLES(UnityGfxDeviceEventType eventType)
{
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		MIKAN_LOG_INFO("Unity") << "Binding OpenGL Device";
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_OpenGL, nullptr);
	}
	else if (eventType == kUnityGfxDeviceEventShutdown)
	{
		MIKAN_LOG_INFO("Unity") << "Unbinding OpenGL Device";
		Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi_OpenGL, nullptr);
	}
}
#endif