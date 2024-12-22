using System;
using System.Threading.Tasks;

namespace MikanXR
{
	public class MikanRenderTargetAPI
	{
		private IntPtr _mikanContext;
		private MikanRequestManager _requestManager;

		public MikanRenderTargetAPI(MikanRequestManager requestManager)
		{
			_requestManager = requestManager;
		}

		public void Initialize(IntPtr mikanContext)
		{
			_mikanContext= mikanContext;
		}

		public MikanAPIResult SetGraphicsDeviceInterface(
			MikanClientGraphicsApi api,
			IntPtr graphicsDeviceInterface)
		{
			int result = 
				MikanCoreNative.Mikan_SetGraphicsDeviceInterface(
					_mikanContext, api, graphicsDeviceInterface);
			return (MikanAPIResult)result;
		}

		public MikanAPIResult GetGraphicsDeviceInterface(
			MikanClientGraphicsApi api,
			out IntPtr outGraphicsDeviceInterface)
		{
			int result = 
				MikanCoreNative.Mikan_GetGraphicsDeviceInterface(
					_mikanContext, api, out outGraphicsDeviceInterface);
			return (MikanAPIResult)result;
		}

		public IntPtr GetPackDepthTextureResourcePtr()
		{
			return MikanCoreNative.Mikan_GetPackDepthTextureResourcePtr(_mikanContext);
		}

		public bool TryProcessRequest(MikanRequest request, out Task<MikanResponse> outFuture)
		{
			outFuture= null;

			if (request is AllocateRenderTargetTextures)
			{
				outFuture = RequestAllocateRenderTargetTextures(request);
				return true;
			}
			else if (request is WriteColorRenderTargetTexture)
			{
				outFuture = RequestWriteColorRenderTargetTexture(request);
				return true;
			}
			else if (request is WriteDepthRenderTargetTexture)
			{
				outFuture = RequestWriteDepthRenderTargetTexture(request);
				return true;
			}
			else if (request is PublishRenderTargetTextures) 
			{
				outFuture = RequestPublishRenderTargetTextures(request);
				return true;
			}
			else if (request is FreeRenderTargetTextures)
			{
				outFuture = RequestFreeRenderTargetTextures(request);
				return true;
			}

			return false;
		}

		private Task<MikanResponse> RequestAllocateRenderTargetTextures(MikanRequest request)
		{
			var allocateRequest = request as AllocateRenderTargetTextures;
			MikanRenderTargetDescriptor desiredDescriptor= allocateRequest.descriptor;
			MikanRenderTargetDescriptor_Native desiredDescriptor_Native= 
				new MikanRenderTargetDescriptor_Native() { 
					color_buffer_type= desiredDescriptor.color_buffer_type,
					depth_buffer_type= desiredDescriptor.depth_buffer_type,
					width= desiredDescriptor.width,
					height= desiredDescriptor.height,
					graphicsAPI= desiredDescriptor.graphicsAPI
				};

			MikanAPIResult result =
				(MikanAPIResult)MikanCoreNative.Mikan_AllocateRenderTargetTextures(
					_mikanContext, ref desiredDescriptor_Native);
			if (result == MikanAPIResult.Success)
			{
				// Actual descriptor might differ from desired descriptor based on render target writer's capabilities
				MikanRenderTargetDescriptor_Native actualDescriptor_Native;
				result= (MikanAPIResult)MikanCoreNative.Mikan_GetRenderTargetDescriptor(
					_mikanContext, out actualDescriptor_Native);
				if (result == MikanAPIResult.Success)
				{
					// Replace the descriptor in the request with the actual descriptor params 
					// that were used to allocate the render target textures
					allocateRequest.descriptor =
						new MikanRenderTargetDescriptor()
						{
							color_buffer_type = actualDescriptor_Native.color_buffer_type,
							depth_buffer_type = actualDescriptor_Native.depth_buffer_type,
							width = actualDescriptor_Native.width,
							height = actualDescriptor_Native.height,
							graphicsAPI = actualDescriptor_Native.graphicsAPI
						};

					return _requestManager.SendRequest(allocateRequest);
				}
			}

			return _requestManager.AddResponseHandler(-1, MikanAPIResult.RequestFailed);
		}

		private Task<MikanResponse> RequestWriteColorRenderTargetTexture(MikanRequest request)
		{
			var writeRequest = request as WriteColorRenderTargetTexture;
			IntPtr apiColorTexturePtr= writeRequest.apiColorTexturePtr;

			MikanAPIResult result =
				(MikanAPIResult)MikanCoreNative.Mikan_WriteColorRenderTargetTexture(
					_mikanContext, apiColorTexturePtr);

			return _requestManager.MakeImmediateResponse(result);
		}

		private Task<MikanResponse> RequestWriteDepthRenderTargetTexture(MikanRequest request)
		{
			var writeRequest = request as WriteDepthRenderTargetTexture;
			IntPtr apiDepthTexturePtr= writeRequest.apiDepthTexturePtr;
			float zNear= writeRequest.zNear;
			float zFar= writeRequest.zFar;

			MikanAPIResult result =
				(MikanAPIResult)MikanCoreNative.Mikan_WriteDepthRenderTargetTexture(
					_mikanContext, apiDepthTexturePtr, zNear, zFar);

			return _requestManager.MakeImmediateResponse(result);
		}

		private Task<MikanResponse> RequestPublishRenderTargetTextures(MikanRequest request)
		{
			return _requestManager.SendRequest(request);
		}

		private Task<MikanResponse> RequestFreeRenderTargetTextures(MikanRequest request)
		{
			// Free any locally allocated resources
			MikanCoreNative.Mikan_FreeRenderTargetTextures(_mikanContext);

			// Tell the server to free the render target resources too
			return _requestManager.SendRequest(request);
		}
	}
}