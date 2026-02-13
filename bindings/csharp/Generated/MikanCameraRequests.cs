// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class AllocateCameraRenderTargetTextures : MikanRequest
	{
		public static new readonly long classId= -6829988777871465530;

		public int camera_id;
		public MikanRenderTargetDescriptor descriptor;
	};

	public class FreeCameraRenderTargetTextures : MikanRequest
	{
		public static new readonly long classId= 7456227028164264559;

		public int camera_id;
	};

	public class PublishCameraRenderTargetTextures : MikanRequest
	{
		public static new readonly long classId= 9149889041987241612;

		public int camera_id;
		public long frame_index;
	};

	public class WriteCameraColorRenderTargetTexture : MikanRequest
	{
		public static new readonly long classId= 6474596006422811888;

		public int camera_id;
		public IntPtr api_color_texture_ptr;
	};

	public class WriteCameraDepthRenderTargetTexture : MikanRequest
	{
		public static new readonly long classId= -1802673963658192754;

		public int camera_id;
		public IntPtr api_depth_texture_ptr;
		public float z_near;
		public float z_far;
	};

}
