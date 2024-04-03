using System.Collections;

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

	public struct MikanVRDeviceList : MikanResponse
	{
		public List<int> vr_device_id_list;

		public MikanVRDeviceList() : MikanResponse(typeof(MikanVRDeviceList).Name) {}
	};

	public struct MikanVRDeviceInfo : MikanResponse
	{
		public MikanVRDeviceApi vr_device_api;
		public MikanVRDeviceType vr_device_type;
		public string device_path;

		public MikanVRDeviceInfo() : MikanResponse(typeof(MikanVRDeviceInfo).Name) {}
	};
	
}