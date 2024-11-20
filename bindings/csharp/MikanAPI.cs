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
		public MikanRequestManager RequestManager => _requestManager;
		
		private MikanEventManager _eventManager;
		public MikanEventManager EventManager => _eventManager;
		
		private MikanRenderTargetAPI _renderTargetAPI;
		public MikanRenderTargetAPI RenderTargetAPI => _renderTargetAPI;

		public MikanAPI()
		{
			_nativeLogCallback = new MikanCoreNative.NativeLogCallback(InternalLogCallback);
			_requestManager = new MikanRequestManager(_nativeLogCallback);
			_eventManager = new MikanEventManager(_nativeLogCallback);
			
			// Create the child APIs
			_renderTargetAPI = new MikanRenderTargetAPI(_requestManager);
		}

		public void Dispose()
		{

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

		public MikanResult Shutdown()
		{
			int result = MikanCoreNative.Mikan_Shutdown();
			return (MikanResult)result;
		}

		public int GetCoreSDKVersion()
		{
			return MikanCoreNative.Mikan_GetCoreSDKVersion();
		}

		public string GetClientUniqueID()
		{
			return MikanCoreNative.GetClientUniqueID();
		}
		
		public bool GetIsInitialized()
		{
			return MikanCoreNative.Mikan_GetIsInitialized();
		}

		public MikanResult SetClientInfo(MikanClientInfo clientInfo)
		{
			// Stamp with the core sdk version
			clientInfo.mikanCoreSdkVersion = GetCoreSDKVersion();
			clientInfo.clientId = GetClientUniqueID();

			// Serialize enumerations from strings rather than from integers
			var stringEnumConverter = new Newtonsoft.Json.Converters.StringEnumConverter();
			string clientInfoString = JsonConvert.SerializeObject(clientInfo, stringEnumConverter);
			int result = MikanCoreNative.Mikan_SetClientInfo(clientInfoString);

			return (MikanResult)result;
		}

		public MikanResult Connect(string host="", string port="")
		{
			int result = MikanCoreNative.Mikan_Connect(host, port);
			return (MikanResult)result;
		}

		public bool GetIsConnected()
		{
			return MikanCoreNative.Mikan_GetIsConnected();
		}

		public MikanResult FetchNextEvent(out MikanEvent outEvent)
		{
			return _eventManager.FetchNextEvent(out outEvent);
		}

		public MikanResult Disconnect()
		{
			int result = MikanCoreNative.Mikan_Disconnect();
			return (MikanResult)result;
		}

		public void	SetLogCallback(MikanLogCallback logCallback)
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