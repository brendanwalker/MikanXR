using System;
using System.Text.Json;
using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanClientInfo
	{
		public string clientId { get; set; }
		public string engineName { get; set; }
		public string engineVersion { get; set; }
		public string applicationName { get; set; }
		public string applicationVersion { get; set; }
		public string xrDeviceName { get; set; }
		public MikanClientGraphicsApi graphicsAPI { get; set; }
		public int mikanCoreVersion { get; set; }
		public bool supportsRGB24 { get; set; }
		public bool supportsRGBA32 { get; set; }
		public bool supportsBGR32 { get; set; }
		public bool supportsDepth { get; set; }
	}

	public class MikanAPI : IDisposable
	{
		public delegate void MikanLogCallback(
			MikanLogLevel log_level,
			string log_message);

		private static MikanAPI _instance;

		private MikanCoreNative.NativeLogCallback _nativeLogCallback;
		private MikanLogCallback _logCallback;
		
		private MikanRequestManager _requestManager;
		public MikanRequestManager RequestManager => _requestManager;
		
		private MikanEventManager _eventManager;
		public MikanEventManager EventManager => _eventManager;
		
		private MikanVideoSourceAPI _videoSourceAPI;
		public MikanVideoSourceAPI VideoSourceAPI => _videoSourceAPI;
		
		private MikanVRDeviceAPI _vrDeviceAPI;
		public MikanVRDeviceAPI VRDeviceAPI => _vrDeviceAPI;
		
		private MikanScriptAPI _scriptAPI;
		public MikanScriptAPI ScriptAPI => _scriptAPI;
		
		private MikanStencilAPI _stencilAPI;
		public MikanStencilAPI StencilAPI => _stencilAPI;
		
		private MikanSpatialAnchorAPI _spatialAnchorAPI;
		public MikanSpatialAnchorAPI SpatialAnchorAPI => _spatialAnchorAPI;

		public MikanAPI()
		{
			_nativeLogCallback = new MikanCoreNative.NativeLogCallback(InternalLogCallback);
			_requestManager = new MikanRequestManager(_nativeLogCallback);
			_eventManager = new MikanEventManager(_nativeLogCallback);
			
			// Create the child APIs
			_videoSourceAPI = new MikanVideoSourceAPI(_requestManager);	
			_vrDeviceAPI = new MikanVRDeviceAPI(_requestManager);	
			_scriptAPI = new MikanScriptAPI(_requestManager);
			_stencilAPI = new MikanStencilAPI(_requestManager);
			_spatialAnchorAPI = new MikanSpatialAnchorAPI(_requestManager);
			
			// Register base response types (child API classes will register their own types)
			_requestManager.AddResponseFactory<MikanResponse>();
			
			// Register all event types
			_eventManager.AddEventFactory<MikanConnectedEvent>();
			_eventManager.AddEventFactory<MikanDisconnectedEvent>();
			_eventManager.AddEventFactory<MikanVideoSourceOpenedEvent>();
			_eventManager.AddEventFactory<MikanVideoSourceClosedEvent>();
			_eventManager.AddEventFactory<MikanVideoSourceNewFrameEvent>();
			_eventManager.AddEventFactory<MikanVideoSourceAttachmentChangedEvent>();
			_eventManager.AddEventFactory<MikanVideoSourceIntrinsicsChangedEvent>();
			_eventManager.AddEventFactory<MikanVideoSourceModeChangedEvent>();
			_eventManager.AddEventFactory<MikanVRDevicePoseUpdateEvent>();
			_eventManager.AddEventFactory<MikanVRDeviceListUpdateEvent>();
			_eventManager.AddEventFactory<MikanAnchorPoseUpdateEvent>();
			_eventManager.AddEventFactory<MikanAnchorListUpdateEvent>();
			_eventManager.AddEventFactory<MikanScriptMessagePostedEvent>();
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
			MikanResult result = 
				(MikanResult)MikanCoreNative.Mikan_AllocateRenderTargetBuffers(ref descriptor, out int requestId);

			return _requestManager.AddResponseHandler(requestId, result);
		}

		public MikanResult PublishRenderTargetTexture(
			IntPtr apiTexturePtr, 
			ref MikanClientFrameRendered frameInfo)
		{
			MikanResult result = 
				(MikanResult)MikanCoreNative.Mikan_PublishRenderTargetTexture(apiTexturePtr, ref frameInfo);

			return result;
		}

		public Task<MikanResponse> FreeRenderTargetBuffers()
		{
			MikanResult result = (MikanResult)MikanCoreNative.Mikan_FreeRenderTargetBuffers(out int requestId);

			return _requestManager.AddResponseHandler(requestId, result);
		}

		public MikanResult SetClientInfo(MikanClientInfo clientInfo)
		{
			// Stamp with the core sdk version
			clientInfo.mikanCoreVersion = GetCoreSDKVersion();

			string clientInfoString = JsonSerializer.Serialize(clientInfo);
			int result = MikanCoreNative.Mikan_SetClientProperty("clientInfo", clientInfoString);

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