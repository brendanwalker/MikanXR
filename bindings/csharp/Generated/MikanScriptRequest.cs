// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct SendScriptMessage : MikanRequest
	{
		public MikanScriptMessageInfo message { get; set; }
	};

	public struct GetVRDeviceList : MikanRequest
	{
	};

	public struct GetVRDeviceInfo : MikanRequest
	{
		public int deviceId { get; set; }
	};

	public struct SubscribeToVRDevicePoseUpdates : MikanRequest
	{
		public int deviceId { get; set; }
	};

	public struct UnsubscribeFromVRDevicePoseUpdates : MikanRequest
	{
		public int deviceId { get; set; }
	};

	public struct MikanVRDeviceListResponse : MikanResponse
	{
		public List<int> vr_device_id_list { get; set; }
	};

	public struct MikanVRDeviceInfoResponse : MikanResponse
	{
		public MikanVRDeviceInfo vr_device_info { get; set; }
	};

}
