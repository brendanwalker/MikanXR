// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanCameraAttachmentChangedEvent : MikanEvent
	{
		public static new readonly long classId= -8381604214289685729;

	};

	public class MikanCameraIntrinsicsChangedEvent : MikanEvent
	{
		public static new readonly long classId= 1819820719769351702;

	};

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

}
