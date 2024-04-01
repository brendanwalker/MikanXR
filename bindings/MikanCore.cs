using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace MikanXR
{
	public class MikanCore
	{
		public delegate void MikanLogCallback(
			MikanLogLevel log_level,
			string log_message);

		private static MikanCore _instance;

		private MikanCoreNative.NativeLogCallback _nativeLogCallback;
		private MikanLogCallback _logCallback;
		
		private MikanRequestManager _requestManager;

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
			_requestManager = new MikanRequestManager(_nativeLogCallback);
		}

		public MikanResult Initialize(MikanLogLevel minLogLevel)
		{
			MikanResult result = (MikanResult)MikanCoreNative.Mikan_Initialize(minLogLevel, _nativeLogCallback);
			if (result != MikanResult.Success)
			{
				return result;
			}
			
			result= _requestManager.Initialize();
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

			return _requestManager.AddResponseHandler(requestId, result);
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

			return _requestManager.AddResponseHandler(requestId, result);
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
	}
}