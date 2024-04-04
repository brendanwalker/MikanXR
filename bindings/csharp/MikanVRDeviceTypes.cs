using System.Collections.Generic;

namespace MikanXR
{
	public enum MikanVRDeviceApi
	{
		INVALID,
		STEAM_VR,
	};

	public enum MikanVRDeviceType
	{
		INVALID,
		HMD,
		CONTROLLER,
		TRACKER
	};

	public class MikanVRDeviceList : MikanResponse
	{
		public List<int> vr_device_id_list;

		public MikanVRDeviceList() : base(typeof(MikanVRDeviceList).Name) {}
	};

	public class MikanVRDeviceInfo : MikanResponse
	{
		public MikanVRDeviceApi vr_device_api;
		public MikanVRDeviceType vr_device_type;
		public string device_path;

		public MikanVRDeviceInfo() : base(typeof(MikanVRDeviceInfo).Name) {}
	};
	
}