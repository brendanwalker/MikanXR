// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class AllocateCameraRenderTargetTextures : MikanRequest
	{
		public int camera_id;
		public MikanRenderTargetDescriptor descriptor;

		public AllocateCameraRenderTargetTextures()
		{
			requestTypeName = "AllocateCameraRenderTargetTextures";
		}
	};

	public class FreeCameraRenderTargetTextures : MikanRequest
	{
		public int camera_id;

		public FreeCameraRenderTargetTextures()
		{
			requestTypeName = "FreeCameraRenderTargetTextures";
		}
	};

	public class PublishCameraRenderTargetTextures : MikanRequest
	{
		public int camera_id;
		public long frame_index;

		public PublishCameraRenderTargetTextures()
		{
			requestTypeName = "PublishCameraRenderTargetTextures";
		}
	};

	public class WriteCameraColorRenderTargetTexture : MikanRequest
	{
		public int camera_id;
		public IntPtr api_color_texture_ptr;

		public WriteCameraColorRenderTargetTexture()
		{
			requestTypeName = "WriteCameraColorRenderTargetTexture";
		}
	};

	public class WriteCameraDepthRenderTargetTexture : MikanRequest
	{
		public int camera_id;
		public IntPtr api_depth_texture_ptr;
		public float z_near;
		public float z_far;

		public WriteCameraDepthRenderTargetTexture()
		{
			requestTypeName = "WriteCameraDepthRenderTargetTexture";
		}
	};

	public class WriteCameraShadowRenderTargetTexture : MikanRequest
	{
		public int camera_id;
		public IntPtr api_shadow_texture_ptr;

		public WriteCameraShadowRenderTargetTexture()
		{
			requestTypeName = "WriteCameraShadowRenderTargetTexture";
		}
	};

}
