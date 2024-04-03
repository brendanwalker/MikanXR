using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanAPI
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
		
		public static MikanAPI Instance
		{
			get
			{
				if (_instance == null)
				{
					_instance = new MikanAPI();
				}
				return _instance;
			}
		}

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
			_eventManager.addEventFactory<MikanConnectedEvent>();
			_eventManager.addEventFactory<MikanDisconnectedEvent>();
			_eventManager.addEventFactory<MikanVideoSourceOpenedEvent>();
			_eventManager.addEventFactory<MikanVideoSourceClosedEvent>();
			_eventManager.addEventFactory<MikanVideoSourceNewFrameEvent>();
			_eventManager.addEventFactory<MikanVideoSourceAttachmentChangedEvent>();
			_eventManager.addEventFactory<MikanVideoSourceIntrinsicsChangedEvent>();
			_eventManager.addEventFactory<MikanVideoSourceModeChangedEvent>();
			_eventManager.addEventFactory<MikanVRDevicePoseUpdateEvent>();
			_eventManager.addEventFactory<MikanVRDeviceListUpdateEvent>();
			_eventManager.addEventFactory<MikanAnchorPoseUpdateEvent>();
			_eventManager.addEventFactory<MikanAnchorListUpdateEvent>();
			_eventManager.addEventFactory<MikanScriptMessagePostedEvent>();			
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

		public MikanResult FetchNextEvent(out MikanEvent outEvent)
		{
			return _eventManager.FetchNextEvent(out MikanEvent outEvent);
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