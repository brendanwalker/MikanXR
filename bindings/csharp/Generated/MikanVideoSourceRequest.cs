// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetVideoSourceIntrinsics : MikanRequest
	{
		public int video_source_id;
	};

	public class GetVideoSourceMode : MikanRequest
	{
		public int video_source_id;
	};

	public class MikanVideoSourceIntrinsicsResponse : MikanResponse
	{
		public MikanVideoSourceIntrinsics intrinsics;
	};

	public class MikanVideoSourceModeResponse : MikanResponse
	{
		public MikanVideoSourceType video_source_type;
		public string video_source_api;
		public string device_path;
		public string video_mode_name;
		public int resolution_x;
		public int resolution_y;
		public float frame_rate;
	};

	public class SetUSBVideoSourceDevice : MikanRequest
	{
		public int video_source_id;
		public string device_path;
	};

	public class SetUSBVideoSourceFormat : MikanRequest
	{
		public int video_source_id;
		public string format;
	};

	public class SetUSBVideoSourceFrameRate : MikanRequest
	{
		public int video_source_id;
		public string frame_rate;
	};

	public class SetUSBVideoSourceResolution : MikanRequest
	{
		public int video_source_id;
		public string resolution;
	};

}
