// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetVideoSourceIntrinsics : MikanRequest
	{
		public int video_source_id;

		public GetVideoSourceIntrinsics()
		{
			requestTypeName = "GetVideoSourceIntrinsics";
		}
	};

	public class GetVideoSourceMode : MikanRequest
	{
		public int video_source_id;

		public GetVideoSourceMode()
		{
			requestTypeName = "GetVideoSourceMode";
		}
	};

	public class MikanVideoSourceIntrinsicsResponse : MikanResponse
	{
		public MikanVideoSourceIntrinsics intrinsics;

		public MikanVideoSourceIntrinsicsResponse()
		{
			responseTypeName = "MikanVideoSourceIntrinsicsResponse";
		}
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

		public MikanVideoSourceModeResponse()
		{
			responseTypeName = "MikanVideoSourceModeResponse";
		}
	};

	public class SetUSBVideoSourceDevice : MikanRequest
	{
		public int video_source_id;
		public string device_path;

		public SetUSBVideoSourceDevice()
		{
			requestTypeName = "SetUSBVideoSourceDevice";
		}
	};

	public class SetUSBVideoSourceFormat : MikanRequest
	{
		public int video_source_id;
		public string format;

		public SetUSBVideoSourceFormat()
		{
			requestTypeName = "SetUSBVideoSourceFormat";
		}
	};

	public class SetUSBVideoSourceFrameRate : MikanRequest
	{
		public int video_source_id;
		public string frame_rate;

		public SetUSBVideoSourceFrameRate()
		{
			requestTypeName = "SetUSBVideoSourceFrameRate";
		}
	};

	public class SetUSBVideoSourceResolution : MikanRequest
	{
		public int video_source_id;
		public string resolution;

		public SetUSBVideoSourceResolution()
		{
			requestTypeName = "SetUSBVideoSourceResolution";
		}
	};

}
