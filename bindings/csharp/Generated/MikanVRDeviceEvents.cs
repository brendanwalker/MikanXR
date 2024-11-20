// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanVRDeviceListUpdateEvent : MikanEvent
	{
	};

	public class MikanVRDevicePoseUpdateEvent : MikanEvent
	{
		public MikanMatrix4f transform;
		public int device_id;
		public ulong frame;
	};

}
