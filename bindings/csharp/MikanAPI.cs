using Newtonsoft.Json;
using System;
using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanAPI : IDisposable
	{
		public delegate void MikanLogCallback(
			MikanLogLevel log_level,
			string log_message);

		private MikanCoreNative.NativeLogCallback _nativeLogCallback;
		private MikanLogCallback _logCallback;
		
		private MikanRequestManager _requestManager;
		private MikanEventManager _eventManager;
		private MikanRenderTargetAPI _renderTargetAPI;

		private IntPtr _mikanContext= IntPtr.Zero;

		// -- API Lifecycle ----

		public MikanAPI()
		{
			_nativeLogCallback = new MikanCoreNative.NativeLogCallback(InternalLogCallback);
			_requestManager = new MikanRequestManager(_nativeLogCallback);
			_eventManager = new MikanEventManager(_nativeLogCallback);
			_renderTargetAPI = new MikanRenderTargetAPI(_requestManager);
		}

		public void Dispose()
		{			
			Shutdown();
			_requestManager= null;
			_eventManager= null;
			_renderTargetAPI= null;
		}

		public MikanResult Initialize(MikanLogLevel minLogLevel)
		{
			MikanResult result = 
				(MikanResult)MikanCoreNative.Mikan_Initialize(
					minLogLevel, _nativeLogCallback, out _mikanContext);
			if (result != MikanResult.Success)
			{
				return result;
			}
			
			result= _requestManager.Initialize(_mikanContext);
			if (result != MikanResult.Success)
			{
				return result;
			}

			_eventManager.Initialize(_mikanContext);
			_renderTargetAPI.Initialize(_mikanContext);

			return MikanResult.Success;
		}

		public bool GetIsInitialized()
		{
			return MikanCoreNative.Mikan_GetIsInitialized(_mikanContext);
		}

		public MikanResult Shutdown()
		{
			MikanResult result = MikanResult.NotConnected;

			if (_mikanContext != IntPtr.Zero)
			{
				int intResult = MikanCoreNative.Mikan_Shutdown(_mikanContext);
				_mikanContext = IntPtr.Zero;
				result= (MikanResult)intResult;
			}

			return result;
		}

		// -- Client Info ----

		public int GetCoreSDKVersion()
		{
			return MikanCoreNative.Mikan_GetCoreSDKVersion();
		}

		public string GetClientUniqueID()
		{
			return MikanCoreNative.GetClientUniqueID(_mikanContext);
		}

		public MikanResult SetGraphicsDeviceInterface(
			MikanClientGraphicsApi api,
			IntPtr graphicsDeviceInterface)
		{
			return _renderTargetAPI.SetGraphicsDeviceInterface(api, graphicsDeviceInterface);
		}

		public MikanResult GetGraphicsDeviceInterface(
			MikanClientGraphicsApi api,
			out IntPtr outGraphicsDeviceInterface)
		{
			return _renderTargetAPI.GetGraphicsDeviceInterface(api, out outGraphicsDeviceInterface);
		}

		public IntPtr GetPackDepthTextureResourcePtr()
		{
			return _renderTargetAPI.GetPackDepthTextureResourcePtr();
		}

		// -- Client Info ----

		public MikanResult Connect(ConnectRequest request, string host="", string port="")
		{
			// Stamp the request with the core sdk version and client id
			request.clientInfo.mikanCoreSdkVersion = GetCoreSDKVersion();
			request.clientInfo.clientId = GetClientUniqueID();

			string connectionRequestJson = JsonSerializer.serializeToJsonString(request);
			int result = MikanCoreNative.Mikan_Connect(_mikanContext, connectionRequestJson, host, port);
			return (MikanResult)result;
		}

		public bool GetIsConnected()
		{
			return MikanCoreNative.Mikan_GetIsConnected(_mikanContext);
		}

		public MikanResult Disconnect()
		{
			int result = MikanCoreNative.Mikan_Disconnect(_mikanContext);
			return (MikanResult)result;
		}

		// -- Messaging ----
		
		public Task<MikanResponse> SendRequest(MikanRequest request)
		{
			Task<MikanResponse> response;
			if (!_renderTargetAPI.TryProcessRequest(request, out response))
			{
				response= _requestManager.SendRequest(request);
			}

			return response;
		}

		public MikanResult FetchNextEvent(out MikanEvent outEvent)
		{
			return _eventManager.FetchNextEvent(out outEvent);
		}

		// -- Logging ----

		public void SetLogCallback(MikanLogCallback logCallback)
		{
			_logCallback = logCallback;
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