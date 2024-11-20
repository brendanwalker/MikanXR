// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetVideoSourceAttachment : MikanRequest
	{
	};

	public class GetVideoSourceIntrinsics : MikanRequest
	{
	};

	public class GetVideoSourceMode : MikanRequest
	{
	};

	public class MikanVideoSourceIntrinsicsResponse : MikanResponse
	{
		public MikanVideoSourceIntrinsics intrinsics;
	};

	public class MikanVideoSourceAttachmentInfoResponse : MikanResponse
	{
		public int attached_vr_device_id;
		public MikanMatrix4f vr_device_offset_xform;
	};

	public class MikanVideoSourceModeResponse : MikanResponse
	{
		public MikanVideoSourceType video_source_type;
		public MikanVideoSourceApi video_source_api;
		public string device_path;
		public string video_mode_name;
		public int resolution_x;
		public int resolution_y;
		public float frame_rate;
	};

}
