// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetScene : MikanRequest
	{
		public static new readonly long classId= -5182807597180949141;

		public int scene_id;
	};

	public class GetSceneList : MikanRequest
	{
		public static new readonly long classId= 5690599502798408243;

	};

	public class MikanSceneInfoResponse : MikanResponse
	{
		public static new readonly long classId= 9019592612085483640;

		public MikanSceneInfo scene_info;
	};

	public class MikanSceneListResponse : MikanResponse
	{
		public static new readonly long classId= 5317311404979047028;

		public List<int> scene_id_list;
	};

}
