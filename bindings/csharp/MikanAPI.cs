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

		public MikanAPIResult Initialize(MikanLogLevel minLogLevel)
		{
			MikanAPIResult result = 
				(MikanAPIResult)MikanCoreNative.Mikan_Initialize(
					minLogLevel, _nativeLogCallback, out _mikanContext);
			if (result != MikanAPIResult.Success)
			{
				return result;
			}
			
			result= _requestManager.Initialize(_mikanContext);
			if (result != MikanAPIResult.Success)
			{
				return result;
			}

			_eventManager.Initialize(_mikanContext);
			_renderTargetAPI.Initialize(_mikanContext);

			return MikanAPIResult.Success;
		}

		public bool GetIsInitialized()
		{
			return MikanCoreNative.Mikan_GetIsInitialized(_mikanContext);
		}

		public MikanAPIResult Shutdown()
		{
			MikanAPIResult result = MikanAPIResult.NotConnected;

			if (_mikanContext != IntPtr.Zero)
			{
				int intResult = MikanCoreNative.Mikan_Shutdown(_mikanContext);
				_mikanContext = IntPtr.Zero;
				result= (MikanAPIResult)intResult;
			}

			return result;
		}

		// -- Client Info ----

		public int GetClientAPIVersion()
		{
			return MikanCoreNative.Mikan_GetClientAPIVersion();
		}

		public string GetClientUniqueID()
		{
			return MikanCoreNative.GetClientUniqueID(_mikanContext);
		}

		public MikanClientInfo AllocateClientInfo()
		{
			MikanClientInfo clientInfo = new MikanClientInfo();

			// Stamp the request with the client API version and client id
			clientInfo.apiVersion.version = GetClientAPIVersion();
			clientInfo.clientId = GetClientUniqueID();

			return clientInfo;
		}

		public MikanAPIResult SetGraphicsDeviceInterface(
			MikanClientGraphicsApi api,
			IntPtr graphicsDeviceInterface)
		{
			return _renderTargetAPI.SetGraphicsDeviceInterface(api, graphicsDeviceInterface);
		}

		public MikanAPIResult GetGraphicsDeviceInterface(
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

		public MikanAPIResult Connect(string host="", string port="")
		{
			int result = MikanCoreNative.Mikan_Connect(_mikanContext, host, port);
			return (MikanAPIResult)result;
		}

		public bool GetIsConnected()
		{
			return MikanCoreNative.Mikan_GetIsConnected(_mikanContext);
		}

		public MikanAPIResult Disconnect()
		{
			int result = MikanCoreNative.Mikan_Disconnect(_mikanContext, 0, "");
			return (MikanAPIResult)result;
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

		public MikanAPIResult FetchNextEvent(out MikanEvent outEvent)
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