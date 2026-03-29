// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
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

	public class MikanVRDeviceComponentValues : MikanTransformComponentValues
	{
		public MikanVRDeviceApi vr_device_api;
		public MikanVRDeviceType vr_device_type;
		public int vr_device_index;
		public string vr_device_path;
		public List<string> socket_names;
	};

	public class MikanVRObjectSystemValues : MikanSystemValues
	{
		public List<string> vr_device_path_list;
	};

}
