// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct GetVideoSourceAttachment : MikanRequest
	{
	};

	public struct GetVideoSourceIntrinsics : MikanRequest
	{
	};

	public struct GetVideoSourceMode : MikanRequest
	{
	};

	public struct MikanVideoSourceIntrinsicsResponse : MikanResponse
	{
		public MikanVideoSourceIntrinsics intrinsics { get; set; }
	};

	public struct MikanVideoSourceAttachmentInfoResponse : MikanResponse
	{
		public int attached_vr_device_id { get; set; }
		public MikanMatrix4f vr_device_offset_xform { get; set; }
	};

	public struct MikanVideoSourceModeResponse : MikanResponse
	{
		public MikanVideoSourceType video_source_type { get; set; }
		public MikanVideoSourceApi video_source_api { get; set; }
		public string device_path { get; set; }
		public string video_mode_name { get; set; }
		public int resolution_x { get; set; }
		public int resolution_y { get; set; }
		public float frame_rate { get; set; }
	};

}
