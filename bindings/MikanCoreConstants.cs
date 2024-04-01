namespace MikanXR
{
	public static class Constants
	{
		public const int INVALID_MIKAN_ID = -1;
	}

	/// Result enum in response to a client API request
	public enum MikanResult
	{
		Success,  ///< General Success Result	
		GeneralError, ///< General Error Result
		Uninitialized,
		NullParam,
		BufferTooSmall,
		InitFailed,
		ConnectionFailed,
		AlreadyConnected,
		NotConnected,
		SocketError,
		NoData,
		Timeout,
		Canceled,
		UnknownClient,
		UnknownFunction,
		FailedFunctionSend,
		FunctionResponseTimeout,
		MalformedParameters,
		MalformedResponse,
		InvalidAPI,
		SharedTextureError,
		NoVideoSource,
		NoVideoSourceAssignedTracker,
		InvalidDeviceId,
		InvalidStencilID,
		InvalidAnchorID,
	};

	public enum MikanLogLevel
	{
		Trace,
		Debug,
		Info,
		Warning,
		Error,
		Fatal
	};

	public enum MikanClientGraphicsApi
	{
		UNKNOWN = -1,

		Direct3D9,
		Direct3D11,
		Direct3D12,
		OpenGL,
	};

	public enum MikanColorBufferType
	{
		NOCOLOR,
		RGB24,
		RGBA32, // DXGI_FORMAT_R8G8B8A8_UNORM / DXGI_FORMAT_R8G8B8A8_TYPELESS
		BGRA32, // DXGI_FORMAT_B8G8R8A8_UNORM / DXGI_FORMAT_B8G8R8A8_TYPELESS
	};

	public enum MikanDepthBufferType
	{
		NODEPTH,
		DEPTH16,
		DEPTH32,
	};
}