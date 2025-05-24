// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanCameraNameUpdateEvent : MikanEvent
	{
		public static new readonly long classId= 6712693634199996648;

		public int camera_id;
		public string camera_name;
	};

	public class MikanCameraNewFrameEvent : MikanEvent
	{
		public static new readonly long classId= -514294351884360997;

		public MikanVector3f cameraForward;
		public MikanVector3f cameraUp;
		public MikanVector3f cameraPosition;
		public long frame;
	};

	public class MikanCameraPoseUpdateEvent : MikanEvent
	{
		public static new readonly long classId= 6499335169538115146;

		public MikanTransform transform;
		public int camera_id;
	};

}
