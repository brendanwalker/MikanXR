using System;
using System.Threading.Tasks;

namespace MikanXR
{
	public class MikanRenderTargetAPI
	{
		private MikanRequestManager _requestManager;

		public MikanRenderTargetAPI(MikanRequestManager requestManager)
		{
			_requestManager = requestManager;
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

		public Task<MikanResponse> AllocateRenderTargetTextures(ref MikanRenderTargetDescriptor descriptor)
		{
			MikanResult result =
				(MikanResult)MikanCoreNative.Mikan_AllocateRenderTargetTextures(
					ref descriptor, out int requestId);

			return _requestManager.AddResponseHandler(requestId, result);
		}

		public MikanResult WriteColorRenderTargetTexture(
			IntPtr apiColorTexturePtr)
		{
			MikanResult result =
				(MikanResult)MikanCoreNative.Mikan_WriteColorRenderTargetTexture(
					apiColorTexturePtr);

			return result;
		}

		public MikanResult WriteDepthRenderTargetTexture(
			IntPtr apiDepthTexturePtr,
			float zNear,
			float zFar)
		{
			MikanResult result =
				(MikanResult)MikanCoreNative.Mikan_WriteDepthRenderTargetTexture(
					apiDepthTexturePtr, zNear, zFar);

			return result;
		}

		public MikanResult PublishRenderTargetTextures(
			ref MikanClientFrameRendered frameInfo)
		{
			MikanResult result =
				(MikanResult)MikanCoreNative.Mikan_PublishRenderTargetTextures(
					ref frameInfo);

			return result;
		}

		public Task<MikanResponse> FreeRenderTargetTextures()
		{
			MikanResult result = (MikanResult)MikanCoreNative.Mikan_FreeRenderTargetTextures(out int requestId);

			return _requestManager.AddResponseHandler(requestId, result);
		}

		public IntPtr GetPackDepthTextureResourcePtr()
		{
			return MikanCoreNative.Mikan_GetPackDepthTextureResourcePtr();
		}

	}
}