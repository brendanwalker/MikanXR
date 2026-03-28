// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class AllocateCameraRenderTargetTextures : MikanRequest
	{
		public int camera_id;
		public MikanRenderTargetDescriptor descriptor;
	};

	public class FreeCameraRenderTargetTextures : MikanRequest
	{
		public int camera_id;
	};

	public class PublishCameraRenderTargetTextures : MikanRequest
	{
		public int camera_id;
		public long frame_index;
	};

	public class WriteCameraColorRenderTargetTexture : MikanRequest
	{
		public int camera_id;
		public IntPtr api_color_texture_ptr;
	};

	public class WriteCameraDepthRenderTargetTexture : MikanRequest
	{
		public int camera_id;
		public IntPtr api_depth_texture_ptr;
		public float z_near;
		public float z_far;
	};

}
