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
		public static new readonly long classId= -912859617642764411;

	};

	public class MikanTrackingVolumeComponentValues : MikanComponentValues
	{
		public static new readonly long classId= 8298175138156239447;

		public int origin_marker_id;
	};

	public class MikanVRTrackingVolumeComponentValues : MikanTrackingVolumeComponentValues
	{
		public static new readonly long classId= -4734525919723628573;

		public MikanTrackingRuntime tracking_runtime;
		public int charuco_mount_id;
		public MikanVector3f charuco_mount_offset_mm;
		public int utility_marker_id;
		public List<int> tracking_mount_ids;
		public MikanMatrix4f vr_device_pose_offset;
	};

}
