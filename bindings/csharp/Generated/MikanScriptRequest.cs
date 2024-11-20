// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class SendScriptMessage : MikanRequest
	{
		public MikanScriptMessageInfo message;
	};

	public class GetVRDeviceList : MikanRequest
	{
	};

	public class GetVRDeviceInfo : MikanRequest
	{
		public int deviceId;
	};

	public class SubscribeToVRDevicePoseUpdates : MikanRequest
	{
		public int deviceId;
	};

	public class UnsubscribeFromVRDevicePoseUpdates : MikanRequest
	{
		public int deviceId;
	};

	public class MikanVRDeviceListResponse : MikanResponse
	{
		public List<int> vr_device_id_list;
	};

	public class MikanVRDeviceInfoResponse : MikanResponse
	{
		public MikanVRDeviceInfo vr_device_info;
	};

}
