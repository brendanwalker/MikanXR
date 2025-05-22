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

	public class GetCameraInfo : MikanRequest
	{
		public static new readonly long classId= -383140838344567898;

		public int camera_id;
	};

	public class GetCameraList : MikanRequest
	{
		public static new readonly long classId= -737282717304326718;

	};

	public class MikanCameraInfoResponse : MikanResponse
	{
		public static new readonly long classId= -1502983427939995947;

		public MikanCameraInfo camera_info;
	};

	public class MikanCameraListResponse : MikanResponse
	{
		public static new readonly long classId= 4580879187161507129;

		public List<int> camera_id_list;
	};

}
