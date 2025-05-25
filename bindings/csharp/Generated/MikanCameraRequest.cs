// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class FindCameraByName : MikanRequest
	{
		public static new readonly long classId= -454698135979206047;

		public string camera_name;
	};

	public class GetCameraAttachment : MikanRequest
	{
		public static new readonly long classId= 8728192020460588051;

	};

	public class GetCameraInfo : MikanRequest
	{
		public static new readonly long classId= -383140838344567898;

		public int camera_id;
	};

	public class GetCameraIntrinsics : MikanRequest
	{
		public static new readonly long classId= 7200699089540330418;

	};

	public class GetCameraList : MikanRequest
	{
		public static new readonly long classId= -737282717304326718;

	};

	public class MikanCameraAttachmentInfoResponse : MikanResponse
	{
		public static new readonly long classId= -6026741417411940752;

		public int attached_vr_device_id;
		public MikanMatrix4f vr_device_offset_xform;
	};

	public class MikanCameraInfoResponse : MikanResponse
	{
		public static new readonly long classId= -1502983427939995947;

		public MikanCameraInfo camera_info;
	};

	public class MikanCameraIntrinsicsResponse : MikanResponse
	{
		public static new readonly long classId= -3291677122109534735;

		public MikanCameraIntrinsics intrinsics;
	};

	public class MikanCameraListResponse : MikanResponse
	{
		public static new readonly long classId= 4580879187161507129;

		public List<int> camera_id_list;
	};

}
