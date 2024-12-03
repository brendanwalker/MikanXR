// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetVRDeviceInfo : MikanRequest
	{
		public static new readonly ulong classId= 14646694700585510003;

		public int deviceId;
	};

	public class GetVRDeviceList : MikanRequest
	{
		public static new readonly ulong classId= 6797511981020466783;

	};

	public class MikanVRDeviceInfoResponse : MikanResponse
	{
		public static new readonly ulong classId= 15802109375944855716;

		public MikanVRDeviceInfo vr_device_info;
	};

	public class MikanVRDeviceListResponse : MikanResponse
	{
		public static new readonly ulong classId= 17942092411167833784;

		public List<int> vr_device_id_list;
	};

	public class SendScriptMessage : MikanRequest
	{
		public static new readonly ulong classId= 15439907534475020145;

		public MikanScriptMessageInfo message;
	};

	public class SubscribeToVRDevicePoseUpdates : MikanRequest
	{
		public static new readonly ulong classId= 12290234507554725059;

		public int deviceId;
	};

	public class UnsubscribeFromVRDevicePoseUpdates : MikanRequest
	{
		public static new readonly ulong classId= 796067758472639459;

		public int deviceId;
	};

}
