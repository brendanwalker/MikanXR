// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Enums
	public enum MikanVRDeviceApi
	{
		INVALID= 0,
		STEAM_VR= 1,
	};

	public enum MikanVRDeviceType
	{
		INVALID= 0,
		HMD= 1,
		CONTROLLER= 2,
		TRACKER= 3,
	};

	// Structs
	public struct MikanVRDeviceInfo
	{
		public MikanVRDeviceApi vr_device_api { get; set; }
		public MikanVRDeviceType vr_device_type { get; set; }
		public string device_path { get; set; }
	};

}
