// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Enums
	public enum MikanResult
	{
		Success= 0,
		GeneralError= 1,
		Uninitialized= 2,
		NullParam= 3,
		BufferTooSmall= 4,
		InitFailed= 5,
		ConnectionFailed= 6,
		AlreadyConnected= 7,
		NotConnected= 8,
		SocketError= 9,
		NoData= 10,
		Timeout= 11,
		Canceled= 12,
		UnknownClient= 13,
		UnknownFunction= 14,
		FailedFunctionSend= 15,
		FunctionResponseTimeout= 16,
		MalformedParameters= 17,
		MalformedResponse= 18,
		InvalidAPI= 19,
		SharedTextureError= 20,
		NoVideoSource= 21,
		NoVideoSourceAssignedTracker= 22,
		InvalidDeviceId= 23,
		InvalidStencilID= 24,
		InvalidAnchorID= 25,
	};

	public enum MikanLogLevel
	{
		Trace= 0,
		Debug= 1,
		Info= 2,
		Warning= 3,
		Error= 4,
		Fatal= 5,
	};

	public enum MikanClientGraphicsApi
	{
		UNKNOWN= -1,
		Direct3D9= 0,
		Direct3D11= 1,
		Direct3D12= 2,
		OpenGL= 3,
		Metal= 4,
		Vulkan= 5,
	};

	public enum MikanColorBufferType
	{
		NOCOLOR= 0,
		RGB24= 1,
		RGBA32= 2,
		BGRA32= 3,
	};

	public enum MikanDepthBufferType
	{
		NOCOLOR= 0,
		FLOAT_DEVICE_DEPTH= 1,
		FLOAT_SCENE_DEPTH= 2,
		PACK_DEPTH_RGBA= 3,
	};

}
