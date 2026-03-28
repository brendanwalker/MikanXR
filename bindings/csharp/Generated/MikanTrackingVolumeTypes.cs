// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public enum MikanTrackingRuntime
	{
		INVALID= -1,
		SteamVR= 0,
	};

	public enum MikanTrackingVolumeType
	{
		INVALID= -1,
		marker= 0,
		vr= 1,
	};

	public class MikanMarkerTrackingVolumeComponentValues : MikanTrackingVolumeComponentValues
	{
	};

	public class MikanTrackingVolumeComponentValues : MikanComponentValues
	{
		public int origin_marker_id;
	};

	public class MikanVRTrackingVolumeComponentValues : MikanTrackingVolumeComponentValues
	{
		public MikanTrackingRuntime tracking_runtime;
		public int charuco_mount_id;
		public MikanVector3f charuco_mount_offset_mm;
		public int utility_marker_id;
		public List<int> tracking_mount_ids;
		public MikanMatrix4f vr_device_pose_offset;
	};

}
