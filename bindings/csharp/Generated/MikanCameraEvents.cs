// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanCameraListUpdateEvent : MikanEvent
	{
		public static new readonly long classId= -4014800876341333635;

	};

	public class MikanCameraNameUpdateEvent : MikanEvent
	{
		public static new readonly long classId= 6712693634199996648;

		public int camera_id;
		public string camera_name;
	};

	public class MikanCameraPoseUpdateEvent : MikanEvent
	{
		public static new readonly long classId= 6499335169538115146;

		public MikanTransform transform;
		public int camera_id;
	};

}
