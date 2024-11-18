// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct MikanVRDeviceListUpdateEvent : MikanEvent
	{
	};

	public struct MikanVRDevicePoseUpdateEvent : MikanEvent
	{
		public MikanMatrix4f transform { get; set; }
		public int device_id { get; set; }
		public ulong frame { get; set; }
	};

}
