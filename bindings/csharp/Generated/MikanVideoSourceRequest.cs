// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetVideoSourceMode : MikanRequest
	{
		public static new readonly long classId= -5470313175782314738;

	};

	public class MikanVideoSourceModeResponse : MikanResponse
	{
		public static new readonly long classId= -1059487460754321771;

		public MikanVideoSourceType video_source_type;
		public MikanVideoSourceApi video_source_api;
		public string device_path;
		public string video_mode_name;
		public int resolution_x;
		public int resolution_y;
		public float frame_rate;
	};

}
