// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public enum MikanStageTrackingSystem
	{
		StaticMarker= 0,
		SteamVR= 1,
	};

	public class MikanStageInfo
	{
		public static readonly long classId= -4443825647652398383;

		public int stage_id;
		public string stage_name;
		public MikanStageTrackingSystem tracking_system;
		public int origin_marker_id;
		public float origin_marker_size;
		public int utility_marker_id;
		public float utility_marker_size;
	};

}
