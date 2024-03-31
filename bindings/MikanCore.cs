using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

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

	[StructLayout(LayoutKind.Sequential)]
	public struct MikanRenderTargetDescriptor
	{
		MikanColorBufferType color_buffer_type;
		MikanDepthBufferType depth_buffer_type;
		uint width;
		uint height;
		MikanClientGraphicsApi graphicsAPI;
	};

	[StructLayout(LayoutKind.Sequential)]
	struct MikanClientFrameRendered
	{
		ulong frame_index;
	};

	protected class MikanCoreNative
	{
		[global::System.Runtime.InteropServices.UnmanagedFunctionPointer(global::System.Runtime.InteropServices.CallingConvention.Cdecl)]
		public delegate void NativeLogCallback(
			int log_level,
			[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPStr)]
			string log_message);

		[global::System.Runtime.InteropServices.UnmanagedFunctionPointer(global::System.Runtime.InteropServices.CallingConvention.Cdecl)]
		public delegate void NativeResponseCallback(
			int request_id,
			[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPUTF8Str)]
			string utf8_response_string,
			IntPtr userdata);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_Initialize(MikanLogLevel min_log_level, NativeLogCallback log_callback);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_GetCoreSDKVersion();

		[DllImport("MikanCore.dll")]
		public static extern bool Mikan_GetIsInitialized();

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_SetGraphicsDeviceInterface(
			MikanClientGraphicsApi api, 
			IntPtr graphicsDeviceInterface);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_GetGraphicsDeviceInterface(
			MikanClientGraphicsApi api, 
			out IntPtr outGraphicsDeviceInterface);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_AllocateRenderTargetBuffers(
			ref MikanRenderTargetDescriptor descriptor, 
			out int out_request_id);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_PublishRenderTargetTexture(
			IntPtr ApiTexturePtr, 
			ref MikanClientFrameRendered frame_info);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_FreeRenderTargetBuffers(out int out_request_id);

		[DllImport("MikanCore.dll", CharSet = CharSet.Ansi)]
		public static extern int Mikan_SetClientProperty(
			string key, 
			[MarshalAs(UnmanagedType.LPUTF8Str)]
			string value);

		[DllImport("MikanCore.dll", CharSet = CharSet.Ansi)]
		public static extern int Mikan_Connect(string host, string port);

		[DllImport("MikanCore.dll")]
		public static extern bool Mikan_GetIsConnected();

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_FetchNextEvent(
			UIntPtr utf8_buffer_size,
			[MarshalAs(UnmanagedType.LPUTF8Str)] 
			StringBuilder out_utf8_buffer,
			out UIntPtr out_utf8_bytes_written);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_SendRequest(
			[MarshalAs(UnmanagedType.LPUTF8Str)]
			string utf8_request_name, 
			[MarshalAs(UnmanagedType.LPUTF8Str)]
			string utf8_payload,
			int request_version,
			out int out_request_id);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_SetResponseCallback(
			NativeResponseCallback callback, 
			IntPtr callback_userdata);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_Disconnect();

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_Shutdown();
	}

	struct MikanResponse
	{
		public int requestId { get; set; }
		public MikanResult resultCode { get; set; }
	};

	class IMikanResponseFactory
	{
		virtual MikanResponse CreateResponse(string utfJsonString) = 0;
	}

	class MikanResponseFactory<T> where T : MikanResponse, IMikanResponseFactory
	{
		public override MikanResponse CreateResponse(string utfJsonString)
		{
			T response= JsonSerializer.Deserialize<T>(utfJsonString);

			return response;
		}
	}

	public class MikanCore
	{
		public delegate void MikanLogCallback(
			MikanLogLevel log_level,
			string log_message);

		public delegate void MikanResponseCallback(
			MikanRequestId request_id,
			string utf8_response_string);

		private static MikanCore _instance;

		private MikanCoreNative.NativeLogCallback _nativeLogCallback;
		private MikanCoreNative.NativeResponseCallback _nativeResponseCallback;

		private MikanLogCallback _logCallback;

		public static MikanCore Instance
		{
			get
			{
				if (_instance == null)
				{
					_instance = new MikanCore();
				}
				return _instance;
			}
		}

		public MikanCore()
		{
			_nativeLogCallback = new MikanCoreNative.NativeLogCallback(InternalLogCallback);
			_nativeResponseCallback = new MikanCoreNative.NativeResponseCallback(InternalResponseCallback);
		}

		public MikanResult Initialize(MikanLogLevel minLogLevel)
		{
			MikanResult result = (MikanResult)MikanCoreNative.Mikan_Initialize(minLogLevel, _nativeLogCallback);
			if (result != MikanResult.Success)
			{
				return result;
			}

			result = (MikanResult)MikanCoreNative.Mikan_SetResponseCallback(_nativeResponseCallback, IntPtr.Zero);
			if (result != MikanResult.Success)
			{
				return result;
			}

			return MikanResult.Success;
		}

		public int GetCoreSDKVersion()
		{
			return MikanCoreNative.Mikan_GetCoreSDKVersion();
		}

		public bool GetIsInitialized()
		{
			return MikanCoreNative.Mikan_GetIsInitialized();
		}

		public MikanResult SetGraphicsDeviceInterface(
			MikanClientGraphicsApi api, 
			IntPtr graphicsDeviceInterface)
		{
			int result = MikanCoreNative.Mikan_SetGraphicsDeviceInterface(api, graphicsDeviceInterface);
			return (MikanResult)result;
		}

		public MikanResult GetGraphicsDeviceInterface(
			MikanClientGraphicsApi api, 
			out IntPtr outGraphicsDeviceInterface)
		{
			int result = MikanCoreNative.Mikan_GetGraphicsDeviceInterface(api, out outGraphicsDeviceInterface);
			return (MikanResult)result;
		}

		public Task<MikanResponse> AllocateRenderTargetBuffers(ref MikanRenderTargetDescriptor descriptor)
		{
			int result = MikanCoreNative.Mikan_AllocateRenderTargetBuffers(ref descriptor, out int requestId);

			return AddResponseHandler(requestId, result);
		}

		public MikanResult PublishRenderTargetTexture(
			IntPtr apiTexturePtr, 
			ref MikanClientFrameRendered frameInfo)
		{
			int result = MikanCoreNative.Mikan_PublishRenderTargetTexture(apiTexturePtr, ref frameInfo);
			return (MikanResult)result;
		}

		public Task<MikanResponse> FreeRenderTargetBuffers(out MikanRequestId outRequestId)
		{
			int result = MikanCoreNative.Mikan_FreeRenderTargetBuffers(out int requestId);

			return AddResponseHandler(requestId, result);
		}

		public MikanResult SetClientProperty(string key, string value)
		{
			int result = MikanCoreNative.Mikan_SetClientProperty(key, value);
			return (MikanResult)result;
		}

		public MikanResult Connect(string host, string port)
		{
			int result = MikanCoreNative.Mikan_Connect(host, port);
			return (MikanResult)result;
		}

		public bool GetIsConnected()
		{
			return MikanCoreNative.Mikan_GetIsConnected();
		}

		public MikanResult FetchNextEvent(out string outUtf8Buffer)
		{
			StringBuilder utf8Buffer = new StringBuilder(1024);
			UIntPtr utf8BufferSize = (UIntPtr)utf8Buffer.Capacity;
			UIntPtr utf8BytesWritten;
			int result = MikanCoreNative.Mikan_FetchNextEvent(utf8BufferSize, utf8Buffer, out utf8BytesWritten);
			outUtf8Buffer = utf8Buffer.ToString();
			return (MikanResult)result;
		}

		public Task<MikanResponse> SendRequest(
			string utf8RequestName, 
			string utf8Payload, 
			int requestVersion)
		{
			int result = 
				MikanCoreNative.Mikan_SendRequest(
					utf8RequestName, utf8Payload, requestVersion, out int requestId);

			return AddResponseHandler(requestId, result);
		}

		Dictionary<string, IMikanResponseFactory> _responseFactory = new Dictionary<string, IMikanResponseFactory>();
		public void AddResponseFactory<T>()
		{
			IMikanResponseFactory factory = new MikanResponseFactory<T>();

			_responseFactory.Add(typeof(T).Name, factory);
		}

		struct PendingRequest
		{
			int requestId;
			TaskCompletionSource<MikanResponse> promise;
		};
		Dictionary<int, PendingRequest> _pendingRequests = new Dictionary<int, PendingRequest>();

		protected Task<MikanResponse> AddResponseHandler(int requestId, MikanResult result)
		{
			TaskCompletionSource<MikanResponse> promise = new TaskCompletionSource<MikanResponse>();

            if (result == MikanResult.Success)
            {
                PendingRequest pendingRequest = new PendingRequest() { 
					requestId = requestId, 
					promise = promise 
				};

				lock (_pendingRequests)
				{
					_pendingRequests.Add(requestId, pendingRequest);
				}
            }
			else
			{
				MikanResponse response = new MikanResponse()
				{
					requestId = requestId, 
					resultCode = result 
				};

				promise.SetResult(response);
			}

            return promise.Task;
		}

		public MikanResult Disconnect()
		{
			int result = MikanCoreNative.Mikan_Disconnect();
			return (MikanResult)result;
		}

		public MikanResult Shutdown()
		{
			int result = MikanCoreNative.Mikan_Shutdown();
			return (MikanResult)result;
		}

		private void InternalLogCallback(int logLevel, string logMessage)
		{
			if (_logCallback != null)
			{
				_logCallback((MikanLogLevel)logLevel, logMessage);
			}
		}

		private void InternalResponseCallback(int requestId, string utf8ResponseString, IntPtr userData)
		{
			PendingRequest pendingRequest;

			lock (_pendingRequests)
			{
				if (_pendingRequests.TryGetValue(requestId, out pendingRequest))
				{
					_pendingRequests.Remove(requestId);
				}
			}

			if (pendingRequest != null)
			{
				MikanResponse response = parseResponseString(utf8ResponseString);

				if (response == null)
				{
					response = new MikanResponse()
					{
						requestId = requestId,
						resultCode = MikanResult.MalformedResponse
					};
				}

				pendingRequest.promise.SetResult(response);
			}
		}

		private MikanResponse parseResponseString(string utf8ResponseString)
		{
			MikanResponse response = null;

			JsonDocument document = JsonDocument.Parse(utf8ResponseString);
			JsonElement root = document.RootElement;

			// Check if the key "responseType" exists
			if (root.TryGetProperty("responseType", out JsonElement responseTypeElement))
			{
				// Check if the value of "responseType" is a string
				if (responseTypeElement.ValueKind == JsonValueKind.String)
				{
					// Get the string value of "responseType"
					string responseType = responseTypeElement.GetString();
					
					if (_responseFactory.TryGetValue(responseType, out IMikanResponseFactory factory))
					{
						response = factory.CreateResponse(utf8ResponseString);
					}
					else
					{
						InternalLogCallback((int)MikanLogLevel.Error, "Unknown response type: " + responseType);
					}
				}
				else
				{
					InternalLogCallback((int)MikanLogLevel.Error, "responseType is not a string.");
				}
			}
			else
			{
				InternalLogCallback((int)MikanLogLevel.Error, "responseType key not found.");
			}

			// Dispose of the JsonDocument to free resources
			document.Dispose();

			return response;
		}
	}
}